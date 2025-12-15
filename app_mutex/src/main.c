#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app_mutex, LOG_LEVEL_INF);

/* Thread stack sizes and priorities */
#define STACK_SIZE           512
#define COUNTER_THREAD_PRIO  5
#define COORDINATOR_PRIO     4  /* Higher priority (lower number) */

/* Number of increments each thread will perform */
#define NUM_INCREMENTS  1000

/* Shared resource - a counter */
static volatile uint32_t shared_counter = 0;

/* Mutex to protect the shared resource */
K_MUTEX_DEFINE(counter_mutex);

/* Thread stacks */
K_THREAD_STACK_DEFINE(thread1_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(thread2_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(thread3_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(coordinator_stack, STACK_SIZE);

/* Thread control blocks */
struct k_thread thread1_data;
struct k_thread thread2_data;
struct k_thread thread3_data;
struct k_thread coordinator_data;

/* Flag to control whether to use mutex */
static bool use_mutex = false;

/**
 * @brief Thread function that increments shared counter
 *
 * This function demonstrates the critical section where multiple threads
 * access and modify a shared resource. When use_mutex is false, race
 * conditions can occur. When true, mutex protects the critical section.
 */
void counter_thread(void *arg1, void *arg2, void *arg3)
{
    uint32_t thread_id = (uint32_t)(uintptr_t)arg1;
    uint32_t local_count = 0;

    LOG_INF("Counter thread %u started", thread_id);

    for (int i = 0; i < NUM_INCREMENTS; i++) {
        if (use_mutex) {
            /* CRITICAL SECTION - PROTECTED */
            k_mutex_lock(&counter_mutex, K_FOREVER);

            /* Read-Modify-Write operation */
            uint32_t temp = shared_counter;
            k_busy_wait(10);  /* Simulate some processing */
            shared_counter = temp + 1;
            local_count++;

            k_mutex_unlock(&counter_mutex);
        } else {
            /* CRITICAL SECTION - UNPROTECTED (Race Condition!) */
            uint32_t temp = shared_counter;
            k_busy_wait(10);  /* Simulate some processing */
            shared_counter = temp + 1;
            local_count++;
        }
    }

    LOG_INF("Counter thread %u finished - incremented %u times", thread_id, local_count);
}

/**
 * @brief Demonstrate race condition without mutex
 */
void test_without_mutex(void)
{
    LOG_WRN("=== TEST 1: WITHOUT MUTEX (Race Condition) ===");

    shared_counter = 0;
    use_mutex = false;

    /* Create three threads that will compete for the shared counter */
    k_tid_t tid1 = k_thread_create(&thread1_data, thread1_stack, STACK_SIZE,
                                    counter_thread,
                                    (void *)1, NULL, NULL,
                                    COUNTER_THREAD_PRIO, 0, K_NO_WAIT);

    k_tid_t tid2 = k_thread_create(&thread2_data, thread2_stack, STACK_SIZE,
                                    counter_thread,
                                    (void *)2, NULL, NULL,
                                    COUNTER_THREAD_PRIO, 0, K_NO_WAIT);

    k_tid_t tid3 = k_thread_create(&thread3_data, thread3_stack, STACK_SIZE,
                                    counter_thread,
                                    (void *)3, NULL, NULL,
                                    COUNTER_THREAD_PRIO, 0, K_NO_WAIT);

    /* Wait for all threads to complete */
    k_thread_join(tid1, K_FOREVER);
    k_thread_join(tid2, K_FOREVER);
    k_thread_join(tid3, K_FOREVER);

    LOG_WRN("Expected counter value: %u", NUM_INCREMENTS * 3);
    LOG_WRN("Actual counter value: %u", shared_counter);

    if (shared_counter != NUM_INCREMENTS * 3) {
        LOG_ERR("RACE CONDITION DETECTED! Lost %u increments",
                (NUM_INCREMENTS * 3) - shared_counter);
    }
}

/**
 * @brief Demonstrate proper synchronization with mutex
 */
void test_with_mutex(void)
{
    LOG_INF("=== TEST 2: WITH MUTEX (Protected) ===");

    shared_counter = 0;
    use_mutex = true;

    /* Create three threads that will safely share the counter */
    k_tid_t tid1 = k_thread_create(&thread1_data, thread1_stack, STACK_SIZE,
                                    counter_thread,
                                    (void *)1, NULL, NULL,
                                    COUNTER_THREAD_PRIO, 0, K_NO_WAIT);

    k_tid_t tid2 = k_thread_create(&thread2_data, thread2_stack, STACK_SIZE,
                                    counter_thread,
                                    (void *)2, NULL, NULL,
                                    COUNTER_THREAD_PRIO, 0, K_NO_WAIT);

    k_tid_t tid3 = k_thread_create(&thread3_data, thread3_stack, STACK_SIZE,
                                    counter_thread,
                                    (void *)3, NULL, NULL,
                                    COUNTER_THREAD_PRIO, 0, K_NO_WAIT);

    /* Wait for all threads to complete */
    k_thread_join(tid1, K_FOREVER);
    k_thread_join(tid2, K_FOREVER);
    k_thread_join(tid3, K_FOREVER);

    LOG_INF("Expected counter value: %u", NUM_INCREMENTS * 3);
    LOG_INF("Actual counter value: %u", shared_counter);

    if (shared_counter == NUM_INCREMENTS * 3) {
        LOG_INF("SUCCESS! Mutex protected the shared resource");
    } else {
        LOG_ERR("UNEXPECTED! Still have issues with %u lost increments",
                (NUM_INCREMENTS * 3) - shared_counter);
    }
}

/**
 * @brief Demonstrate mutex timeout
 */
void test_mutex_timeout(void)
{
    LOG_INF("=== TEST 3: Mutex Timeout Demo ===");

    /* Lock the mutex */
    k_mutex_lock(&counter_mutex, K_FOREVER);
    LOG_INF("Coordinator thread: Mutex locked");

    /* Try to lock again with timeout (will fail) */
    int ret = k_mutex_lock(&counter_mutex, K_MSEC(100));
    if (ret == -EAGAIN) {
        LOG_WRN("Coordinator thread: Timeout - mutex already locked!");
    }

    /* Unlock the mutex */
    k_mutex_unlock(&counter_mutex);
    LOG_INF("Coordinator thread: Mutex unlocked");

    /* Now we can lock it again */
    ret = k_mutex_lock(&counter_mutex, K_MSEC(100));
    if (ret == 0) {
        LOG_INF("Coordinator thread: Successfully locked mutex again");
        k_mutex_unlock(&counter_mutex);
    }
}

/**
 * @brief Coordinator thread - orchestrates all tests
 *
 * This thread runs all the mutex tests. The main() function only
 * creates this thread and sleeps forever.
 */
void coordinator_thread(void *arg1, void *arg2, void *arg3)
{
    LOG_INF("Coordinator thread started");
    LOG_INF("========================================");

    /* Give system time to settle */
    k_msleep(100);

    /* Test 1: Demonstrate race condition */
    test_without_mutex();
    k_msleep(500);

    /* Test 2: Demonstrate proper mutex protection */
    test_with_mutex();
    k_msleep(500);

    /* Test 3: Demonstrate mutex timeout */
    test_mutex_timeout();

    LOG_INF("========================================");
    LOG_INF("Coordinator thread: All tests completed");

    /* Coordinator thread exits after completing tests */
    LOG_INF("Coordinator thread exiting");
}

/**
 * @brief Main function - only creates coordinator thread and sleeps
 *
 * This demonstrates that main() doesn't need to do all the work.
 * It simply spawns a coordinator thread and then sleeps forever,
 * letting other threads handle the application logic.
 */
int main(void)
{
    LOG_INF("Mutex Example Application Started");
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
