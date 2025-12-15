# Mutex Example

This example demonstrates the use of **Zephyr kernel mutexes** for protecting shared resources in multi-threaded applications.

## What You'll Learn

- Understanding race conditions in concurrent programming
- How to use mutexes to protect critical sections
- Mutex locking and unlocking mechanisms
- Mutex timeout handling
- Thread synchronization best practices

---

## Why Learn This?

**Mutexes are fundamental to multi-threaded embedded systems.** Without them, your system will have unpredictable bugs that are nearly impossible to debug.

### The Problem Without Mutexes

Imagine two threads reading a sensor and updating a display:
```
Thread 1: Read temperature = 25°C, update display...
Thread 2: Read humidity = 60%, update display...
```

Without protection, both threads might write to the display simultaneously, causing:
- **Corrupted display output** - Mixed characters, garbled text
- **Lost data** - One reading overwrites the other
- **System crashes** - Hardware registers left in inconsistent states

### Real-World Scenarios Where Mutexes Are Critical

| Industry | Use Case | What Breaks Without Mutex |
|----------|----------|---------------------------|
| **Medical Devices** | Multiple sensors updating patient vitals display | Wrong vital signs displayed → life-threatening |
| **Automotive** | Engine control threads accessing CAN bus | Corrupted messages → engine malfunction |
| **Industrial Automation** | Multiple control loops updating actuator state | Equipment damage, safety hazards |
| **IoT Devices** | Sensor data + network stack sharing flash storage | Corrupted configuration, device bricked |
| **Robotics** | Motion control + sensor fusion updating position | Robot moves unpredictably, collisions |

### Why Race Conditions Are Dangerous

Race conditions are **hard to detect** because:
- ❌ They don't always happen (timing-dependent)
- ❌ They disappear when you add debug prints (changes timing)
- ❌ They work fine in testing, fail in production
- ❌ They may only occur under heavy load or specific conditions

**Learning mutexes now prevents months of debugging nightmares later.**

### What You'll Be Able to Do After This

✅ Protect shared variables (counters, flags, state machines)
✅ Safely share hardware resources (UART, SPI, I2C, display)
✅ Coordinate multiple threads accessing the same data structures
✅ Prevent data corruption in real-time systems
✅ Debug race conditions when they occur

**Bottom line:** If your embedded system has more than one thread, you NEED to understand mutexes. This is not optional knowledge—it's mandatory for professional embedded development.

---

## What is a Mutex?

A **mutex** (mutual exclusion) is a synchronization primitive that prevents multiple threads from simultaneously accessing a shared resource. Only one thread can hold the mutex at a time.

### Why Do We Need Mutexes?

When multiple threads access and modify shared data without synchronization, **race conditions** can occur, leading to:
- Data corruption
- Inconsistent state
- Unpredictable behavior
- Lost updates

---

## Architecture

This example demonstrates a **thread-based architecture** where:

1. **Main thread** - Only creates the coordinator thread and sleeps forever
2. **Coordinator thread** - Orchestrates all tests and spawns counter threads
3. **Counter threads** - Three worker threads that increment the shared counter

This shows that `main()` doesn't need to do all the work - it can delegate to other threads!

## How This Example Works

### The Shared Resource

```c
static volatile uint32_t shared_counter = 0;
```

A simple counter that multiple threads will try to increment.

### The Problem (Without Mutex)

**Test 1** demonstrates what happens when threads compete for the shared resource **without protection**:

```c
/* UNPROTECTED - Race Condition! */
uint32_t temp = shared_counter;
k_busy_wait(10);  // Context switch can happen here!
shared_counter = temp + 1;
```

**Expected Result:** Counter = 3000 (3 threads × 1000 increments)
**Actual Result:** Counter < 3000 (lost updates due to race condition)

### The Solution (With Mutex)

**Test 2** shows proper synchronization using a mutex:

