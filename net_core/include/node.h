#define MAX_HASH_COUNT_LIMIT 100

/**
 * @brief Add a message to a queue so that it can be procecedd later.
 */
void node_enqueue(struct message *msg);

/**
 * @brief Send a messsage over the radio with an added header.
 */
int node_send(struct message *msg);

/**
 * @brief Node receive callback.
 */
void node_receive(struct message *msg);

/**
 * @brief Node processing thread.
 */
void node_thread(void *p1, void *p2, void *p3);

/**
 * @brief Initialize node
 */
void init_node();