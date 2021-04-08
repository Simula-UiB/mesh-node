#include <message.h>

/**
 * @brief Send message through mesh node
 *
 * Message length must be less than MAX_MESSAGE_SIZE
 *
 * @param[in] msg Mesh message struct.
 *
 * @returns Number of bytes sent, or negative error code if there was an error.
 *
 */
int mesh_send_broadcast(uint8_t *data, size_t len);

int mesh_send(uint8_t *data, uint8_t *dst, size_t len);

/**
 * @brief Mesh receive callback
 *
 * @param[in] msg Mesh message struct
 */
void mesh_receive(uint8_t *data, size_t len, uint8_t *src, bool broadcast);
