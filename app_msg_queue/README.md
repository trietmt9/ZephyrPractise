# Message Queue Example

This example demonstrates the use of **Zephyr kernel message queues** for thread-safe data passing between multiple producer threads and a consumer thread in a real-world environmental monitoring scenario.

## What You'll Learn

- Message queues for passing structured data between threads
- Multiple producer, single consumer pattern
- Queue overflow handling
- Thread-safe communication without explicit locking
- Asynchronous data collection from multiple sources
- Statistics tracking and monitoring

---

## Why Learn This?

**Message queues are the backbone of data-driven embedded systems.** They allow threads to exchange structured data safely and efficiently—essential for any system that collects, processes, or transmits information.

### The Problem Without Message Queues

Imagine a weather station with 3 sensors and a data logger:
```
Temperature thread → reads every 2 seconds
Humidity thread    → reads every 3 seconds
Pressure thread    → reads every 2.5 seconds
Logger thread      → needs to collect all readings
```

**Without message queues, you'd have to:**
- ❌ Use 3 separate global variables + 3 mutexes (complex, error-prone)
- ❌ Risk losing data when sensors read faster than logger can process
- ❌ Implement custom buffering with circular buffers + synchronization (reinventing the wheel)
- ❌ Handle thread coordination manually (difficult to get right)

**With a message queue:**
```c
K_MSGQ_DEFINE(sensor_msgq, sizeof(sensor_data_t), 10, 4);

Sensors: k_msgq_put(&sensor_msgq, &data, K_NO_WAIT);  // Send data
Logger:  k_msgq_get(&sensor_msgq, &data, K_FOREVER);  // Receive data
```

Clean, thread-safe, buffered communication—automatically handled by the kernel!

### Real-World Scenarios Where Message Queues Are Essential

| Industry | Use Case | Why Message Queue Is Needed |
|----------|----------|---------------------------|
| **IoT Devices** | Multiple sensors → cloud uplink thread | Buffer data during network outages; decouple sampling from transmission |
| **Medical Devices** | Vital sign monitors → storage + display threads | Ensure no readings are lost; maintain temporal ordering |
| **Industrial Automation** | Production sensors → data logger + control threads | High-speed data collection with guaranteed buffering |
| **Smart Home** | Motion/door/temp sensors → automation controller | Handle asynchronous events; prioritize processing |
| **Data Acquisition** | ADC sampling threads → FFT processing thread | Buffer high-rate samples for batch processing |

### Four Critical Problems Message Queues Solve

#### 1. **Decoupling Producers from Consumers**
```
Producer runs at 100 Hz → Queue (buffer) → Consumer processes at 50 Hz
```

**Without queue:** Producer must wait for consumer, or data gets lost.
**With queue:** Producer keeps running; queue buffers data. If queue fills, system can handle gracefully.

#### 2. **Multi-Source Data Collection** (This Example!)
```
Temp sensor (2s) ─┐
Humidity (3s)     ├──→ Message Queue ──→ Logger thread
Pressure (2.5s) ──┘
```

**Without queue:** Complex synchronization, manual buffering, race conditions.
**With queue:** All sensors send independently; logger processes in order.

#### 3. **Guaranteed Data Ordering** (FIFO)
```
Sensor reads: T1, T2, T3, T4 → Queue → Logger gets: T1, T2, T3, T4
```

**Critical for:** Time-series data, state machines, protocol processing.

#### 4. **Asynchronous Communication Without Blocking**
```c
// Producer doesn't wait for consumer
k_msgq_put(&msgq, &data, K_NO_WAIT);  // Returns immediately

// Consumer waits efficiently (no busy loop)
k_msgq_get(&msgq, &data, K_FOREVER);  // Sleeps until data arrives
```

**Result:** Responsive producers, efficient consumers, minimal CPU waste.

### Why Message Queues Are Different From Other Primitives

| Primitive | Purpose | Data Transfer |
|-----------|---------|---------------|
| **Mutex** | Protect shared data | No - only synchronization |
| **Semaphore** | Signal events, count resources | No - only a count value |
| **Message Queue** | Pass structured data | **Yes - actual data exchange** |

