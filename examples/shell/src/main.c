#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <logging/log.h>
#include <zephyr.h>

#include <logging/log.h>
#include <logging/log_ctrl.h>
#include <sys/printk.h>
#include <sys/util.h>

#include <shell/shell.h>

#include <common.h>
#include <mesh.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

int cmd_mesh_broadcast(const struct shell *shell, size_t argc, char **argv)
{
    uint8_t send_buf[MAX_PAYLOAD_SIZE];
    size_t len = hex2bin(argv[1], strlen(argv[1]), send_buf, MAX_PAYLOAD_SIZE);
    if (len == 0)
    {
        return -1;
    }
    return mesh_send_broadcast(send_buf, len);
}

int cmd_mesh_send(const struct shell *shell, size_t argc, char **argv)
{
    uint8_t send_buf[MAX_PAYLOAD_SIZE];
    uint8_t mac_buf[MAC_LEN];
    size_t len = hex2bin(argv[1], strlen(argv[1]), mac_buf, MAC_LEN);
    if (len == 0)
    {
        return -1;
    }
    len = hex2bin(argv[2], strlen(argv[2]), send_buf, MAX_PAYLOAD_SIZE);
    if (len == 0)
    {
        return -1;
    }
    return mesh_send(send_buf, mac_buf, len);
}

SHELL_STATIC_SUBCMD_SET_CREATE(
    sub_mesh,
    SHELL_CMD_ARG(send, NULL, "Send a packet to the stored destination", cmd_mesh_send, 3, 0),
    SHELL_CMD_ARG(broadcast, NULL, "Broadcast a packet", cmd_mesh_broadcast, 2, 0),
    SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(mesh, &sub_mesh, "Mesh commands", NULL);

void main(void)
{
    LOG_INF("Shell application on app core started.");
    init_mesh();
}

/**
 * @brief Mesh receive callback
 */
void mesh_receive(uint8_t *data, size_t len, uint8_t *src, bool broadcast)
{
    char hex[MAX_PAYLOAD_SIZE * 2];
    bin2hex(data, len, hex, MAX_PAYLOAD_SIZE * 2);
    printk("%s\n", hex);

    LOG_INF("Received mesh message. Length: %d. Broadcast: %s", len, broadcast ? "true" : "false");
    LOG_HEXDUMP_INF(src, MAC_LEN, "Source:");
    LOG_HEXDUMP_INF(data, len, "Message");
}
