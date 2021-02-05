#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <zephyr.h>
#include <arch/cpu.h>
#include <sys/byteorder.h>
#include <logging/log.h>
#include <drivers/ipm.h>

#include <sys/util.h>
#include <logging/log.h>
#include <logging/log_ctrl.h>

#include <openamp/open_amp.h>
#include <metal/sys.h>
#include <metal/device.h>
#include <metal/alloc.h>

#include <ipc/rpmsg_service.h>

#include <common.h>
#include <mesh.h>


LOG_MODULE_REGISTER(mesh_access, GLOBAL_LOG_LEVEL);

static int endpoint_id;

int rpmsg_cb(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src, 
        void *priv)
{
    /* Fill mesh message struct */
    uint8_t msg_data[MAX_MESSAGE_SIZE];
    struct mesh_msg msg = {
        .data = msg_data,
        .len = len
    };
    memcpy(msg_data, data, len);

    /* Call callback function with mesh message */
    mesh_receive(msg);

    return RPMSG_SUCCESS;
}

int mesh_send(struct mesh_msg msg)
{
    if (msg.len > MAX_MESSAGE_SIZE)
    {
        LOG_ERR("Mesh message too long");
        return -1;
    }
    /* Send IPC message. Returns number of bytes sent, or negative error code */
    return rpmsg_service_send(endpoint_id, msg.data, msg.len);
}

/* Register endpoint before RPMsg Service is initialized. */
int register_endpoint(const struct device *arg)
{
    int status;
    status = rpmsg_service_register_endpoint("mesh_node", rpmsg_cb);

    if (status < 0) {
        LOG_ERR("Registering endpoint failed with %d", status);
        return status;
    }

    endpoint_id = status;

    LOG_INF("RPMsg service registered. Endpoint id: %d", endpoint_id);

    return 0;
}

SYS_INIT(register_endpoint, POST_KERNEL, CONFIG_RPMSG_SERVICE_EP_REG_PRIORITY);