**Message queues = Semaphore signaling + Data buffer + Thread safety**

### Common Embedded Use Cases

✅ **Sensor to Logger** (this example)
- Environmental monitoring, industrial sensors, medical vitals

✅ **ISR to Thread**
- UART received data, ADC samples, network packets

✅ **Command/Response Pattern**
- User input → command queue → worker thread
- Worker → response queue → display thread

✅ **Pipeline Processing**
- Capture thread → preprocessing queue → processing thread → output queue → display

✅ **Event-Driven Systems**
- Button press, timer expiry, sensor threshold → event queue → state machine

### What You'll Be Able to Do After This

✅ Build multi-sensor data collection systems (IoT, monitoring)
✅ Implement producer-consumer architectures safely
✅ Handle asynchronous data streams without data loss
✅ Decouple fast producers from slow consumers
✅ Create responsive, event-driven embedded applications

**Bottom line:** If your embedded system collects, processes, or routes data between threads, you need message queues. This is the standard pattern in professional IoT, data acquisition, and communication systems.

---

## What is a Message Queue?

A **message queue** is a kernel object that allows threads to exchange fixed-size data items asynchronously. Unlike semaphores (which signal events) or mutexes (which protect resources), message queues are designed for **data transfer**.

### Key Characteristics

| Feature | Description |
|---------|-------------|
| **Fixed Size** | All messages must be the same size |
| **FIFO Order** | First In, First Out (queue behavior) |
| **Blocking/Non-blocking** | Can wait for space/data or return immediately |
| **Thread Safe** | No mutex needed - kernel handles synchronization |
| **Copy Semantics** | Data is copied into/out of queue |

### When to Use Message Queues

✅ **USE message queues when:**
- Passing structured data between threads
- Multiple producers sending to one consumer
- Decoupling producers from consumers
- Buffering data from different sources
- You need FIFO ordering guarantees

❌ **DON'T use message queues when:**
- Only signaling events (use semaphore instead)
- Large data structures (use pointers + semaphore instead)
- Protecting shared variables (use mutex instead)
- Single bit flags (use semaphore or event instead)

---

## Architecture

This example implements an **Environmental Monitoring System** commonly found in:
- IoT weather stations
- Smart home devices
- Industrial monitoring equipment
- Medical devices
- Agricultural sensors

```
┌─────────────────────────────────────────────────────┐
│                   SENSOR THREADS                    │
│                  (Producers)                        │
├─────────────────────────────────────────────────────┤
│                                                     │
│  ┌──────────────────┐         Every 2000ms         │
│  │ Temperature      │────────────────┐             │
│  │ Sensor Thread    │                │             │
│  └──────────────────┘                │             │
│                                      ▼             │
│  ┌──────────────────┐         ┌──────────────┐    │
│  │ Humidity         │────────▶│  Message     │    │
│  │ Sensor Thread    │ Every   │  Queue       │    │
│  └──────────────────┘ 3000ms  │  (10 slots)  │    │
│                                └──────────────┘    │
│  ┌──────────────────┐         ▲                    │
│  │ Pressure         │─────────┘                    │
│  │ Sensor Thread    │ Every 2500ms                 │
│  └──────────────────┘                              │
│                                                     │
└─────────────────────────────────────────────────────┘
                        │
                        │ k_msgq_get()
                        │ (blocking)
                        ▼
┌─────────────────────────────────────────────────────┐
│              DATA LOGGER THREAD                     │
│                  (Consumer)                         │
├─────────────────────────────────────────────────────┤
│                                                     │
│  • Receives sensor messages                        │
│  • Logs data with timestamp                        │
│  • Tracks statistics                               │
│  • Could write to flash/SD/network                 │
│                                                     │
└─────────────────────────────────────────────────────┘
```

**Thread Priorities:**
- Sensor threads: Priority 5 (lower priority)
- Logger thread: Priority 4 (higher priority - processes data quickly)

---

## How This Example Works

### Message Structure

Each sensor reading is packaged into a message:

