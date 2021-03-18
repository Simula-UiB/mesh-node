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
#include <msg.h>

#include <ipc.h>
#include <node.h>
#include <radio.h>

LOG_MODULE_REGISTER(main, GLOBAL_LOG_LEVEL);

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

void main(void)
{
    /* NRFX init */
    init_power_clock();
    LOG_INF("Power and clock initialized");

    /* Radio init */
    init_radio();
    LOG_INF("Radio initialized");

    /* Radio init */
    init_node();
    LOG_INF("Node initialized");

    LOG_INF("Mesh node on network core started.");

    k_msleep(2000); // Allow logs time to flush
}

/**
 * @brief Callback for received radio frames
 */
void radio_receive_cb(uint8_t *data, size_t length)
{
    node_enqueue(data, length);
}

/**
 * @brief Callback for received IPC messages
 *
 * Forward messages to mesh network
 */
void ipc_receive_cb(struct message *msg)
{
    if (msg->payload_len > MAX_MESSAGE_SIZE + HEADER_LENGTH)
    {
        LOG_ERR("IPC message too large: %d", msg->payload_len);
        return;
    }
    LOG_DBG("ipc_receive_cb()");
    int ret = node_send(msg);
    if (ret != 0)
    {
        LOG_ERR("Node send failed with: %d", ret);
    }
}

void node_receive(struct message *msg)
{
    LOG_HEXDUMP_DBG(msg->payload, msg->payload_len, "Node RX data");
    ipc_send(msg);
}
