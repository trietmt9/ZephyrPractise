/**
 * @file main.c
 * @brief Work Queue Example - Smart Home Controller
 *
 * Real-world scenario: Smart home device with multiple input sources
 * (buttons, sensors, timers) that need to trigger processing tasks.
 *
 * Work queues allow ISRs and threads to defer time-consuming work
 * to a dedicated worker thread, keeping ISRs fast and responsive.
 *
 * This pattern is common in:
 * - Smart home devices (button handling, sensor processing)
 * - Industrial controllers (event processing, data logging)
 * - IoT devices (network uploads, batch processing)
 * - User interface systems (input handling, display updates)
 * - Communication systems (packet processing, protocol handling)
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>

LOG_MODULE_REGISTER(workq_demo, LOG_LEVEL_INF);

/*===========================================================================
 * Configuration
 *===========================================================================*/

#define CUSTOM_WORKQ_STACK_SIZE    1024
#define CUSTOM_WORKQ_PRIORITY      5

#define BUTTON_DEBOUNCE_MS         50
#define SENSOR_BATCH_TIMEOUT_MS    5000
#define STATUS_CHECK_PERIOD_MS     10000

#define MAX_SENSOR_BATCH           10

/*===========================================================================
 * Data Structures
 *===========================================================================*/

/**
 * @brief Button event types
 */
typedef enum {
    BUTTON_PRESS,
    BUTTON_RELEASE,
    BUTTON_LONG_PRESS
} button_event_t;

/**
 * @brief Button work item
 *
 * Contains both the work structure and the event data.
 * This is a common pattern for passing data to work handlers.
 */
struct button_work {
    struct k_work work;           /* Must be first member */
    button_event_t event;
    uint32_t button_id;
    uint32_t timestamp_ms;
};

/**
 * @brief Sensor data batch
 */
struct sensor_batch {
    float values[MAX_SENSOR_BATCH];
    uint32_t count;
    struct k_work_delayable upload_work;
};

/**
 * @brief Periodic status check work
 */
struct status_work {
    struct k_work_delayable work;
    uint32_t check_count;
};

/*===========================================================================
 * Global Variables
 *===========================================================================*/

/* Custom work queue for high-priority tasks */
static K_THREAD_STACK_DEFINE(custom_workq_stack, CUSTOM_WORKQ_STACK_SIZE);
static struct k_work_q custom_workq;

/* Button work items (pre-allocated pool) */
static struct button_work button_work_items[3];

/* Sensor batch for network upload */
static struct sensor_batch sensor_data;

/* Periodic status check */
static struct status_work status_checker;

/* Statistics */
static uint32_t button_events_processed = 0;
static uint32_t sensor_batches_uploaded = 0;
static uint32_t status_checks_performed = 0;

/*===========================================================================
 * Work Queue Handlers
 *===========================================================================*/

/**
 * @brief Handle button events
 *
 * This runs in work queue thread context, NOT ISR context.
 * We can do time-consuming operations here:
 * - Debouncing logic
 * - Flash writes
 * - Network operations
 * - Display updates
 */
static void button_work_handler(struct k_work *work)
{
    /* Get the container structure */
    struct button_work *btn_work = CONTAINER_OF(work, struct button_work, work);

    button_events_processed++;

    LOG_INF("Button Handler: Button %u - %s (time: %u ms)",
            btn_work->button_id,
            btn_work->event == BUTTON_PRESS ? "PRESSED" :
            btn_work->event == BUTTON_RELEASE ? "RELEASED" : "LONG PRESS",
            btn_work->timestamp_ms);

    /* Simulate button processing work */
    switch (btn_work->event) {
        case BUTTON_PRESS:
            LOG_INF("  → Action: Toggle light #%u", btn_work->button_id);
            /* In real system: GPIO toggle, send command, etc. */
            break;

        case BUTTON_LONG_PRESS:
            LOG_INF("  → Action: Start scene #%u", btn_work->button_id);
            /* In real system: Complex scene activation */
            break;

        case BUTTON_RELEASE:
            LOG_DBG("  → Button released (no action)");
            break;
    }

    /* Simulate processing time (debouncing, flash write, etc.) */
    k_msleep(BUTTON_DEBOUNCE_MS);

    LOG_DBG("Button Handler: Completed");
}

/**
 * @brief Upload sensor data batch to network
 *
 * This is a DELAYED work item that runs after a timeout.
 * Useful for batching operations to reduce network traffic.
 */
