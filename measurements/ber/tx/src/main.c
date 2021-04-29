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

#include <common.h>

#include <radio.h>

#define MSG_SIZE 255
#define MSG_COUNT 100000

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

    double lambda = 0.5;
    uint32_t count = 0;

    k_msleep(4000);

    while (true) 
    {
        uint8_t data[MSG_SIZE] = {0};
        k_usleep(10 + (int)(delay(lambda)*1000));
        radio_send(data, MSG_SIZE);
        LOG_INF("Sent zero packet. Length: %d", MSG_SIZE);
        if (++count >= MSG_COUNT) {
            break;
        }
    }
}
