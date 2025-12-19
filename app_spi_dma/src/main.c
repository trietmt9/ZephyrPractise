#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "spi_dma.h"

LOG_MODULE_REGISTER(app_spi_dma, CONFIG_LOG_MAX_LEVEL);

#define TEST_DATA_SIZE    64    /* Larger buffer to show DMA benefits */
#define LOOP_DELAY_MS     3000

/* Callback flag for async operations */
static volatile bool async_transfer_done = false;
static volatile int async_transfer_status = 0;

/* Async transfer callback */
static void transfer_complete_callback(int status, void *user_data)
{
    async_transfer_done = true;
    async_transfer_status = status;

    if (status == 0) {
        LOG_INF("Async transfer completed successfully via callback");
    } else {
        LOG_ERR("Async transfer failed with status: %d", status);
    }
}

int main(void)
{
    int ret;
    uint8_t tx_buffer[TEST_DATA_SIZE];
    uint8_t rx_buffer[TEST_DATA_SIZE];
    uint8_t test_counter = 0;
    uint32_t start_time, end_time, duration_us;

    LOG_INF("========================================");
    LOG_INF("SPI DMA Device Driver Example Started");
    LOG_INF("========================================");

    /* Initialize SPI DMA device */
    ret = spi_dma_init();
    if (ret < 0) {
        LOG_ERR("Failed to initialize SPI DMA device: %d", ret);
        return ret;
    }

    LOG_INF("SPI DMA initialized successfully");
    LOG_INF("Buffer size: %d bytes", TEST_DATA_SIZE);
    LOG_INF("Starting SPI DMA communication tests...\n");

    while (1) {
        /* Prepare test data */
        for (int i = 0; i < TEST_DATA_SIZE; i++) {
            tx_buffer[i] = test_counter + i;
            rx_buffer[i] = 0x00;  /* Clear receive buffer */
        }

        LOG_INF("========================================");
        LOG_INF("Test iteration: %d", test_counter);
        LOG_INF("========================================");

        /* Test 1: Synchronous SPI DMA Write */
        LOG_INF("\n[Test 1] Synchronous DMA Write Operation");
        LOG_INF("TX Data (first 8 bytes):");
        for (int i = 0; i < 8; i++) {
            LOG_INF("  [%d]: 0x%02X", i, tx_buffer[i]);
        }

        start_time = k_cyc_to_us_floor32(k_cycle_get_32());
        ret = spi_dma_write(tx_buffer, TEST_DATA_SIZE);
        end_time = k_cyc_to_us_floor32(k_cycle_get_32());
        duration_us = end_time - start_time;

        if (ret < 0) {
            LOG_ERR("Sync write test failed: %d", ret);
        } else {
            LOG_INF("Sync write test passed");
            LOG_INF("Transfer time: %u us (%d bytes)", duration_us, TEST_DATA_SIZE);
        }

        k_msleep(200);

        /* Test 2: Synchronous SPI DMA Full-Duplex Transfer */
        LOG_INF("\n[Test 2] Synchronous DMA Full-Duplex Transfer");
        LOG_INF("TX Data (first 8 bytes):");
        for (int i = 0; i < 8; i++) {
            LOG_INF("  [%d]: 0x%02X", i, tx_buffer[i]);
        }

        start_time = k_cyc_to_us_floor32(k_cycle_get_32());
        ret = spi_dma_transfer(tx_buffer, rx_buffer, TEST_DATA_SIZE);
        end_time = k_cyc_to_us_floor32(k_cycle_get_32());
        duration_us = end_time - start_time;

        if (ret < 0) {
            LOG_ERR("Sync transfer test failed: %d", ret);
        } else {
            LOG_INF("Sync transfer test passed");
            LOG_INF("Transfer time: %u us (%d bytes)", duration_us, TEST_DATA_SIZE);

            LOG_INF("RX Data (first 8 bytes):");
            for (int i = 0; i < 8; i++) {
                LOG_INF("  [%d]: 0x%02X", i, rx_buffer[i]);
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
                LOG_INF("Loopback verification: PASSED (all %d bytes)", TEST_DATA_SIZE);
            } else {
                LOG_WRN("Loopback verification: FAILED");
                LOG_WRN("(Connect MISO to MOSI for loopback test)");
            }
        }

        k_msleep(200);

        /* Test 3: Asynchronous SPI DMA Transfer with Callback */
        LOG_INF("\n[Test 3] Asynchronous DMA Transfer with Callback");

        /* Clear receive buffer */
        for (int i = 0; i < TEST_DATA_SIZE; i++) {
            rx_buffer[i] = 0x00;
        }

        async_transfer_done = false;
        async_transfer_status = 0;

        LOG_INF("Initiating async transfer...");
        start_time = k_cyc_to_us_floor32(k_cycle_get_32());

        ret = spi_dma_transfer_async(tx_buffer, rx_buffer, TEST_DATA_SIZE,
                                     transfer_complete_callback, NULL);

        if (ret < 0) {
            LOG_ERR("Failed to initiate async transfer: %d", ret);
        } else {
            LOG_INF("Async transfer initiated");
            LOG_INF("Main thread can do other work while DMA transfers data...");

            /* Simulate doing other work */
            for (int i = 0; i < 3; i++) {
                LOG_INF("  Main thread working... (%d/3)", i + 1);
                k_msleep(10);
            }

            /* Wait for completion with timeout */
            int wait_ret = spi_dma_wait_complete(K_MSEC(1000));
            end_time = k_cyc_to_us_floor32(k_cycle_get_32());
            duration_us = end_time - start_time;

            if (wait_ret == 0) {
                LOG_INF("Async transfer completed");
                LOG_INF("Total time (including wait): %u us", duration_us);

                if (async_transfer_status == 0) {
                    LOG_INF("RX Data (first 8 bytes):");
                    for (int i = 0; i < 8; i++) {
                        LOG_INF("  [%d]: 0x%02X", i, rx_buffer[i]);
                    }
                }
            } else {
                LOG_ERR("Async transfer timeout!");
            }
        }

        /* Test 4: Performance comparison info */
        LOG_INF("\n[Performance Info]");
        LOG_INF("DMA allows CPU to perform other tasks during transfer");
        LOG_INF("Especially beneficial for large data transfers");
        LOG_INF("Current transfer size: %d bytes", TEST_DATA_SIZE);

        LOG_INF("\n========================================");
        LOG_INF("Test iteration %d completed\n", test_counter);

        test_counter++;
        k_msleep(LOOP_DELAY_MS);
    }

    return 0;
}
