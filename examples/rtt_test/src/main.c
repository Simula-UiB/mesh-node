#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <logging/log.h>
#include <zephyr.h>

#include <logging/log.h>
#include <logging/log_ctrl.h>
#include <sys/printk.h>
#include <sys/util.h>

#include <device.h>
#include <drivers/gpio.h>
#include <inttypes.h>

#include <shell/shell.h>

#include <common.h>
#include <mesh.h>

#define L0_NODE DT_ALIAS(led0)

#define L0_GPIO_LABEL DT_GPIO_LABEL(L0_NODE, gpios)
#define L0_GPIO_PIN DT_GPIO_PIN(L0_NODE, gpios)
#define L0_GPIO_FLAGS (GPIO_OUTPUT | DT_GPIO_FLAGS(L0_NODE, gpios))

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

static const struct device *led0;

static uint32_t tx_time = 0;

int cmd_send(const struct shell *shell, size_t argc, char **argv)
{
    if (argc < 2)
    {
        return -1;
    }
    uint8_t mac_buf[MAC_LEN];
    size_t len = hex2bin(argv[1], strlen(argv[1]), mac_buf, MAC_LEN);
    if (len != MAC_LEN)
    {
        return -1;
    }
    int length = 1;
    length = atoi(argv[2]);
    if (length == 0)
    {
        length = 1;
    }

    uint8_t data[MAX_PAYLOAD_SIZE];
    LOG_HEXDUMP_INF(data, length, "sent:");

    tx_time = k_cycle_get_32();
    mesh_send(data, mac_buf, length);

    k_msleep(10);
    gpio_pin_set(led0, L0_GPIO_PIN, 0);

    return 0;
}

SHELL_CMD_REGISTER(send, NULL, "Send test packet", cmd_send);

void main(void)
{
    LOG_INF("Latency test application on app core started.");
    init_mesh();

    led0 = device_get_binding(L0_GPIO_LABEL);
    gpio_pin_configure(led0, L0_GPIO_PIN, L0_GPIO_FLAGS);

    gpio_pin_set(led0, L0_GPIO_PIN, 0);
}

/**
 * @brief Mesh receive callback
 */
void mesh_receive(uint8_t *data, size_t len, uint8_t *src, bool broadcast)
{
    /* Record time immediately */
    uint32_t recv_time = k_cycle_get_32();

    uint32_t diff = recv_time - tx_time;
    if (recv_time < tx_time)
    {
        /* Correct overflow */
        diff = (UINT32_MAX - tx_time) + recv_time;
    }
    /* Convert to microseconds */
    uint32_t latency = k_cyc_to_us_floor32(diff);
    printk("rtt: %d\n", latency);

    LOG_INF("Received mesh message. Length: %d. Broadcast: %s", len, broadcast ? "true" : "false");
    LOG_HEXDUMP_INF(src, MAC_LEN, "Source:");
    LOG_HEXDUMP_INF(data, len, "Message");
}
