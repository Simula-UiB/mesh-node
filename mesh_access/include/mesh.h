#include <message.h>

/**
 * @brief Initialize the mesh library.
 *
 * Must be ran before messages can be received.
 * 
 */
void init_mesh();

/**
 * @brief Broadcast a message through mesh network.
 *
 * Message length must be less than MAX_PAYLOAD_SIZE
 *
 * @param[in] data Array containing the message.
 *
 * @param[in] len The length of the message.
 * 
 * @returns Number of bytes sent, or negative error code if there was an error.
 *
 */
int mesh_send_broadcast(uint8_t *data, size_t len);

/**
 * @brief Send message to a given address through the mesh network.
 *
 * Message length must be less than MAX_PAYLOAD_SIZE
 *
 * @param[in] data Array containing the message.
 * 
 * @param[in] dst Array of length MAC_LEN containing the receiver address.
 *
 * @param[in] len The length of the message.
 *
 * @returns Number of bytes sent, or negative error code if there was an error.
 *
 */
int mesh_send(uint8_t *data, uint8_t *dst, size_t len);

/**
 * @brief Mesh receive callback
 *
 * @param[in] data Array containing the message.
 *
 * @param[in] len The length of the message.
 * 
 * @param[in] src Array of length MAC_LEN containing the sender address.
 *
 * @param[in] broadcast Is true if the message was sent as broadcast.
 * 
 */
void mesh_receive(uint8_t *data, size_t len, uint8_t *src, bool broadcast);
