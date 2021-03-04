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
#include <msg.h>
#include <ipc.h>

LOG_MODULE_REGISTER(ipc, GLOBAL_LOG_LEVEL);

K_HEAP_DEFINE(ipc_heap, (sizeof(struct mesh_msg)+MAX_MESSAGE_SIZE)*10);

static int endpoint_id;

int rpmsg_cb(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src,
             void *priv)
{
    /* Fill IPC message struct */
    struct mesh_msg *msg = (struct mesh_msg*) k_heap_alloc(&ipc_heap, sizeof(struct mesh_msg), K_NO_WAIT);
    if (msg == NULL)
    {
        LOG_ERR("Cannot allocate heap memory");
        return RPMSG_ERR_NO_MEM;
    }
    msg->data = (uint8_t*) k_heap_alloc(&ipc_heap, len, K_NO_WAIT);
    if (msg->data == NULL)
    {
        LOG_ERR("Cannot allocate heap memory");
        return RPMSG_ERR_NO_MEM;
    }
    msg->len = len;
    memcpy(msg->data, data, len);

    /* Call callback function with ipc message */
    ipc_receive_cb(msg);

    k_heap_free(&ipc_heap, msg->data);
    k_heap_free(&ipc_heap, msg);

    return RPMSG_SUCCESS;
}

int ipc_send(struct mesh_msg *msg)
{
    int ret = rpmsg_service_send(endpoint_id, msg->data, msg->len);
    if (ret < 0)
    {
        return ret;
    }
    return 0;
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
