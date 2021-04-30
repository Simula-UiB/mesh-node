#include <stdint.h>
#include <stdio.h>

#include <logging/log.h>
#include <zephyr.h>

#include <logging/log.h>
#include <logging/log_ctrl.h>
#include <sys/util.h>

#include <shell/shell.h>

#include <common.h>
#include <mesh.h>

LOG_MODULE_REGISTER(main, GLOBAL_LOG_LEVEL);

uint8_t marco[5];
uint8_t polo[4];

void main(void)
{
    LOG_INF("Example app on app core started.");
    init_mesh();

    sprintf(marco, "marco");
    sprintf(polo, "polo");
}

int cmd_marco(const struct shell *shell, size_t argc, char **argv)
{
    LOG_INF("sending");
    mesh_send_broadcast(marco, 5);
    return 0;
}

SHELL_CMD_REGISTER(marco, NULL, "Send marco", cmd_marco);

/**
 * @brief Mesh receive callback
 */
void mesh_receive(uint8_t *data, size_t len, uint8_t *src, bool broadcast)
{
    LOG_INF("Received mesh message. Length: %d. Broadcast: %s", len, broadcast ? "true" : "false");
    LOG_HEXDUMP_INF(src, MAC_LEN, "Source:");
    LOG_HEXDUMP_INF(data, len, "Message");
    if (memcmp(data, marco, len) == 0)
    {
        LOG_INF("replying");
        mesh_send(polo, src, 4);
    }
    else if (memcmp(data, polo, len) == 0)
    {
        LOG_HEXDUMP_INF(src, MAC_LEN, "Got polo from source:");
    }
}
