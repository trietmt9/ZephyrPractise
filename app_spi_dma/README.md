# SPI with DMA Device Driver Example

This example demonstrates how to use the SPI (Serial Peripheral Interface) driver with DMA (Direct Memory Access) support in Zephyr RTOS. DMA enables efficient data transfers without continuous CPU intervention, making it ideal for high-throughput applications.

## Overview

The example demonstrates:
- SPI device initialization with DMA support
- Synchronous SPI DMA write operations
- Synchronous SPI DMA full-duplex transfers
- Asynchronous SPI DMA transfers with callbacks
- Performance measurement and timing analysis
- Loopback testing capability
- CPU offloading benefits of DMA

## What is DMA?

Direct Memory Access (DMA) is a feature that allows hardware subsystems to access main memory independently of the CPU. For SPI:

### Benefits of DMA:
- **CPU Efficiency**: CPU can perform other tasks during data transfer
- **Higher Throughput**: Faster data transfer rates
- **Lower Latency**: Reduced interrupt overhead
- **Power Efficiency**: CPU can enter low-power states during transfers

### When to Use DMA:
- Large data transfers (typically > 16 bytes)
- High-frequency transfers
- When CPU needs to perform other tasks during transfer
- Real-time applications requiring predictable timing

## Hardware Requirements

- STM32 Nucleo board (or compatible board with SPI and DMA support)
- Optional: SPI device or loopback connection (connect MISO to MOSI)

## Pin Configuration

The example uses SPI1 with DMA2 on STM32:

### SPI Pins:
- **SCK (Clock)**: PA5 (SPI1_SCK)
- **MISO (Master In Slave Out)**: PA6 (SPI1_MISO)
- **MOSI (Master Out Slave In)**: PA7 (SPI1_MOSI)
- **CS (Chip Select)**: PA4 (GPIO controlled)

### DMA Configuration:
- **DMA Controller**: DMA2
- **TX Channel**: DMA2 Stream 3, Channel 3
- **RX Channel**: DMA2 Stream 0, Channel 3

## SPI Configuration

- **Frequency**: 2 MHz (increased from basic example)
- **Mode**: SPI Mode 3 (CPOL=1, CPHA=1)
- **Word Size**: 8 bits
- **Master Mode**: Yes
- **DMA**: Enabled for both TX and RX

## Project Structure

```
app_spi_dma/
├── boards/
│   └── spi_dma.overlay      # Device tree overlay with DMA config
├── driver/
│   ├── inc/
│   │   └── spi_dma.h       # SPI DMA driver header
│   └── src/
│       └── spi_dma.c       # SPI DMA driver implementation
├── src/
│   └── main.c              # Main application with tests
├── CMakeLists.txt          # Build configuration
├── prj.conf                # Project configuration (DMA enabled)
└── README.md               # This file
```

## Building and Running

### Build the application:

```bash
cd app_spi_dma
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

## Expected Output

The application will continuously perform SPI DMA operations with detailed timing information:

```
*** Booting Zephyr OS build v3.x.x ***
========================================
SPI DMA Device Driver Example Started
========================================
[00:00:00.001,000] <inf> spi_dma_driver: SPI DMA device initialized successfully
[00:00:00.001,000] <inf> spi_dma_driver: Frequency: 2000000 Hz
[00:00:00.002,000] <inf> spi_dma_driver: DMA enabled for optimized transfers
[00:00:00.002,000] <inf> app_spi_dma: SPI DMA initialized successfully
[00:00:00.003,000] <inf> app_spi_dma: Buffer size: 64 bytes
[00:00:00.003,000] <inf> app_spi_dma: Starting SPI DMA communication tests...

========================================
Test iteration: 0
========================================

[Test 1] Synchronous DMA Write Operation
TX Data (first 8 bytes):
  [0]: 0x00
  [1]: 0x01
  [2]: 0x02
  [3]: 0x03
  [4]: 0x04
  [5]: 0x05
  [6]: 0x06
  [7]: 0x07
Sync write test passed
Transfer time: 324 us (64 bytes)

[Test 2] Synchronous DMA Full-Duplex Transfer
TX Data (first 8 bytes):
  [0]: 0x00
  [1]: 0x01
  [2]: 0x02
  [3]: 0x03
  [4]: 0x04
  [5]: 0x05
  [6]: 0x06
  [7]: 0x07
