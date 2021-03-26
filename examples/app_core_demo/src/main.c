#include <stdint.h>
#include <stdio.h>

#include <logging/log.h>
#include <zephyr.h>

#include <logging/log.h>
#include <logging/log_ctrl.h>
#include <sys/util.h>

#include <common.h>
#include <mesh.h>

LOG_MODULE_REGISTER(main, GLOBAL_LOG_LEVEL);

void main(void)
{
    LOG_INF("Example app on app core started.");

    uint8_t data[64];
    sprintf(data, "Hello world!");

    k_msleep(2000); // Allow logs time to flush

    while (1)
    {
        // Send a message every two seconds
        k_msleep(2000);
        LOG_DBG("Sending mesh message. Length: %d", 12);
        mesh_send(data, 12);
    }
}

/**
 * @brief Mesh receive callback
 */
void mesh_receive(uint8_t *data, size_t len)
{
    LOG_INF("Received mesh message. Length: %d", len);
    LOG_HEXDUMP_INF(data, len, "Message");
}
