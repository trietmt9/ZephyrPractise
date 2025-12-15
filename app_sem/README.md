# Semaphore Example

This example demonstrates the use of **Zephyr kernel semaphores** for thread synchronization, signaling, and resource management in multi-threaded applications.

## What You'll Learn

- Binary semaphores for thread signaling
- Counting semaphores for resource pool management
- Producer-consumer pattern with semaphores
- Thread coordination and synchronization
- Difference between semaphores and mutexes

---

## What is a Semaphore?

A **semaphore** is a synchronization primitive that maintains a count. Threads can:
- **Take** (`k_sem_take`) - Decrease count (blocks if count is 0)
- **Give** (`k_sem_give`) - Increase count (wakes waiting threads)

### Types of Semaphores

| Type | Initial Count | Max Count | Use Case |
|------|---------------|-----------|----------|
| **Binary** | 0 or 1 | 1 | Signaling between threads |
| **Counting** | N | N | Resource pool management |

### Semaphore vs Mutex

| Feature | Semaphore | Mutex |
|---------|-----------|-------|
| **Purpose** | Signaling, counting | Mutual exclusion |
| **Ownership** | No owner | Owner thread only |
| **Give/Take** | Any thread | Only lock owner can unlock |
| **Count** | 0 to N | 0 or 1 (locked/unlocked) |
| **Use from ISR** | ✅ Yes (give only) | ❌ No |

---

## Architecture

This example uses a **coordinator pattern** where work is delegated to threads:

```
main() thread (priority 0, default)
    │
    ├─> Creates coordinator_thread (priority 4)
    └─> Sleeps forever with k_sleep(K_FOREVER)

coordinator_thread
    │
    ├─> Test 1: Binary Semaphore
    │   ├─> receiver_thread (waits for signal)
    │   └─> sender_thread (sends signal)
    │
    ├─> Test 2: Counting Semaphore
    │   ├─> worker1_thread (uses resource)
    │   ├─> worker2_thread (uses resource)
    │   └─> worker3_thread (uses resource)
    │
    └─> Test 3: Producer-Consumer
        ├─> producer_thread (adds items)
        └─> consumer_thread (removes items)
```

---

## How This Example Works

### Test 1: Binary Semaphore (Signaling)

**Purpose:** Demonstrate thread-to-thread signaling

```c
K_SEM_DEFINE(binary_sem, 0, 1);  // Start at 0, max 1

// Receiver thread
k_sem_take(&binary_sem, K_FOREVER);  // Blocks until signaled
LOG_INF("Signal received!");

// Sender thread (runs later)
k_sem_give(&binary_sem);  // Wakes up receiver
```

**Flow:**
1. Receiver starts first, calls `k_sem_take()` → blocks (count is 0)
2. Sender waits 2 seconds
3. Sender calls `k_sem_give()` → count becomes 1, receiver wakes up
4. Receiver proceeds with work

**Use Cases:**
- Waiting for initialization to complete
- Signaling event occurrence
- Simple producer-consumer with 1 item

---

### Test 2: Counting Semaphore (Resource Pool)

**Purpose:** Manage limited resources (3 resources available)

```c
K_SEM_DEFINE(resource_sem, 3, 3);  // Start with 3, max 3

void worker_thread(void *id) {
    k_sem_take(&resource_sem, K_FOREVER);  // Acquire resource
    // Use resource for 2 seconds
    k_msleep(2000);
    k_sem_give(&resource_sem);  // Release resource
}
```

**How It Works:**
- Semaphore starts with count = 3 (3 resources available)
- Each `k_sem_take()` decrements count
- When count reaches 0, next thread blocks
- Each `k_sem_give()` increments count, waking blocked threads

**Scenario:**
```
Time    Count   Event
----    -----   -----
0ms     3       Worker1 takes → count=2
0ms     2       Worker2 takes → count=1
0ms     1       Worker3 takes → count=0
2000ms  1       Worker1 gives → count=1
2000ms  2       Worker2 gives → count=2
2000ms  3       Worker3 gives → count=3
```

**Use Cases:**
- Connection pool management
- Buffer management
- Hardware resource allocation

---

### Test 3: Producer-Consumer Pattern

**Purpose:** Coordinate producers and consumers with bounded buffer

**Components:**
```c
#define BUFFER_SIZE 5

K_SEM_DEFINE(empty_slots, 5, 5);   // Track empty buffer slots
K_SEM_DEFINE(filled_slots, 0, 5);  // Track filled buffer slots
K_MUTEX_DEFINE(buffer_mutex);      // Protect buffer access
```

