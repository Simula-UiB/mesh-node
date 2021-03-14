#define HEADER_LENGTH 22

#define SRC_MAC_POS 0
#define ORIGINAL_SRC_MAC_POS 6
#define DST_MAC_POS 12
#define MSG_NUMBER_POS 18
#define TTL_POS 20
#define PAYLOAD_LENGTH_POS 21
#define DATA_POS 22

#define MAX_HASH_COUNT_LIMIT 100

/**
 * @brief Add a message to a queue so that it can be procecedd later.
 */
void node_enqueue(uint8_t *data, uint8_t length);

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