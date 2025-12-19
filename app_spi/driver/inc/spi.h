#ifndef SPI_DRIVER_H
#define SPI_DRIVER_H

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>

/* SPI Configuration Structure */
struct spi_dev_config {
    const struct device *spi_dev;
    struct spi_config spi_cfg;
    struct spi_cs_control cs_ctrl;
};

/**
 * @brief Initialize SPI device
 *
 * @return 0 on success, negative error code on failure
 */
int spi_device_init(void);

/**
 * @brief Write data to SPI device
 *
 * @param tx_buf Pointer to transmit buffer
 * @param len Length of data to transmit
 * @return 0 on success, negative error code on failure
 */
int spi_device_write(const uint8_t *tx_buf, size_t len);

/**
 * @brief Read data from SPI device
 *
 * @param rx_buf Pointer to receive buffer
 * @param len Length of data to receive
 * @return 0 on success, negative error code on failure
 */
int spi_device_read(uint8_t *rx_buf, size_t len);

/**
 * @brief Write and read data simultaneously (full duplex)
 *
 * @param tx_buf Pointer to transmit buffer
 * @param rx_buf Pointer to receive buffer
 * @param len Length of data to transfer
 * @return 0 on success, negative error code on failure
 */
int spi_device_transfer(const uint8_t *tx_buf, uint8_t *rx_buf, size_t len);

#endif /* SPI_DRIVER_H */
