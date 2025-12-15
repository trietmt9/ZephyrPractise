/**
 * @file main.c
 * @brief Environmental Monitoring System using Message Queues
 *
 * Real-world scenario: Multiple sensor threads collect environmental data
 * (temperature, humidity, pressure) and send it to a central data logger
 * thread via message queue for processing and storage.
 *
 * This pattern is common in:
 * - IoT devices
 * - Weather stations
 * - Industrial monitoring systems
 * - Smart home devices
 * - Medical equipment
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>

LOG_MODULE_REGISTER(data_logger, LOG_LEVEL_INF);

/*===========================================================================
 * Configuration
 *===========================================================================*/

#define SENSOR_THREAD_STACK_SIZE    512
#define LOGGER_THREAD_STACK_SIZE    1024
#define SENSOR_THREAD_PRIORITY      5
#define LOGGER_THREAD_PRIORITY      4

#define TEMP_SAMPLE_PERIOD_MS       2000
#define HUMIDITY_SAMPLE_PERIOD_MS   3000
#define PRESSURE_SAMPLE_PERIOD_MS   2500

#define MSG_QUEUE_SIZE              10

/*===========================================================================
 * Data Structures
 *===========================================================================*/

/**
 * @brief Sensor types
 */
typedef enum {
    SENSOR_TYPE_TEMPERATURE,
    SENSOR_TYPE_HUMIDITY,
    SENSOR_TYPE_PRESSURE,
    SENSOR_TYPE_MAX
} sensor_type_t;

/**
 * @brief Sensor data message structure
 *
 * This is the message format sent through the message queue.
 * Contains sensor type, value, timestamp, and sequence number.
 */
typedef struct {
    sensor_type_t type;        /* Type of sensor */
    float value;               /* Sensor reading value */
    uint32_t timestamp_ms;     /* System uptime when read */
    uint32_t sequence;         /* Sequence number for tracking */
} sensor_data_msg_t;

/**
 * @brief Sensor statistics
 */
typedef struct {
    uint32_t samples_read;
    uint32_t samples_sent;
    uint32_t send_failures;
    float min_value;
    float max_value;
    float last_value;
} sensor_stats_t;

/*===========================================================================
 * Global Variables
 *===========================================================================*/

/* Message queue for sensor data */
K_MSGQ_DEFINE(sensor_msgq, sizeof(sensor_data_msg_t), MSG_QUEUE_SIZE, 4);

/* Sensor statistics */
static sensor_stats_t temp_stats = {0};
static sensor_stats_t humidity_stats = {0};
static sensor_stats_t pressure_stats = {0};

