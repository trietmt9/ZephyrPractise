# Work Queue Example

This example demonstrates the use of **Zephyr kernel work queues** for deferring time-consuming work from ISRs and managing background tasks in a smart home controller scenario.

## What You'll Learn

- Work queues for deferring ISR work to thread context
- System work queue vs custom work queues
- Immediate work submission vs delayed work
- Work item patterns and data passing
- Real-world ISR best practices
- Batching operations for efficiency

---

## Why Learn This?

**Work queues are the bridge between fast ISRs and complex processing.** They are THE solution to one of embedded systems' most critical requirements: **ISRs must complete in microseconds, but real work takes milliseconds.**

### The Golden Rule of Interrupts

**ISR Rule #1: Get in, get out, FAST.**

Interrupts disable other interrupts (or raise priority). If your ISR is slow:
- ❌ **Other interrupts are delayed** → missed events, lost data
- ❌ **System becomes unresponsive** → UI freezes, watchdog timeouts
- ❌ **Real-time deadlines missed** → control loops fail
- ❌ **Stack overflows** → ISRs run on limited interrupt stack
- ❌ **Priority inversion** → high-priority work waits for low-priority ISR

### The Problem Without Work Queues

Imagine a button interrupt on a smart home device:

```c
/* ❌ BAD: Doing everything in ISR */
void button_isr(void)
{
    // These operations are TOO SLOW for ISR context:
    debounce_button();        // 50ms delay - BLOCKS OTHER INTERRUPTS!
    update_display();         // 100ms SPI transaction - DISASTER!
    write_to_flash();         // 10ms flash write - TERRIBLE!
    send_network_packet();    // 500ms network delay - CATASTROPHIC!

    // Total: 660ms with interrupts disabled → SYSTEM DEAD
}
```

**What happens:**
- Other button presses missed
- UART data lost
- Watchdog timer expires → system resets
- Control loops miss deadlines → equipment damage
- User sees frozen interface

**Without work queues, you're forced to:**
- ❌ Use slow ISRs (breaks real-time requirements)
- ❌ Implement complex flag polling in main loop (unreliable, slow response)
- ❌ Create dedicated threads for every event source (wastes memory)
- ❌ Use message queues for simple events (overkill, complex)

### The Solution: Work Queues

```c
/* ✅ GOOD: Fast ISR + Work Queue */
void button_isr(void)
{
    k_work_submit(&button_work);  // <5µs - submits work and returns
    // ISR done! Work processed in thread context later
}

void button_work_handler(struct k_work *work)
{
    // Runs in thread context - can take as long as needed:
    debounce_button();        // ✅ OK
    update_display();         // ✅ OK
    write_to_flash();         // ✅ OK
    send_network_packet();    // ✅ OK
}
```

**Result:** ISR completes in microseconds, heavy work processed in thread context.

### Real-World Scenarios Where Work Queues Are Critical

| Industry | Use Case | Why Work Queue Is Needed |
|----------|----------|---------------------------|
| **Smart Home** | Button press ISR → toggle lights, update cloud | Can't do network I/O in ISR; work queue defers to thread |
| **Industrial Control** | Sensor threshold exceeded → log data, trigger alarm | Flash writes and I2C are too slow for ISR context |
| **Medical Devices** | ECG spike detected → process waveform, update display | Complex DSP algorithms can't run in ISR |
| **Automotive** | CAN message received → parse protocol, update state | Protocol processing is multi-step and time-consuming |
| **IoT Devices** | UART byte received → assemble packet, validate, process | Packet processing involves memory allocation, validation |
| **Data Acquisition** | ADC conversion complete → filter, store, batch upload | Batch operations require buffering and network access |

### Five Critical Problems Work Queues Solve

#### 1. **ISR Deferral** (The Primary Use Case)

**Problem:** GPIO interrupt fires, need to toggle LED via I2C expander (5ms operation)

