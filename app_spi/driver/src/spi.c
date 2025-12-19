#include "spi.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(spi_driver, LOG_LEVEL_DBG);

#define SPI_NODE DT_NODELABEL(spi_device)

static struct spi_dev_config spi_config;

int spi_device_init(void)
{
    int ret;

    /* Get SPI device */
    spi_config.spi_dev = DEVICE_DT_GET(DT_BUS(SPI_NODE));
    if (!device_is_ready(spi_config.spi_dev)) {
        LOG_ERR("SPI device not ready");
        return -ENODEV;
    }

    /* Configure CS control */
    spi_config.cs_ctrl.gpio = SPI_CS_GPIOS_DT_SPEC_GET(SPI_NODE);
    if (!spi_cs_is_gpio_dt(&spi_config.cs_ctrl)) {
        LOG_ERR("SPI CS is not a GPIO");
        return -EINVAL;
    }

    /* Configure SPI parameters */
    spi_config.spi_cfg.frequency = DT_PROP(SPI_NODE, spi_max_frequency);
    spi_config.spi_cfg.operation = SPI_OP_MODE_MASTER |
                                   SPI_MODE_CPOL |
                                   SPI_MODE_CPHA |
                                   SPI_WORD_SET(8) |
                                   SPI_LINES_SINGLE;
    spi_config.spi_cfg.slave = DT_REG_ADDR(SPI_NODE);
    spi_config.spi_cfg.cs = &spi_config.cs_ctrl;

    LOG_INF("SPI device initialized successfully");
    LOG_INF("Frequency: %d Hz", spi_config.spi_cfg.frequency);

    return 0;
}

int spi_device_write(const uint8_t *tx_buf, size_t len)
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

    ret = spi_write_dt(&(struct spi_dt_spec){
        .bus = spi_config.spi_dev,
        .config = spi_config.spi_cfg
    }, &tx_buf_set);

    if (ret < 0) {
        LOG_ERR("SPI write failed: %d", ret);
        return ret;
    }

    LOG_DBG("SPI write successful: %d bytes", len);
    return 0;
}

int spi_device_read(uint8_t *rx_buf, size_t len)
{
    int ret;
    struct spi_buf rx_spi_buf = {
        .buf = rx_buf,
        .len = len
    };
    struct spi_buf_set rx_buf_set = {
        .buffers = &rx_spi_buf,
        .count = 1
    };

    ret = spi_read_dt(&(struct spi_dt_spec){
        .bus = spi_config.spi_dev,
        .config = spi_config.spi_cfg
    }, &rx_buf_set);

    if (ret < 0) {
        LOG_ERR("SPI read failed: %d", ret);
        return ret;
    }

    LOG_DBG("SPI read successful: %d bytes", len);
    return 0;
}

int spi_device_transfer(const uint8_t *tx_buf, uint8_t *rx_buf, size_t len)
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

    ret = spi_transceive_dt(&(struct spi_dt_spec){
        .bus = spi_config.spi_dev,
        .config = spi_config.spi_cfg
    }, &tx_buf_set, &rx_buf_set);

    if (ret < 0) {
        LOG_ERR("SPI transfer failed: %d", ret);
        return ret;
    }

    LOG_DBG("SPI transfer successful: %d bytes", len);
    return 0;
}