Sync transfer test passed
Transfer time: 325 us (64 bytes)
RX Data (first 8 bytes):
  [0]: 0x00
  [1]: 0x01
  [2]: 0x02
  [3]: 0x03
  [4]: 0x04
  [5]: 0x05
  [6]: 0x06
  [7]: 0x07
Loopback verification: PASSED (all 64 bytes)

[Test 3] Asynchronous DMA Transfer with Callback
Initiating async transfer...
Async transfer initiated
Main thread can do other work while DMA transfers data...
  Main thread working... (1/3)
  Main thread working... (2/3)
  Main thread working... (3/3)
Async transfer completed successfully via callback
Async transfer completed
Total time (including wait): 350 us

[Performance Info]
DMA allows CPU to perform other tasks during transfer
Especially beneficial for large data transfers
Current transfer size: 64 bytes

========================================
Test iteration 0 completed
```

## Loopback Testing

To test without an external SPI device:
1. Connect MISO (PA6) to MOSI (PA7) with a jumper wire
2. The loopback test will verify that all 64 bytes match
3. Performance timing shows actual transfer speed

## Driver API

### Initialization

#### `int spi_dma_init(void)`
Initializes the SPI device with DMA support.

**Returns**: 0 on success, negative error code on failure

### Synchronous Operations

#### `int spi_dma_write(const uint8_t *tx_buf, size_t len)`
Writes data using DMA (blocking until complete).

**Parameters**:
- `tx_buf`: Pointer to transmit buffer
- `len`: Number of bytes to transmit

**Returns**: 0 on success, negative error code on failure

#### `int spi_dma_transfer(const uint8_t *tx_buf, uint8_t *rx_buf, size_t len)`
Performs full-duplex transfer using DMA (blocking).

**Parameters**:
- `tx_buf`: Pointer to transmit buffer
- `rx_buf`: Pointer to receive buffer
- `len`: Number of bytes to transfer

**Returns**: 0 on success, negative error code on failure

### Asynchronous Operations

#### `int spi_dma_write_async(const uint8_t *tx_buf, size_t len, spi_dma_callback_t callback, void *user_data)`
Initiates asynchronous write with callback notification.

**Parameters**:
- `tx_buf`: Pointer to transmit buffer
- `len`: Number of bytes to transmit
- `callback`: Function called on completion
- `user_data`: Optional user data passed to callback

**Returns**: 0 on success, negative error code on failure

#### `int spi_dma_transfer_async(const uint8_t *tx_buf, uint8_t *rx_buf, size_t len, spi_dma_callback_t callback, void *user_data)`
Initiates asynchronous full-duplex transfer with callback.

**Parameters**:
- `tx_buf`: Pointer to transmit buffer
- `rx_buf`: Pointer to receive buffer
- `len`: Number of bytes to transfer
- `callback`: Function called on completion
- `user_data`: Optional user data passed to callback

**Returns**: 0 on success, negative error code on failure

### Utility Functions

#### `bool spi_dma_is_transfer_complete(void)`
Checks if the current transfer is complete.

**Returns**: true if complete, false if still in progress

#### `int spi_dma_wait_complete(k_timeout_t timeout)`
Waits for transfer completion with optional timeout.

**Parameters**:
- `timeout`: Timeout value (use K_FOREVER for infinite wait)

**Returns**: 0 on success, -ETIMEDOUT on timeout

## Callback Function Type

```c
typedef void (*spi_dma_callback_t)(int status, void *user_data);
```

**Parameters**:
- `status`: 0 on success, negative error code on failure
- `user_data`: User data passed when initiating async operation

## Performance Comparison

### Without DMA (Polling/Interrupt):
- CPU busy during entire transfer
- Higher CPU utilization
- Suitable for small transfers

### With DMA:
- CPU free during transfer
- Lower CPU utilization
- Ideal for large buffers (64+ bytes)
- Better for real-time systems

### Typical Transfer Times (64 bytes @ 2 MHz):
- **DMA Transfer**: ~320-350 μs
- **CPU Overhead**: Minimal (<5%)

## Customization

### Changing Buffer Size

Edit `src/main.c`:
```c
#define TEST_DATA_SIZE    128    /* Increase for larger transfers */
```

### Changing SPI Frequency

Edit `boards/spi_dma.overlay`:
```dts
spi-max-frequency = <4000000>; /* 4 MHz */
```

### Changing DMA Channels

If using different SPI peripheral or board, update DMA configuration in `boards/spi_dma.overlay`:
```dts
dmas = <&dma2 3 3 0x20440 0x03>,  /* SPI1_TX */
       <&dma2 0 3 0x20480 0x03>;  /* SPI1_RX */
