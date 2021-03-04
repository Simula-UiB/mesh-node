/**
 * @brief Send message to other core with IPC
 */
int ipc_send(struct mesh_msg *msg);

/**
 * @brief IPC receive callback
 */
void ipc_receive_cb(struct mesh_msg *msg);

/**
 * @brief IPC init
 */
void ipc_init(struct k_heap *heap);