```c
/* PROTECTED with mutex */
k_mutex_lock(&counter_mutex, K_FOREVER);

uint32_t temp = shared_counter;
k_busy_wait(10);
shared_counter = temp + 1;

k_mutex_unlock(&counter_mutex);
```

**Result:** Counter = 3000 (exactly as expected)

### Mutex Timeout

**Test 3** demonstrates timeout behavior:

```c
int ret = k_mutex_lock(&counter_mutex, K_MSEC(100));
if (ret == -EAGAIN) {
    // Timeout occurred - mutex was not available
}
```

---

## Building and Running

### Prerequisites

- Zephyr RTOS environment set up
- Board supported by Zephyr

### Build

```bash
cd app_mutex
west build -b <your_board> . -p always
```

### Flash

```bash
west flash
```

### View Output

```bash
screen /dev/ttyACM0 115200
```

---

## Expected Output

```
*** Booting Zephyr OS build... ***
[00:00:00.000,000] <inf> app_mutex: Mutex Example Application Started
[00:00:00.001,000] <inf> app_mutex: Main thread: Creating coordinator thread
[00:00:00.002,000] <inf> app_mutex: Main thread: Coordinator created, going to sleep
[00:00:00.003,000] <inf> app_mutex: Coordinator thread started
[00:00:00.003,000] <inf> app_mutex: ========================================
[00:00:00.200,000] <wrn> app_mutex: === TEST 1: WITHOUT MUTEX (Race Condition) ===
[00:00:00.201,000] <inf> app_mutex: Counter thread 1 started
[00:00:00.201,000] <inf> app_mutex: Counter thread 2 started
[00:00:00.201,000] <inf> app_mutex: Counter thread 3 started
[00:00:10.500,000] <inf> app_mutex: Counter thread 1 finished - incremented 1000 times
[00:00:10.500,000] <inf> app_mutex: Counter thread 2 finished - incremented 1000 times
[00:00:10.500,000] <inf> app_mutex: Counter thread 3 finished - incremented 1000 times
[00:00:10.500,000] <wrn> app_mutex: Expected counter value: 3000
[00:00:10.500,000] <wrn> app_mutex: Actual counter value: 2456
[00:00:10.500,000] <err> app_mutex: RACE CONDITION DETECTED! Lost 544 increments

[00:00:11.000,000] <inf> app_mutex: === TEST 2: WITH MUTEX (Protected) ===
[00:00:11.001,000] <inf> app_mutex: Counter thread 1 started
[00:00:11.001,000] <inf> app_mutex: Counter thread 2 started
[00:00:11.001,000] <inf> app_mutex: Counter thread 3 started
[00:00:21.300,000] <inf> app_mutex: Counter thread 1 finished - incremented 1000 times
[00:00:21.300,000] <inf> app_mutex: Counter thread 2 finished - incremented 1000 times
[00:00:21.300,000] <inf> app_mutex: Counter thread 3 finished - incremented 1000 times
[00:00:21.300,000] <inf> app_mutex: Expected counter value: 3000
[00:00:21.300,000] <inf> app_mutex: Actual counter value: 3000
[00:00:21.300,000] <inf> app_mutex: SUCCESS! Mutex protected the shared resource

[00:00:21.800,000] <inf> app_mutex: === TEST 3: Mutex Timeout Demo ===
[00:00:21.801,000] <inf> app_mutex: Coordinator thread: Mutex locked
[00:00:21.901,000] <wrn> app_mutex: Coordinator thread: Timeout - mutex already locked!
[00:00:21.902,000] <inf> app_mutex: Coordinator thread: Mutex unlocked
[00:00:21.903,000] <inf> app_mutex: Coordinator thread: Successfully locked mutex again
[00:00:22.000,000] <inf> app_mutex: ========================================
[00:00:22.000,000] <inf> app_mutex: Coordinator thread: All tests completed
[00:00:22.001,000] <inf> app_mutex: Coordinator thread exiting
```

---

## Thread Architecture

This example uses a **coordinator pattern** where work is delegated to threads:

