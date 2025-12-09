# External Interrupt (EXTI) Application

This example demonstrates GPIO external interrupts in Zephyr RTOS. The application uses hardware interrupts to detect button presses and cycles through different LED modes without polling.

## Features

- **GPIO Hardware Interrupts** - Edge-triggered interrupt on button press
- **Interrupt Service Routine (ISR)** - Fast button press detection
- **Multiple LED Modes** - OFF, ON, Blink 1Hz, Blink 2Hz
- **No Polling Required** - Efficient interrupt-driven design
- **Callback System** - Structured interrupt handling

## Hardware Requirements

- **Board**: STM32 Nucleo F446RE (or compatible)
- **LED**: Uses onboard LED on PA5
- **Button**: Uses onboard button on PC13
- **USB Cable**: For flashing and serial output

## Project Files

```
app_exti/
├── CMakeLists.txt          # Build configuration
├── prj.conf                # Enables GPIO and logging
├── boards/
│   └── exti.overlay        # GPIO and interrupt configuration
├── src/
│   └── main.c              # Interrupt handling logic
└── README.md               # This file
```

## How It Works

### Mode System

The application has 4 modes that cycle when you press the button:

- **Mode 0**: LED OFF
- **Mode 1**: LED ON
- **Mode 2**: LED Blinking at 1Hz (500ms period)
- **Mode 3**: LED Blinking at 2Hz (250ms period)

Button press: 0 → 1 → 2 → 3 → 0 → 1 ...

### Devicetree Overlay (`boards/exti.overlay`)

```dts
/ {
    buttons {
        compatible = "gpio-keys";
        my_button: button_0 {
            gpios = <&gpioc 13 (GPIO_PULL_DOWN | GPIO_ACTIVE_HIGH)>;
        };
    };

    leds {
        compatible = "gpio-leds";
        my_led: led_0 {
            gpios = <&gpioa 5 GPIO_PUSH_PULL>;
        };
    };

    aliases {
        sw0 = &my_button;
        led0 = &my_led;
    };
};
```

**Key Points:**
- Button on PC13 with pull-down resistor
- LED on PA5 in push-pull mode
- Aliases allow easy reference in code

### Interrupt Flow

```
Button Press (Hardware)
    ↓
GPIO Interrupt Triggered
    ↓
ISR: btn_pressed() callback
    ↓
Mode variable updated
    ↓
Main loop detects change
    ↓
LED state updated
```

### ISR (Interrupt Service Routine)

```c
void btn_pressed(const struct device* dev,
                 struct gpio_callback* cb,
                 uint32_t pins)
{
    mode++;
    if(mode > 3) {
        mode = 0;
    }
    LOG_DBG("Button pressed! Mode: %d", mode);
}
```

**Important**: Keep ISR fast! Only update variables, don't do heavy processing.

### Main Loop

```c
while(1)
{
    if(mode != prev_mode) {
        // Mode changed - update LED
        switch(mode) {
            case 0: gpio_pin_set_dt(&led, 0); break;  // OFF
            case 1: gpio_pin_set_dt(&led, 1); break;  // ON
            case 2: /* Blink 1Hz */ break;
            case 3: /* Blink 2Hz */ break;
        }
    }

    // Handle blinking modes
    if(mode == 2) {
        gpio_pin_toggle_dt(&led);
        k_msleep(500);  // 1Hz
    } else if(mode == 3) {
        gpio_pin_toggle_dt(&led);
        k_msleep(250);  // 2Hz
    }
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
cd app_exti
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
[00:00:00.001] <inf> app_exti: ============= EXTERNAL INTERRUPTS EXAMPLE =============
[00:00:00.002] <inf> app_exti: =================== Setup interrupt complete ===================
[00:00:00.003] <inf> app_exti: =================== Press the button to change LED states ===================

[00:00:05.123] <dbg> app_exti: Button pressed! Mode: 1
[00:00:05.124] <inf> app_exti: Mode changed to: 1
[00:00:05.124] <inf> app_exti: LED ON

[00:00:07.456] <dbg> app_exti: Button pressed! Mode: 2
[00:00:07.457] <inf> app_exti: Mode changed to: 2
[00:00:07.457] <inf> app_exti: LED BLINKING - 1Hz

[00:00:10.789] <dbg> app_exti: Button pressed! Mode: 3
[00:00:10.790] <inf> app_exti: Mode changed to: 3
[00:00:10.790] <inf> app_exti: LED BLINKING - 2Hz

[00:00:12.345] <dbg> app_exti: Button pressed! Mode: 0
[00:00:12.346] <inf> app_exti: Mode changed to: 0
[00:00:12.346] <inf> app_exti: LED OFF
```

