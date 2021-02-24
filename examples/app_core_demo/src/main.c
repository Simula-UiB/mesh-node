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
    struct mesh_msg msg = {
        .data = data,
        .len = 12};

    k_msleep(2000); // Allow logs time to flush

    while (1)
    {
        // Send a message every two seconds
        k_msleep(2000);
        LOG_DBG("Sending mesh message. Length: %d", msg.len);
        mesh_send(msg);
    }
}

/**
 * @brief Mesh receive callback
 */
void mesh_receive(struct mesh_msg msg)
{
    LOG_INF("Received mesh message. Length: %d", msg.len);
    LOG_HEXDUMP_INF(msg.data, msg.len, "Message");
}
