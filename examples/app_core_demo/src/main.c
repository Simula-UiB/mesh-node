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
    init_mesh();

    uint8_t data[MAX_PAYLOAD_SIZE];
    sprintf(data, "Hello world!");
    size_t len = strlen(data);

    k_msleep(2000); // Allow logs time to flush

    while (1)
    {
        // Send a message every two seconds
        k_msleep(2000);
        LOG_DBG("Sending mesh message. Length: %d", len);
        mesh_send_broadcast(data, len);
    }
}

/**
 * @brief Mesh receive callback
 */
void mesh_receive(uint8_t *data, size_t len, uint8_t *src, bool broadcast)
{
    LOG_INF("Received mesh message. Length: %d. Broadcast: %s", len, broadcast ? "true" : "false");
    LOG_HEXDUMP_INF(src, MAC_LEN, "Source:");
    LOG_HEXDUMP_INF(data, len, "Message");
}