```c
typedef struct {
    sensor_type_t type;        /* TEMPERATURE, HUMIDITY, or PRESSURE */
    float value;               /* Sensor reading value */
    uint32_t timestamp_ms;     /* System uptime when read */
    uint32_t sequence;         /* Sequence number for tracking */
} sensor_data_msg_t;
```

### Message Queue Definition

```c
/* Queue holds 10 messages, each sizeof(sensor_data_msg_t), 4-byte aligned */
K_MSGQ_DEFINE(sensor_msgq, sizeof(sensor_data_msg_t), MSG_QUEUE_SIZE, 4);
```

**Parameters:**
- `sensor_msgq` - Queue name
- `sizeof(sensor_data_msg_t)` - Message size (must be fixed)
- `MSG_QUEUE_SIZE` (10) - Maximum messages in queue
- `4` - Memory alignment (typically 4 bytes)

### Producer Pattern (Sensor Threads)

Each sensor thread follows this pattern:

```c
void sensor_thread_generic(sensor_type_t type, uint32_t period_ms,
                           float (*read_fn)(void), sensor_stats_t *stats)
{
    while (1) {
        /* 1. Read sensor value */
        float value = read_fn();

        /* 2. Prepare message */
        sensor_data_msg_t msg = {
            .type = type,
            .value = value,
            .timestamp_ms = k_uptime_get_32(),
            .sequence = sequence++
        };

        /* 3. Try to send to queue (non-blocking) */
        int ret = k_msgq_put(&sensor_msgq, &msg, K_NO_WAIT);

        if (ret == 0) {
            /* Success - message queued */
            update_sensor_stats(stats, value, true);
        } else if (ret == -ENOMSG) {
            /* Queue full - data lost! */
            update_sensor_stats(stats, value, false);
            LOG_WRN("Queue full! Data lost");
        }

        /* 4. Wait for next sample period */
        k_msleep(period_ms);
    }
}
```

**Key Points:**
- Uses `K_NO_WAIT` to avoid blocking producers
- Handles queue overflow gracefully (logs warning)
- Each sensor has its own sampling rate
- Messages are copied into queue

### Consumer Pattern (Logger Thread)

The logger thread processes all sensor data:

```c
void logger_thread(void *arg1, void *arg2, void *arg3)
{
    sensor_data_msg_t msg;

    while (1) {
        /* Wait for message (blocks until available) */
        int ret = k_msgq_get(&sensor_msgq, &msg, K_FOREVER);

        if (ret == 0) {
            /* Process the sensor data */
            process_sensor_data(&msg);

            /* In real system: write to flash, send to network, etc. */
        }
    }
}
```

**Key Points:**
- Uses `K_FOREVER` to block until data available
- Processes messages in FIFO order
- Single consumer ensures ordered processing
- Message is copied out of queue

---

## Sampling Rates and Queue Behavior

**Sensor Periods:**
- Temperature: 2000ms (0.5 Hz)
- Humidity: 3000ms (0.33 Hz)
- Pressure: 2500ms (0.4 Hz)

**Message Rate:**
- ~1.23 messages/second on average
- Queue size: 10 messages
- Queue can buffer ~8 seconds of data if logger stops

**Queue Overflow Scenarios:**

Queue fills up when:
- Logger thread is blocked (processing takes too long)
- Network transmission delays
- Flash write operations
- Logger thread suspended

When queue is full:
- `k_msgq_put()` with `K_NO_WAIT` returns `-ENOMSG`
- Message is lost (by design - old data less valuable)
- Warning logged and statistics updated

---

## Building and Running

### Prerequisites

- Zephyr RTOS environment set up
- Board supported by Zephyr

### Build

```bash
cd app_msg_queue
west build -b <your_board> . -p always
```

Example for STM32F411E-DISCO:
```bash
west build -b stm32f411e_disco . -p always
```

### Flash

```bash
west flash
```

### View Output

```bash
screen /dev/ttyACM0 115200
# or
minicom -D /dev/ttyACM0 -b 115200
```

---

## Expected Output

