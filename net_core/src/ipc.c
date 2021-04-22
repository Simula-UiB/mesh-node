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

#include <ipc.h>

LOG_MODULE_REGISTER(ipc, GLOBAL_LOG_LEVEL);

uint8_t ipc_send_buf[MAX_MESSAGE_SIZE];

static int endpoint_id;

int rpmsg_cb(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src,
             void *priv)
{
    if (len == 0)
    {
        return RPMSG_SUCCESS;
    }
    /* Fill IPC message struct */
    struct message *msg = message_from_buffer(data, len);
    if (msg == NULL)
    {
        LOG_INF("Failed to parse msg.");
        return RPMSG_ERR_NO_MEM;
    }

    /* Call callback function with ipc message */
    ipc_receive_cb(msg);

    return RPMSG_SUCCESS;
}

int ipc_send(struct message *msg)
{
    size_t size = message_to_buffer(ipc_send_buf, msg);
    return rpmsg_service_send(endpoint_id, ipc_send_buf, size);
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
