# Hello World Application

This is the simplest Zephyr RTOS application demonstrating basic logging functionality and output methods.

## Features

- **printk() output** - Kernel print function for basic output
- **Logging levels** - Demonstrates INFO, DEBUG, WARNING, and ERROR levels
- **Standalone application** - Minimal working example
- **Quick build verification** - Perfect for testing your setup

## Hardware Requirements

- **Board**: STM32 Nucleo F446RE (or any Zephyr-supported board)
- **USB Cable**: For flashing and serial output

## Project Files

```
app_helloworld/
├── CMakeLists.txt          # Build configuration
├── prj.conf                # Enables logging
├── src/
│   └── main.c              # Application entry point
└── README.md               # This file
```

No devicetree overlay is needed for this simple example.

## How It Works

### Main Code

```c
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app_helloworld, CONFIG_LOG_MAX_LEVEL);

int main(void)
{
    printk("Hello World sent using printk()\n");
    LOG_INF("Helloworld sent using Log INFO level");
    LOG_DBG("Helloworld sent using Log DEBUG level");
    LOG_WRN("Helloworld sent using Log WARNING level");
    LOG_ERR("Helloworld sent using Log ERROR level");
    return 0;
}
```

### Output Methods

**1. printk()** - Simple kernel print function
- No logging infrastructure needed
- Minimal overhead
- Always printed regardless of log level

**2. LOG_xxx()** - Structured logging with levels
- Filterable by severity
- Includes timestamps
- Can be disabled at compile time

## Building and Flashing

### Prerequisites

Activate the Zephyr environment:

```bash
source ~/zephyrproject/.venv/bin/activate
source ~/zephyrproject/zephyr/zephyr-env.sh
```

### Build

```bash
cd app_helloworld
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
*** Booting Zephyr OS build v3.x.x ***
Hello World sent using printk()
[00:00:00.001,000] <inf> app_helloworld: Helloworld sent using Log INFO level
[00:00:00.001,000] <dbg> app_helloworld: Helloworld sent using Log DEBUG level
[00:00:00.001,000] <wrn> app_helloworld: Helloworld sent using Log WARNING level
[00:00:00.001,000] <err> app_helloworld: Helloworld sent using Log ERROR level
```

## Logging Levels Explained

### Log Levels (from highest to lowest severity)

1. **LOG_ERR** - Error conditions
2. **LOG_WRN** - Warning conditions
3. **LOG_INF** - Informational messages
4. **LOG_DBG** - Debug-level messages

### Configuring Log Levels

In `prj.conf`:

```
# Set maximum compile-time log level (includes all up to this level)
CONFIG_LOG_MAX_LEVEL=4

# Set default runtime log level
CONFIG_LOG_DEFAULT_LEVEL=3
```

**Log level values:**
- 0 = OFF
- 1 = ERR
- 2 = WRN
- 3 = INF
- 4 = DBG

### Difference Between MAX_LEVEL and DEFAULT_LEVEL

- **CONFIG_LOG_MAX_LEVEL**: Compile-time limit (code removed if above)
- **CONFIG_LOG_DEFAULT_LEVEL**: Runtime default (can be changed)

When registering a module:
```c
// Use MAX_LEVEL - includes all log levels
LOG_MODULE_REGISTER(my_module, CONFIG_LOG_MAX_LEVEL);

// Use DEFAULT_LEVEL - excludes DEBUG by default
LOG_MODULE_REGISTER(my_module, CONFIG_LOG_DEFAULT_LEVEL);
```

## Troubleshooting

### Build Errors

#### "west: command not found"

Make sure west is installed and in PATH:
```bash
pip3 install --user west
export PATH="$HOME/.local/bin:$PATH"
```

#### "Could not find Zephyr"

Activate environment:
```bash
source ~/zephyrproject/.venv/bin/activate
source ~/zephyrproject/zephyr/zephyr-env.sh
echo $ZEPHYR_BASE
```

