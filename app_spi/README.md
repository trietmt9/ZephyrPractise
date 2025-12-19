# SPI Device Driver Example

This example demonstrates how to use the SPI (Serial Peripheral Interface) driver in Zephyr RTOS. It includes a custom driver abstraction layer and shows basic SPI communication operations.

## Overview

The example demonstrates:
- SPI device initialization
- SPI write operations
- SPI read operations
- Full-duplex SPI transfer (transmit and receive simultaneously)
- Loopback testing capability
- Error handling and logging

## Hardware Requirements

- STM32 Nucleo board (or compatible board with SPI support)
- Optional: SPI device (EEPROM, sensor, etc.) or loopback connection (connect MISO to MOSI)

## Pin Configuration

The example uses SPI1 with the following default pins (STM32):
- **SCK (Clock)**: PA5 (SPI1_SCK)
- **MISO (Master In Slave Out)**: PA6 (SPI1_MISO)
- **MOSI (Master Out Slave In)**: PA7 (SPI1_MOSI)
- **CS (Chip Select)**: PA4 (GPIO controlled)

## SPI Configuration

- **Frequency**: 1 MHz
- **Mode**: SPI Mode 3 (CPOL=1, CPHA=1)
- **Word Size**: 8 bits
- **Master Mode**: Yes

## Project Structure

```
app_spi/
├── boards/
│   └── spi.overlay          # Device tree overlay for SPI configuration
├── driver/
│   ├── inc/
│   │   └── spi.h           # SPI driver header file
│   └── src/
│       └── spi.c           # SPI driver implementation
├── src/
│   └── main.c              # Main application code
├── CMakeLists.txt          # Build configuration
├── prj.conf                # Project configuration
└── README.md               # This file
```

## Building and Running

### Build the application:

```bash
cd app_spi
west build -b nucleo_f446re
```

### Flash to the board:

```bash
west flash
```

### Monitor serial output:

```bash
minicom -D /dev/ttyACM0 -b 115200
```

Or use your preferred serial terminal.

## Expected Output

The application will continuously perform SPI operations and display the results:

```
*** Booting Zephyr OS build v3.x.x ***
[00:00:00.001,000] <inf> app_spi: SPI Device Driver Example Started
[00:00:00.001,000] <inf> app_spi: ========================================
[00:00:00.002,000] <inf> spi_driver: SPI device initialized successfully
[00:00:00.002,000] <inf> spi_driver: Frequency: 1000000 Hz
[00:00:00.003,000] <inf> app_spi: SPI initialized successfully
[00:00:00.003,000] <inf> app_spi: Starting SPI communication tests...

[00:00:00.004,000] <inf> app_spi: Test iteration: 0
[00:00:00.004,000] <inf> app_spi: ----------------------------------------
[00:00:00.005,000] <inf> app_spi: Test 1: SPI Write Operation
[00:00:00.005,000] <inf> app_spi: TX Data:
0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07
[00:00:00.106,000] <inf> app_spi: Write test passed

[00:00:00.107,000] <inf> app_spi: Test 2: SPI Full-Duplex Transfer
[00:00:00.107,000] <inf> app_spi: TX Data:
0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07
[00:00:00.108,000] <inf> app_spi: Transfer test passed
[00:00:00.108,000] <inf> app_spi: RX Data:
0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07
[00:00:00.109,000] <inf> app_spi: Loopback verification: PASSED
[00:00:00.109,000] <inf> app_spi: ========================================
```

## Loopback Testing

To test the SPI driver without an external device:
1. Connect MISO (PA6) to MOSI (PA7) with a jumper wire
2. The loopback test will verify that transmitted data matches received data
3. If not connected, you'll see "Loopback verification: FAILED"

## Driver API

The driver provides the following functions:

### `int spi_device_init(void)`
Initializes the SPI device with the configuration from the device tree.

**Returns**: 0 on success, negative error code on failure

### `int spi_device_write(const uint8_t *tx_buf, size_t len)`
Writes data to the SPI device.

**Parameters**:
- `tx_buf`: Pointer to transmit buffer
- `len`: Number of bytes to transmit

**Returns**: 0 on success, negative error code on failure

### `int spi_device_read(uint8_t *rx_buf, size_t len)`
Reads data from the SPI device.

**Parameters**:
- `rx_buf`: Pointer to receive buffer
- `len`: Number of bytes to receive

**Returns**: 0 on success, negative error code on failure

### `int spi_device_transfer(const uint8_t *tx_buf, uint8_t *rx_buf, size_t len)`
Performs full-duplex SPI transfer (simultaneous transmit and receive).

**Parameters**:
- `tx_buf`: Pointer to transmit buffer
- `rx_buf`: Pointer to receive buffer
- `len`: Number of bytes to transfer

**Returns**: 0 on success, negative error code on failure

## Customization

### Changing SPI Frequency
Edit `boards/spi.overlay` and modify the `spi-max-frequency` property:
```dts
spi-max-frequency = <2000000>; /* 2 MHz */
```

### Changing SPI Mode
Edit `driver/src/spi.c` and modify the operation flags in `spi_device_init()`:
```c
spi_config.spi_cfg.operation = SPI_OP_MODE_MASTER |
                               SPI_MODE_CPOL |      /* Change these */
                               SPI_MODE_CPHA |      /* for different modes */
                               SPI_WORD_SET(8) |
                               SPI_LINES_SINGLE;
```

SPI Modes:
- **Mode 0**: CPOL=0, CPHA=0 (remove both flags)
- **Mode 1**: CPOL=0, CPHA=1 (SPI_MODE_CPHA only)
- **Mode 2**: CPOL=1, CPHA=0 (SPI_MODE_CPOL only)
- **Mode 3**: CPOL=1, CPHA=1 (both flags)

### Using with Real SPI Devices

To communicate with an actual SPI device (e.g., SPI EEPROM, sensor):
1. Update the device tree overlay to match your device's requirements
2. Implement device-specific command sequences in your application
3. Adjust timing and frequency as needed for your device

## Troubleshooting

### SPI device not ready
- Check that the SPI peripheral is enabled in your board's device tree
- Verify pin configuration matches your board

### Communication errors
- Verify SPI mode (CPOL/CPHA) matches your device
- Check frequency is within device limits
- Ensure proper wiring and connections
- Verify chip select polarity

### Build errors
- Ensure Zephyr environment is properly set up
- Check that your board supports SPI1
- Verify all required drivers are enabled in prj.conf

## References

- [Zephyr SPI Driver Documentation](https://docs.zephyrproject.org/latest/hardware/peripherals/spi.html)
- [SPI Protocol Overview](https://en.wikipedia.org/wiki/Serial_Peripheral_Interface)
- [STM32 SPI Reference Manual](https://www.st.com/resource/en/reference_manual/)

## License

This example is provided as-is for educational purposes.
