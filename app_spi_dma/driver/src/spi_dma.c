#include "spi_dma.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(spi_dma_driver, LOG_LEVEL_DBG);

#define SPI_NODE DT_NODELABEL(spi_device)

static struct spi_dma_config spi_dma_config;
static volatile bool transfer_complete = true;

/* SPI signal for async operations */
static void spi_dma_signal_handler(const struct device *dev,
                                   int result,
                                   void *data)
{
    struct spi_dma_config *config = (struct spi_dma_config *)data;

    transfer_complete = true;
    config->last_status = result;

    /* Signal semaphore */
    k_sem_give(&config->transfer_sem);

    /* Call user callback if registered */
    if (config->callback) {
        config->callback(result, config->user_data);
    }

    if (result == 0) {
        LOG_DBG("DMA transfer completed successfully");
    } else {
        LOG_ERR("DMA transfer failed with error: %d", result);
    }
}

int spi_dma_init(void)
{
    int ret;

    /* Get SPI device */
    spi_dma_config.spi_dev = DEVICE_DT_GET(DT_BUS(SPI_NODE));
    if (!device_is_ready(spi_dma_config.spi_dev)) {
        LOG_ERR("SPI device not ready");
        return -ENODEV;
    }

    /* Configure CS control */
    spi_dma_config.cs_ctrl.gpio = SPI_CS_GPIOS_DT_SPEC_GET(SPI_NODE);
    if (!spi_cs_is_gpio_dt(&spi_dma_config.cs_ctrl)) {
        LOG_ERR("SPI CS is not a GPIO");
        return -EINVAL;
    }

    /* Configure SPI parameters */
    spi_dma_config.spi_cfg.frequency = DT_PROP(SPI_NODE, spi_max_frequency);
    spi_dma_config.spi_cfg.operation = SPI_OP_MODE_MASTER |
                                       SPI_MODE_CPOL |
                                       SPI_MODE_CPHA |
                                       SPI_WORD_SET(8) |
                                       SPI_LINES_SINGLE;
    spi_dma_config.spi_cfg.slave = DT_REG_ADDR(SPI_NODE);
    spi_dma_config.spi_cfg.cs = &spi_dma_config.cs_ctrl;

    /* Initialize semaphore for synchronous operations */
    k_sem_init(&spi_dma_config.transfer_sem, 0, 1);

    /* Initialize callback to NULL */
    spi_dma_config.callback = NULL;
    spi_dma_config.user_data = NULL;
    spi_dma_config.last_status = 0;

    LOG_INF("SPI DMA device initialized successfully");
    LOG_INF("Frequency: %d Hz", spi_dma_config.spi_cfg.frequency);
    LOG_INF("DMA enabled for optimized transfers");

    return 0;
}

int spi_dma_write(const uint8_t *tx_buf, size_t len)
{
    int ret;
    struct spi_buf tx_spi_buf = {
        .buf = (void *)tx_buf,
        .len = len
    };
    struct spi_buf_set tx_buf_set = {
        .buffers = &tx_spi_buf,
        .count = 1
    };

    transfer_complete = false;

    ret = spi_write_dt(&(struct spi_dt_spec){
        .bus = spi_dma_config.spi_dev,
        .config = spi_dma_config.spi_cfg
    }, &tx_buf_set);

    if (ret < 0) {
        LOG_ERR("SPI DMA write failed: %d", ret);
        transfer_complete = true;
        return ret;
    }

    transfer_complete = true;
    LOG_DBG("SPI DMA write successful: %d bytes", len);
    return 0;
}