## GPIO Interrupt API

### Interrupt Types

```c
// Edge-triggered interrupts
GPIO_INT_EDGE_RISING      // Trigger on low-to-high transition
GPIO_INT_EDGE_FALLING     // Trigger on high-to-low transition
GPIO_INT_EDGE_BOTH        // Trigger on any edge

// Level-triggered interrupts
GPIO_INT_LEVEL_LOW        // Trigger while pin is low
GPIO_INT_LEVEL_HIGH       // Trigger while pin is high
```

### Setup Steps

**1. Configure Pin as Input:**
```c
gpio_pin_configure_dt(&btn, GPIO_INPUT);
```

**2. Configure Interrupt:**
```c
gpio_pin_interrupt_configure(btn.port, btn.pin, GPIO_INT_EDGE_RISING);
```

**3. Initialize Callback:**
```c
gpio_init_callback(&btn_callback_data, btn_pressed, BIT(btn.pin));
```

**4. Register Callback:**
```c
gpio_add_callback(btn.port, &btn_callback_data);
```

## Troubleshooting

### Build Errors

#### "Could not find Zephyr"

Make sure environment is activated:
```bash
source ~/zephyrproject/.venv/bin/activate
source ~/zephyrproject/zephyr/zephyr-env.sh
```

#### Devicetree Errors

Common issues:
- Missing semicolons
- Wrong GPIO flags
- Incorrect pin numbers

#### Kconfig Error

Make sure `prj.conf` has:
```
CONFIG_GPIO=y
CONFIG_LOG=y
```

### Runtime Issues

#### Interrupt Not Firing

1. **Check interrupt configuration:**
   ```c
   // For active-high button (pressed = high)
   GPIO_INT_EDGE_RISING

   // For active-low button (pressed = low)
   GPIO_INT_EDGE_FALLING
   ```

2. **Verify button connection:**
   - Check button is on PC13
   - Verify pull-down/pull-up configuration
   - Test with multimeter or oscilloscope

3. **Add debug logging:**
   ```c
   void btn_pressed(...) {
       printk("ISR triggered!\n");  // Quick debug
   }
   ```

#### Multiple Triggers Per Press

This is caused by button bounce. Solutions:

**Software Debouncing:**
```c
static uint32_t last_press_time = 0;

void btn_pressed(...) {
    uint32_t now = k_uptime_get_32();

    // Ignore if pressed within last 200ms
    if((now - last_press_time) < 200) {
        return;
    }

    last_press_time = now;
    mode++;
}
```

**Hardware Debouncing:**
- Add 0.1µF capacitor across button
- Use RC filter circuit

#### ISR Crashes or Hangs

**Problem**: Doing too much work in ISR

**Solution**: Defer work to main thread

```c
K_SEM_DEFINE(button_sem, 0, 1);

void btn_pressed(...) {
    k_sem_give(&button_sem);  // Just signal
}

int main() {
    while(1) {
        k_sem_take(&button_sem, K_FOREVER);
        // Do heavy work here
        process_button_press();
    }
}
```

#### No Debug Messages

Enable debug level in `prj.conf`:
```
CONFIG_LOG_MAX_LEVEL=4
```

