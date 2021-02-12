/**
 * @brief Add a message to a queue so that it can be procecedd later.
 */
void node_enqueue(struct ipc_msg msg);

/**
 * @brief Send a messsage over the radio with an added header.
 */
int node_send(uint8_t * data, uint8_t length);

/**
 * @brief Node receive callback.
 */
void node_receive(struct ipc_msg msg);

/**
 * @brief Node processing thread.
 */
void node_thread(void * p1, void * p2, void * p3);