```

Refer to your MCU's reference manual for correct DMA stream/channel mapping.

### Changing SPI Mode

Edit `driver/src/spi_dma.c` in `spi_dma_init()`:
```c
spi_dma_config.spi_cfg.operation = SPI_OP_MODE_MASTER |
                                   /* Change these for different modes */
                                   SPI_MODE_CPOL |  /* Mode 0: remove */
                                   SPI_MODE_CPHA |  /* Mode 0: remove */
                                   SPI_WORD_SET(8) |
                                   SPI_LINES_SINGLE;
```

## Advanced Usage

### Integrating with Real SPI Devices

Example for SPI Flash memory:

```c
/* Write command to SPI flash */
uint8_t cmd[] = {0x02, 0x00, 0x00, 0x00}; /* Write command + address */
uint8_t data[256]; /* Page data */

/* Send command */
spi_dma_write(cmd, sizeof(cmd));

/* Write data using DMA */
spi_dma_transfer_async(data, NULL, sizeof(data),
                       flash_write_done, NULL);
```

### Using Async with Work Queues

```c
void process_data(struct k_work *work)
{
    /* Process received data */
}

K_WORK_DEFINE(data_work, process_data);

void rx_callback(int status, void *user_data)
{
    if (status == 0) {
        k_work_submit(&data_work);
    }
}

/* In main */
spi_dma_transfer_async(tx_buf, rx_buf, len, rx_callback, NULL);
```

## Troubleshooting

### DMA Not Working
- Verify DMA controller is enabled in device tree (`&dma2 { status = "okay"; }`)
- Check DMA stream/channel mapping for your MCU
- Ensure `CONFIG_DMA=y` and `CONFIG_SPI_STM32_DMA=y` in prj.conf

### Transfer Errors
- Verify buffer alignment (some DMA controllers require aligned buffers)
- Check buffer is not in stack (use static or heap for DMA)
- Ensure buffers remain valid during async operations

### Performance Issues
- Increase SPI frequency if device supports it
- Use larger buffers to amortize setup overhead
- Check DMA priority settings in device tree

### Build Errors
- Ensure your board has DMA support
- Verify DMA configuration matches your MCU's capabilities
- Check that SPI peripheral supports DMA

## Technical Notes

### DMA Configuration Flags

In the device tree overlay, DMA configuration uses these flags:
```
0x20440  /* TX: Peripheral-to-Memory, Memory increment, Priority high */
0x20480  /* RX: Memory-to-Peripheral, Memory increment, Priority high */
```

### Memory Considerations

- DMA buffers should be in RAM (not flash or stack for async)
- Consider using `__aligned(4)` for buffer declarations
- Ensure buffers remain valid during async operations

### STM32-Specific DMA Mapping

Different STM32 SPI peripherals use different DMA channels:
- **SPI1**: Typically DMA2
- **SPI2**: Typically DMA1
- **SPI3**: Varies by MCU

Consult your STM32 reference manual for exact mapping.

## References

- [Zephyr SPI Driver Documentation](https://docs.zephyrproject.org/latest/hardware/peripherals/spi.html)
- [Zephyr DMA Documentation](https://docs.zephyrproject.org/latest/hardware/peripherals/dma.html)
- [STM32 DMA Reference Manual](https://www.st.com/resource/en/reference_manual/)
- [Direct Memory Access (DMA) Overview](https://en.wikipedia.org/wiki/Direct_memory_access)

## Comparison with Basic SPI Example

| Feature | app_spi | app_spi_dma |
|---------|---------|-------------|
| DMA Support | No | Yes |
| Buffer Size | 8 bytes | 64 bytes |
| Async Operations | No | Yes |
| CPU Efficiency | Lower | Higher |
| Transfer Speed | 1 MHz | 2 MHz |
| Use Case | Simple, small transfers | Large transfers, high throughput |

## License

This example is provided as-is for educational purposes.
