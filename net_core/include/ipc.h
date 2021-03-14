/**
 * @brief Send message to other core with IPC
 */
int ipc_send(struct message *msg);

/**
 * @brief IPC receive callback
 */
void ipc_receive_cb(struct message *msg);
