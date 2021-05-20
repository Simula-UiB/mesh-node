#include <stdio.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>

#include <logging/log.h>
#include <logging/log_ctrl.h>
#include <random/rand32.h>
#include <math.h>
#include <zephyr.h>


#include <nrfx.h>
#include <nrfx_clock.h>
#include <nrfx_power.h>

#include <device.h>
#include <drivers/gpio.h>

#include <common.h>
#include <radio.h>

#define L0_NODE DT_ALIAS(led0)

#define L0_GPIO_LABEL DT_GPIO_LABEL(L0_NODE, gpios)
#define L0_GPIO_PIN DT_GPIO_PIN(L0_NODE, gpios)
#define L0_GPIO_FLAGS (GPIO_INPUT | DT_GPIO_FLAGS(L0_NODE, gpios))

#define MSG_SIZE 10
#define MSG_COUNT 1000

LOG_MODULE_REGISTER(main, GLOBAL_LOG_LEVEL);

static struct gpio_callback gpio_cb_data;
static const struct device *led0;

static bool sending = false;

void gpio_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    sending = true;
}

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
 * @brief Exponential random delay
 */
static double delay(double lambda){
    double u = (float)sys_rand32_get() / ((float)UINT32_MAX);
    return -log(1 - u) / lambda;
}

void main(void)
{
    /* NRFX init */
    init_power_clock();
    LOG_INF("Power and clock initialized");

    /* Radio init */
    init_radio();
    LOG_INF("Radio initialized");

    /* GPIO */
    led0 = device_get_binding(L0_GPIO_LABEL);
    gpio_pin_configure(led0, L0_GPIO_PIN, L0_GPIO_FLAGS);
    gpio_pin_interrupt_configure(led0, L0_GPIO_PIN, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&gpio_cb_data, gpio_callback, BIT(L0_GPIO_PIN));
    gpio_add_callback(led0, &gpio_cb_data);
    gpio_pin_set(led0, L0_GPIO_PIN, 0);

    double alpha = 1;
    double lambda = 1/(0.5*(MSG_SIZE+1)*alpha);
    uint32_t count = 0;

    k_msleep(4000);
    
    uint8_t data[MSG_SIZE] = {0};

    while (true) 
    {
        k_usleep((int)(delay(lambda)));
        if (sending)
        {
            radio_send(data, MSG_SIZE);
            LOG_INF("Sent zero packet. Length: %d", MSG_SIZE);
            if (++count >= MSG_COUNT) {
                sending = false;
                count = 0;
            }
        }
    }
}