```c
/* ❌ Can't do this in ISR - I2C takes milliseconds */
void button_isr(void) {
    i2c_write(LED_EXPANDER, ...);  // BLOCKS 5ms - DISASTER!
}

/* ✅ Defer to work queue */
void button_isr(void) {
    k_work_submit(&led_work);  // <5µs
}

void led_work_handler(struct k_work *work) {
    i2c_write(LED_EXPANDER, ...);  // Safe in thread context
}
```

**What you can do in work handlers but NOT in ISRs:**
- ✅ I2C/SPI communication (millisecond delays)
- ✅ Flash/SD card writes (10-100ms)
- ✅ Network operations (seconds)
- ✅ Display updates (milliseconds)
- ✅ malloc/free (can block)
- ✅ Use mutexes (can sleep)
- ✅ Heavy computation (FFT, crypto, etc.)

#### 2. **Delayed Work** (Timer-Triggered Tasks)

**Problem:** Need to check sensor every 10 seconds

```c
/* ❌ Without work queue: Create dedicated thread + sleep loop */
void sensor_thread(void) {
    while (1) {
        check_sensor();
        k_msleep(10000);  // Wastes thread stack (1KB+)
    }
}

/* ✅ With delayed work: No dedicated thread needed */
K_WORK_DELAYABLE_DEFINE(sensor_work, check_sensor);
k_work_schedule(&sensor_work, K_SECONDS(10));  // Reschedules itself
```

**Benefits:**
- No dedicated thread stack (saves 1-4KB per periodic task)
- Centralized scheduling
- Easy cancellation

#### 3. **Batching Operations** (This Example!)

**Problem:** Sensor readings arrive individually but should be uploaded in batches

```c
/* Collect readings */
add_sensor_reading(temp);  // Adds to batch

/* After timeout OR batch full, automatically upload */
void upload_handler(struct k_work *work) {
    upload_batch_to_cloud();  // Send all at once
}
```

**Use cases:**
- Network uploads (reduce packet overhead)
- Flash writes (wear leveling)
- Database inserts (transaction efficiency)

#### 4. **Work Prioritization** (Custom Work Queues)

**Problem:** Button handling needs higher priority than background logging

```c
/* System work queue: Default priority */
k_work_submit(&log_work);

/* Custom high-priority work queue */
k_work_submit_to_queue(&ui_workq, &button_work);
```

**Result:** UI-critical work preempts background tasks.

#### 5. **Avoiding Thread Proliferation**

**Without work queues:**
```
Button1 → Thread 1 (1KB stack)
Button2 → Thread 2 (1KB stack)
Button3 → Thread 3 (1KB stack)
Timer1  → Thread 4 (1KB stack)
Timer2  → Thread 5 (1KB stack)
Total: 5KB stack + 5 thread control blocks
```

**With work queue:**
```
All events → Work Queue → 1 Worker Thread (1KB stack)
Total: 1KB stack + 1 thread control block
```

**Savings: 80% memory reduction!**

### Work Queue vs Other Primitives

| Scenario | Best Solution | Why |
|----------|--------------|-----|
| ISR needs to trigger processing | **Work Queue** | Moves work to thread context |
| ISR needs to signal thread | Semaphore | Simpler if thread already exists |
| ISR needs to send data | Message Queue | If data must be passed |
| Periodic background task | **Delayed Work** | No dedicated thread needed |
| One-time deferred task | **Work Queue** | Simple and efficient |
| Multiple priority levels | **Multiple Work Queues** | Separate queues for different priorities |

### What You'll Be Able to Do After This

✅ Write fast, correct ISRs that defer heavy work
✅ Eliminate dedicated threads for periodic tasks (save memory)
✅ Implement button debouncing properly
✅ Batch operations for network efficiency
✅ Build responsive systems that never "freeze"
✅ Pass embedded systems interviews (work queues are commonly tested!)

### The Cost of Not Understanding Work Queues

**Real-world failures from slow ISRs:**
- 🔴 Medical device FDA recall: ISR blocked for 200ms, missed critical alarm
- 🔴 Industrial controller: Slow ISR caused watchdog timeout, emergency shutdown
- 🔴 IoT device: UART ISR processing took 50ms, incoming bytes lost, corruption
- 🔴 Automotive: CAN ISR delay caused ECU communication failure

