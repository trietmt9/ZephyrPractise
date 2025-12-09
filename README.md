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

### System Requirements

- **Operating System**: Linux (Ubuntu/Debian recommended), macOS, or Windows (WSL2)
- **Python**: Version 3.8 or newer
- **Git**: For cloning repositories
- **CMake**: Version 3.20.0 or newer

### Install Dependencies (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install --no-install-recommends git cmake ninja-build gperf \
  ccache dfu-util device-tree-compiler wget \
  python3-dev python3-pip python3-setuptools python3-tk python3-wheel xz-utils file \
  make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1
```

---

## Installing Zephyr RTOS

### Step 1: Install West (Zephyr's Meta-Tool)

```bash
pip3 install --user west
```

Add west to your PATH (add to `~/.bashrc` or `~/.zshrc`):

```bash
export PATH="$HOME/.local/bin:$PATH"
```

Reload your shell:

```bash
source ~/.bashrc
```

### Step 2: Initialize Zephyr Workspace

```bash
# Create workspace directory
mkdir -p ~/zephyrproject
cd ~/zephyrproject

# Initialize west workspace
west init

# Update all repositories
west update
```

### Step 3: Install Python Dependencies

```bash
cd ~/zephyrproject
pip3 install -r zephyr/scripts/requirements.txt
```

### Step 4: Install Zephyr SDK

Download and install the Zephyr SDK (toolchain):

```bash
cd ~
wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.5/zephyr-sdk-0.16.5_linux-x86_64.tar.xz
tar xf zephyr-sdk-0.16.5_linux-x86_64.tar.xz
cd zephyr-sdk-0.16.5
./setup.sh
```

**Note**: Check [Zephyr SDK releases](https://github.com/zephyrproject-rtos/sdk-ng/releases) for the latest version.

### Step 5: Set Up Environment Variables

Add to your `~/.bashrc` or `~/.zshrc`:

```bash
export ZEPHYR_BASE=~/zephyrproject/zephyr
export ZEPHYR_SDK_INSTALL_DIR=~/zephyr-sdk-0.16.5
```

Reload your shell:

```bash
source ~/.bashrc
```

### Step 6: Verify Installation

```bash
cd ~/zephyrproject/zephyr
west build -p auto -b qemu_x86 samples/hello_world
west build -t run
```

If you see "Hello World!" output, Zephyr is successfully installed!

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

# 2. Build for your board
west build -b nucleo_f446re . -p always

# 3. Flash to hardware
west flash

# 4. View serial output
screen /dev/ttyACM0 115200
```

**Important**: The `.` in the build command specifies the current directory as your standalone application location.

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

## Target Hardware

### STM32 Nucleo F446RE

**Specifications:**
- **MCU**: STM32F446RET6
- **Core**: ARM Cortex-M4F @ 180 MHz
- **Flash**: 512 KB
- **RAM**: 128 KB
- **Onboard LED**: PA5 (Green LED)
- **User Button**: PC13

---

## Common Issues

### "west: command not found"

Make sure west is in your PATH:
```bash
export PATH="$HOME/.local/bin:$PATH"
```

### "Could not find Zephyr"

Activate the Zephyr environment:
```bash
source ~/zephyrproject/.venv/bin/activate
source ~/zephyrproject/zephyr/zephyr-env.sh
echo $ZEPHYR_BASE  # Should show /home/username/zephyrproject/zephyr
```

### Flash Permissions (Linux)

Add your user to the dialout group:
```bash
sudo usermod -a -G dialout $USER
```

Log out and back in for changes to take effect.

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
