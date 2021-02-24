#include <stdio.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <logging/log.h>
#include <logging/log_ctrl.h>
#include <zephyr.h>

#include <nrfx.h>
#include <nrfx_clock.h>
#include <nrfx_power.h>

#include <common.h>
#include <ipc.h>
#include <radio.h>

LOG_MODULE_REGISTER(main, GLOBAL_LOG_LEVEL);

#define THREAD_STACK_SIZE 2048
#define THREAD_PRIORITY 5

K_THREAD_STACK_DEFINE(radio_rx_stack_area, THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(radio_tx_stack_area, THREAD_STACK_SIZE);
struct k_thread radio_rx_thread_data;
struct k_thread radio_tx_thread_data;

/* Define IPC message queues (ring buffer), with size 10 */
K_MSGQ_DEFINE(ipc_rx_msgq, sizeof(struct ipc_msg), 10, 4);

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
 *
 * Forwards messages from IPC layer to Radio layer
 */
void radio_tx_thread(void *p1, void *p2, void *p3)
{
    LOG_INF("USB to Radio thread started");
    k_msleep(500);

    struct ipc_msg msg;

    while (true)
    {
        k_msgq_get(&ipc_rx_msgq, &msg, K_FOREVER);
        int ret = radio_send(msg.data, msg.len);
        if (ret != 0)
        {
            LOG_ERR("Radio send failed with: %d", ret);
        }
    }
}

/**
 * @brief Radio RX thread entrypoint
 *
 * Forwards messages from radio layer to IPC layer
 */
void radio_rx_thread(void *p1, void *p2, void *p3)
{
    LOG_INF("Radio to USB thread started");
    k_msleep(500);
    uint8_t radio_rx[MAX_MESSAGE_SIZE];
    struct ipc_msg msg = {
        .data = radio_rx};

    while (true)
    {
        /* Wait for received frame from radio */
        LOG_DBG("Calling radio receive");
        size_t len = radio_receive(radio_rx, MAX_MESSAGE_SIZE);
        LOG_HEXDUMP_DBG(radio_rx, len, "Radio RX data");
        /* Send received frame over IPC */
        msg.len = len;
        ipc_send(msg);
    }
}

void main(void)
{
    /* NRFX init */
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

/**
 * @brief Callback for received IPC messages
 *
 * Forward messages to mesh network (currently directly to radio)
 */
void ipc_receive(struct ipc_msg msg)
{
    while (k_msgq_put(&ipc_rx_msgq, &msg, K_NO_WAIT) != 0)
    {
        /* message queue is full: purge old data & try again */
        k_msgq_purge(&ipc_rx_msgq);
    }
}