**Bottom line:** Work queues are not optional for professional embedded development. If your system has interrupts (and all embedded systems do), you MUST understand how to defer work properly. This is fundamental knowledge that separates hobby projects from production-quality firmware.

---

## What is a Work Queue?

A **work queue** is a kernel object that manages a queue of work items (tasks) to be processed by a dedicated worker thread. Work items are submitted from ISRs or threads and executed asynchronously in thread context.

### Key Characteristics

| Feature | Description |
|---------|-------------|
| **Thread Context** | Work runs in thread (not ISR) - can sleep, block, use mutexes |
| **FIFO Execution** | Work items processed in order of submission |
| **ISR-Safe Submission** | Can submit work from ISR context |
| **Delayed Work** | Can schedule work to run after a timeout |
| **Lightweight** | Shares one worker thread for many work items |

### Types of Work Queues

#### 1. System Work Queue

Built-in, general-purpose work queue:

```c
K_WORK_DEFINE(my_work, work_handler);
k_work_submit(&my_work);  // Uses system work queue
```

**Pros:**
- Already exists (no setup needed)
- Shared among all subsystems

**Cons:**
- Medium priority (might not meet all timing needs)
- Shared (your work waits behind others)

#### 2. Custom Work Queue

User-created work queue with custom priority:

```c
static K_THREAD_STACK_DEFINE(my_workq_stack, 1024);
static struct k_work_q my_workq;

k_work_queue_start(&my_workq, my_workq_stack,
                   K_THREAD_STACK_SIZEOF(my_workq_stack),
                   PRIORITY, NULL);

k_work_submit_to_queue(&my_workq, &my_work);  // Use custom queue
```

**Pros:**
- Custom priority (higher for UI, lower for logging)
- Isolated (your work not delayed by others)

**Cons:**
- Uses extra stack memory
- Need to manage lifecycle

### Types of Work Items

#### 1. Immediate Work

Executes as soon as worker thread is scheduled:

```c
static void work_handler(struct k_work *work)
{
    LOG_INF("Work executing");
}

K_WORK_DEFINE(my_work, work_handler);

/* From ISR or thread */
k_work_submit(&my_work);  // Queued immediately
```

#### 2. Delayable Work

Executes after a specified timeout:

```c
static void delayed_handler(struct k_work *work)
{
    LOG_INF("Delayed work executing");
}

K_WORK_DELAYABLE_DEFINE(delayed_work, delayed_handler);

/* Schedule for 5 seconds from now */
k_work_schedule(&delayed_work, K_SECONDS(5));

/* Reschedule (common for periodic tasks) */
k_work_reschedule(&delayed_work, K_SECONDS(10));
```

---

## Architecture

This example implements a **Smart Home Controller** with:

```
┌─────────────────────────────────────────────────────┐
│                   EVENT SOURCES                     │
├─────────────────────────────────────────────────────┤
│                                                     │
│  Button ISRs        Sensor Readings   Timers       │
│      │                    │              │         │
│      │ k_work_submit()    │              │         │
│      ▼                    ▼              ▼         │
│  ┌────────────────────────────────────────────┐   │
│  │         CUSTOM WORK QUEUE                  │   │
│  │      (High Priority Thread)                │   │
│  │                                            │   │
│  │  ┌──────────┐  ┌──────────┐  ┌─────────┐  │   │
│  │  │ Button   │  │ Sensor   │  │ Status  │  │   │
│  │  │ Handler  │  │ Upload   │  │ Check   │  │   │
│  │  └──────────┘  └──────────┘  └─────────┘  │   │
│  └────────────────────────────────────────────┘   │
│                                                     │
└─────────────────────────────────────────────────────┘
         │              │               │
         ▼              ▼               ▼
    Toggle Light   Upload Batch    Log Statistics
    Update UI      to Cloud        Report Status
```

**Work Items:**
1. **Button Work** - Immediate work triggered by button ISR
2. **Sensor Upload** - Delayed work that batches sensor readings
3. **Status Check** - Periodic delayed work (reschedules itself)