```
main() thread (priority 0, default)
    │
    ├─> Creates coordinator_thread (priority 4)
    └─> Sleeps forever with k_sleep(K_FOREVER)

coordinator_thread
    │
    ├─> Runs Test 1 (without mutex)
    │   ├─> Creates counter_thread 1 (priority 5)
    │   ├─> Creates counter_thread 2 (priority 5)
    │   └─> Creates counter_thread 3 (priority 5)
    │
    ├─> Runs Test 2 (with mutex)
    │   ├─> Creates counter_thread 1 (priority 5)
    │   ├─> Creates counter_thread 2 (priority 5)
    │   └─> Creates counter_thread 3 (priority 5)
    │
    ├─> Runs Test 3 (timeout demo)
    └─> Exits
```

**Key Points:**
- Main thread has **no application logic** - only spawns coordinator and sleeps
- Coordinator thread orchestrates all tests
- Counter threads are created dynamically for each test
- Priority: Coordinator (4) > Counter threads (5)

This pattern is useful for:
- Complex applications with multiple subsystems
- Separating initialization from runtime logic
- Clean architecture with clear responsibilities

---

## Key Concepts

### 1. Mutex Definition

```c
K_MUTEX_DEFINE(counter_mutex);
```

Statically defines a mutex at compile time.

### 2. Locking a Mutex

```c
k_mutex_lock(&counter_mutex, K_FOREVER);  // Wait forever
k_mutex_lock(&counter_mutex, K_MSEC(100)); // Wait up to 100ms
k_mutex_lock(&counter_mutex, K_NO_WAIT);   // Don't wait at all
```

### 3. Unlocking a Mutex

```c
k_mutex_unlock(&counter_mutex);
```

**Important:** Only the thread that locked the mutex can unlock it!

### 4. Return Values

- `0` - Success
- `-EAGAIN` - Timeout occurred
- `-EBUSY` - Mutex not available (with K_NO_WAIT)

---

## Best Practices

### ✅ DO:
- Always unlock mutexes you've locked
- Keep critical sections as short as possible
- Use timeouts to avoid deadlocks
- Lock mutexes in the same order across threads
- Document which mutex protects which data

### ❌ DON'T:
- Lock mutex from ISR (use semaphores instead)
- Hold mutex while sleeping for long periods
- Forget to unlock (causes deadlock)
- Unlock a mutex you didn't lock
- Use mutex for ISR-to-thread communication

---

## Common Pitfalls

### Deadlock Example

```c
// Thread 1:
k_mutex_lock(&mutex_a, K_FOREVER);
k_mutex_lock(&mutex_b, K_FOREVER);  // Waits for mutex_b

// Thread 2:
k_mutex_lock(&mutex_b, K_FOREVER);
k_mutex_lock(&mutex_a, K_FOREVER);  // Waits for mutex_a
// DEADLOCK! Both threads waiting for each other
```

**Solution:** Always lock mutexes in the same order.

### Priority Inversion

When a low-priority thread holds a mutex needed by a high-priority thread.

**Solution:** Zephyr mutexes support priority inheritance to mitigate this.

---

## Experiment Ideas

1. **Change NUM_INCREMENTS** - Try different values to see how race conditions vary
2. **Add more threads** - See how contention affects performance
3. **Adjust k_busy_wait()** - Longer delays = more race conditions without mutex
4. **Implement priority inheritance** - See how it prevents priority inversion

---

## Related Examples

- **app_thread** - Basic thread creation and management
- **app_semaphore** - Using semaphores for signaling
- **app_msgq** - Message queues for inter-thread communication

---

## References

- [Zephyr Mutex Documentation](https://docs.zephyrproject.org/latest/kernel/services/synchronization/mutexes.html)
- [Zephyr Threading Documentation](https://docs.zephyrproject.org/latest/kernel/services/threads/index.html)
- [Synchronization Primitives Guide](https://docs.zephyrproject.org/latest/kernel/services/synchronization/index.html)