/* Thread stacks */
K_THREAD_STACK_DEFINE(temp_sensor_stack, SENSOR_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(humidity_sensor_stack, SENSOR_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(pressure_sensor_stack, SENSOR_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(logger_stack, LOGGER_THREAD_STACK_SIZE);

/* Thread control blocks */
struct k_thread temp_sensor_data;
struct k_thread humidity_sensor_data;
struct k_thread pressure_sensor_data;
struct k_thread logger_data;

/*===========================================================================
 * Sensor Simulation Functions
 *===========================================================================*/

/**
 * @brief Simulate reading temperature sensor
 * @return Temperature in Celsius (20.0 to 30.0)
 */
static float read_temperature_sensor(void)
{
    /* Simulate temperature reading with some variation */
    /* Real implementation would read from actual hardware (I2C, SPI, etc.) */
    uint32_t rand = sys_rand32_get();
    float base_temp = 25.0f;
    float variation = (rand % 1000) / 100.0f - 5.0f;  /* ±5°C variation */
    return base_temp + variation;
}

/**
 * @brief Simulate reading humidity sensor
 * @return Relative humidity in % (40.0 to 80.0)
 */
static float read_humidity_sensor(void)
{
    /* Simulate humidity reading */
    /* Real implementation would read from actual hardware */
    uint32_t rand = sys_rand32_get();
    float base_humidity = 60.0f;
    float variation = (rand % 1000) / 50.0f - 10.0f;  /* ±20% variation */
    return base_humidity + variation;
}

/**
 * @brief Simulate reading pressure sensor
 * @return Atmospheric pressure in hPa (980.0 to 1020.0)
 */
static float read_pressure_sensor(void)
{
    /* Simulate pressure reading */
    /* Real implementation would read from actual hardware */
    uint32_t rand = sys_rand32_get();
    float base_pressure = 1013.25f;
    float variation = (rand % 1000) / 50.0f - 10.0f;  /* ±20 hPa variation */
    return base_pressure + variation;
}

/*===========================================================================
 * Helper Functions
 *===========================================================================*/

/**
 * @brief Get sensor type name as string
 */
static const char* sensor_type_to_string(sensor_type_t type)
{
    switch (type) {
        case SENSOR_TYPE_TEMPERATURE: return "Temperature";
        case SENSOR_TYPE_HUMIDITY:    return "Humidity";
        case SENSOR_TYPE_PRESSURE:    return "Pressure";
        default:                      return "Unknown";
    }
}

/**
 * @brief Get sensor unit string
 */
static const char* sensor_unit_string(sensor_type_t type)
{
    switch (type) {
        case SENSOR_TYPE_TEMPERATURE: return "°C";
        case SENSOR_TYPE_HUMIDITY:    return "%";
        case SENSOR_TYPE_PRESSURE:    return "hPa";
        default:                      return "";
    }
}

/**
 * @brief Update sensor statistics
 */
static void update_sensor_stats(sensor_stats_t *stats, float value, bool sent)
{
    stats->samples_read++;
    stats->last_value = value;

    if (stats->samples_read == 1) {
        stats->min_value = value;
        stats->max_value = value;
    } else {
        if (value < stats->min_value) stats->min_value = value;
        if (value > stats->max_value) stats->max_value = value;
    }

    if (sent) {
        stats->samples_sent++;
    } else {
        stats->send_failures++;
    }
}

/*===========================================================================
 * Sensor Threads
 *===========================================================================*/

/**
 * @brief Generic sensor thread function
 *
 * @param type Sensor type
 * @param period_ms Sampling period in milliseconds
 * @param read_fn Function pointer to read sensor
 * @param stats Pointer to statistics structure
 */
static void sensor_thread_generic(sensor_type_t type,
                                  uint32_t period_ms,
                                  float (*read_fn)(void),
                                  sensor_stats_t *stats)
{
    uint32_t sequence = 0;
    sensor_data_msg_t msg;

    LOG_INF("%s sensor thread started (period: %u ms)",
            sensor_type_to_string(type), period_ms);

    while (1) {
        /* Read sensor value */
        float value = read_fn();

        /* Prepare message */
        msg.type = type;
        msg.value = value;
        msg.timestamp_ms = k_uptime_get_32();
        msg.sequence = sequence++;

        /* Try to send message to queue */
        int ret = k_msgq_put(&sensor_msgq, &msg, K_NO_WAIT);

        if (ret == 0) {
            /* Success */
            update_sensor_stats(stats, value, true);
            LOG_DBG("%s: %.2f %s [seq:%u] -> Queue",
                    sensor_type_to_string(type),
                    value,
                    sensor_unit_string(type),
                    msg.sequence);
        } else if (ret == -ENOMSG) {
            /* Queue full - this is important to handle! */
            update_sensor_stats(stats, value, false);
            LOG_WRN("%s: Queue full! Data lost (%.2f %s)",
                    sensor_type_to_string(type),
                    value,
                    sensor_unit_string(type));
        }

        /* Wait for next sample period */
        k_msleep(period_ms);
    }
}

/**
 * @brief Temperature sensor thread
 */
void temperature_sensor_thread(void *arg1, void *arg2, void *arg3)
{
    sensor_thread_generic(SENSOR_TYPE_TEMPERATURE,
                         TEMP_SAMPLE_PERIOD_MS,
                         read_temperature_sensor,
                         &temp_stats);
}

/**
 * @brief Humidity sensor thread
 */
void humidity_sensor_thread(void *arg1, void *arg2, void *arg3)
{
    sensor_thread_generic(SENSOR_TYPE_HUMIDITY,
                         HUMIDITY_SAMPLE_PERIOD_MS,
                         read_humidity_sensor,
                         &humidity_stats);
}

/**
 * @brief Pressure sensor thread
 */
void pressure_sensor_thread(void *arg1, void *arg2, void *arg3)
{
    sensor_thread_generic(SENSOR_TYPE_PRESSURE,
                         PRESSURE_SAMPLE_PERIOD_MS,
                         read_pressure_sensor,
                         &pressure_stats);
}

/*===========================================================================
 * Data Logger Thread
 *===========================================================================*/

/**
 * @brief Process and log sensor data
 *
 * In a real system, this would:
 * - Write to flash/SD card
 * - Send to cloud via network
 * - Update display
 * - Trigger alarms if thresholds exceeded
 */
static void process_sensor_data(const sensor_data_msg_t *msg)
{
    /* Log the data with nice formatting */
    LOG_INF("[LOG] %s: %.2f %s | Time: %u ms | Seq: %u",
            sensor_type_to_string(msg->type),
            (double)msg->value,
            sensor_unit_string(msg->type),
            msg->timestamp_ms,
            msg->sequence);

    /* In real implementation, you would:
     *
     * 1. Write to persistent storage
     *    - Flash file system (LittleFS, FAT)
     *    - SD card
     *    - EEPROM
     *
     * 2. Send to network
     *    - MQTT broker
     *    - HTTP POST to cloud
     *    - LoRaWAN uplink
     *
     * 3. Check thresholds and trigger actions
     *    if (msg->value > THRESHOLD) {
     *        trigger_alarm();
     *    }
     *
     * 4. Update statistics or moving averages
     *
     * 5. Update display/UI
     */

    /* Simulate processing time */
    k_msleep(100);
}

/**
 * @brief Logger thread - receives and processes sensor data
 */
void logger_thread(void *arg1, void *arg2, void *arg3)
{
    sensor_data_msg_t msg;
    uint32_t total_messages = 0;

    LOG_INF("Data logger thread started");
    LOG_INF("Waiting for sensor data...");
    LOG_INF("========================================");

    while (1) {
        /* Wait for message from queue (blocks until available) */
        int ret = k_msgq_get(&sensor_msgq, &msg, K_FOREVER);

        if (ret == 0) {
            /* Successfully received message */
            total_messages++;
            process_sensor_data(&msg);

            /* Print statistics every 10 messages */
            if (total_messages % 10 == 0) {
                LOG_INF("----------------------------------------");
                LOG_INF("Statistics (Total processed: %u)", total_messages);
                LOG_INF("  Temp:     Read:%3u Sent:%3u Failed:%u",
                        temp_stats.samples_read,
                        temp_stats.samples_sent,
                        temp_stats.send_failures);
                LOG_INF("  Humidity: Read:%3u Sent:%3u Failed:%u",
                        humidity_stats.samples_read,
                        humidity_stats.samples_sent,
                        humidity_stats.send_failures);
                LOG_INF("  Pressure: Read:%3u Sent:%3u Failed:%u",
                        pressure_stats.samples_read,
                        pressure_stats.samples_sent,
                        pressure_stats.send_failures);
                LOG_INF("  Queue: %u/%u used",
                        k_msgq_num_used_get(&sensor_msgq),
                        MSG_QUEUE_SIZE);
                LOG_INF("========================================");
            }
        }
    }
}

/*===========================================================================
 * Main Function
 *===========================================================================*/

/**
 * @brief Main function - creates sensor and logger threads
 */
int main(void)
{
    LOG_INF("===========================================");
    LOG_INF(" Environmental Monitoring System");
    LOG_INF(" Message Queue Demonstration");
    LOG_INF("===========================================");
    LOG_INF("");

    /* Initialize statistics */
    temp_stats.min_value = 999.0f;
    humidity_stats.min_value = 999.0f;
    pressure_stats.min_value = 9999.0f;

    LOG_INF("Starting data logger thread...");
    k_thread_create(&logger_data, logger_stack, LOGGER_THREAD_STACK_SIZE,
                    logger_thread,
                    NULL, NULL, NULL,
                    LOGGER_THREAD_PRIORITY, 0, K_NO_WAIT);

    k_msleep(100);  /* Let logger start first */

    LOG_INF("Starting sensor threads...");

    /* Create temperature sensor thread */
    k_thread_create(&temp_sensor_data, temp_sensor_stack, SENSOR_THREAD_STACK_SIZE,
                    temperature_sensor_thread,
                    NULL, NULL, NULL,
                    SENSOR_THREAD_PRIORITY, 0, K_NO_WAIT);

    /* Create humidity sensor thread */
    k_thread_create(&humidity_sensor_data, humidity_sensor_stack, SENSOR_THREAD_STACK_SIZE,
                    humidity_sensor_thread,
                    NULL, NULL, NULL,
                    SENSOR_THREAD_PRIORITY, 0, K_NO_WAIT);

    /* Create pressure sensor thread */
    k_thread_create(&pressure_sensor_data, pressure_sensor_stack, SENSOR_THREAD_STACK_SIZE,
                    pressure_sensor_thread,
                    NULL, NULL, NULL,
                    SENSOR_THREAD_PRIORITY, 0, K_NO_WAIT);

    LOG_INF("System initialized successfully");
    LOG_INF("Main thread going to sleep...");
    LOG_INF("");

    /* Main thread sleeps forever */
    while (1) {
        k_sleep(K_FOREVER);
    }

    return 0;
}
