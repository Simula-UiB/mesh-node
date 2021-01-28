#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <zephyr.h>
#include <logging/log.h>
#include <logging/log_ctrl.h>

#include <common.h>
#include <radio.h>

#include <nrfx.h>
#include <nrfx_clock.h>
#include <nrfx_power.h>

LOG_MODULE_REGISTER(main, GLOBAL_LOG_LEVEL);

#define THREAD_STACK_SIZE 2048
#define THREAD_PRIORITY 5

K_THREAD_STACK_DEFINE(radio_rx_stack_area, THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(radio_tx_stack_area, THREAD_STACK_SIZE);
struct k_thread radio_rx_thread_data;
struct k_thread radio_tx_thread_data;

/**
 * @brief Initialize power and clock peripherals
 */
static void init_power_clock(void)
{
    nrfx_clock_init(NULL);
    nrfx_power_init(NULL);
    nrfx_clock_start(NRF_CLOCK_DOMAIN_HFCLK);
    nrfx_clock_start(NRF_CLOCK_DOMAIN_LFCLK);
    while (!(nrfx_clock_hfclk_is_running() &&
            nrfx_clock_lfclk_is_running()))
    {
        /* Just waiting */
    }
}

/**
 * @brief Radio TX thread entrypoint
 */
void radio_tx_thread(void * p1, void * p2, void * p3)
{
    LOG_INF("USB to Radio thread started");
    k_msleep(500);
    
    uint8_t rx[MAX_MESSAGE_SIZE];
    int len = 12;
    sprintf(rx, "Hello world!");

    while (true)
    {
        radio_send(rx, len);
        k_msleep(2000);
    }
}

/**
 * @brief Radio to USB thread entrypoint
 */
void radio_rx_thread(void * p1, void * p2, void * p3)
{
    LOG_INF("Radio to USB thread started");
    k_msleep(500);
    uint8_t radio_rx[MAX_MESSAGE_SIZE];
    int length;

    while (true)
    {
        LOG_DBG("Calling radio receive");
        length = radio_receive(radio_rx, MAX_MESSAGE_SIZE);
        LOG_HEXDUMP_DBG(radio_rx, length, "Radio RX data");
    }
}


void main(void)
{
    init_power_clock();
    LOG_INF("Power and clock initialized");

    /* Radio init */
    init_radio();
    LOG_INF("Radio initialized");

    /* Spawn threads */
    k_thread_create(&radio_rx_thread_data,
                    radio_rx_stack_area,
                    THREAD_STACK_SIZE,
                    radio_rx_thread,
                    NULL, NULL, NULL,
                    THREAD_PRIORITY,
                    0, K_NO_WAIT);

    k_thread_create(&radio_tx_thread_data,
                    radio_tx_stack_area,
                    THREAD_STACK_SIZE,
                    radio_tx_thread,
                    NULL, NULL, NULL,
                    THREAD_PRIORITY,
                    0, K_NO_WAIT);

    LOG_INF("Mesh node on network core started.");
    
    k_msleep(2000); // Allow logs time to flush
}