**Producer Logic:**
```c
void producer_thread(void) {
    for (int i = 0; i < 10; i++) {
        k_sem_take(&empty_slots, K_FOREVER);    // Wait for space

        k_mutex_lock(&buffer_mutex, K_FOREVER);
        buffer[write_idx] = i;                   // Add item
        write_idx = (write_idx + 1) % BUFFER_SIZE;
        k_mutex_unlock(&buffer_mutex);

        k_sem_give(&filled_slots);               // Signal item ready
    }
}
```

**Consumer Logic:**
```c
void consumer_thread(void) {
    for (int i = 0; i < 10; i++) {
        k_sem_take(&filled_slots, K_FOREVER);    // Wait for item

        k_mutex_lock(&buffer_mutex, K_FOREVER);
        int item = buffer[read_idx];             // Remove item
        read_idx = (read_idx + 1) % BUFFER_SIZE;
        k_mutex_unlock(&buffer_mutex);

        k_sem_give(&empty_slots);                // Signal space available
    }
}
```

**Why Both Semaphores AND Mutex?**
- **Semaphores** - Track slots (empty/filled)
- **Mutex** - Protect buffer structure from concurrent access

**Flow Example:**
```
Buffer: [_][_][_][_][_]  empty=5, filled=0

Producer adds item 0:
  take empty_slots (4 empty)
  lock mutex
  buffer[0] = 0
  unlock mutex
  give filled_slots (1 filled)

Buffer: [0][_][_][_][_]  empty=4, filled=1

Consumer takes item 0:
  take filled_slots (0 filled)
  lock mutex
  item = buffer[0]
  unlock mutex
  give empty_slots (5 empty)

Buffer: [_][_][_][_][_]  empty=5, filled=0
```

---

## Building and Running

### Prerequisites

- Zephyr RTOS environment set up
- Board supported by Zephyr

### Build

```bash
cd app_sem
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
[00:00:00.000,000] <inf> app_sem: Semaphore Example Application Started
[00:00:00.001,000] <inf> app_sem: Main thread: Creating coordinator thread
[00:00:00.002,000] <inf> app_sem: Main thread: Coordinator created, going to sleep
[00:00:00.003,000] <inf> app_sem: Coordinator thread started
[00:00:00.003,000] <inf> app_sem: ========================================

[00:00:00.100,000] <wrn> app_sem: === TEST 1: Binary Semaphore (Signaling) ===
[00:00:00.101,000] <inf> app_sem: Receiver: Waiting for signal...
[00:00:00.201,000] <inf> app_sem: Sender: Will send signal in 2 seconds...
[00:00:02.201,000] <inf> app_sem: Sender: Sending signal now!
[00:00:02.201,000] <inf> app_sem: Sender: Signal sent
[00:00:02.202,000] <inf> app_sem: Receiver: Signal received! Proceeding with work
[00:00:02.302,000] <inf> app_sem: Receiver: Work completed
[00:00:02.303,000] <inf> app_sem: Binary semaphore test completed

[00:00:02.800,000] <wrn> app_sem: === TEST 2: Counting Semaphore (Resource Pool) ===
[00:00:02.801,000] <inf> app_sem: Resource pool has 3 resources available
[00:00:02.802,000] <inf> app_sem: Starting 3 workers (should all get resources)...
[00:00:02.803,000] <inf> app_sem: Worker 1: Requesting resource...
[00:00:02.803,000] <inf> app_sem: Worker 1: Got resource! Working for 2 seconds...
[00:00:02.804,000] <inf> app_sem: Worker 2: Requesting resource...
[00:00:02.804,000] <inf> app_sem: Worker 2: Got resource! Working for 2 seconds...
[00:00:02.805,000] <inf> app_sem: Worker 3: Requesting resource...
[00:00:02.805,000] <inf> app_sem: Worker 3: Got resource! Working for 2 seconds...
[00:00:04.805,000] <inf> app_sem: Worker 1: Releasing resource
[00:00:04.806,000] <inf> app_sem: Worker 2: Releasing resource
[00:00:04.807,000] <inf> app_sem: Worker 3: Releasing resource
[00:00:04.808,000] <inf> app_sem: Counting semaphore test completed

[00:00:05.300,000] <wrn> app_sem: === TEST 3: Producer-Consumer Pattern ===
[00:00:05.301,000] <inf> app_sem: Buffer size: 5 items
[00:00:05.302,000] <inf> app_sem: Items to produce: 10
[00:00:05.303,000] <inf> app_sem: Producer: Added item 0 at index 0
[00:00:05.304,000] <inf> app_sem: Consumer: Consumed item 0 from index 0
[00:00:05.604,000] <inf> app_sem: Producer: Added item 1 at index 1
[00:00:05.804,000] <inf> app_sem: Consumer: Consumed item 1 from index 1
[00:00:05.904,000] <inf> app_sem: Producer: Added item 2 at index 2
...
[00:00:09.500,000] <inf> app_sem: Producer: Finished producing all items
[00:00:10.000,000] <inf> app_sem: Consumer: Finished consuming all items
[00:00:10.001,000] <inf> app_sem: Producer-Consumer test completed

[00:00:10.002,000] <inf> app_sem: ========================================
[00:00:10.003,000] <inf> app_sem: Coordinator thread: All tests completed
[00:00:10.004,000] <inf> app_sem: Coordinator thread exiting
```

