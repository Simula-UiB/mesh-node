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

#include <device.h>
#include <drivers/gpio.h>

#include <common.h>

#include <radio.h>

#define L0_NODE DT_ALIAS(led0)

#define L0_GPIO_LABEL DT_GPIO_LABEL(L0_NODE, gpios)
#define L0_GPIO_PIN DT_GPIO_PIN(L0_NODE, gpios)
#define L0_GPIO_FLAGS (GPIO_OUTPUT | DT_GPIO_FLAGS(L0_NODE, gpios))

#define MSG_COUNT 1000
#define NODES 3
#define MAX_PKTS (MSG_COUNT*NODES)
#define PKT_LEN 10

LOG_MODULE_REGISTER(main, LOG_LEVEL_ERR);

static const struct device *led0;

static uint32_t packets_received = 0;
static uint32_t packet_index = 0;
static uint32_t bit_errors[MAX_PKTS] = {0};

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

    /* GPIO */
    led0 = device_get_binding(L0_GPIO_LABEL);
    gpio_pin_configure(led0, L0_GPIO_PIN, L0_GPIO_FLAGS);
    gpio_pin_set(led0, L0_GPIO_PIN, 0);
    LOG_INF("GPIO initialized");

    k_msleep(10000);

    /* How many times should we ask for packets */
    uint32_t rounds = 100;

    printk("STARTING\n");

    for (uint32_t i = 0; i < rounds; i++)
    {
        /* Tell tx to start sending */
        LOG_INF("Starting round %d", i);
        gpio_pin_set(led0, L0_GPIO_PIN, 1);
        k_msleep(10);
        gpio_pin_set(led0, L0_GPIO_PIN, 0);


        k_msleep(1000);

        uint32_t received = packets_received;
        while (true)
        {
            k_msleep(1000);
            if (received == packets_received)
            {
                LOG_INF("No packets received in last second. Received: %d", received);
                /* No packets received in a second, report results and go to next batch. */
                for (uint32_t j = 0; j < packet_index; j++)
                {
                    printk("%d\n", bit_errors[j]);
                    k_usleep(50);
                }
                break;
            }
            received = packets_received;
        }
        packet_index = 0;
    }
    printk("DONE\n");
}

/**
 * @brief Callback for received radio frames
 */
void radio_receive_cb(uint32_t ones, uint32_t length)
{
    if (length == PKT_LEN)
    {
        packets_received++;
        bit_errors[packet_index++] = ones;
    }
}
