#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "spi.h"

LOG_MODULE_REGISTER(app_spi, CONFIG_LOG_MAX_LEVEL);

#define TEST_DATA_SIZE    8
#define LOOP_DELAY_MS     2000

int main(void)
{
    int ret;
    uint8_t tx_buffer[TEST_DATA_SIZE];
    uint8_t rx_buffer[TEST_DATA_SIZE];
    uint8_t test_counter = 0;

    LOG_INF("SPI Device Driver Example Started");
    LOG_INF("========================================");

    /* Initialize SPI device */
    ret = spi_device_init();
    if (ret < 0) {
        LOG_ERR("Failed to initialize SPI device: %d", ret);
        return ret;
    }

    LOG_INF("SPI initialized successfully");
    LOG_INF("Starting SPI communication tests...\n");

    while (1) {
        /* Prepare test data */
        for (int i = 0; i < TEST_DATA_SIZE; i++) {
            tx_buffer[i] = test_counter + i;
            rx_buffer[i] = 0x00;  /* Clear receive buffer */
        }

        LOG_INF("Test iteration: %d", test_counter);
        LOG_INF("----------------------------------------");

        /* Test 1: SPI Write */
        LOG_INF("Test 1: SPI Write Operation");

        for (int i = 0; i < TEST_DATA_SIZE; i++) {
            LOG_INF("TX Data: 0x%02X ", tx_buffer[i]);
        }

        ret = spi_device_write(tx_buffer, TEST_DATA_SIZE);
        if (ret < 0) {
            LOG_ERR("Write test failed: %d", ret);
        } else {
            LOG_INF("Write test passed");
        }

        k_msleep(100);

        /* Test 2: SPI Full-Duplex Transfer (Loopback test) */
        LOG_INF("\nTest 2: SPI Full-Duplex Transfer");
        for (int i = 0; i < TEST_DATA_SIZE; i++) {
            LOG_INF("TX Data: 0x%02X ", tx_buffer[i]);
        }

        ret = spi_device_transfer(tx_buffer, rx_buffer, TEST_DATA_SIZE);
        if (ret < 0) {
            LOG_ERR("Transfer test failed: %d", ret);
        } else {
            LOG_INF("Transfer test passed");

            for (int i = 0; i < TEST_DATA_SIZE; i++) {
                LOG_INF("RX Data: 0x%02X ", rx_buffer[i]);
            }

            /* Verify loopback (if MISO and MOSI are connected) */
            bool loopback_success = true;
            for (int i = 0; i < TEST_DATA_SIZE; i++) {
                if (tx_buffer[i] != rx_buffer[i]) {
                    loopback_success = false;
                    break;
                }
            }

            if (loopback_success) {
                LOG_INF("Loopback verification: PASSED");
            } else {
                LOG_WRN("Loopback verification: FAILED (connect MISO to MOSI for loopback)");
            }
        }

        LOG_INF("========================================\n");

        test_counter++;
        k_msleep(LOOP_DELAY_MS);
    }

    return 0;
}
