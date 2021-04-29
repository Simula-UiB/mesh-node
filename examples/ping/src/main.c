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
}

/**
 * @brief Mesh receive callback
 */
void mesh_receive(uint8_t *data, size_t len, uint8_t *src, bool broadcast)
{
    LOG_INF("Received mesh message. Length: %d. Broadcast: %s", len, broadcast ? "true" : "false");
    LOG_HEXDUMP_INF(src, MAC_LEN, "Source:");
    LOG_HEXDUMP_INF(data, len, "Message");
    if (!broadcast)
    {
        mesh_send(data, src, len);
    }
}
