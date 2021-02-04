#include <net/buf.h>

/**
 * @brief Initialize RPMsg
 */
int rpmsg_platform_init(void);

/**
 * @brief Send message using RPMsg
 */
int rpmsg_platform_send(struct net_buf_simple *buf, size_t len);

/**
 * @brief RPMsg receive callback
 */
void rpmsg_rx(uint8_t *data, size_t len);