int spi_dma_transfer(const uint8_t *tx_buf, uint8_t *rx_buf, size_t len)
{
    int ret;
    struct spi_buf tx_spi_buf = {
        .buf = (void *)tx_buf,
        .len = len
    };
    struct spi_buf rx_spi_buf = {
        .buf = rx_buf,
        .len = len
    };
    struct spi_buf_set tx_buf_set = {
        .buffers = &tx_spi_buf,
        .count = 1
    };
    struct spi_buf_set rx_buf_set = {
        .buffers = &rx_spi_buf,
        .count = 1
    };

    transfer_complete = false;

    ret = spi_transceive_dt(&(struct spi_dt_spec){
        .bus = spi_dma_config.spi_dev,
        .config = spi_dma_config.spi_cfg
    }, &tx_buf_set, &rx_buf_set);

    if (ret < 0) {
        LOG_ERR("SPI DMA transfer failed: %d", ret);
        transfer_complete = true;
        return ret;
    }

    transfer_complete = true;
    LOG_DBG("SPI DMA transfer successful: %d bytes", len);
    return 0;
}

int spi_dma_write_async(const uint8_t *tx_buf, size_t len,
                        spi_dma_callback_t callback, void *user_data)
{
    int ret;
    struct spi_buf tx_spi_buf = {
        .buf = (void *)tx_buf,
        .len = len
    };
    struct spi_buf_set tx_buf_set = {
        .buffers = &tx_spi_buf,
        .count = 1
    };

    /* Set callback and user data */
    spi_dma_config.callback = callback;
    spi_dma_config.user_data = user_data;
    transfer_complete = false;

    /* Reset semaphore */
    k_sem_reset(&spi_dma_config.transfer_sem);

    /* Note: Zephyr's SPI API doesn't have built-in async with callbacks,
     * but DMA is used automatically when configured. For true async operation,
     * you would need to use the DMA API directly or implement a work queue.
     * This implementation simulates async behavior. */

    ret = spi_write_dt(&(struct spi_dt_spec){
        .bus = spi_dma_config.spi_dev,
        .config = spi_dma_config.spi_cfg
    }, &tx_buf_set);

    if (ret < 0) {
        LOG_ERR("SPI DMA async write failed: %d", ret);
        transfer_complete = true;
        return ret;
    }

    /* Simulate async completion */
    spi_dma_signal_handler(spi_dma_config.spi_dev, ret, &spi_dma_config);

    LOG_DBG("SPI DMA async write initiated: %d bytes", len);
    return 0;
}

int spi_dma_transfer_async(const uint8_t *tx_buf, uint8_t *rx_buf, size_t len,
                           spi_dma_callback_t callback, void *user_data)
{
    int ret;
    struct spi_buf tx_spi_buf = {
        .buf = (void *)tx_buf,
        .len = len
    };
    struct spi_buf rx_spi_buf = {
        .buf = rx_buf,
        .len = len
    };
    struct spi_buf_set tx_buf_set = {
        .buffers = &tx_spi_buf,
        .count = 1
    };
    struct spi_buf_set rx_buf_set = {
        .buffers = &rx_spi_buf,
        .count = 1
    };

    /* Set callback and user data */
    spi_dma_config.callback = callback;
    spi_dma_config.user_data = user_data;
    transfer_complete = false;

    /* Reset semaphore */
    k_sem_reset(&spi_dma_config.transfer_sem);

    ret = spi_transceive_dt(&(struct spi_dt_spec){
        .bus = spi_dma_config.spi_dev,
        .config = spi_dma_config.spi_cfg
    }, &tx_buf_set, &rx_buf_set);

    if (ret < 0) {
        LOG_ERR("SPI DMA async transfer failed: %d", ret);
        transfer_complete = true;
        return ret;
    }

    /* Simulate async completion */
    spi_dma_signal_handler(spi_dma_config.spi_dev, ret, &spi_dma_config);

    LOG_DBG("SPI DMA async transfer initiated: %d bytes", len);
    return 0;
}

bool spi_dma_is_transfer_complete(void)
{
    return transfer_complete;
}

int spi_dma_wait_complete(k_timeout_t timeout)
{
    return k_sem_take(&spi_dma_config.transfer_sem, timeout);
}
