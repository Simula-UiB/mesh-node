/**
 * @brief Send message to other core with IPC
 */
int ipc_send(struct ipc_msg msg);

/**
 * @brief IPC receive callback
 */
void ipc_receive_cb(struct ipc_msg msg);