```
*** Booting Zephyr OS build... ***
[00:00:00.000,000] <inf> data_logger: ===========================================
[00:00:00.001,000] <inf> data_logger:  Environmental Monitoring System
[00:00:00.002,000] <inf> data_logger:  Message Queue Demonstration
[00:00:00.003,000] <inf> data_logger: ===========================================

[00:00:00.100,000] <inf> data_logger: Starting data logger thread...
[00:00:00.200,000] <inf> data_logger: Starting sensor threads...
[00:00:00.201,000] <inf> data_logger: System initialized successfully

[00:00:00.300,000] <inf> data_logger: Temperature sensor thread started (period: 2000 ms)
[00:00:00.301,000] <inf> data_logger: Humidity sensor thread started (period: 3000 ms)
[00:00:00.302,000] <inf> data_logger: Pressure sensor thread started (period: 2500 ms)
[00:00:00.303,000] <inf> data_logger: Data logger thread started
[00:00:00.304,000] <inf> data_logger: Waiting for sensor data...
[00:00:00.305,000] <inf> data_logger: ========================================

[00:00:02.000,000] <inf> data_logger: [LOG] Temperature: 24.32 °C | Time: 2000 ms | Seq: 0
[00:00:02.500,000] <inf> data_logger: [LOG] Pressure: 1015.67 hPa | Time: 2500 ms | Seq: 0
[00:00:03.000,000] <inf> data_logger: [LOG] Humidity: 58.45 % | Time: 3000 ms | Seq: 0
[00:00:04.000,000] <inf> data_logger: [LOG] Temperature: 26.78 °C | Time: 4000 ms | Seq: 1
[00:00:05.000,000] <inf> data_logger: [LOG] Pressure: 1012.34 hPa | Time: 5000 ms | Seq: 1
[00:00:06.000,000] <inf> data_logger: [LOG] Temperature: 23.89 °C | Time: 6000 ms | Seq: 2
[00:00:06.000,000] <inf> data_logger: [LOG] Humidity: 61.23 % | Time: 6000 ms | Seq: 1
[00:00:07.500,000] <inf> data_logger: [LOG] Pressure: 1009.87 hPa | Time: 7500 ms | Seq: 2
[00:00:08.000,000] <inf> data_logger: [LOG] Temperature: 25.12 °C | Time: 8000 ms | Seq: 3
[00:00:09.000,000] <inf> data_logger: [LOG] Humidity: 59.67 % | Time: 9000 ms | Seq: 2

[00:00:10.000,000] <inf> data_logger: ----------------------------------------
[00:00:10.001,000] <inf> data_logger: Statistics (Total processed: 10)
[00:00:10.002,000] <inf> data_logger:   Temp:     Read:  5 Sent:  5 Failed:0
[00:00:10.003,000] <inf> data_logger:   Humidity: Read:  3 Sent:  3 Failed:0
[00:00:10.004,000] <inf> data_logger:   Pressure: Read:  3 Sent:  3 Failed:0
[00:00:10.005,000] <inf> data_logger:   Queue: 0/10 used
[00:00:10.006,000] <inf> data_logger: ========================================
```

**Key Observations:**
- Messages appear in time order (FIFO)
- Different sensors have different rates
- Statistics show all messages sent successfully
- Queue usage typically low (good consumer speed)

---

## Key Concepts

### 1. Message Queue Definition

```c
K_MSGQ_DEFINE(name, msg_size, max_msgs, align);
```

**Example:**
```c
K_MSGQ_DEFINE(sensor_msgq, sizeof(sensor_data_msg_t), 10, 4);
```

### 2. Sending Messages (Producer)

```c
int k_msgq_put(struct k_msgq *msgq, const void *data, k_timeout_t timeout);
```

**Timeout Options:**
- `K_NO_WAIT` - Return immediately if queue full
- `K_FOREVER` - Wait indefinitely for space
- `K_MSEC(100)` - Wait up to 100ms

**Return Values:**
- `0` - Success
- `-ENOMSG` - Queue full (K_NO_WAIT or timeout)
- `-EAGAIN` - Timeout expired

**Example:**
```c
sensor_data_msg_t msg = { .type = TEMP, .value = 25.0 };
int ret = k_msgq_put(&sensor_msgq, &msg, K_NO_WAIT);
if (ret != 0) {
    printk("Queue full!\n");
}
```

