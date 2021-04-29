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

#include <radio.h>

#define MSG_SIZE 255
#define MSG_COUNT 1000

LOG_MODULE_REGISTER(main, GLOBAL_LOG_LEVEL);

static uint32_t bit_errors = 0;
static uint32_t packets_received = 0;
static uint32_t bits_received = 0;

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

    while (true)
    {
        k_msleep(2000);
        printk("packets: %d, bits_received: %d, bit_errors: %d\n", packets_received, bits_received, bit_errors);
    }
}

/**
 * @brief Callback for received radio frames
 */
void radio_receive_cb(uint32_t ones, uint32_t length)
{
    packets_received++;
    bit_errors += ones;
    bits_received += 8*length;
}
