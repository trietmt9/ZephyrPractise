# Timer Application

This example demonstrates Zephyr kernel timer usage for periodic tasks. The application toggles an LED every 500ms using a timer callback function.

## Features

- **Kernel timer initialization** - Using `k_timer_init()`
- **Periodic callbacks** - Executes every 500ms
- **GPIO control in callback** - LED toggling from timer handler
- **Timer synchronization** - Using `k_timer_status_sync()`

## Hardware Requirements

- **Board**: STM32 Nucleo F446RE (or compatible)
- **LED**: Uses onboard LED on PA5
- **USB Cable**: For flashing and serial output

## Project Files

```
app_timer/
├── CMakeLists.txt          # Build configuration
├── prj.conf                # Enables GPIO and logging
├── boards/
│   └── timer.overlay       # LED GPIO configuration
├── src/
│   └── main.c              # Timer implementation
└── README.md               # This file
```

## How It Works

### Timer Configuration

The application creates a kernel timer that:
1. Initializes with a callback function
2. Starts with 500ms initial delay
3. Never repeats (using `K_NO_WAIT` for period)
4. Toggles LED in the callback

### Devicetree Overlay (`boards/timer.overlay`)

```dts
/ {
    aliases {
        led0 = &greenled;
    };

    leds {
        compatible = "gpio-leds";
        greenled: led_0 {
            gpios = <&gpioa 5 GPIO_ACTIVE_HIGH>;
            label = "Green LED";
        };
    };
};
```

**Key Points:**
- Alias `led0` points to the LED node
- Uses GPIO port A, pin 5
- Active high configuration

### Main Code Logic

```c
// Define timer
struct k_timer my_timer;

// Timer callback function
void led_blink_handler(struct k_timer *timer)
{
    if(timer == &my_timer) {
        gpio_pin_toggle_dt(&led);
        LOG_INF("GPIO toggled");
    }
}

int main()
{
    // Initialize timer with callback
    k_timer_init(&my_timer, led_blink_handler, NULL);

    // Start timer (500ms delay, no repeat)
    k_timer_start(&my_timer, K_MSEC(500), K_NO_WAIT);

    // Wait for timer to expire
    k_timer_status_sync(&my_timer);

    return 0;
}
```

## Building and Flashing

### Prerequisites

Activate the Zephyr environment:

```bash
source ~/zephyrproject/.venv/bin/activate
source ~/zephyrproject/zephyr/zephyr-env.sh
```

### Build

```bash
cd app_timer
west build -b nucleo_f446re . -p always
```

### Flash

```bash
west flash
```

### View Output

```bash
screen /dev/ttyACM0 115200
```

## Expected Output

```
[00:00:00.500,000] <inf> app_timer: GPIO toggled
```

The onboard green LED should toggle once after 500ms.

## Timer API Overview

### Timer Initialization

```c
void k_timer_init(struct k_timer *timer,
                  k_timer_expiry_t expiry_fn,
                  k_timer_stop_t stop_fn)
```

- **timer**: Pointer to timer structure
- **expiry_fn**: Callback when timer expires
- **stop_fn**: Callback when timer stops (can be NULL)

### Starting a Timer

```c
void k_timer_start(struct k_timer *timer,
                   k_timeout_t duration,
                   k_timeout_t period)
```

- **duration**: Initial delay before first expiration
- **period**: Interval for periodic timer (use `K_NO_WAIT` or `K_FOREVER` for one-shot)

**Examples:**
```c
// One-shot timer: 500ms delay, no repeat
k_timer_start(&my_timer, K_MSEC(500), K_NO_WAIT);

// Periodic timer: 100ms delay, repeat every 200ms
k_timer_start(&my_timer, K_MSEC(100), K_MSEC(200));

// Immediate start, repeat every 1 second
k_timer_start(&my_timer, K_NO_WAIT, K_SECONDS(1));
```

### Timer Synchronization

```c
uint32_t k_timer_status_sync(struct k_timer *timer)
```

Waits for the timer to expire and returns the number of times it expired.

## Troubleshooting

### Build Errors

#### "Could not find Zephyr"

Make sure environment is activated:
```bash
source ~/zephyrproject/.venv/bin/activate
source ~/zephyrproject/zephyr/zephyr-env.sh
```

#### Devicetree Errors

