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

- **Operating System**: Linux (Ubuntu/Debian recommended), macOS 10.15+, or Windows 10/11
- **Python**: Version 3.9 or newer
- **Git**: For cloning repositories
- **CMake**: Version 3.20.5 or newer
- **Disk Space**: At least 5 GB free

---

## Installing Zephyr RTOS

Choose your operating system:
- [Linux (Ubuntu/Debian)](#linux-installation)
- [macOS](#macos-installation)
- [Windows](#windows-installation)

---

## Linux Installation

### Step 1: Install Dependencies (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install --no-install-recommends git cmake ninja-build gperf \
  ccache dfu-util device-tree-compiler wget \
  python3-dev python3-pip python3-setuptools python3-tk python3-wheel xz-utils file \
  make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1
```

### Step 2: Install West and Create Virtual Environment

```bash
# Install west
pip3 install --user -U west

# Add to PATH (add to ~/.bashrc)
echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc

# Create workspace directory
mkdir -p ~/zephyrproject
cd ~/zephyrproject

# Create Python virtual environment
python3 -m venv .venv
source .venv/bin/activate
```

### Step 3: Initialize Zephyr Workspace

```bash
cd ~/zephyrproject
west init
west update
```

### Step 4: Install Python Dependencies

```bash
pip install -r zephyr/scripts/requirements.txt
```

### Step 5: Install Zephyr SDK

```bash
cd ~
wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.8/zephyr-sdk-0.16.8_linux-x86_64.tar.xz
tar xf zephyr-sdk-0.16.8_linux-x86_64.tar.xz
cd zephyr-sdk-0.16.8
./setup.sh
```

### Step 6: Set Up Environment

Add to `~/.bashrc`:

```bash
export ZEPHYR_BASE=~/zephyrproject/zephyr
```

Reload:
```bash
source ~/.bashrc
```

### Step 7: Verify Installation

```bash
cd ~/zephyrproject
source .venv/bin/activate
source zephyr/zephyr-env.sh
west build -p auto -b qemu_x86 zephyr/samples/hello_world
west build -t run
```

---

## macOS Installation

### Step 1: Install Homebrew (if not installed)

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### Step 2: Install Dependencies

```bash
brew install cmake ninja gperf python3 ccache qemu dtc wget libmagic
```

### Step 3: Install West and Create Virtual Environment

```bash
# Create workspace directory
mkdir -p ~/zephyrproject
cd ~/zephyrproject

# Create Python virtual environment
python3 -m venv .venv
source .venv/bin/activate

# Install west
pip install west
```

### Step 4: Initialize Zephyr Workspace

```bash
cd ~/zephyrproject
west init
west update
```

### Step 5: Install Python Dependencies

```bash
pip install -r zephyr/scripts/requirements.txt
```

### Step 6: Install Zephyr SDK

```bash
cd ~
wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.8/zephyr-sdk-0.16.8_macos-x86_64.tar.xz
# For Apple Silicon (M1/M2/M3):
# wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.8/zephyr-sdk-0.16.8_macos-aarch64.tar.xz

tar xf zephyr-sdk-0.16.8_macos-*.tar.xz
cd zephyr-sdk-0.16.8
./setup.sh
```

### Step 7: Set Up Environment

Add to `~/.zshrc` (macOS default shell):

```bash
export ZEPHYR_BASE=~/zephyrproject/zephyr
```

Reload:
```bash
source ~/.zshrc
```

### Step 8: Verify Installation

```bash
cd ~/zephyrproject
source .venv/bin/activate
source zephyr/zephyr-env.sh
west build -p auto -b qemu_x86 zephyr/samples/hello_world
west build -t run
```

---

## Windows Installation

### Method 1: Using WSL2 (Recommended)

WSL2 provides the best compatibility and performance for Zephyr development on Windows.

#### Step 1: Install WSL2

Open PowerShell as Administrator:

```powershell
wsl --install -d Ubuntu
```

Restart your computer when prompted.

#### Step 2: Open Ubuntu and Follow Linux Installation

After restart, open Ubuntu from Start Menu and follow the [Linux Installation](#linux-installation) steps above.

#### Step 3: Install USB/IP for Hardware Access (Optional)

To access USB devices from WSL2:

```bash
# In WSL2 Ubuntu
sudo apt install linux-tools-generic hwdata
sudo update-alternatives --install /usr/local/bin/usbip usbip /usr/lib/linux-tools/*-generic/usbip 20
```

On Windows (PowerShell as Admin):
```powershell
winget install --interactive --exact dorssel.usbipd-win
```

Connect USB device:
```powershell
# In PowerShell (Admin)
usbipd list
usbipd bind --busid <BUSID>
usbipd attach --wsl --busid <BUSID>
```

### Method 2: Native Windows Installation

#### Step 1: Install Chocolatey Package Manager

Open PowerShell as Administrator:

```powershell
Set-ExecutionPolicy Bypass -Scope Process -Force
[System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
```

#### Step 2: Install Dependencies

```powershell
choco install cmake --installargs 'ADD_CMAKE_TO_PATH=System' -y
choco install ninja gperf python git dtc-msys2 wget 7zip -y
```

#### Step 3: Install West

```powershell
pip3 install -U west
```

#### Step 4: Initialize Zephyr Workspace

```powershell
cd %HOMEPATH%
mkdir zephyrproject
cd zephyrproject

# Create virtual environment
python -m venv .venv
.venv\Scripts\activate.bat

# Initialize workspace
west init
west update
```

#### Step 5: Install Python Dependencies

```powershell
cd %HOMEPATH%\zephyrproject
pip install -r zephyr\scripts\requirements.txt
```

#### Step 6: Install Zephyr SDK

Download from: https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.8/zephyr-sdk-0.16.8_windows-x86_64.7z

Extract to `C:\zephyr-sdk-0.16.8` and run `setup.cmd`

#### Step 7: Set Environment Variables

Add to System Environment Variables:
- `ZEPHYR_BASE`: `%HOMEPATH%\zephyrproject\zephyr`

#### Step 8: Verify Installation

```powershell
cd %HOMEPATH%\zephyrproject
.venv\Scripts\activate.bat
zephyr\zephyr-env.cmd
west build -p auto -b qemu_x86 zephyr\samples\hello_world
west build -t run
```

---

## Quick Setup Summary

### Every Terminal Session (All Platforms)

Before building, activate the Zephyr environment:

**Linux/macOS:**
```bash
cd ~/zephyrproject
source .venv/bin/activate
source zephyr/zephyr-env.sh
```

**Windows (Native):**
```powershell
cd %HOMEPATH%\zephyrproject
.venv\Scripts\activate.bat
zephyr\zephyr-env.cmd
```

**Pro Tip**: Create an alias (Linux/macOS in `~/.bashrc` or `~/.zshrc`):
```bash
alias zephyr-env='cd ~/zephyrproject && source .venv/bin/activate && source zephyr/zephyr-env.sh'
```

---

## Verification Checklist

After installation, verify everything works:

- [ ] `west --version` shows version info
- [ ] `cmake --version` shows 3.20.5 or newer
- [ ] `python --version` shows 3.9 or newer
- [ ] `$ZEPHYR_BASE` environment variable is set
- [ ] Hello World sample builds and runs
- [ ] Board can be flashed (if hardware connected)

If you see "Hello World!" from the QEMU sample, you're ready to develop!

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

## Common Issues & Troubleshooting

### All Platforms

#### "west: command not found"

**Linux/macOS:**
```bash
export PATH="$HOME/.local/bin:$PATH"
# Add to ~/.bashrc or ~/.zshrc permanently
```

**Windows:**
```powershell
# Reinstall west or check PATH in System Environment Variables
pip3 install --upgrade west
```

#### "Could not find Zephyr"

Activate the Zephyr environment:

**Linux/macOS:**
```bash
cd ~/zephyrproject
source .venv/bin/activate
source zephyr/zephyr-env.sh
echo $ZEPHYR_BASE  # Should show path to zephyr directory
```

**Windows:**
```powershell
cd %HOMEPATH%\zephyrproject
.venv\Scripts\activate.bat
zephyr\zephyr-env.cmd
echo %ZEPHYR_BASE%
```

#### Build Fails with "CMake Error"

Clean the build directory:
```bash
rm -rf build
west build -p auto -b <your_board> .
```

### Linux-Specific Issues

#### Flash Permissions

Add your user to dialout and plugdev groups:
```bash
sudo usermod -a -G dialout $USER
sudo usermod -a -G plugdev $USER
```

Log out and back in for changes to take effect.

#### ST-Link udev Rules

If ST-Link isn't recognized:
```bash
sudo nano /etc/udev/rules.d/49-stlinkv2.rules
```

Add:
```
SUBSYSTEM=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="3744", MODE="0666"
SUBSYSTEM=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="3748", MODE="0666"
KERNEL=="ttyACM*", ATTRS{idVendor}=="0483", MODE="0666"
```

Reload rules:
```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```

### macOS-Specific Issues

#### "xcrun: error: invalid active developer path"

Install Xcode Command Line Tools:
```bash
xcode-select --install
```

#### USB Device Not Found

Check System Preferences → Security & Privacy → Privacy → USB

Grant terminal access to USB devices.

#### Homebrew Installation Issues

Update Homebrew:
```bash
brew update
brew upgrade
```

### Windows-Specific Issues

#### WSL2 USB Device Not Visible

Use usbipd to attach USB devices:
```powershell
# In PowerShell (Admin)
usbipd list
usbipd bind --busid <BUSID>
usbipd attach --wsl --busid <BUSID>
```

#### Native Windows: "cmake not found"

Ensure CMake is in PATH:
```powershell
# Reinstall with PATH option
choco install cmake --installargs 'ADD_CMAKE_TO_PATH=System' -y --force
```

Close and reopen PowerShell after installation.

#### Python Virtual Environment Issues

If `.venv\Scripts\activate.bat` fails:
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

#### Long Path Issues

Enable long path support:
```powershell
# In PowerShell (Admin)
New-ItemProperty -Path "HKLM:\SYSTEM\CurrentControlSet\Control\FileSystem" -Name "LongPathsEnabled" -Value 1 -PropertyType DWORD -Force
```

### Board-Specific Issues

#### STM32F411E-DISCO: No Virtual COM Port

The STM32F411E-DISCO board does not have VCP support. Use:
- External USB-to-UART adapter on PA2 (TX) / PA3 (RX)
- Solder SB10 and SB11 bridges to enable USART2

#### Serial Port Not Found

**Linux:**
```bash
ls /dev/tty*
# Look for /dev/ttyACM* or /dev/ttyUSB*
```

**macOS:**
```bash
ls /dev/cu.*
# Look for /dev/cu.usbmodem* or /dev/cu.usbserial*
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