#### Build fails with "No such file or directory"

Make sure to include `.` in build command:
```bash
west build -b nucleo_f446re .
```

### Runtime Issues

#### No Output on Serial

1. Check USB connection: `ls /dev/ttyACM*`
2. Verify correct baud rate (115200)
3. Try different terminal program
4. Check board is powered

#### Missing DEBUG Messages

DEBUG logs may be disabled by default. Check `prj.conf`:
```
CONFIG_LOG=y
CONFIG_LOG_MAX_LEVEL=4
```

And use `CONFIG_LOG_MAX_LEVEL` in code:
```c
LOG_MODULE_REGISTER(app_helloworld, CONFIG_LOG_MAX_LEVEL);
```

#### Garbled Output

- Check baud rate matches (115200)
- Verify USB cable quality
- Try different USB port
- Reset board

## Modifying the Code

### Add Continuous Output

```c
int main(void)
{
    int counter = 0;

    printk("Hello World Application Started\n");

    while(1) {
        LOG_INF("Counter: %d", counter++);
        k_sleep(K_SECONDS(1));
    }

    return 0;
}
```

### Add Board Information

```c
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

int main(void)
{
    printk("Hello World!\n");
    printk("Board: %s\n", CONFIG_BOARD);
    printk("Zephyr version: %s\n", KERNEL_VERSION_STRING);
    printk("CPU: %s\n", CONFIG_SOC);

    return 0;
}
```

### Conditional Logging

```c
int main(void)
{
    int temperature = 75;

    if (temperature > 100) {
        LOG_ERR("Temperature too high: %d°C", temperature);
    } else if (temperature > 80) {
        LOG_WRN("Temperature elevated: %d°C", temperature);
    } else {
        LOG_INF("Temperature normal: %d°C", temperature);
    }

    return 0;
}
```

### Use Multiple Modules

```c
LOG_MODULE_REGISTER(main_module, LOG_LEVEL_INF);

// Define another module
#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(sensor_module);

void sensor_read(void)
{
    LOG_MODULE_DECLARE(sensor_module);
    LOG_DBG("Sensor read function called");
}
```

## Configuration Options

### Common prj.conf Settings

```
# Enable logging
CONFIG_LOG=y

# Set log levels
CONFIG_LOG_MAX_LEVEL=4
CONFIG_LOG_DEFAULT_LEVEL=3

# Enable timestamps
CONFIG_LOG_TIMESTAMP=y

# Buffer settings
CONFIG_LOG_BUFFER_SIZE=1024

# Backend settings
CONFIG_LOG_BACKEND_UART=y
CONFIG_LOG_BACKEND_RTT=n

# Printk settings
CONFIG_PRINTK=y
CONFIG_EARLY_CONSOLE=y
```

## printk() vs LOG_xxx()

### When to use printk()

- Simple debug output
- Early boot messages (before logging initialized)
- Minimal overhead needed
- Quick prototyping

```c
printk("Simple message: %d\n", value);
```

### When to use LOG_xxx()

- Production code
- Filterable output by severity
- Need timestamps
- Runtime level control

```c
LOG_INF("Structured message: %d", value);
```

## Testing Different Boards

This example works on any Zephyr board:

```bash
# For Nordic nRF52840
west build -b nrf52840dk_nrf52840 . -p always

# For ESP32
west build -b esp32_devkitc_wroom . -p always

# For QEMU (no hardware needed!)
west build -b qemu_x86 . -p always
west build -t run
```

## Learn More

- [Zephyr Logging API](https://docs.zephyrproject.org/latest/services/logging/index.html)
- [printk() Documentation](https://docs.zephyrproject.org/latest/kernel/services/other/printk.html)
- [Kernel Configuration](https://docs.zephyrproject.org/latest/build/kconfig/index.html)
- [Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html)

---

**Back to [Main README](../README.md)**
