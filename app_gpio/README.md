# GPIO Application

This example demonstrates GPIO (General Purpose Input/Output) control using Zephyr RTOS. The application toggles an LED on/off using a button with debouncing logic.

## Features

- **GPIO Output** - Control LED with digital output
- **GPIO Input** - Read button state with pull-up resistor
- **Button Debouncing** - Software debounce algorithm
- **Mode Switching** - Toggle between LED states
- **Logging** - Status messages for debugging

## Hardware Requirements

- **Board**: STM32 Nucleo F446RE (or compatible)
- **LED**: Uses onboard LED on PA5
- **Button**: Uses onboard button on PC13
- **USB Cable**: For flashing and serial output

## Project Files

```
app_gpio/
├── CMakeLists.txt          # Build configuration
├── prj.conf                # Enables GPIO and logging
├── boards/
│   └── gpio.overlay        # LED and button configuration
├── src/
│   └── main.c              # GPIO control logic
└── README.md               # This file
```

## How It Works

### Mode System

The application has two modes that cycle when you press the button:

- **Mode 0**: LED OFF
- **Mode 1**: LED ON

Pressing the button cycles through the modes: 0 → 1 → 0 → 1 ...

### Devicetree Overlay (`boards/gpio.overlay`)

```dts
/{
    aliases {
        myled = &myled;
        mybtn = &mybtn;
    };

    gpio_leds {
        compatible = "gpio-leds";
        myled: myled {
            gpios = <&gpioa 5 GPIO_ACTIVE_HIGH>;
        };
    };

    gpio_btn {
        compatible = "gpio-keys";
        mybtn: mybtn {
            gpios = <&gpioc 13 GPIO_ACTIVE_LOW>;
        };
    };
};
```

**Key Points:**
- **LED on PA5**: Active high (1 = LED on)
- **Button on PC13**: Active low (0 = button pressed)
- Uses standard `gpio-leds` and `gpio-keys` bindings

### Debouncing Algorithm

The code implements software debouncing:

```c
btn_state = gpio_pin_get_dt(&mybtn0);
k_msleep(20);  // Wait 20ms

if(btn_state != prev_btn_state) {
    k_msleep(20);  // Wait another 20ms
    btn_state = gpio_pin_get_dt(&mybtn0);

    if(btn_state != prev_btn_state) {
        mode++;  // State changed, update mode
    }
}
```

This prevents false triggers from mechanical bounce.

## Building and Flashing

### Prerequisites

Activate the Zephyr environment:

```bash
source ~/zephyrproject/.venv/bin/activate
source ~/zephyrproject/zephyr/zephyr-env.sh
```

### Build

```bash
cd app_gpio
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

Press the blue button to cycle between modes:

```
[00:00:01.234] <inf> app_gpio: mode has changed to 1
[00:00:01.234] <inf> app_gpio: MODE 1: LED is on

[00:00:03.456] <inf> app_gpio: mode has changed to 2
[00:00:03.456] <inf> app_gpio: Mode has reset to 0
[00:00:03.456] <inf> app_gpio: MODE 0: LED is off

[00:00:05.678] <inf> app_gpio: mode has changed to 1
[00:00:05.678] <inf> app_gpio: MODE 1: LED is on
```

The green LED on PA5 will turn on/off with each button press.

## GPIO API Overview

### Configuration

```c
// Configure as output (initially inactive/low)
gpio_pin_configure_dt(&gpio_spec, GPIO_OUTPUT_INACTIVE);

// Configure as input with pull-up
gpio_pin_configure_dt(&gpio_spec, GPIO_INPUT | GPIO_PULL_UP);

// Configure as input with pull-down
gpio_pin_configure_dt(&gpio_spec, GPIO_INPUT | GPIO_PULL_DOWN);
```

### Reading and Writing

```c
// Read pin state (returns 0 or 1)
int state = gpio_pin_get_dt(&gpio_spec);

// Set pin high
gpio_pin_set_dt(&gpio_spec, 1);

// Set pin low
gpio_pin_set_dt(&gpio_spec, 0);

// Toggle pin
gpio_pin_toggle_dt(&gpio_spec);
```

### Device Tree Spec

```c
// Get GPIO spec from devicetree
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

// Check if device is ready
if (!gpio_is_ready_dt(&led)) {
    printk("GPIO device not ready\n");
    return -1;
}
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

Common issues in `gpio.overlay`:
- **Missing semicolons**: All nodes must end with `};`
- **Missing ampersand**: Use `<&gpioa 5>` not `<gpioa 5>`
- **Wrong compatible**: Use `"gpio-leds"` and `"gpio-keys"`

#### Kconfig Error

Make sure `prj.conf` has:
```
CONFIG_GPIO=y
CONFIG_LOG=y
```

### Runtime Issues

#### LED Not Responding

1. Check that GPIO configuration succeeded (check logs)
2. Verify LED is connected to PA5
3. Check button is connected to PC13
4. Ensure proper power supply

#### Button Not Detecting Presses