---

## How This Example Works

### 1. Custom Work Queue Setup

```c
/* Define stack for work queue thread */
static K_THREAD_STACK_DEFINE(custom_workq_stack, 1024);
static struct k_work_q custom_workq;

/* Start custom work queue with priority 5 */
k_work_queue_start(&custom_workq,
                  custom_workq_stack,
                  K_THREAD_STACK_SIZEOF(custom_workq_stack),
                  5,  /* Priority */
                  NULL);
```

Creates a dedicated worker thread at priority 5 for processing work items.

### 2. Button Event Handling (ISR Deferral)

**Work Item Structure:**
```c
struct button_work {
    struct k_work work;        /* Must be first! */
    uint32_t button_id;        /* Custom data */
    button_event_t event;
};
```

**Initialize:**
```c
k_work_init(&button_work_items[i].work, button_work_handler);
```

**Submit from ISR:**
```c
void simulate_button_isr(uint32_t button_id, button_event_t event)
{
    struct button_work *work = &button_work_items[button_id];
    work->button_id = button_id;
    work->event = event;

    /* Fast! Returns immediately */
    k_work_submit_to_queue(&custom_workq, &work->work);
}
```

**Process in thread:**
```c
void button_work_handler(struct k_work *work)
{
    struct button_work *btn = CONTAINER_OF(work, struct button_work, work);

    /* Can do time-consuming operations here */
    k_msleep(50);  /* Debounce */
    toggle_light(btn->button_id);
    update_display();
    write_to_flash();
}
```

### 3. Sensor Batch Upload (Delayed Work)

**Initialize:**
```c
k_work_init_delayable(&sensor_data.upload_work, sensor_upload_handler);
```

**Add readings with smart scheduling:**
```c
void add_sensor_reading(float temp)
{
    sensor_data.values[sensor_data.count++] = temp;

    if (sensor_data.count == 1) {
        /* First reading - schedule upload after timeout */
        k_work_reschedule_for_queue(&custom_workq,
                                   &sensor_data.upload_work,
                                   K_MSEC(5000));
    }

    if (sensor_data.count >= MAX_BATCH) {
        /* Batch full - upload immediately */
        k_work_reschedule_for_queue(&custom_workq,
                                   &sensor_data.upload_work,
                                   K_NO_WAIT);
    }
}
```

**Upload handler:**
```c
void sensor_upload_handler(struct k_work *work)
{
    /* Upload batch to cloud/server */
    upload_to_network(sensor_data.values, sensor_data.count);
    sensor_data.count = 0;  /* Reset batch */
}
```

**How it works:**
- First reading triggers 5-second timeout
- If 10 readings arrive before timeout → immediate upload
- Otherwise, uploads after 5 seconds with partial batch
- Reduces network traffic by batching

### 4. Periodic Status Check (Self-Rescheduling Work)

**Initialize:**
```c
k_work_init_delayable(&status_checker.work, status_check_handler);
k_work_schedule(&status_checker.work, K_SECONDS(10));
```

**Handler that reschedules itself:**
```c
void status_check_handler(struct k_work *work)
{
    struct k_work_delayable *dwork = k_work_delayable_from_work(work);

    /* Do periodic work */
    log_statistics();
    check_system_health();

    /* Reschedule for next check */
    k_work_reschedule_for_queue(&custom_workq, dwork, K_SECONDS(10));
}
```

**Result:** Periodic task without dedicated thread!

---

## Building and Running

### Prerequisites

- Zephyr RTOS environment set up
- Board supported by Zephyr

### Build

