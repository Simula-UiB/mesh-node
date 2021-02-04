/**
 * @brief Mesh message struct
 */
struct mesh_msg {
    size_t len;
    uint8_t * data;
};

/**
 * @brief Send message through mesh node
 */
int mesh_send(struct mesh_msg msg);

/**
 * @brief Mesh receive callback
 */
void mesh_receive(struct mesh_msg msg);