static void sensor_upload_handler(struct k_work *work)
{
    struct k_work_delayable *dwork = k_work_delayable_from_work(work);
    struct sensor_batch *batch = CONTAINER_OF(dwork, struct sensor_batch, upload_work);

    if (batch->count == 0) {
        LOG_DBG("Upload Handler: No data to upload");
        return;
    }

    sensor_batches_uploaded++;

    LOG_INF("Upload Handler: Uploading %u sensor readings to cloud", batch->count);

    /* Simulate network upload (in real system: HTTP POST, MQTT publish, etc.) */
    for (uint32_t i = 0; i < batch->count; i++) {
        LOG_INF("  → Reading[%u]: %.2f °C", i, (double)batch->values[i]);
    }

    /* Simulate network transmission time */
    k_msleep(100);

    LOG_INF("Upload Handler: Batch uploaded successfully");

    /* Reset batch */
    batch->count = 0;
}

/**
 * @brief Periodic status check
 *
 * This is a DELAYED work item that reschedules itself.
 * Common pattern for periodic tasks.
 */
static void status_check_handler(struct k_work *work)
{
    struct k_work_delayable *dwork = k_work_delayable_from_work(work);
    struct status_work *status = CONTAINER_OF(dwork, struct status_work, work);

    status->check_count++;
    status_checks_performed++;

    LOG_INF("========================================");
    LOG_INF("Status Check #%u", status->check_count);
    LOG_INF("  Button events processed: %u", button_events_processed);
    LOG_INF("  Sensor batches uploaded: %u", sensor_batches_uploaded);
    LOG_INF("  Status checks performed: %u", status_checks_performed);
    LOG_INF("  System uptime: %u seconds", k_uptime_get_32() / 1000);
    LOG_INF("========================================");

    /* Reschedule this work for next status check */
    k_work_reschedule_for_queue(&custom_workq, dwork, K_MSEC(STATUS_CHECK_PERIOD_MS));
}

/*===========================================================================
 * Simulated ISR and Event Generators
 *===========================================================================*/

/**
 * @brief Simulate button interrupt handler
 *
 * In a real system, this would be called from GPIO ISR.
 * ISR must be FAST - we just submit work and return.
 */
static void simulate_button_isr(uint32_t button_id, button_event_t event)
{
    /* Get pre-allocated work item */
    struct button_work *work = &button_work_items[button_id % 3];

    /* Fill in event data */
    work->button_id = button_id;
    work->event = event;
    work->timestamp_ms = k_uptime_get_32();

    /* Submit work to CUSTOM work queue (high priority) */
    k_work_submit_to_queue(&custom_workq, &work->work);

    /* ISR returns immediately - work will be processed in work queue thread */
}

/**
 * @brief Add sensor reading to batch
 *
 * This could be called from ISR or thread context.
 * When batch is full, triggers immediate upload.
 * Otherwise, upload happens after timeout.
 */
static void add_sensor_reading(float temperature)
{
    if (sensor_data.count < MAX_SENSOR_BATCH) {
        sensor_data.values[sensor_data.count++] = temperature;
        LOG_DBG("Sensor: Added reading %.2f °C (batch: %u/%u)",
                (double)temperature, sensor_data.count, MAX_SENSOR_BATCH);

        /* If this is the first reading, schedule upload timeout */
        if (sensor_data.count == 1) {
            k_work_reschedule_for_queue(&custom_workq,
                                       &sensor_data.upload_work,
                                       K_MSEC(SENSOR_BATCH_TIMEOUT_MS));
            LOG_DBG("Sensor: Upload scheduled in %u ms", SENSOR_BATCH_TIMEOUT_MS);
        }

        /* If batch is full, upload immediately */
        if (sensor_data.count >= MAX_SENSOR_BATCH) {
            LOG_INF("Sensor: Batch full! Triggering immediate upload");
            k_work_reschedule_for_queue(&custom_workq,
                                       &sensor_data.upload_work,
                                       K_NO_WAIT);
        }
    } else {
        LOG_WRN("Sensor: Batch full, reading dropped!");
    }
}

/**
 * @brief Simulate sensor reading
 */
static float simulate_temperature_sensor(void)
{
    uint32_t rand = sys_rand32_get();
    float base_temp = 22.0f;
    float variation = (rand % 1000) / 100.0f - 5.0f;  /* ±5°C */
    return base_temp + variation;
}

