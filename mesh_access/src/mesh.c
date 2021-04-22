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

#include <mesh.h>

LOG_MODULE_REGISTER(mesh_access, GLOBAL_LOG_LEVEL);

uint8_t mesh_send_buf[MAX_MESSAGE_SIZE];

uint8_t broadcast_addr[MAC_LEN] = {[0 ... MAC_LEN - 1] = 0xff};

static int endpoint_id;

int rpmsg_cb(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src,
             void *priv)
{
    struct message *msg = message_from_buffer(data, len);

    /* Call callback function with mesh message */
    mesh_receive(msg->payload, msg->payload_len, msg->src_mac,
                 memcmp(msg->dst_mac, broadcast_addr, MAC_LEN) == 0);

    message_free(msg);

    return RPMSG_SUCCESS;
}

void init_mesh()
{
    while (!rpmsg_service_endpoint_is_bound(endpoint_id))
    {
        k_sleep(K_MSEC(1));
    }
    uint8_t data[0];
    rpmsg_service_send(endpoint_id, data, 0);
}

int mesh_send(uint8_t *data, uint8_t *dst, size_t len)
{
    if (len > MAX_MESSAGE_SIZE)
    {
        LOG_ERR("Mesh message too long");
        return -1;
    }

    struct message message = {
        .payload_len = len,
        .payload = data};
    memcpy(message.dst_mac, dst, MAC_LEN);
    size_t size = message_to_buffer(mesh_send_buf, &message);
    /* Send IPC message. Returns number of bytes sent, or negative error code */
    return rpmsg_service_send(endpoint_id, mesh_send_buf, size);
}

int mesh_send_broadcast(uint8_t *data, size_t len)
{
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
