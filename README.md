# Zephyr RTOS Tutorial

This repository contains examples and tutorials for learning Zephyr RTOS development using **standalone (freestanding) applications**.

## What are Standalone Zephyr Applications?

Standalone Zephyr applications exist **outside** the main Zephyr source tree (not under `zephyr/samples`). This approach offers several advantages:

- **Independent version control** - Manage your code separately from Zephyr
- **Cleaner collaboration** - Easier to work in teams without conflicts
- **Better code reuse** - Portable across different projects
- **IP separation** - Keep proprietary code separate from Zephyr
- **Hardware portability** - Easy to adapt for different boards

## Table of Contents

- [Quick Start](#quick-start)
- [Prerequisites](#prerequisites)
- [Installing Zephyr RTOS](#installing-zephyr-rtos)
- [Project Structure](#project-structure)
- [Building Standalone Applications](#building-standalone-applications)
- [Creating a New Standalone Project](#creating-a-new-standalone-project)
- [Building and Flashing](#building-and-flashing)
- [Example Projects](#example-projects)
- [Troubleshooting](#troubleshooting)
- [Resources](#resources)
- [Board Information](#board-information)

---

## Quick Start

If you already have Zephyr installed, here's how to quickly build and run an example:

```bash
# 1. Clone this repository
git clone <your-repo-url>
cd ZephyrTutorial

# 2. Activate Zephyr environment
source ~/zephyrproject/.venv/bin/activate
source ~/zephyrproject/zephyr/zephyr-env.sh

# 3. Build an example (e.g., PWM)
cd app_pwm
west build -b nucleo_f446re . -p always

# 4. Flash to your board
west flash

# 5. View output
screen /dev/ttyACM0 115200
```

**Don't have Zephyr installed yet?** Continue to the [Prerequisites](#prerequisites) section below.

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

### Typical Standalone Project Structure

Each standalone application in this repository follows this structure:

```
app_example/
├── CMakeLists.txt          # Build configuration
├── prj.conf                # Kconfig options
├── boards/
│   └── example.overlay     # Devicetree overlay (optional)
└── src/
    └── main.c              # Application source code
```

### Required Files Explained

**1. CMakeLists.txt** - Build configuration file:
```cmake
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(app_example)
target_sources(app PRIVATE src/main.c)
```

**2. prj.conf** - Kconfig options for enabling features:
```
CONFIG_GPIO=y
CONFIG_PWM=y
CONFIG_LOG=y
```

**3. main.c** - Application entry point:
```c
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

int main(void) {
    printk("Hello World! Running on %s\n", CONFIG_BOARD);
    return 0;
}
```

**4. boards/*.overlay** (Optional) - Devicetree overlay for hardware configuration.

---

## Building Standalone Applications

### Prerequisites Before Building

Every time you start a new terminal session, you must activate the Zephyr environment:

```bash
# Activate Python virtual environment
source ~/zephyrproject/.venv/bin/activate

# Source Zephyr environment variables
source ~/zephyrproject/zephyr/zephyr-env.sh
```

**Pro Tip**: Add these to your `~/.bashrc` for automatic activation:
```bash
# Auto-activate Zephyr environment
alias zephyr-env='source ~/zephyrproject/.venv/bin/activate && source ~/zephyrproject/zephyr/zephyr-env.sh'
```

Then simply run `zephyr-env` before building.

### Environment Variables Check

Verify your environment is set up correctly:

```bash
echo $ZEPHYR_BASE
# Should output: /home/username/zephyrproject/zephyr

which west
# Should show: /home/username/zephyrproject/.venv/bin/west
```

### Creating a New Standalone Project

Want to create your own standalone application? Here's how:

**1. Create project directory structure:**
```bash
mkdir -p my_new_app/src
cd my_new_app
```

**2. Create `CMakeLists.txt`:**
```cmake
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(my_new_app)
target_sources(app PRIVATE src/main.c)
```

**3. Create `prj.conf` (add any needed configs):**
```
CONFIG_GPIO=y
CONFIG_LOG=y
```

**4. Create `src/main.c`:**
```c
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

int main(void) {
    printk("My New App!\n");
    return 0;
}
```

**5. Build and run:**
```bash
west build -b nucleo_f446re . -p always
west flash
```

**Optional: Add devicetree overlay (for GPIO, PWM, etc.):**
```bash
mkdir boards
# Create boards/my_overlay.overlay
```

Then add to CMakeLists.txt:
```cmake
set(DTC_OVERLAY_FILE "boards/my_overlay.overlay")
```

---

## Building and Flashing

### Step-by-Step Build Process

**1. Activate Zephyr Environment** (if not already done):
```bash
source ~/zephyrproject/.venv/bin/activate
source ~/zephyrproject/zephyr/zephyr-env.sh
```

**2. Navigate to Your Project**:
```bash
cd ~/Documents/ZephyrTutorial/app_pwm
```

**3. Build for Your Target Board**:
```bash
west build -b nucleo_f446re . -p always
```

**Note**: The `.` specifies the current directory as the application source.

### Common Build Options

- `-b <board>`: Specify target board (e.g., `nucleo_f446re`, `rpi_pico`, `nrf52840dk_nrf52840`)
- `-p auto`: Automatically clean when configuration changes
- `-p always`: Always do a pristine (clean) build
- `--pristine`: Force a clean build
- `.`: Current directory (specifies standalone app location)

### Build Examples

**Build PWM example:**
```bash
cd app_pwm
west build -b nucleo_f446re . -p always
```

**Build with different board:**
```bash
cd app_gpio
west build -b nrf52840dk_nrf52840 . -p always
```

**Rebuild without cleaning:**
```bash
west build
```

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

All examples are standalone applications that can be built independently.

### 1. Hello World (`app_helloworld`)

Basic "Hello World" application demonstrating Zephyr initialization.

**Build and Flash:**
```bash
cd app_helloworld
west build -b nucleo_f446re . -p always
west flash
```

**Expected Output:**
```
Hello World! Running on nucleo_f446re
```

### 2. GPIO Control (`app_gpio`)

Examples of controlling GPIO pins and reading inputs.

**Features:**
- LED control
- Button input
- GPIO interrupts

**Build and Flash:**
```bash
cd app_gpio
west build -b nucleo_f446re . -p always
west flash
```

### 3. Timer (`app_timer`)

Kernel timer examples for periodic tasks.

**Features:**
- Timer initialization with `k_timer_init()`
- Periodic callbacks every 500ms
- Timer synchronization with `k_timer_status_sync()`
- LED toggling in timer callback

**Build and Flash:**
```bash
cd app_timer
west build -b nucleo_f446re . -p always
west flash
```

**Files:**
- `src/main.c` - Timer implementation
- `boards/timer.overlay` - LED GPIO configuration

### 4. PWM (`app_pwm`)

PWM signal generation for motor control, LED dimming, and servo control.

**Features:**
- 1kHz PWM frequency
- Duty cycle control (0-100%)
- Smooth brightness ramping
- 100 steps for smooth transitions

**Build and Flash:**
```bash
cd app_pwm
west build -b nucleo_f446re . -p always
west flash
```

**Files:**
- `src/main.c` - PWM duty cycle ramping
- `boards/pwm.overlay` - PWM2 channel 1 on PA5 (TIM2_CH1)

**Expected Output:**
```
PWM Demo: 0% to 100% duty cycle at 1kHz
PWM ready on channel 1
--- Ramping UP ---
Duty:   0% | Pulse:       0 ns
Duty:   1% | Pulse:   10000 ns
...
```

### 5. External Interrupts (`app_exti`)

Handling external hardware interrupts.

**Build and Flash:**
```bash
cd app_exti
west build -b nucleo_f446re . -p always
west flash
```

---

## Troubleshooting

### Common Issues

#### 1. "west: command not found"

**Solution:** Make sure west is installed and in your PATH:
```bash
pip3 install --user west
export PATH="$HOME/.local/bin:$PATH"
```

#### 2. CMake Error: "Could not find Zephyr" (Standalone Apps)

This is the most common error with standalone applications.

**Cause:** Missing environment variables or Python virtual environment not activated.

**Solution:**
```bash
# 1. Activate Python virtual environment
source ~/zephyrproject/.venv/bin/activate

# 2. Source Zephyr environment
source ~/zephyrproject/zephyr/zephyr-env.sh

# 3. Verify ZEPHYR_BASE is set
echo $ZEPHYR_BASE
# Should show: /home/username/zephyrproject/zephyr

# 4. Now try building again
west build -b nucleo_f446re . -p always
```

**Alternative:** Check your CMakeLists.txt has the correct `find_package` line:
```cmake
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
```

#### 3. "No such file or directory" when building

**Cause:** Not specifying the current directory `.` in the build command.

**Wrong:**
```bash
west build -b nucleo_f446re
```

**Correct:**
```bash
west build -b nucleo_f446re .
```

The `.` tells west where your standalone application is located.

#### 4. Devicetree Errors

**Common devicetree issues we fixed in this tutorial:**
- **Missing semicolons**: Every node must end with `};`
  - Example: `leds { ... }` should be `leds { ... };`
- **Invalid alias names**: Use hyphens `-` not underscores `_` in alias names
  - Wrong: `pwm_led0 = &pwm_led0;`
  - Correct: `pwm-led0 = &pwm_led0;`
- **Missing ampersands**: Phandle references need `&`
  - Wrong: `<gpioa 5 GPIO_ACTIVE_HIGH>`
  - Correct: `<&gpioa 5 GPIO_ACTIVE_HIGH>`
- **Typos in compatible strings**: Check spelling
  - Wrong: `compatible = "gpios-leds";` or `compatiple = "gpio-leds";`
  - Correct: `compatible = "gpio-leds";`

**Example of correct overlay file:**
```dts
/ {
    aliases {
        pwm-led0 = &pwm_led0;    // Note: hyphen, not underscore
    };

    pwmleds {
        compatible = "pwm-leds";
        pwm_led0: pwm_led_0 {
            pwms = <&pwm2 1 PWM_MSEC(1) PWM_POLARITY_NORMAL>;
        };
    };
};
```

#### 5. Invalid Kconfig Options

If you see Kconfig errors, check that config options exist:
- Use `CONFIG_GPIO=y` not `CONFIG_LOG_GPIO=y`
- Use `CONFIG_PWM=y` for PWM support
- Search available configs: `west build -t menuconfig`

#### 6. CMakeLists.txt Syntax Errors

**Wrong:**
```cmake
set(DTC_OVERLAY_FILE = "boards/pwm.overlay")  # Don't use '='
target_sources(app PRIVATE src/main.c_)        # Typo in filename
```

**Correct:**
```cmake
set(DTC_OVERLAY_FILE "boards/pwm.overlay")
target_sources(app PRIVATE src/main.c)
```

#### 7. Flash Permissions (Linux)

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

## Useful Reference: PWM Calculations

When working with PWM, here's how to calculate periods and frequencies:

### Frequency to Period Conversion

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

### Zephyr PWM Time Macros

```c
PWM_SEC(1)    // 1,000,000,000 ns = 1 second
PWM_MSEC(20)  //    20,000,000 ns = 20 milliseconds
PWM_USEC(100) //       100,000 ns = 100 microseconds
PWM_NSEC(500) //           500 ns = 500 nanoseconds
```

### Example: 50Hz PWM Calculation

```c
// Method 1: Direct calculation
#define PWM_PERIOD_50HZ  PWM_MSEC(20)     // 20ms = 50Hz

// Method 2: Using division
#define PWM_PERIOD_50HZ  PWM_SEC(1)/50U   // 1 second / 50 = 20ms

// Method 3: Manual nanoseconds
#define PWM_PERIOD_50HZ  20000000U        // 20,000,000 ns
```

### Duty Cycle Calculation

Duty cycle is the percentage of time the signal is HIGH:

```c
uint32_t period = PWM_MSEC(20);           // 20ms period (50Hz)
uint32_t duty_50_percent = period / 2;    // 10ms HIGH, 10ms LOW
uint32_t duty_25_percent = period / 4;    // 5ms HIGH, 15ms LOW
uint32_t duty_75_percent = (period * 3) / 4; // 15ms HIGH, 5ms LOW
```

---

## License

This tutorial repository is provided as-is for educational purposes.

---

## Contributing

Feel free to add more examples or improve existing ones!

---

**Happy Coding with Zephyr RTOS!**