1. Verify button is on PC13
2. Check pull-up configuration
3. Try adjusting debounce delay
4. Add logging to see button state:
   ```c
   printk("Button state: %d\n", btn_state);
   ```

#### Debouncing Issues

If button triggers multiple times:
- Increase debounce delay (from 20ms to 50ms)
- Add more delay between reads
- Consider using hardware interrupts instead

## Modifying the Code

### Add More Modes

```c
switch(mode)
{
    case 0:
        gpio_pin_set_dt(&myled0, 0);
        LOG_INF("MODE 0: LED off");
        break;
    case 1:
        gpio_pin_set_dt(&myled0, 1);
        LOG_INF("MODE 1: LED on");
        break;
    case 2:
        // Blinking mode
        gpio_pin_toggle_dt(&myled0);
        k_msleep(500);
        LOG_INF("MODE 2: LED blinking");
        break;
}

if(mode > 2) {
    mode = 0;
}
```

### Use Different Pins

Modify `boards/gpio.overlay`:

```dts
gpio_leds {
    compatible = "gpio-leds";
    myled: myled {
        gpios = <&gpiob 0 GPIO_ACTIVE_HIGH>;  // Use PB0
    };
};

gpio_btn {
    compatible = "gpio-keys";
    mybtn: mybtn {
        gpios = <&gpioa 0 GPIO_ACTIVE_LOW>;  // Use PA0
    };
};
```

### Use GPIO Interrupts

For better responsiveness, use interrupts instead of polling:

```c
static struct gpio_callback button_cb_data;

void button_pressed(const struct device *dev,
                    struct gpio_callback *cb,
                    uint32_t pins)
{
    mode++;
    if(mode > 1) mode = 0;

    gpio_pin_set_dt(&myled0, mode);
    LOG_INF("Mode changed to %d", mode);
}

int main()
{
    // Configure GPIO
    gpio_pin_configure_dt(&mybtn0, GPIO_INPUT | GPIO_PULL_UP);
    gpio_pin_configure_dt(&myled0, GPIO_OUTPUT_INACTIVE);

    // Configure interrupt
    gpio_pin_interrupt_configure_dt(&mybtn0,
                                    GPIO_INT_EDGE_FALLING);

    // Setup callback
    gpio_init_callback(&button_cb_data,
                      button_pressed,
                      BIT(mybtn0.pin));

    gpio_add_callback(mybtn0.port, &button_cb_data);

    while(1) {
        k_sleep(K_FOREVER);
    }
}
```

### Control Multiple LEDs

Add more LEDs to `gpio.overlay`:

```dts
gpio_leds {
    compatible = "gpio-leds";

    led0: led_0 {
        gpios = <&gpioa 5 GPIO_ACTIVE_HIGH>;
    };

    led1: led_1 {
        gpios = <&gpiob 0 GPIO_ACTIVE_HIGH>;
    };

    led2: led_2 {
        gpios = <&gpioc 7 GPIO_ACTIVE_HIGH>;
    };
};
```

## Common GPIO Flags

### Input Configuration

- `GPIO_INPUT` - Configure as input
- `GPIO_PULL_UP` - Enable internal pull-up resistor
- `GPIO_PULL_DOWN` - Enable internal pull-down resistor

### Output Configuration

- `GPIO_OUTPUT` - Configure as output
- `GPIO_OUTPUT_INIT_LOW` - Output starting low
- `GPIO_OUTPUT_INIT_HIGH` - Output starting high
- `GPIO_OUTPUT_INACTIVE` - Output starting in inactive state
- `GPIO_OUTPUT_ACTIVE` - Output starting in active state

### Interrupt Configuration

- `GPIO_INT_EDGE_RISING` - Trigger on rising edge
- `GPIO_INT_EDGE_FALLING` - Trigger on falling edge
- `GPIO_INT_EDGE_BOTH` - Trigger on both edges
- `GPIO_INT_LEVEL_LOW` - Trigger while low
- `GPIO_INT_LEVEL_HIGH` - Trigger while high

## API Reference

### GPIO Functions

- `GPIO_DT_SPEC_GET(node, prop)` - Get GPIO from devicetree
- `gpio_is_ready_dt(spec)` - Check if GPIO device is ready
- `gpio_pin_configure_dt(spec, flags)` - Configure GPIO pin
- `gpio_pin_get_dt(spec)` - Read GPIO pin state
- `gpio_pin_set_dt(spec, value)` - Write GPIO pin state
- `gpio_pin_toggle_dt(spec)` - Toggle GPIO pin
- `gpio_pin_interrupt_configure_dt(spec, flags)` - Configure interrupt

## Learn More

- [Zephyr GPIO API](https://docs.zephyrproject.org/latest/hardware/peripherals/gpio.html)
- [GPIO Devicetree Bindings](https://docs.zephyrproject.org/latest/build/dts/api/bindings/gpio/gpio-leds.html)
- [STM32 GPIO Documentation](https://www.st.com/resource/en/reference_manual/rm0390-stm32f446xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)

---

**Back to [Main README](../README.md)**
