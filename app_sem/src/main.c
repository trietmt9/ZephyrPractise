#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app_sem, LOG_LEVEL_INF);

/* Thread stack sizes and priorities */
#define STACK_SIZE          512
#define WORKER_THREAD_PRIO  6
#define COORDINATOR_PRIO    4

/* Configuration for producer-consumer test */
#define BUFFER_SIZE         5
#define NUM_ITEMS_TO_PRODUCE 10

/* Binary semaphore for signaling */
K_SEM_DEFINE(binary_sem, 0, 1);

/* Counting semaphore for resource pool (3 resources available) */
K_SEM_DEFINE(resource_sem, 3, 3);

/* Semaphores for producer-consumer */
K_SEM_DEFINE(empty_slots, BUFFER_SIZE, BUFFER_SIZE);  /* Track empty slots */
K_SEM_DEFINE(filled_slots, 0, BUFFER_SIZE);           /* Track filled slots */
K_MUTEX_DEFINE(buffer_mutex);                         /* Protect buffer access */

/* Shared circular buffer for producer-consumer */
static int buffer[BUFFER_SIZE];
static int write_idx = 0;
static int read_idx = 0;

/* Thread stacks */
K_THREAD_STACK_DEFINE(sender_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(receiver_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(worker1_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(worker2_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(worker3_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(producer_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(consumer_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(coordinator_stack, STACK_SIZE);

/* Thread control blocks */
struct k_thread sender_data;
struct k_thread receiver_data;
struct k_thread worker1_data;
struct k_thread worker2_data;
struct k_thread worker3_data;
struct k_thread producer_data;
struct k_thread consumer_data;
struct k_thread coordinator_data;

/*===========================================================================
 * TEST 1: Binary Semaphore - Thread Signaling
 *===========================================================================*/

/**
 * @brief Receiver thread waits for signal
 */
void receiver_thread(void *arg1, void *arg2, void *arg3)
{
    LOG_INF("Receiver: Waiting for signal...");

    /* Wait for semaphore (blocks until sender signals) */
    k_sem_take(&binary_sem, K_FOREVER);

    LOG_INF("Receiver: Signal received! Proceeding with work");
    k_msleep(100);
    LOG_INF("Receiver: Work completed");
}

/**
 * @brief Sender thread sends signal after delay
 */
void sender_thread(void *arg1, void *arg2, void *arg3)
{
    LOG_INF("Sender: Will send signal in 2 seconds...");
    k_msleep(2000);

    LOG_INF("Sender: Sending signal now!");
    k_sem_give(&binary_sem);

    LOG_INF("Sender: Signal sent");
}

void test_binary_semaphore(void)
{
    LOG_WRN("=== TEST 1: Binary Semaphore (Signaling) ===");

    /* Reset semaphore to 0 */
    k_sem_reset(&binary_sem);

    /* Create receiver first (will block waiting) */
    k_tid_t receiver_tid = k_thread_create(&receiver_data, receiver_stack, STACK_SIZE,
                                           receiver_thread,
                                           NULL, NULL, NULL,
                                           WORKER_THREAD_PRIO, 0, K_NO_WAIT);

    k_msleep(100);  /* Let receiver start and block */

    /* Create sender (will signal after delay) */
    k_tid_t sender_tid = k_thread_create(&sender_data, sender_stack, STACK_SIZE,
                                         sender_thread,
                                         NULL, NULL, NULL,
                                         WORKER_THREAD_PRIO, 0, K_NO_WAIT);

    /* Wait for both threads */
    k_thread_join(receiver_tid, K_FOREVER);
    k_thread_join(sender_tid, K_FOREVER);

    LOG_INF("Binary semaphore test completed\n");
}

/*===========================================================================
 * TEST 2: Counting Semaphore - Resource Pool
 *===========================================================================*/

/**
 * @brief Worker thread that acquires and releases resources
 */
void worker_thread(void *arg1, void *arg2, void *arg3)
{
    uint32_t worker_id = (uint32_t)(uintptr_t)arg1;

    LOG_INF("Worker %u: Requesting resource...", worker_id);

    /* Try to acquire resource (may block if all 3 are in use) */
    int ret = k_sem_take(&resource_sem, K_MSEC(3000));

    if (ret == 0) {
        LOG_INF("Worker %u: Got resource! Working for 2 seconds...", worker_id);
        k_msleep(2000);

        LOG_INF("Worker %u: Releasing resource", worker_id);
        k_sem_give(&resource_sem);
    } else {
        LOG_ERR("Worker %u: Timeout - couldn't get resource", worker_id);
    }
}

void test_counting_semaphore(void)
{
    LOG_WRN("=== TEST 2: Counting Semaphore (Resource Pool) ===");
    LOG_INF("Resource pool has 3 resources available");

    /* Reset semaphore to 3 available resources */
    k_sem_reset(&resource_sem);
    k_sem_give(&resource_sem);
    k_sem_give(&resource_sem);
    k_sem_give(&resource_sem);

    /* Create 3 worker threads - all should get resources immediately */
    LOG_INF("Starting 3 workers (should all get resources)...");

    k_tid_t worker1_tid = k_thread_create(&worker1_data, worker1_stack, STACK_SIZE,
                                          worker_thread,
                                          (void *)1, NULL, NULL,
                                          WORKER_THREAD_PRIO, 0, K_NO_WAIT);

    k_tid_t worker2_tid = k_thread_create(&worker2_data, worker2_stack, STACK_SIZE,
                                          worker_thread,
                                          (void *)2, NULL, NULL,
                                          WORKER_THREAD_PRIO, 0, K_NO_WAIT);

    k_tid_t worker3_tid = k_thread_create(&worker3_data, worker3_stack, STACK_SIZE,
                                          worker_thread,
                                          (void *)3, NULL, NULL,
                                          WORKER_THREAD_PRIO, 0, K_NO_WAIT);

    k_msleep(500);  /* Let first 3 workers acquire resources */

    /* Wait for all workers */
    k_thread_join(worker1_tid, K_FOREVER);
    k_thread_join(worker2_tid, K_FOREVER);
    k_thread_join(worker3_tid, K_FOREVER);

    LOG_INF("Counting semaphore test completed\n");
}

/*===========================================================================
 * TEST 3: Producer-Consumer Pattern
 *===========================================================================*/

/**
 * @brief Producer thread - adds items to buffer
 */
void producer_thread(void *arg1, void *arg2, void *arg3)
{
    for (int i = 0; i < NUM_ITEMS_TO_PRODUCE; i++) {
        /* Wait for empty slot */
        k_sem_take(&empty_slots, K_FOREVER);

        /* Lock buffer for exclusive access */
        k_mutex_lock(&buffer_mutex, K_FOREVER);

        /* Produce item */
        buffer[write_idx] = i;
        LOG_INF("Producer: Added item %d at index %d", i, write_idx);
        write_idx = (write_idx + 1) % BUFFER_SIZE;

        k_mutex_unlock(&buffer_mutex);

        /* Signal that slot is now filled */
        k_sem_give(&filled_slots);

        k_msleep(300);  /* Simulate production time */
    }

    LOG_INF("Producer: Finished producing all items");
}

/**
 * @brief Consumer thread - removes items from buffer
 */
void consumer_thread(void *arg1, void *arg2, void *arg3)
{
    for (int i = 0; i < NUM_ITEMS_TO_PRODUCE; i++) {
        /* Wait for filled slot */
        k_sem_take(&filled_slots, K_FOREVER);

        /* Lock buffer for exclusive access */
        k_mutex_lock(&buffer_mutex, K_FOREVER);

        /* Consume item */
        int item = buffer[read_idx];
        LOG_INF("Consumer: Consumed item %d from index %d", item, read_idx);
        read_idx = (read_idx + 1) % BUFFER_SIZE;

        k_mutex_unlock(&buffer_mutex);

        /* Signal that slot is now empty */
        k_sem_give(&empty_slots);

        k_msleep(500);  /* Simulate consumption time */
    }

    LOG_INF("Consumer: Finished consuming all items");
}

void test_producer_consumer(void)
{
    LOG_WRN("=== TEST 3: Producer-Consumer Pattern ===");
    LOG_INF("Buffer size: %d items", BUFFER_SIZE);
    LOG_INF("Items to produce: %d", NUM_ITEMS_TO_PRODUCE);

    /* Reset everything */
    write_idx = 0;
    read_idx = 0;
    k_sem_reset(&empty_slots);
    k_sem_reset(&filled_slots);

    /* Initialize semaphores */
    for (int i = 0; i < BUFFER_SIZE; i++) {
        k_sem_give(&empty_slots);
    }

    /* Create producer and consumer threads */
    k_tid_t producer_tid = k_thread_create(&producer_data, producer_stack, STACK_SIZE,
                                           producer_thread,
                                           NULL, NULL, NULL,
                                           WORKER_THREAD_PRIO, 0, K_NO_WAIT);

    k_tid_t consumer_tid = k_thread_create(&consumer_data, consumer_stack, STACK_SIZE,
                                           consumer_thread,
                                           NULL, NULL, NULL,
                                           WORKER_THREAD_PRIO, 0, K_NO_WAIT);

    /* Wait for both to complete */
    k_thread_join(producer_tid, K_FOREVER);
    k_thread_join(consumer_tid, K_FOREVER);

    LOG_INF("Producer-Consumer test completed\n");
}

/*===========================================================================
 * Coordinator Thread - Orchestrates All Tests
 *===========================================================================*/

/**
 * @brief Coordinator thread - runs all semaphore tests
 */
void coordinator_thread(void *arg1, void *arg2, void *arg3)
{
    LOG_INF("Coordinator thread started");
    LOG_INF("========================================");

    k_msleep(100);

    /* Test 1: Binary semaphore for thread signaling */
    test_binary_semaphore();
    k_msleep(500);

    /* Test 2: Counting semaphore for resource pool */
    test_counting_semaphore();
    k_msleep(500);

    /* Test 3: Producer-consumer pattern */
    test_producer_consumer();

    LOG_INF("========================================");
    LOG_INF("Coordinator thread: All tests completed");
    LOG_INF("Coordinator thread exiting");
}

/*===========================================================================
 * Main Function - Only Creates Coordinator and Sleeps
 *===========================================================================*/

/**
 * @brief Main function - creates coordinator thread and sleeps forever
 */
int main(void)
{
    LOG_INF("Semaphore Example Application Started");
    LOG_INF("Main thread: Creating coordinator thread");

    /* Create coordinator thread that will run all tests */
    k_thread_create(&coordinator_data, coordinator_stack, STACK_SIZE,
                    coordinator_thread,
                    NULL, NULL, NULL,
                    COORDINATOR_PRIO, 0, K_NO_WAIT);

    LOG_INF("Main thread: Coordinator created, going to sleep");

    /* Main thread sleeps forever - all work is done by other threads */
    while (1) {
        k_sleep(K_FOREVER);
    }

    return 0;
}