---

## Key Concepts

### 1. Semaphore Definition

```c
K_SEM_DEFINE(my_sem, initial_count, limit);
```

### 2. Taking a Semaphore (Decrement)

```c
k_sem_take(&my_sem, K_FOREVER);    // Wait forever
k_sem_take(&my_sem, K_MSEC(100));  // Wait up to 100ms
k_sem_take(&my_sem, K_NO_WAIT);    // Don't wait at all
```

**Return Values:**
- `0` - Success
- `-EAGAIN` - Timeout
- `-EBUSY` - Not available (K_NO_WAIT)

### 3. Giving a Semaphore (Increment)

```c
k_sem_give(&my_sem);
```

Can be called from **any thread** or even **ISR** (unlike mutexes!)

### 4. Reset Semaphore

```c
k_sem_reset(&my_sem);  // Set count to 0
```

### 5. Query Count

```c
unsigned int count = k_sem_count_get(&my_sem);
```

---

## Best Practices

### ✅ DO:
- Use binary semaphores for signaling
- Use counting semaphores for resource pools
- Combine semaphores with mutexes in producer-consumer
- Give semaphores from ISRs (for signaling)
- Check return values for timeouts

### ❌ DON'T:
- Use semaphore for mutual exclusion (use mutex instead)
- Assume ownership semantics (any thread can give)
- Forget to initialize with correct count
- Mix up empty/filled semaphore logic in producer-consumer

---

## Common Patterns

### Pattern 1: Wait for Event
```c
K_SEM_DEFINE(event_sem, 0, 1);

// Thread 1: Wait for event
k_sem_take(&event_sem, K_FOREVER);

// ISR or Thread 2: Signal event
k_sem_give(&event_sem);
```

### Pattern 2: Resource Pool
```c
K_SEM_DEFINE(resources, 5, 5);  // 5 resources

// Acquire
k_sem_take(&resources, K_FOREVER);
use_resource();
k_sem_give(&resources);  // Release
```

### Pattern 3: Bounded Buffer
```c
K_SEM_DEFINE(empty, N, N);
K_SEM_DEFINE(full, 0, N);
K_MUTEX_DEFINE(lock);

// Producer
k_sem_take(&empty, K_FOREVER);
k_mutex_lock(&lock, K_FOREVER);
add_item();
k_mutex_unlock(&lock);
k_sem_give(&full);

// Consumer
k_sem_take(&full, K_FOREVER);
k_mutex_lock(&lock, K_FOREVER);
remove_item();
k_mutex_unlock(&lock);
k_sem_give(&empty);
```

---

## When to Use Semaphore vs Mutex

| Scenario | Use |
|----------|-----|
| Signaling between threads | **Semaphore** (binary) |
| Protecting shared data | **Mutex** |
| Resource counting | **Semaphore** (counting) |
| Producer-consumer | **Both** (semaphores for slots, mutex for buffer) |
| ISR signaling thread | **Semaphore** (can give from ISR) |

---

## Experiment Ideas

1. **Change buffer size** - Try BUFFER_SIZE=1 (no buffering) or BUFFER_SIZE=10
2. **Add more producers/consumers** - See how it scales
3. **Adjust timing** - Make producer faster/slower than consumer
4. **Overflow test** - What happens if producer never stops?
5. **Resource starvation** - Create more workers than resources

---

## Related Examples

- **app_mutex** - Mutex for mutual exclusion
- **app_thread** - Basic thread creation
- **app_msgq** - Message queues for data passing

---

## References

- [Zephyr Semaphore Documentation](https://docs.zephyrproject.org/latest/kernel/services/synchronization/semaphores.html)
- [Producer-Consumer Problem](https://en.wikipedia.org/wiki/Producer%E2%80%93consumer_problem)
- [Synchronization Primitives Guide](https://docs.zephyrproject.org/latest/kernel/services/synchronization/index.html)
