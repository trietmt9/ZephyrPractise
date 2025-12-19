#ifndef SPI_DMA_DRIVER_H
#define SPI_DMA_DRIVER_H

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/dma.h>

/* SPI DMA callback function type */
typedef void (*spi_dma_callback_t)(int status, void *user_data);

/* SPI DMA Configuration Structure */
struct spi_dma_config {
    const struct device *spi_dev;
    struct spi_config spi_cfg;
    struct spi_cs_control cs_ctrl;
    spi_dma_callback_t callback;
    void *user_data;
    struct k_sem transfer_sem;
    int last_status;
};

/**
 * @brief Initialize SPI device with DMA support
 *
 * @return 0 on success, negative error code on failure
 */
int spi_dma_init(void);

/**
 * @brief Write data to SPI device using DMA
 *
 * @param tx_buf Pointer to transmit buffer
 * @param len Length of data to transmit
 * @return 0 on success, negative error code on failure
 */
int spi_dma_write(const uint8_t *tx_buf, size_t len);

/**
 * @brief Write and read data simultaneously using DMA (full duplex)
 *
 * @param tx_buf Pointer to transmit buffer
 * @param rx_buf Pointer to receive buffer
 * @param len Length of data to transfer
 * @return 0 on success, negative error code on failure
 */
int spi_dma_transfer(const uint8_t *tx_buf, uint8_t *rx_buf, size_t len);

/**
 * @brief Asynchronous write with callback
 *
 * @param tx_buf Pointer to transmit buffer
 * @param len Length of data to transmit
 * @param callback Callback function to call on completion
 * @param user_data User data to pass to callback
 * @return 0 on success, negative error code on failure
 */
int spi_dma_write_async(const uint8_t *tx_buf, size_t len,
                        spi_dma_callback_t callback, void *user_data);

/**
 * @brief Asynchronous transfer with callback
 *
 * @param tx_buf Pointer to transmit buffer
 * @param rx_buf Pointer to receive buffer
 * @param len Length of data to transfer
 * @param callback Callback function to call on completion
 * @param user_data User data to pass to callback
 * @return 0 on success, negative error code on failure
 */
int spi_dma_transfer_async(const uint8_t *tx_buf, uint8_t *rx_buf, size_t len,
                           spi_dma_callback_t callback, void *user_data);

/**
 * @brief Check if SPI DMA transfer is complete
 *
 * @return true if transfer is complete, false otherwise
 */
bool spi_dma_is_transfer_complete(void);

/**
 * @brief Wait for ongoing SPI DMA transfer to complete
 *
 * @param timeout_ms Timeout in milliseconds (K_FOREVER for no timeout)
 * @return 0 on success, -ETIMEDOUT on timeout
 */
int spi_dma_wait_complete(k_timeout_t timeout);

#endif /* SPI_DMA_DRIVER_H */
