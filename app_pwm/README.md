# PWM Application

This example demonstrates PWM (Pulse Width Modulation) signal generation using Zephyr RTOS. The application smoothly ramps LED brightness from 0% to 100% duty cycle and back down.

## Features

- **1kHz PWM frequency** - Fast enough for smooth LED dimming
- **0-100% duty cycle control** - Full range brightness control
- **Smooth ramping** - 100 steps for gradual transitions
- **Device tree configuration** - Proper PWM peripheral setup

## Hardware Requirements

- **Board**: STM32 Nucleo F446RE (or compatible)
- **LED**: Uses onboard LED on PA5 (connected to TIM2_CH1)
- **USB Cable**: For flashing and serial output

## Project Files

```
app_pwm/
├── CMakeLists.txt          # Build configuration
├── prj.conf                # Enables PWM and logging
├── boards/
│   └── pwm.overlay         # PWM2 configuration for PA5
├── src/
│   └── main.c              # PWM duty cycle ramping
└── README.md               # This file
```

## How It Works

### PWM Configuration

The application uses **Timer 2, Channel 1** mapped to **PA5**:

- **Frequency**: 1kHz (1ms period)
- **Period**: `PWM_MSEC(1)` = 1,000,000 nanoseconds
- **Steps**: 100 (each step = 10,000 ns = 1%)

### Devicetree Overlay (`boards/pwm.overlay`)

```dts
/ {
    aliases {
        pwm-led0 = &pwm_led0;
    };

    pwmleds {
        compatible = "pwm-leds";
        pwm_led0: pwm_led_0 {
            pwms = <&pwm2 1 PWM_MSEC(1) PWM_POLARITY_NORMAL>;
        };
    };
};

&timers2 {
    status = "okay";

    pwm2: pwm {
        status = "okay";
        pinctrl-0 = <&tim2_ch1_pa5>;
        pinctrl-names = "default";
    };
};
```

**Key Points:**
- Alias uses hyphen: `pwm-led0` (not underscore)
- `&pwm2` references PWM2 peripheral
- Channel 1 on Timer 2
- Pin PA5 via `tim2_ch1_pa5` pinctrl

### Main Code Logic

```c
// 1kHz PWM (1ms period)
#define PWM_PERIOD_NS  PWM_MSEC(1)
#define NUM_STEPS      100

// Ramp UP: 0% to 100%
for (int i = 0; i <= NUM_STEPS; i++) {
    pulse_width = i * (PWM_PERIOD_NS / NUM_STEPS);
    pwm_set_dt(&pwm_led, PWM_PERIOD_NS, pulse_width);
}

// Ramp DOWN: 100% to 0%
for (int i = NUM_STEPS; i >= 0; i--) {
    pulse_width = i * (PWM_PERIOD_NS / NUM_STEPS);
    pwm_set_dt(&pwm_led, PWM_PERIOD_NS, pulse_width);
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
cd app_pwm
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

Or use minicom:
```bash
minicom -D /dev/ttyACM0 -b 115200
```

## Expected Output

```
PWM Demo: 0% to 100% duty cycle at 1kHz
PWM ready on channel 1

--- Ramping UP ---
Duty:   0% | Pulse:       0 ns
Duty:   1% | Pulse:   10000 ns
Duty:   2% | Pulse:   20000 ns
...
Duty:  99% | Pulse:  990000 ns
Duty: 100% | Pulse: 1000000 ns