Common issues in `timer.overlay`:
- **Missing semicolons**: All nodes must end with `};`
- **Alias names**: Use hyphens not underscores (`led-0` not `led_0` for aliases)
- **Missing ampersand**: Use `<&gpioa 5>` not `<gpioa 5>`
- **Wrong compatible**: Should be `"gpio-leds"` not `"gpios-leds"`

#### Kconfig Error

Make sure `prj.conf` has:
```
CONFIG_GPIO=y
CONFIG_LOG=y
```

### Runtime Issues

#### LED Not Toggling

1. Check that GPIO initialization succeeded
2. Verify LED is connected to PA5
3. Check serial output for error messages
4. Ensure timer callback is being called

#### Timer Not Firing

1. Verify timer initialization: `k_timer_init(&my_timer, handler, NULL)`
2. Check timer start parameters
3. Make sure main() doesn't exit immediately
4. Add logging in callback to verify it's called

#### No Serial Output

- Verify USB cable is connected
- Check device: `ls /dev/ttyACM*`
- Try different baud rate or terminal program

## Modifying the Code

### Change Timer Period

Edit `src/main.c`:

```c
// Faster blinking (200ms)
k_timer_start(&my_timer, K_MSEC(200), K_NO_WAIT);

// Slower blinking (1 second)
k_timer_start(&my_timer, K_SECONDS(1), K_NO_WAIT);
```

### Make Timer Periodic

For continuous blinking:

```c
int main()
{
    // Initialize timer
    k_timer_init(&my_timer, led_blink_handler, NULL);

    // Start periodic timer: immediate start, repeat every 500ms
    k_timer_start(&my_timer, K_NO_WAIT, K_MSEC(500));

    // Keep main running
    while(1) {
        k_sleep(K_FOREVER);
    }

    return 0;
}
```

### Use Multiple Timers

```c
struct k_timer timer1, timer2;

void timer1_handler(struct k_timer *timer)
{
    // Fast blink
    gpio_pin_toggle_dt(&led);
}

void timer2_handler(struct k_timer *timer)
{
    // Print message
    printk("Timer 2 fired\n");
}

int main()
{
    k_timer_init(&timer1, timer1_handler, NULL);
    k_timer_init(&timer2, timer2_handler, NULL);

    k_timer_start(&timer1, K_NO_WAIT, K_MSEC(100));  // 10Hz
    k_timer_start(&timer2, K_NO_WAIT, K_SECONDS(1)); // 1Hz

    while(1) {
        k_sleep(K_FOREVER);
    }
}
```

### Use Different GPIO Pin

Modify `boards/timer.overlay`:

```dts
leds {
    compatible = "gpio-leds";
    myled: led_0 {
        gpios = <&gpiob 0 GPIO_ACTIVE_HIGH>;  // Use PB0 instead
        label = "My LED";
    };
};
```

## Kernel Timer vs Work Queue

**Use kernel timers when:**
- Need precise timing
- Simple callback function
- Running in ISR context is acceptable

**Use work queues when:**
- Need to perform complex operations
- Require sleeping or blocking calls
- Want to defer work from ISR to thread context

Example with work queue:
```c
K_WORK_DEFINE(my_work, work_handler);

void timer_handler(struct k_timer *timer)
{
    k_work_submit(&my_work);  // Schedule work
}

void work_handler(struct k_work *work)
{
    // Complex operations here
    gpio_pin_toggle_dt(&led);
}
```

## API Reference

### Timer Functions

- `k_timer_init()` - Initialize a timer
- `k_timer_start()` - Start a timer
- `k_timer_stop()` - Stop a timer
- `k_timer_status_sync()` - Wait for timer expiration
- `k_timer_status_get()` - Get timer status without waiting

### Timeout Values

- `K_NO_WAIT` - No delay (0)
- `K_FOREVER` - Infinite wait
- `K_MSEC(ms)` - Milliseconds
- `K_SECONDS(s)` - Seconds
- `K_MINUTES(m)` - Minutes
- `K_HOURS(h)` - Hours

## Learn More

- [Zephyr Timer API](https://docs.zephyrproject.org/latest/kernel/services/timing/timers.html)
- [Zephyr Timing Services](https://docs.zephyrproject.org/latest/kernel/services/timing/index.html)
- [Work Queue API](https://docs.zephyrproject.org/latest/kernel/services/threads/workqueue.html)

---

**Back to [Main README](../README.md)**
