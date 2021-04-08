#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <arch/cpu.h>
#include <drivers/ipm.h>
#include <logging/log.h>
#include <sys/byteorder.h>
#include <zephyr.h>

#include <logging/log.h>
#include <logging/log_ctrl.h>
#include <sys/util.h>

#include <metal/alloc.h>
#include <metal/device.h>
#include <metal/sys.h>
#include <openamp/open_amp.h>

#include <ipc/rpmsg_service.h>

#include <common.h>
#include <message.h>

#include <mesh.h>

LOG_MODULE_REGISTER(mesh_access, GLOBAL_LOG_LEVEL);

K_HEAP_DEFINE(mesh_heap, (sizeof(struct message) + MAX_MESSAGE_SIZE) * 10);

uint8_t mesh_send_buf[MAX_MESSAGE_SIZE];

static int endpoint_id;

int rpmsg_cb(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src,
             void *priv)
{
    struct message *message = message_from_buffer(&mesh_heap, data, len);

    /* Call callback function with mesh message */
    mesh_receive(message->payload, message->payload_len);

    k_heap_free(&mesh_heap, message->payload);
    k_heap_free(&mesh_heap, message);

    return RPMSG_SUCCESS;
}

int mesh_send(uint8_t *data, uint8_t dst[static 6], size_t len)
{
    if (len > MAX_MESSAGE_SIZE)
    {
        LOG_ERR("Mesh message too long");
        return -1;
    }

    struct message message = {
        .payload_len = len,
        .payload = data};
    memcpy(message.dst_mac, dst, 6);
    size_t size = message_to_buffer(mesh_send_buf, &message);
    /* Send IPC message. Returns number of bytes sent, or negative error code */
    LOG_HEXDUMP_INF(mesh_send_buf, len + HEADER_LENGTH, "sending:");
    return rpmsg_service_send(endpoint_id, mesh_send_buf, size);
}

int mesh_send_broadcast(uint8_t *data, size_t len)
{
    uint8_t broadcast_addr[6] = {0, 0, 0, 0, 0, 0};

    return mesh_send(data, broadcast_addr, len);
}

/* Register endpoint before RPMsg Service is initialized. */
int register_endpoint(const struct device *arg)
{
    int status;
    status = rpmsg_service_register_endpoint("mesh_node", rpmsg_cb);

    if (status < 0)
    {
        LOG_ERR("Registering endpoint failed with %d", status);
        return status;
    }

    endpoint_id = status;

    LOG_INF("RPMsg service registered. Endpoint id: %d", endpoint_id);

    return 0;
}

SYS_INIT(register_endpoint, POST_KERNEL, CONFIG_RPMSG_SERVICE_EP_REG_PRIORITY);
