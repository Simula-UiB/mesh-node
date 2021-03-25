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

#define L1_NODE DT_ALIAS(led1)

#define L1_GPIO_LABEL DT_GPIO_LABEL(L1_NODE, gpios)
#define L1_GPIO_PIN DT_GPIO_PIN(L1_NODE, gpios)
#define L1_GPIO_FLAGS (GPIO_INPUT | DT_GPIO_FLAGS(L1_NODE, gpios))

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

static struct gpio_callback gpio_cb_data;
static const struct device *led0, *led1;

static uint32_t tx_time = 0;

void gpio_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    tx_time = k_cycle_get_32();
}

int cmd_send(const struct shell *shell, size_t argc, char **argv)
{
    int length = 1;
    if (argc > 1)
    {
        length = atoi(argv[1]);
        if (length == 0)
        {
            length = 1;
        }
    }

    uint8_t data[MAX_MESSAGE_SIZE];
    struct mesh_msg msg = {
        .data = data,
        .len = length};
    LOG_HEXDUMP_INF(msg.data, msg.len, "sent:");

    gpio_pin_set(led0, L0_GPIO_PIN, 1);
    mesh_send(msg);

    k_msleep(10);
    gpio_pin_set(led0, L0_GPIO_PIN, 0);

    return 0;
}

SHELL_CMD_REGISTER(send, NULL, "Send test packet", cmd_send);

void main(void)
{
    LOG_INF("Latency test application on app core started.");

    led0 = device_get_binding(L0_GPIO_LABEL);
    gpio_pin_configure(led0, L0_GPIO_PIN, L0_GPIO_FLAGS);

    led1 = device_get_binding(L1_GPIO_LABEL);
    gpio_pin_configure(led1, L1_GPIO_PIN, L1_GPIO_FLAGS);

    gpio_pin_interrupt_configure(led1, L1_GPIO_PIN, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&gpio_cb_data, gpio_callback, BIT(L1_GPIO_PIN));

    gpio_add_callback(led1, &gpio_cb_data);

    gpio_pin_set(led0, L0_GPIO_PIN, 0);
}

/**
 * @brief Mesh receive callback
 */
void mesh_receive(struct mesh_msg msg)
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
    printk("latency: %d\n", latency);

    LOG_INF("Received mesh message. Length: %d", msg.len);
    LOG_HEXDUMP_INF(msg.data, msg.len, "Message");
}