--- Ramping DOWN ---
Duty: 100% | Pulse: 1000000 ns
Duty:  99% | Pulse:  990000 ns
...
Duty:   1% | Pulse:   10000 ns
Duty:   0% | Pulse:       0 ns
```

You should see the onboard green LED smoothly fade in and out.

## PWM Frequency and Period Calculations

### Formula

```
Period (T) = 1 / Frequency (f)
```

### Common PWM Frequencies

| Application | Frequency | Period | Zephyr Macro |
|-------------|-----------|--------|--------------|
| Servo control | 50 Hz | 20 ms | `PWM_MSEC(20)` |
| LED dimming | 1 kHz | 1 ms | `PWM_MSEC(1)` |
| Motor control | 10 kHz | 100 μs | `PWM_USEC(100)` |
| Audio PWM | 44.1 kHz | ~22.7 μs | `PWM_USEC(23)` |

### Zephyr PWM Macros

```c
PWM_SEC(1)    // 1,000,000,000 ns = 1 second
PWM_MSEC(20)  //    20,000,000 ns = 20 milliseconds
PWM_USEC(100) //       100,000 ns = 100 microseconds
PWM_NSEC(500) //           500 ns = 500 nanoseconds
```

### Example: 50Hz for Servo Control

```c
#define SERVO_PERIOD  PWM_MSEC(20)     // 20ms = 50Hz

// Servo positions (typical)
#define SERVO_MIN     PWM_USEC(1000)   // 1ms pulse = 0°
#define SERVO_CENTER  PWM_USEC(1500)   // 1.5ms pulse = 90°
#define SERVO_MAX     PWM_USEC(2000)   // 2ms pulse = 180°

pwm_set_dt(&pwm_servo, SERVO_PERIOD, SERVO_CENTER);
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
- **Alias names**: Use hyphens not underscores (`pwm-led0` not `pwm_led0`)
- **Missing ampersand**: Use `<&pwm2 ...>` not `<pwm2 ...>`
- **Missing semicolons**: All nodes must end with `};`

#### Kconfig Error

Make sure `prj.conf` has:
```
CONFIG_PWM=y
CONFIG_LOG=y
```

### Runtime Issues

#### PWM Not Working

1. Check LED is connected to PA5
2. Verify Timer 2 is available on your board
3. Check serial output for error messages

#### No Serial Output

- Verify USB cable is connected
- Check device: `ls /dev/ttyACM*`
- Try different terminal: `screen`, `minicom`, or VS Code Serial Monitor

#### Flash Permission Denied

Add user to dialout group:
```bash
sudo usermod -a -G dialout $USER
```

Log out and back in for changes to take effect.

## Modifying the Code

### Change PWM Frequency

Edit `src/main.c`:
```c
// Change from 1kHz to 10kHz
#define PWM_PERIOD_NS  PWM_USEC(100)  // 100μs = 10kHz
```

Also update `boards/pwm.overlay`:
```dts
pwms = <&pwm2 1 PWM_USEC(100) PWM_POLARITY_NORMAL>;
```

### Change Ramp Speed

Edit `src/main.c`:
```c
// Faster ramping
#define STEP_DELAY_MS  10   // Was 20ms

// Slower ramping
#define STEP_DELAY_MS  50
```

### Use Different Pin

To use a different pin, modify `boards/pwm.overlay`:

1. Choose a pin with PWM capability
2. Update the timer and channel
3. Update pinctrl

Example for PA0 (TIM2_CH1 alternative):
```dts
pinctrl-0 = <&tim2_ch1_pa0>;
```

## API Reference

### PWM Functions Used

- `PWM_DT_SPEC_GET(node_id)` - Get PWM device spec from devicetree
- `pwm_is_ready_dt(spec)` - Check if PWM device is ready
- `pwm_set_dt(spec, period, pulse)` - Set PWM period and pulse width

### Macros

- `DT_ALIAS(alias)` - Get devicetree node from alias
- `PWM_MSEC(ms)` - Convert milliseconds to nanoseconds
- `PWM_USEC(us)` - Convert microseconds to nanoseconds
- `PWM_POLARITY_NORMAL` - Normal polarity (active high)

## Learn More

- [Zephyr PWM API](https://docs.zephyrproject.org/latest/hardware/peripherals/pwm.html)
- [Devicetree Guide](https://docs.zephyrproject.org/latest/build/dts/index.html)
- [STM32 Timer Documentation](https://www.st.com/resource/en/reference_manual/rm0390-stm32f446xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)

---

**Back to [Main README](../README.md)**