```bash
cd app_workq
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
[00:00:00.000,000] <inf> workq_demo: ==========================================
[00:00:00.001,000] <inf> workq_demo:  Work Queue Example
[00:00:00.002,000] <inf> workq_demo:  Smart Home Controller Demo
[00:00:00.003,000] <inf> workq_demo: ==========================================

[00:00:00.100,000] <inf> workq_demo: Custom work queue started (priority: 5)
[00:00:00.101,000] <inf> workq_demo: Button work items initialized (count: 3)
[00:00:00.102,000] <inf> workq_demo: Sensor batch work initialized (max batch: 10)
[00:00:00.103,000] <inf> workq_demo: Periodic status check started (every 10 seconds)

[00:00:01.000,000] <inf> workq_demo: Simulating button press on Button 0...
[00:00:01.001,000] <inf> workq_demo: Button Handler: Button 0 - PRESSED (time: 1001 ms)
[00:00:01.002,000] <inf> workq_demo:   → Action: Toggle light #0
[00:00:01.052,000] <dbg> workq_demo: Button Handler: Completed

[00:00:01.500,000] <inf> workq_demo: Simulating button release on Button 0...
[00:00:01.501,000] <inf> workq_demo: Button Handler: Button 0 - RELEASED (time: 1501 ms)

[00:00:02.500,000] <inf> workq_demo: Starting sensor readings...
[00:00:02.501,000] <inf> workq_demo: Sensor reading: 24.32 °C
[00:00:03.300,000] <inf> workq_demo: Sensor reading: 21.67 °C
[00:00:04.100,000] <inf> workq_demo: Sensor reading: 26.89 °C

[00:00:07.500,000] <inf> workq_demo: Upload Handler: Uploading 3 sensor readings to cloud
[00:00:07.501,000] <inf> workq_demo:   → Reading[0]: 24.32 °C
[00:00:07.502,000] <inf> workq_demo:   → Reading[1]: 21.67 °C
[00:00:07.503,000] <inf> workq_demo:   → Reading[2]: 26.89 °C
[00:00:07.604,000] <inf> workq_demo: Upload Handler: Batch uploaded successfully

[00:00:10.000,000] <inf> workq_demo: ========================================
[00:00:10.001,000] <inf> workq_demo: Status Check #1
[00:00:10.002,000] <inf> workq_demo:   Button events processed: 2
[00:00:10.003,000] <inf> workq_demo:   Sensor batches uploaded: 1
[00:00:10.004,000] <inf> workq_demo:   Status checks performed: 1
[00:00:10.005,000] <inf> workq_demo:   System uptime: 10 seconds
[00:00:10.006,000] <inf> workq_demo: ========================================

[00:00:11.000,000] <inf> workq_demo: Simulating rapid button presses (Button 1)...
[00:00:11.001,000] <inf> workq_demo: Button Handler: Button 1 - PRESSED (time: 11001 ms)
[00:00:11.051,000] <inf> workq_demo: Button Handler: Button 1 - RELEASED (time: 11101 ms)
[00:00:11.101,000] <inf> workq_demo: Button Handler: Button 1 - PRESSED (time: 11201 ms)
[00:00:11.151,000] <inf> workq_demo: Button Handler: Button 1 - RELEASED (time: 11301 ms)

[00:00:13.500,000] <inf> workq_demo: Adding more sensor readings to fill batch...
[00:00:15.800,000] <inf> workq_demo: Sensor: Batch full! Triggering immediate upload
[00:00:15.801,000] <inf> workq_demo: Upload Handler: Uploading 10 sensor readings to cloud
[00:00:15.900,000] <inf> workq_demo: Upload Handler: Batch uploaded successfully

[00:00:20.000,000] <inf> workq_demo: ========================================
[00:00:20.001,000] <inf> workq_demo: Status Check #2
[00:00:20.002,000] <inf> workq_demo:   Button events processed: 6
[00:00:20.003,000] <inf> workq_demo:   Sensor batches uploaded: 2
[00:00:20.004,000] <inf> workq_demo:   Status checks performed: 2
[00:00:20.005,000] <inf> workq_demo:   System uptime: 20 seconds
[00:00:20.006,000] <inf> workq_demo: ========================================
```

---

## Key Concepts

### 1. Define Work Item

**Immediate work:**
```c
static void my_handler(struct k_work *work)
{
    LOG_INF("Work executing");
}

K_WORK_DEFINE(my_work, my_handler);
```

