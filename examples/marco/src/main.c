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

uint8_t marco[8];
size_t len_marco;

uint8_t polo[8];
size_t len_polo;

void main(void)
{
    LOG_INF("Example app on app core started.");
    init_mesh();

    len_marco = sprintf(marco, "marco");
    len_polo = sprintf(polo, "polo");
}

int cmd_marco(const struct shell *shell, size_t argc, char **argv)
{
    mesh_send_broadcast(marco, len_marco);
    return 0;
}

SHELL_CMD_REGISTER(marco, NULL, "Send marco", cmd_marco);

/**
 * @brief Mesh receive callback
 */
void mesh_receive(uint8_t *data, size_t len, uint8_t *src, bool broadcast)
{
    if (memcmp(data, marco, len_marco) == 0)
    {
        // TODO Find out why this wait is needed. Witout it the receiver will
        // not receive the message. This only happens if this message was
        // received as broadcast.
        k_msleep(1);
        mesh_send(polo, src, len_polo);
    }
    else if (memcmp(data, polo, len_polo) == 0)
    {
        LOG_HEXDUMP_INF(src, MAC_LEN, "Got polo from source:");
    }
}