### 3. Receiving Messages (Consumer)

```c
int k_msgq_get(struct k_msgq *msgq, void *data, k_timeout_t timeout);
```

**Example:**
```c
sensor_data_msg_t msg;
int ret = k_msgq_get(&sensor_msgq, &msg, K_FOREVER);
if (ret == 0) {
    process_data(&msg);
}
```

### 4. Queue Status Functions

```c
/* Get number of messages in queue */
uint32_t k_msgq_num_used_get(struct k_msgq *msgq);

/* Get number of free slots */
uint32_t k_msgq_num_free_get(struct k_msgq *msgq);

/* Peek at next message without removing */
int k_msgq_peek(struct k_msgq *msgq, void *data);

/* Purge all messages */
void k_msgq_purge(struct k_msgq *msgq);
```

---

## Best Practices

### ✅ DO:

1. **Use fixed-size messages**
   ```c
   /* Good - fixed size structure */
   typedef struct {
       int sensor_id;
       float value;
       uint32_t timestamp;
   } sensor_msg_t;
   ```

2. **Handle queue overflow gracefully**
   ```c
   if (k_msgq_put(&msgq, &msg, K_NO_WAIT) != 0) {
       stats.dropped_messages++;
       LOG_WRN("Message dropped");
   }
   ```

3. **Use appropriate timeouts**
   - Producers: `K_NO_WAIT` (don't block sensors)
   - Consumers: `K_FOREVER` (wait for data)

4. **Size queue appropriately**
   - Consider producer rates
   - Consider consumer processing time
   - Add headroom for burst traffic

5. **Monitor queue usage**
   ```c
   uint32_t used = k_msgq_num_used_get(&msgq);
   if (used > (MAX_SIZE * 0.8)) {
       LOG_WRN("Queue nearly full: %u/%u", used, MAX_SIZE);
   }
   ```

### ❌ DON'T:

1. **Don't send pointers** (use data copy)
   ```c
   /* BAD - pointer invalidates after function returns */
   sensor_data_t data;
   sensor_data_t *ptr = &data;
   k_msgq_put(&msgq, &ptr, K_NO_WAIT);  /* Danger! */

   /* GOOD - send data by value */
   k_msgq_put(&msgq, &data, K_NO_WAIT);
   ```

2. **Don't use for large data**
   ```c
   /* BAD - wastes memory */
   typedef struct {
       uint8_t image[1024*1024];  /* 1MB per message! */
   } huge_msg_t;

   /* GOOD - use pointer + semaphore pattern instead */
   typedef struct {
       uint8_t *image_ptr;
       size_t size;
   } image_msg_t;
   ```

3. **Don't forget alignment**
   ```c
   /* Some architectures need alignment */
   K_MSGQ_DEFINE(msgq, sizeof(msg_t), 10, 4);  /* 4-byte align */
   ```

4. **Don't ignore return values**
   ```c
   /* BAD */
   k_msgq_put(&msgq, &msg, K_NO_WAIT);

   /* GOOD */
   if (k_msgq_put(&msgq, &msg, K_NO_WAIT) != 0) {
       handle_error();
   }
   ```

---

## Common Patterns

### Pattern 1: Multiple Producers, Single Consumer (This Example)

```c
/* One queue, multiple producers */
K_MSGQ_DEFINE(data_msgq, sizeof(data_msg_t), 20, 4);

void sensor1_thread(void) {
    while (1) {
        data_msg_t msg = read_sensor1();
        k_msgq_put(&data_msgq, &msg, K_NO_WAIT);
        k_msleep(1000);
    }
}

void sensor2_thread(void) {
    while (1) {
        data_msg_t msg = read_sensor2();
        k_msgq_put(&data_msgq, &msg, K_NO_WAIT);
        k_msleep(2000);
    }
}

void logger_thread(void) {
    while (1) {
        data_msg_t msg;
        k_msgq_get(&data_msgq, &msg, K_FOREVER);
        log_data(&msg);
    }
}
```

### Pattern 2: Command Queue (Single Producer, Single Consumer)

```c
K_MSGQ_DEFINE(cmd_msgq, sizeof(command_t), 5, 4);

/* Main thread sends commands */
void send_command(command_type_t cmd, uint32_t arg) {
    command_t msg = { .cmd = cmd, .arg = arg };
    k_msgq_put(&cmd_msgq, &msg, K_FOREVER);
}

/* Worker thread processes commands */
void worker_thread(void) {
    while (1) {
        command_t cmd;
        k_msgq_get(&cmd_msgq, &cmd, K_FOREVER);
        execute_command(&cmd);
    }
}
```

### Pattern 3: Mailbox (Single Producer, Multiple Consumers)

```c
K_MSGQ_DEFINE(broadcast_msgq, sizeof(event_t), 10, 4);

void event_producer(void) {
    event_t event = { .type = BUTTON_PRESS };
    k_msgq_put(&broadcast_msgq, &event, K_NO_WAIT);
}

void consumer1_thread(void) {
    event_t event;
    k_msgq_get(&broadcast_msgq, &event, K_FOREVER);
    /* Process event */
}

/* Note: Each consumer gets one message, not broadcast to all! */
/* For broadcast, use multiple queues or k_poll */
```

### Pattern 4: Priority Messages (Two Queues)

```c
K_MSGQ_DEFINE(high_prio_msgq, sizeof(msg_t), 10, 4);
K_MSGQ_DEFINE(low_prio_msgq, sizeof(msg_t), 20, 4);

void processor_thread(void) {
    while (1) {
        msg_t msg;

        /* Check high priority first */
        if (k_msgq_get(&high_prio_msgq, &msg, K_NO_WAIT) == 0) {
            process_urgent(&msg);
        }
        /* Then check low priority */
        else if (k_msgq_get(&low_prio_msgq, &msg, K_NO_WAIT) == 0) {
            process_normal(&msg);
        }
        else {
            /* Both empty, wait for any message */
            k_msleep(10);
        }
    }
}
```

---

## Message Queue vs Other Primitives

| Use Case | Best Choice | Why |
|----------|-------------|-----|
| Pass sensor data between threads | **Message Queue** | Structured data transfer |
| Signal event occurred | Semaphore | No data, just notification |
| Protect shared variable | Mutex | Mutual exclusion needed |
| Wait for multiple events | k_poll | Can wait on multiple objects |
| Pass large buffers | Pointer + Semaphore | Avoid copying large data |
| ISR to thread communication | **Message Queue** or Semaphore | Both ISR-safe |

---

## Real-World Applications

This pattern is used in:

### 1. IoT Sensor Networks
```
Multiple sensors → Message Queue → Cloud uplink thread
- Temperature, humidity, pressure, light sensors
- Queue buffers data during network outages
- FIFO ensures temporal ordering
```

### 2. Industrial Monitoring
```
Production line sensors → Queue → Data logging + Control
- Monitor machine states
- Log to SD card or flash
- Trigger alarms on threshold violations
```

### 3. Medical Devices
```
Vital sign monitors → Queue → Display + Storage + Alerts
- Heart rate, SpO2, temperature
- Continuous monitoring
- Critical alarm processing
```

### 4. Smart Home Devices
```
Multiple sensors → Queue → Home automation controller
- Motion, door, temperature, humidity
- Event correlation
- Automation rules engine
```

### 5. Data Acquisition Systems
```
ADC sampling threads → Queue → FFT/Processing thread
- High-speed data collection
- Buffered processing
- Synchronized multi-channel sampling
```

---

## Performance Considerations

### Memory Usage

Each message queue uses:
```
Memory = (msg_size × max_msgs) + overhead
```

**Example:**
```c
sizeof(sensor_data_msg_t) = 16 bytes
max_msgs = 10
Memory = 16 × 10 = 160 bytes + kernel overhead (~40 bytes)
Total ≈ 200 bytes
```

### Timing

- `k_msgq_put()` - Fast (copy data, update pointers)
- `k_msgq_get()` - Fast (copy data, update pointers)
- Blocking calls wake waiting thread immediately
- No priority inversion issues (unlike mutexes)

### Queue Sizing Formula

```
Queue Size = (Producer Rate × Processing Time) × Safety Factor

Example:
- Producer rate: 1 msg/second
- Processing time: 0.5 seconds
- Safety factor: 2×
- Queue size = 1 × 0.5 × 2 = 1 message minimum
- Use 5-10 for burst handling
```

---

## Debugging Tips

### 1. Monitor Queue Usage

```c
void print_queue_stats(void) {
    uint32_t used = k_msgq_num_used_get(&sensor_msgq);
    uint32_t free = k_msgq_num_free_get(&sensor_msgq);

    printk("Queue: %u/%u used (%u free)\n",
           used, used + free, free);

    if (used > (MSG_QUEUE_SIZE * 0.8)) {
        printk("WARNING: Queue nearly full!\n");
    }
}
```

### 2. Track Dropped Messages

```c
static uint32_t dropped_count = 0;

int ret = k_msgq_put(&msgq, &msg, K_NO_WAIT);
if (ret == -ENOMSG) {
    dropped_count++;
    if (dropped_count % 10 == 0) {
        printk("ERROR: %u messages dropped!\n", dropped_count);
    }
}
```

### 3. Add Sequence Numbers

```c
typedef struct {
    uint32_t sequence;  /* Detect lost messages */
    /* ... other fields ... */
} msg_t;

/* Consumer checks for gaps */
static uint32_t last_seq = 0;
if (msg.sequence != last_seq + 1) {
    printk("Gap detected: expected %u, got %u\n",
           last_seq + 1, msg.sequence);
}
last_seq = msg.sequence;
```

### 4. Timestamps for Latency Measurement

```c
typedef struct {
    uint32_t timestamp_sent;
    /* ... other fields ... */
} msg_t;

/* Producer */
msg.timestamp_sent = k_uptime_get_32();
k_msgq_put(&msgq, &msg, K_NO_WAIT);

/* Consumer */
uint32_t latency_ms = k_uptime_get_32() - msg.timestamp_sent;
printk("Queue latency: %u ms\n", latency_ms);
```

---

## Experiment Ideas

1. **Adjust Queue Size**
   - Try `MSG_QUEUE_SIZE = 2` - see overflow messages
   - Try `MSG_QUEUE_SIZE = 50` - observe memory usage

2. **Change Sampling Rates**
   - Make all sensors 100ms - high message rate
   - Make logger sleep 1 second in `process_sensor_data()` - force overflow

3. **Add More Sensors**
   - Add light sensor, gas sensor, etc.
   - Observe queue behavior with more producers

4. **Implement Data Filtering**
   - Logger only logs temperature changes > 0.5°C
   - Demonstrates consumer-side filtering

5. **Add Flash/SD Card Writing**
   - Simulate slow storage with `k_msleep(500)` in logger
   - See how queue buffers during slow writes

6. **Network Simulation**
   - Add "network thread" that reads from queue
   - Simulate connection loss (queue fills)
   - Simulate reconnection (queue drains)

---

## Related Examples

- **app_sem** - Semaphores for signaling and resource management
- **app_mutex** - Mutex for protecting shared data
- **app_thread** - Basic thread creation and management
- **app_workq** - Work queues for ISR-to-thread communication

---

## References

- [Zephyr Message Queue Documentation](https://docs.zephyrproject.org/latest/kernel/services/data_passing/message_queues.html)
- [Zephyr Kernel Services](https://docs.zephyrproject.org/latest/kernel/services/index.html)
- [Producer-Consumer Problem](https://en.wikipedia.org/wiki/Producer%E2%80%93consumer_problem)
- [Zephyr API Reference - Message Queues](https://docs.zephyrproject.org/latest/doxygen/html/group__msgq__apis.html)

---

## Summary

**Message Queues are perfect for:**
- ✅ Passing structured data between threads
- ✅ Decoupling producers from consumers
- ✅ Buffering asynchronous data streams
- ✅ Maintaining FIFO ordering
- ✅ Thread-safe communication without mutexes

This environmental monitoring example demonstrates a practical, real-world use case that you'll encounter in embedded IoT development. The pattern of multiple sensors feeding data to a central processor is fundamental to many embedded systems.