**Delayable work:**
```c
static void delayed_handler(struct k_work *work)
{
    LOG_INF("Delayed work executing");
}

K_WORK_DELAYABLE_DEFINE(my_delayed_work, delayed_handler);
```

### 2. Submit Work

**To system work queue:**
```c
k_work_submit(&my_work);
```

**To custom work queue:**
```c
k_work_submit_to_queue(&custom_workq, &my_work);
```

**From ISR (both are ISR-safe):**
```c
void my_isr(void)
{
    k_work_submit(&my_work);  // Safe from ISR!
}
```

### 3. Schedule Delayed Work

**One-time delay:**
```c
k_work_schedule(&delayed_work, K_MSEC(1000));  // Run after 1 second
```

**Reschedule (cancel previous, schedule new):**
```c
k_work_reschedule(&delayed_work, K_SECONDS(5));
```

**Cancel delayed work:**
```c
k_work_cancel_delayable(&delayed_work);
```

### 4. Work Item with Data (Common Pattern)

```c
struct my_work_data {
    struct k_work work;    /* Must be first member */
    int custom_field1;
    float custom_field2;
};

static void handler(struct k_work *work)
{
    /* Get containing structure */
    struct my_work_data *data = CONTAINER_OF(work, struct my_work_data, work);

    /* Access custom data */
    LOG_INF("Field1: %d, Field2: %.2f", data->custom_field1, data->custom_field2);
}

/* Initialize */
struct my_work_data my_data;
k_work_init(&my_data.work, handler);
my_data.custom_field1 = 42;
my_data.custom_field2 = 3.14;

/* Submit */
k_work_submit(&my_data.work);
```

### 5. Create Custom Work Queue

```c
#define MY_WORKQ_STACK_SIZE 1024
#define MY_WORKQ_PRIORITY 5

static K_THREAD_STACK_DEFINE(my_workq_stack, MY_WORKQ_STACK_SIZE);
static struct k_work_q my_workq;

/* In main() or initialization */
k_work_queue_start(&my_workq,
                  my_workq_stack,
                  K_THREAD_STACK_SIZEOF(my_workq_stack),
                  MY_WORKQ_PRIORITY,
                  NULL);
```

---

## Best Practices

### ✅ DO:

1. **Keep ISRs minimal - submit work and return**
   ```c
   void gpio_isr(void)
   {
       k_work_submit(&button_work);  // Fast!
       // Don't do anything else
   }
   ```

2. **Use system work queue for non-critical tasks**
   ```c
   k_work_submit(&log_work);  // Logging is not time-critical
   ```

3. **Use custom work queue for different priorities**
   ```c
   k_work_queue_start(&ui_workq, ..., HIGH_PRIORITY, ...);
   k_work_queue_start(&log_workq, ..., LOW_PRIORITY, ...);
   ```

4. **Pass data using CONTAINER_OF pattern**
   ```c
   struct my_work {
       struct k_work work;
       int data;
   };

   void handler(struct k_work *work) {
       struct my_work *w = CONTAINER_OF(work, struct my_work, work);
       use(w->data);
   }
   ```

5. **Use delayed work for periodic tasks**
   ```c
   void periodic_handler(struct k_work *work) {
       do_periodic_task();
       k_work_reschedule(&periodic_work, K_SECONDS(10));  // Reschedule self
   }
   ```

### ❌ DON'T:

1. **Don't do heavy processing in ISRs**
   ```c
   /* ❌ BAD */
   void uart_isr(void) {
       process_packet();  // Too slow!
       write_to_flash();  // WAY too slow!
   }

   /* ✅ GOOD */
   void uart_isr(void) {
       k_work_submit(&process_work);  // Fast!
   }
   ```

2. **Don't block in work handlers unnecessarily**
   ```c
   /* ⚠️ OK but wasteful */
   void work_handler(struct k_work *work) {
       while (1) {
           do_something();
           k_msleep(1000);  // Blocks work queue thread!
       }
   }

   /* ✅ BETTER - use delayed work */
   void work_handler(struct k_work *work) {
       do_something();
       k_work_reschedule(&delayed_work, K_SECONDS(1));
   }
   ```

