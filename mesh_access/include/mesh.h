/**
 * @brief Mesh message struct
 */
struct mesh_msg {
    size_t len;
    uint8_t * data;
};

/**
 * @brief Send message through mesh node
 *
 * Message length must be less than MAX_MESSAGE_SIZE
 *
 * @param[in] msg Mesh message struct.
 *
 * @returns 0 if message was sent. Non-zero if there was an error.
 *
 */
int mesh_send(struct mesh_msg msg);

/**
 * @brief Mesh receive callback
 *
 * @param[in] msg Mesh message struct
 */
void mesh_receive(struct mesh_msg msg);