/*===========================================================================
 * Demonstration Thread
 *===========================================================================*/

/**
 * @brief Demo thread that simulates various events
 */
void demo_thread(void *arg1, void *arg2, void *arg3)
{
    LOG_INF("Demo thread started - simulating smart home events...");
    LOG_INF("");

    k_msleep(1000);

    /* Simulate various button presses over time */
    LOG_INF("Simulating button press on Button 0...");
    simulate_button_isr(0, BUTTON_PRESS);
    k_msleep(500);

    LOG_INF("Simulating button release on Button 0...");
    simulate_button_isr(0, BUTTON_RELEASE);
    k_msleep(1000);

    /* Add some sensor readings */
    LOG_INF("Starting sensor readings...");
    for (int i = 0; i < 3; i++) {
        float temp = simulate_temperature_sensor();
        LOG_INF("Sensor reading: %.2f °C", (double)temp);
        add_sensor_reading(temp);
        k_msleep(800);
    }

    k_msleep(1000);

    /* Multiple rapid button presses */
    LOG_INF("Simulating rapid button presses (Button 1)...");
    simulate_button_isr(1, BUTTON_PRESS);
    k_msleep(100);
    simulate_button_isr(1, BUTTON_RELEASE);
    k_msleep(100);
    simulate_button_isr(1, BUTTON_PRESS);
    k_msleep(100);
    simulate_button_isr(1, BUTTON_RELEASE);

    k_msleep(1500);

    /* Long press */
    LOG_INF("Simulating long press on Button 2...");
    simulate_button_isr(2, BUTTON_LONG_PRESS);

    k_msleep(1000);

    /* Add more sensor readings to trigger batch upload */
    LOG_INF("Adding more sensor readings to fill batch...");
    for (int i = 0; i < 8; i++) {
        float temp = simulate_temperature_sensor();
        add_sensor_reading(temp);
        k_msleep(300);
    }

    LOG_INF("");
    LOG_INF("Demo events completed. System continues running...");
    LOG_INF("Status checks will run every %u seconds", STATUS_CHECK_PERIOD_MS / 1000);
}

/* Demo thread stack and control block */
K_THREAD_STACK_DEFINE(demo_stack, 1024);
struct k_thread demo_data;

/*===========================================================================
 * Main Function
 *===========================================================================*/

int main(void)
{
    LOG_INF("==========================================");
    LOG_INF(" Work Queue Example");
    LOG_INF(" Smart Home Controller Demo");
    LOG_INF("==========================================");
    LOG_INF("");

    /* Initialize custom work queue with higher priority than system queue */
    k_work_queue_start(&custom_workq,
                      custom_workq_stack,
                      K_THREAD_STACK_SIZEOF(custom_workq_stack),
                      CUSTOM_WORKQ_PRIORITY,
                      NULL);

    LOG_INF("Custom work queue started (priority: %d)", CUSTOM_WORKQ_PRIORITY);

    /* Initialize button work items */
    for (int i = 0; i < 3; i++) {
        k_work_init(&button_work_items[i].work, button_work_handler);
    }
    LOG_INF("Button work items initialized (count: 3)");

    /* Initialize sensor batch and delayed work */
    k_work_init_delayable(&sensor_data.upload_work, sensor_upload_handler);
    sensor_data.count = 0;
    LOG_INF("Sensor batch work initialized (max batch: %u)", MAX_SENSOR_BATCH);

    /* Initialize and start periodic status check */
    k_work_init_delayable(&status_checker.work, status_check_handler);
    status_checker.check_count = 0;
    k_work_reschedule_for_queue(&custom_workq,
                               &status_checker.work,
                               K_MSEC(STATUS_CHECK_PERIOD_MS));
    LOG_INF("Periodic status check started (every %u seconds)", STATUS_CHECK_PERIOD_MS / 1000);

    LOG_INF("");
    LOG_INF("System initialized. Starting demo...");
    LOG_INF("");

    /* Create demo thread */
    k_thread_create(&demo_data, demo_stack, K_THREAD_STACK_SIZEOF(demo_stack),
                    demo_thread,
                    NULL, NULL, NULL,
                    7, 0, K_NO_WAIT);

    /* Main thread sleeps forever */
    while (1) {
        k_sleep(K_FOREVER);
    }

    return 0;
}