3. **Don't submit same work item multiple times without waiting**
   ```c
   /* ❌ BAD - can cause issues */
   k_work_submit(&work1);
   k_work_submit(&work1);  // Work1 might still be queued!

   /* ✅ GOOD - check if already pending */
   if (!k_work_is_pending(&work1)) {
       k_work_submit(&work1);
   }
   ```

4. **Don't forget work structure must persist**
   ```c
   /* ❌ BAD - work is on stack! */
   void function(void) {
       struct k_work work;
       k_work_init(&work, handler);
       k_work_submit(&work);
       // Function returns, stack destroyed, work structure invalid!
   }

   /* ✅ GOOD - static or heap allocated */
   static struct k_work work;  // Persists
   ```

---

## Common Patterns

### Pattern 1: ISR Deferral (Button Handling)

```c
/* Work item */
static struct k_work button_work;

static void button_handler(struct k_work *work)
{
    /* Runs in thread context */
    debounce_button();
    update_ui();
    log_event();
}

void button_isr(void)
{
    /* Fast ISR - just submit work */
    k_work_submit(&button_work);
}

/* Init */
k_work_init(&button_work, button_handler);
```

### Pattern 2: Periodic Task Without Dedicated Thread

```c
static void periodic_task(struct k_work *work)
{
    struct k_work_delayable *dwork = k_work_delayable_from_work(work);

    /* Do periodic work */
    check_sensors();
    update_watchdog();

    /* Reschedule */
    k_work_reschedule(dwork, K_SECONDS(10));
}

K_WORK_DELAYABLE_DEFINE(periodic_work, periodic_task);

/* Start periodic task */
k_work_schedule(&periodic_work, K_SECONDS(10));
```

### Pattern 3: Batching with Timeout

```c
#define BATCH_SIZE 10
#define BATCH_TIMEOUT_MS 5000

static struct {
    int items[BATCH_SIZE];
    int count;
    struct k_work_delayable upload_work;
} batch;

void add_item(int item)
{
    batch.items[batch.count++] = item;

    if (batch.count == 1) {
        /* First item - start timeout */
        k_work_schedule(&batch.upload_work, K_MSEC(BATCH_TIMEOUT_MS));
    }

    if (batch.count >= BATCH_SIZE) {
        /* Batch full - upload now */
        k_work_reschedule(&batch.upload_work, K_NO_WAIT);
    }
}

static void upload_handler(struct k_work *work)
{
    upload_batch(batch.items, batch.count);
    batch.count = 0;
}
```

### Pattern 4: Work with Custom Data

```c
struct sensor_work {
    struct k_work work;
    int sensor_id;
    float value;
};

static struct sensor_work sensor_works[3];

static void sensor_handler(struct k_work *work)
{
    struct sensor_work *sw = CONTAINER_OF(work, struct sensor_work, work);
    LOG_INF("Sensor %d: %.2f", sw->sensor_id, sw->value);
}

/* Submit from ISR */
void sensor_isr(int id, float value)
{
    sensor_works[id].sensor_id = id;
    sensor_works[id].value = value;
    k_work_submit(&sensor_works[id].work);
}

/* Init */
for (int i = 0; i < 3; i++) {
    k_work_init(&sensor_works[i].work, sensor_handler);
}
```

---

## Work Queue vs Other Primitives

| Scenario | Use | Why |
|----------|-----|-----|
| ISR needs to trigger processing | **Work Queue** | Defers to thread context |
| ISR just needs to signal | Semaphore | Simpler if thread exists |
| ISR needs to send data | Message Queue or Work Queue | Queue for multiple items, work for single item |
| Periodic background task | **Delayed Work** | No dedicated thread needed |
| Multiple priority levels | **Multiple Work Queues** | Separate queues for priorities |
| Heavy processing from ISR | **Work Queue** | Can't do in ISR context |

---

## Real-World Applications

### 1. Button/Input Handling
```
GPIO ISR → Work Queue → Debounce → Action
```

### 2. UART Packet Processing
```
UART RX ISR → Byte received → When complete packet:
  → Submit work → Parse → Validate → Process
```