And register with max level:
```c
LOG_MODULE_REGISTER(app_exti, CONFIG_LOG_MAX_LEVEL);
```

## Modifying the Code

### Add More Modes

```c
volatile uint8_t mode = 0;

void btn_pressed(...) {
    mode++;
    if(mode > 5) mode = 0;  // 6 modes now
}

// In main loop
switch(mode) {
    case 0: /* LED OFF */
    case 1: /* LED ON */
    case 2: /* Blink 1Hz */
    case 3: /* Blink 2Hz */
    case 4: /* Blink 5Hz */
        gpio_pin_toggle_dt(&led);
        k_msleep(100);
        break;
    case 5: /* PWM fade effect */
        // Implement PWM fading
        break;
}
```

### Use Both Button Edges

```c
// Trigger on both press and release
gpio_pin_interrupt_configure(btn.port, btn.pin, GPIO_INT_EDGE_BOTH);

void btn_pressed(...) {
    int state = gpio_pin_get(dev, pins);

    if(state) {
        LOG_INF("Button pressed");
    } else {
        LOG_INF("Button released");
    }
}
```

### Multiple Buttons

Add to `exti.overlay`:
```dts
buttons {
    compatible = "gpio-keys";

    button0: button_0 {
        gpios = <&gpioc 13 GPIO_PULL_DOWN>;
    };

    button1: button_1 {
        gpios = <&gpioa 0 GPIO_PULL_UP>;
    };
};

aliases {
    sw0 = &button0;
    sw1 = &button1;
};
```

In code:
```c
static const struct gpio_dt_spec btn0 = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const struct gpio_dt_spec btn1 = GPIO_DT_SPEC_GET(DT_ALIAS(sw1), gpios);

static struct gpio_callback btn0_cb_data;
static struct gpio_callback btn1_cb_data;

void btn0_pressed(...) { /* Handler for button 0 */ }
void btn1_pressed(...) { /* Handler for button 1 */ }

// Setup both interrupts
gpio_init_callback(&btn0_cb_data, btn0_pressed, BIT(btn0.pin));
gpio_init_callback(&btn1_cb_data, btn1_pressed, BIT(btn1.pin));
```

### Use Work Queue for Deferred Work

```c
#include <zephyr/kernel.h>

static void button_work_handler(struct k_work *work);
K_WORK_DEFINE(button_work, button_work_handler);

void btn_pressed(...) {
    // Just schedule work, don't do processing in ISR
    k_work_submit(&button_work);
}

static void button_work_handler(struct k_work *work) {
    // Heavy processing here (safe to sleep, log, etc.)
    mode++;
    if(mode > 3) mode = 0;

    LOG_INF("Button processed in work queue");
    update_led_state(mode);
}
```

## Best Practices for ISRs

### ✅ DO in ISR:

- Read/write simple variables
- Signal semaphores/events
- Submit work to work queues
- Quick hardware register access

### ❌ DON'T in ISR:

- Sleep or block (`k_sleep()`, `k_sem_take()`)
- Heavy computation
- Excessive logging
- Long operations
- Call blocking APIs

## Interrupt vs Polling Comparison

| Aspect | Interrupt (EXTI) | Polling |
|--------|------------------|---------|
| CPU Usage | Low (idle when no events) | High (constantly checking) |
| Response Time | Fast (immediate) | Slow (depends on poll rate) |
| Power Consumption | Low | High |
| Complexity | Higher | Lower |
| Debouncing | Needs extra care | Built-in with delays |

## Learn More

- [Zephyr GPIO API](https://docs.zephyrproject.org/latest/hardware/peripherals/gpio.html)
- [Interrupts in Zephyr](https://docs.zephyrproject.org/latest/kernel/services/interrupts.html)
- [Work Queue API](https://docs.zephyrproject.org/latest/kernel/services/threads/workqueue.html)
- [STM32 EXTI Documentation](https://www.st.com/resource/en/reference_manual/rm0390-stm32f446xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)

---

**Back to [Main README](../README.md)**
