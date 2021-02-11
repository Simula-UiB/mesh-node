/**
 * @brief Add a message to a queue so that it can be procecedd later.
 */
void node_enqueue(struct ipc_msg msg);

/**
 * @brief Process packets that have been received by either forwarding,
 * discarding or returning them.
 */
int node_process_packets(uint8_t * data, uint8_t max_length);

/**
 * @brief Send a messsage over the radio with an added header.
 */
int node_send(uint8_t * data, uint8_t length);