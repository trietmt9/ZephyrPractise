# Zephyr RTOS Tutorial

This repository contains standalone Zephyr RTOS application examples for learning embedded development.

## What are Standalone Zephyr Applications?

Standalone Zephyr applications exist **outside** the main Zephyr source tree (not under `zephyr/samples`). This approach offers several advantages:

- **Independent version control** - Manage your code separately from Zephyr
- **Cleaner collaboration** - Easier to work in teams without conflicts
- **Better code reuse** - Portable across different projects
- **IP separation** - Keep proprietary code separate from Zephyr
- **Hardware portability** - Easy to adapt for different boards

---

## Example Projects

Each project has its own README with detailed instructions:

1. **[app_helloworld](app_helloworld/README.md)** - Basic "Hello World" application
2. **[app_gpio](app_gpio/README.md)** - GPIO control and LED toggling
3. **[app_timer](app_timer/README.md)** - Kernel timers for periodic tasks
4. **[app_pwm](app_pwm/README.md)** - PWM signal generation and duty cycle control
5. **[app_exti](app_exti/README.md)** - External interrupt handling

---

## Prerequisites

Before using these examples, you must have Zephyr RTOS installed on your system.

**📚 [Install Zephyr RTOS - Official Guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html)**

Follow the official documentation to:
- Install system dependencies
- Set up Python virtual environment
- Initialize Zephyr workspace with `west`
- Install Zephyr SDK with `west sdk install`

**Quick Check:** If these commands work, you're ready:
```bash
west --version
echo $ZEPHYR_BASE  # Should show path to zephyr directory
```

---

## Building Standalone Applications

### Environment Setup (Required Before Every Build)

Every time you start a new terminal session, activate the Zephyr environment:

```bash
# Activate Python virtual environment
source ~/zephyrproject/.venv/bin/activate

# Source Zephyr environment variables
source ~/zephyrproject/zephyr/zephyr-env.sh
```

**Pro Tip**: Add an alias to your `~/.bashrc`:
```bash
alias zephyr-env='source ~/zephyrproject/.venv/bin/activate && source ~/zephyrproject/zephyr/zephyr-env.sh'
```

Then simply run `zephyr-env` before building.

### Basic Build Process

```bash
# 1. Navigate to project
cd app_example

# 2. Build for your board (replace with your board name)
west build -b <your_board> . -p always

# 3. Flash to hardware
west flash

# 4. View serial output (adjust port and baudrate as needed)
screen /dev/ttyACM0 115200
```

**Important**:
- Replace `<your_board>` with your actual board identifier (e.g., `nucleo_f446re`, `stm32f411e_disco`, `nrf52840dk_nrf52840`)
- Find supported boards: https://docs.zephyrproject.org/latest/boards/index.html
- The `.` in the build command specifies the current directory as your standalone application location

---

## Project Structure

Each standalone application follows this structure:

```
app_example/
├── CMakeLists.txt          # Build configuration
├── prj.conf                # Kconfig options
├── boards/                 # Board-specific files (optional)
│   └── example.overlay     # Devicetree overlay
├── src/
│   └── main.c              # Application source code
└── README.md               # Project documentation
```

### Required Files

**CMakeLists.txt:**
```cmake
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(app_example)
target_sources(app PRIVATE src/main.c)
```

**prj.conf:**
```
CONFIG_GPIO=y
CONFIG_LOG=y
```

**src/main.c:**
```c
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

int main(void) {
    printk("Hello World!\n");
    return 0;
}
```

---

## Common Issues & Troubleshooting

### Environment Not Activated

**Error:** `"Could not find Zephyr"` or `"west: command not found"`

**Solution:** Activate the Zephyr environment before building:

**Linux/macOS:**
```bash
cd ~/zephyrproject
source .venv/bin/activate
source zephyr/zephyr-env.sh
```

**Windows:**
```powershell
cd %HOMEPATH%\zephyrproject
.venv\Scripts\activate.bat
zephyr\zephyr-env.cmd
```

### Build Fails - Clean Build Required

**Error:** CMake configuration errors or stale build artifacts

**Solution:**
```bash
rm -rf build
west build -p always -b <your_board> .
```

### Wrong Build Location

**Error:** `"No such file or directory"` when building

**Solution:** Make sure you're in the project directory and use `.` to specify current directory:
```bash
cd app_example
west build -b <your_board> .
```
The `.` tells west to use the current directory as the application source.

### Device Tree Errors

**Error:** `"parse error"` or `"unknown node"`

**Common Causes:**
- Missing `&` before node references (e.g., `<&gpioa>` not `<gpioa>`)
- Missing `compatible` property in device tree nodes
- Syntax errors in `.overlay` files

### Flash Permissions (Linux)

**Error:** `"Permission denied"` when flashing

**Solution:**
```bash
sudo usermod -a -G dialout $USER
sudo usermod -a -G plugdev $USER
```
Log out and back in for changes to take effect.

### STM32 Board-Specific Issues

#### STM32F411E-DISCO: No Virtual COM Port

This board does not have VCP support. Use:
- External USB-to-UART adapter on PA2 (TX) / PA3 (RX)

#### ST-Link Not Recognized (Linux)

Add udev rules:
```bash
sudo nano /etc/udev/rules.d/49-stlinkv2.rules
```

Add these lines:
```
SUBSYSTEM=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="3748", MODE="0666"
KERNEL=="ttyACM*", ATTRS{idVendor}=="0483", MODE="0666"
```

Reload:
```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```

### Finding Serial Ports

**Linux:**
```bash
ls /dev/ttyACM* /dev/ttyUSB*
```

**macOS:**
```bash
ls /dev/cu.usbmodem* /dev/cu.usbserial*
```

**Windows:**
Check Device Manager → Ports (COM & LPT)

---

## Resources

- **Zephyr Documentation**: https://docs.zephyrproject.org
- **Zephyr GitHub**: https://github.com/zephyrproject-rtos/zephyr
- **Getting Started Guide**: https://docs.zephyrproject.org/latest/getting_started/index.html
- **Board Support**: https://docs.zephyrproject.org/latest/boards/index.html
- **Device Tree Guide**: https://docs.zephyrproject.org/latest/build/dts/index.html

---

## License

This tutorial repository is provided as-is for educational purposes.

---

**Happy Coding with Zephyr RTOS!**