### 3. Sensor Data Batching
```
Periodic sensor readings → Add to batch →
  Timeout or batch full → Upload to cloud
```

### 4. Network Operations
```
ISR receives network packet → Submit work →
  Thread context: Parse, process, respond
```

### 5. Display Updates
```
Data changed → Submit display update work →
  Thread: I2C/SPI transaction to update screen
```

---

## Performance Considerations

### Memory Usage

**System work queue:** ~1-2KB (already allocated)

**Custom work queue:**
```
Stack size + Work queue structure ≈ 1KB + ~100 bytes
```

**Work items:**
```
Immediate: sizeof(struct k_work) = ~24 bytes
Delayed:   sizeof(struct k_work_delayable) = ~48 bytes
```

### Latency

**Work submission:** <5 microseconds (ISR-safe)

**Work execution latency:**
- System work queue: Depends on other work
- Custom work queue: Depends on priority and current work

**Delayed work accuracy:** Within 1 tick (typically 1-10ms)

---

## Debugging Tips

### 1. Check if Work is Pending

```c
if (k_work_is_pending(&my_work)) {
    LOG_WRN("Work still pending, not submitting again");
}
```

### 2. Track Work Execution Count

```c
static uint32_t work_count = 0;

void work_handler(struct k_work *work)
{
    work_count++;
    LOG_DBG("Work executed %u times", work_count);
}
```

### 3. Measure Work Execution Time

```c
void work_handler(struct k_work *work)
{
    uint32_t start = k_uptime_get_32();

    /* Do work */
    heavy_processing();

    uint32_t duration = k_uptime_get_32() - start;
    LOG_INF("Work took %u ms", duration);

    if (duration > 100) {
        LOG_WRN("Work taking too long!");
    }
}
```

### 4. Monitor Work Queue Depth

```c
/* For custom work queue, check thread state */
LOG_INF("Work queue state: %d", k_work_queue_state(&my_workq));
```

---

## Experiment Ideas

1. **ISR Timing Test**
   - Measure ISR execution time with vs without work deferral
   - Use GPIO toggle + oscilloscope or logic analyzer

2. **Batch Size Tuning**
   - Change `MAX_SENSOR_BATCH` from 5 to 20
   - Measure network efficiency (bytes/packet)

3. **Priority Testing**
   - Create two work queues with different priorities
   - Submit work to both, observe execution order

4. **Memory Comparison**
   - Replace periodic delayed work with dedicated thread
   - Compare memory usage (stack + TCB)

5. **Missed Events**
   - Submit work faster than it can be processed
   - Observe queue saturation behavior

---

## Related Examples

- **app_mutex** - Protecting shared data accessed by work handlers
- **app_sem** - Signaling between ISR and thread (alternative to work queues)
- **app_msg_queue** - Passing data from ISR to thread
- **app_thread** - Understanding thread priorities and scheduling

---

## References

- [Zephyr Work Queue Documentation](https://docs.zephyrproject.org/latest/kernel/services/threads/workqueue.html)
- [Zephyr API Reference - Work Queues](https://docs.zephyrproject.org/latest/doxygen/html/group__workqueue__apis.html)
- [Best Practices for Interrupt Handling](https://docs.zephyrproject.org/latest/kernel/services/interrupts.html)
- [Deferred Work Patterns in RTOS](https://www.embedded.com/deferred-interrupt-handling-in-rtos/)

---

## Summary

**Work queues are essential for:**
- ✅ Fast ISR response (microsecond ISR, millisecond processing)
- ✅ Deferring heavy work from interrupt context
- ✅ Eliminating dedicated threads for periodic tasks
- ✅ Batching operations for efficiency
- ✅ Prioritizing different types of background work

This smart home controller example demonstrates practical patterns you'll use in production embedded systems. The ability to write fast ISRs that defer work properly is a fundamental skill for embedded firmware engineers.

**Remember:** ISRs must be FAST. Work queues make that possible while still allowing complex processing. Master this, and you'll write better, more reliable embedded systems.
