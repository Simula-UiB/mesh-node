#include <stdint.h>
#include <stdio.h>

#include <logging/log.h>
#include <zephyr.h>

#include <logging/log.h>
#include <logging/log_ctrl.h>
#include <sys/printk.h>
#include <sys/util.h>

#include <device.h>
#include <drivers/gpio.h>
#include <inttypes.h>

#include <common.h>
#include <mesh.h>

#define SW0_NODE DT_ALIAS(sw0)

#define SW0_GPIO_LABEL DT_GPIO_LABEL(SW0_NODE, gpios)
#define SW0_GPIO_PIN DT_GPIO_PIN(SW0_NODE, gpios)
#define SW0_GPIO_FLAGS (GPIO_INPUT | DT_GPIO_FLAGS(SW0_NODE, gpios))

#define SW1_NODE DT_ALIAS(sw1)

#define SW1_GPIO_LABEL DT_GPIO_LABEL(SW1_NODE, gpios)
#define SW1_GPIO_PIN DT_GPIO_PIN(SW1_NODE, gpios)
#define SW1_GPIO_FLAGS (GPIO_INPUT | DT_GPIO_FLAGS(SW1_NODE, gpios))

#define SW2_NODE DT_ALIAS(sw2)

#define SW2_GPIO_LABEL DT_GPIO_LABEL(SW2_NODE, gpios)
#define SW2_GPIO_PIN DT_GPIO_PIN(SW2_NODE, gpios)
#define SW2_GPIO_FLAGS (GPIO_INPUT | DT_GPIO_FLAGS(SW2_NODE, gpios))

#define SW3_NODE DT_ALIAS(sw3)

#define SW3_GPIO_LABEL DT_GPIO_LABEL(SW3_NODE, gpios)
#define SW3_GPIO_PIN DT_GPIO_PIN(SW3_NODE, gpios)
#define SW3_GPIO_FLAGS (GPIO_INPUT | DT_GPIO_FLAGS(SW3_NODE, gpios))

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

static struct gpio_callback button_cb_data;

void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    uint8_t data[64];
    switch (pins)
    {
    case BIT(SW0_GPIO_PIN):
        sprintf(data, "Hello world from 1!");
        break;
    case BIT(SW1_GPIO_PIN):
        sprintf(data, "Hello world from 2!");
        break;
    case BIT(SW2_GPIO_PIN):
        sprintf(data, "Hello world from 3!");
        break;
    case BIT(SW3_GPIO_PIN):
        sprintf(data, "Hello world from 4!");
        break;
    }
    size_t len = strlen(data);
    LOG_HEXDUMP_INF(data, len, "sent:");
    mesh_send(data, len);
}

void main(void)
{
    LOG_INF("Example app on app core started.");
    const struct device *button0, *button1, *button2, *button3;

    button0 = device_get_binding(SW0_GPIO_LABEL);
    gpio_pin_configure(button0, SW0_GPIO_PIN, SW0_GPIO_FLAGS);
    gpio_pin_interrupt_configure(button0, SW0_GPIO_PIN, GPIO_INT_EDGE_TO_ACTIVE);

    button1 = device_get_binding(SW1_GPIO_LABEL);
    gpio_pin_configure(button1, SW1_GPIO_PIN, SW1_GPIO_FLAGS);
    gpio_pin_interrupt_configure(button1, SW1_GPIO_PIN, GPIO_INT_EDGE_TO_ACTIVE);

    button2 = device_get_binding(SW2_GPIO_LABEL);
    gpio_pin_configure(button2, SW2_GPIO_PIN, SW2_GPIO_FLAGS);
    gpio_pin_interrupt_configure(button2, SW2_GPIO_PIN, GPIO_INT_EDGE_TO_ACTIVE);

    button3 = device_get_binding(SW3_GPIO_LABEL);
    gpio_pin_configure(button3, SW3_GPIO_PIN, SW3_GPIO_FLAGS);
    gpio_pin_interrupt_configure(button2, SW3_GPIO_PIN, GPIO_INT_EDGE_TO_ACTIVE);

    gpio_init_callback(&button_cb_data, button_pressed, BIT(SW0_GPIO_PIN) | BIT(SW1_GPIO_PIN) | BIT(SW2_GPIO_PIN) | BIT(SW3_GPIO_PIN));

    gpio_add_callback(button0, &button_cb_data);
    gpio_add_callback(button1, &button_cb_data);
    gpio_add_callback(button2, &button_cb_data);
    gpio_add_callback(button3, &button_cb_data);
}

/**
 * @brief Mesh receive callback
 */
void mesh_receive(uint8_t *data, size_t len)
{
    LOG_INF("Received mesh message. Length: %d", len);
    LOG_HEXDUMP_INF(data, len, "Message");
}
