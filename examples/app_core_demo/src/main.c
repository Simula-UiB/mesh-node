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

#include <common.h>

#include <openamp/open_amp.h>
#include <metal/sys.h>
#include <metal/device.h>
#include <metal/alloc.h>

#include <ipc/rpmsg_service.h>

LOG_MODULE_REGISTER(main, GLOBAL_LOG_LEVEL);

static int endpoint_id;

int rpmsg_cb(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src, 
        void *priv)
{
    LOG_INF("Received message. Length: %d", len);
    LOG_HEXDUMP_INF(data, len, "Message:");

    return RPMSG_SUCCESS;
}

void main(void)
{
    LOG_INF("Example app on app core started.");

    uint8_t msg[64];
    sprintf(msg, "Hello");
    
    k_msleep(2000); // Allow logs time to flush

    while(1)
    {
        // Send a message every two seconds
        k_msleep(2000);
        rpmsg_service_send(endpoint_id, msg, 5);
    }

}

/* Make sure we register endpoint before RPMsg Service is initialized. */
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
