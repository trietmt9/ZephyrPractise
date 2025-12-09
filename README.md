# Zephyr RTOS Tutorial

This repository contains examples and tutorials for learning Zephyr RTOS development.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Installing Zephyr RTOS](#installing-zephyr-rtos)
- [Project Structure](#project-structure)
- [Building and Flashing](#building-and-flashing)
- [Example Projects](#example-projects)
- [Troubleshooting](#troubleshooting)

---

## Prerequisites

### System Requirements

- **Operating System**: Linux (Ubuntu/Debian recommended), macOS, or Windows (WSL2)
- **Python**: Version 3.8 or newer
- **Git**: For cloning repositories
- **CMake**: Version 3.20.0 or newer
- **Device Tree Compiler (dtc)**: For devicetree processing

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

West is Zephyr's build and configuration tool. Install it using pip:

```bash
pip3 install --user west
```

Add west to your PATH (add to `~/.bashrc` or `~/.zshrc`):

```bash
export PATH="$HOME/.local/bin:$PATH"
```

Reload your shell:

```bash
source ~/.bashrc  # or source ~/.zshrc
```

### Step 2: Initialize Zephyr Workspace

Create a workspace directory and initialize Zephyr:

```bash
# Create workspace directory
mkdir -p ~/zephyrproject
cd ~/zephyrproject

# Initialize west workspace
west init zephyrproject

# Update all repositories
cd zephyrproject
west update
```

### Step 3: Install Python Dependencies

```bash
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

## Project Structure

This tutorial repository contains several example projects:

```
ZephyrTutorial/
├── zephyrproject/          # Zephyr RTOS installation
├── app_helloworld/         # Basic Hello World example
├── app_gpio/               # GPIO control examples
├── app_timer/              # Timer examples
├── app_pwm/                # PWM examples
└── app_exti/               # External interrupt examples
```

### Typical Project Structure

```
app_example/
├── CMakeLists.txt          # Build configuration
├── prj.conf                # Kconfig options
├── boards/
│   └── example.overlay     # Devicetree overlay
└── src/
    └── main.c              # Application source code
```

---

## Building and Flashing

### Build a Project

Navigate to your project directory and build for your target board:

```bash
cd app_pwm
west build -p auto -b nucleo_f446re
```

**Common build options:**
- `-p auto`: Automatically clean previous builds when configuration changes
- `-b <board>`: Specify target board (e.g., `nucleo_f446re`, `nrf52840dk_nrf52840`)
- `--pristine`: Force a clean build

### Flash to Hardware

After building, flash the binary to your board:

```bash
west flash
```

### View Serial Output

Monitor serial output using one of these methods:

**Using screen:**
```bash
screen /dev/ttyACM0 115200
```

**Using minicom:**
```bash
minicom -D /dev/ttyACM0 -b 115200
```

**Using VS Code Serial Monitor:**
Install the "Serial Monitor" extension and configure the port.

---

## Example Projects

### 1. Hello World (`app_helloworld`)

Basic "Hello World" application demonstrating Zephyr initialization.

```bash
cd app_helloworld
west build -p auto -b nucleo_f446re
west flash
```

### 2. GPIO Control (`app_gpio`)

Examples of controlling GPIO pins and reading inputs.

**Features:**
- LED control
- Button input
- GPIO interrupts

### 3. Timer (`app_timer`)

Kernel timer examples for periodic tasks.

**Features:**
- Timer initialization
- Periodic callbacks
- Timer synchronization

### 4. PWM (`app_pwm`)

PWM signal generation for motor control, LED dimming, etc.

**Features:**
- 1kHz PWM generation
- Duty cycle control (0-100%)
- Smooth ramping

```bash
cd app_pwm
west build -p auto -b nucleo_f446re
west flash
```

### 5. External Interrupts (`app_exti`)

Handling external hardware interrupts.

---

## Troubleshooting

### Common Issues

#### 1. "west: command not found"

**Solution:** Make sure west is installed and in your PATH:
```bash
pip3 install --user west
export PATH="$HOME/.local/bin:$PATH"
```

#### 2. CMake Error: "Could not find Zephyr"

**Solution:** Set the `ZEPHYR_BASE` environment variable:
```bash
export ZEPHYR_BASE=~/zephyrproject/zephyr
```

#### 3. Devicetree Errors

**Common devicetree issues:**
- **Missing semicolons**: Every node must end with `};`
- **Invalid alias names**: Use hyphens `-` not underscores `_` in alias names
- **Missing ampersands**: Phandle references need `&` (e.g., `<&gpioa 5>`)
- **Typos in compatible strings**: Check spelling (e.g., `"gpio-leds"` not `"gpios-leds"`)

#### 4. Invalid Kconfig Options

If you see Kconfig errors, check that config options exist:
- Use `CONFIG_GPIO=y` not `CONFIG_LOG_GPIO=y`
- Search available configs: `west build -t menuconfig`

#### 5. Flash Permissions (Linux)

Add your user to the dialout group for USB device access:
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

## Board Information

### STM32 Nucleo F446RE

**Specifications:**
- **MCU**: STM32F446RET6
- **Core**: ARM Cortex-M4F @ 180 MHz
- **Flash**: 512 KB
- **RAM**: 128 KB
- **Onboard LED**: PA5 (Green LED)
- **User Button**: PC13

**Pinout reference**: Available in the board documentation.

---

## License

This tutorial repository is provided as-is for educational purposes.

---

## Contributing

Feel free to add more examples or improve existing ones!

---

**Happy Coding with Zephyr RTOS!**
