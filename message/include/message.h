#define MAX_MESSAGE_SIZE 255

#define MAC_LEN 3

#define SRC_MAC_POS 0
#define ORIGINAL_SRC_MAC_POS (SRC_MAC_POS + MAC_LEN)
#define DST_MAC_POS (ORIGINAL_SRC_MAC_POS + MAC_LEN)
#define MSG_NUMBER_POS (DST_MAC_POS + MAC_LEN)
#define TTL_POS (MSG_NUMBER_POS + 1)
#define PAYLOAD_LENGTH_POS (TTL_POS + 1)
#define DATA_POS (PAYLOAD_LENGTH_POS + 1)

#define HEADER_LENGTH (DATA_POS)

#define MAX_PAYLOAD_SIZE (MAX_MESSAGE_SIZE - HEADER_LENGTH)

struct message
{
    uint8_t src_mac[MAC_LEN];
    uint8_t original_src_mac[MAC_LEN];
    uint8_t dst_mac[MAC_LEN];
    uint8_t msg_number;
    uint8_t ttl;
    uint8_t payload_len;
    uint8_t *payload;
};

/**
 * @brief Convert the bytes in an array into a message struct.
 *
 * The message is parsed and the struct is allocated onto the message heap.
 * 
 * The message must be valid, i.e.
 *  - payload_len field must match length + HEADER_LENGTH
 *  - length must be bigger than HEADER_LENGTH
 *  - length must be smaller than MAX_MESSAGE_SIZE
 *
 * @param[in] data Array containing the message.
 *
 * @param[in] length The length of the message.
 * 
 * @returns A pointer to the struct.
 *
 */
struct message *message_from_buffer(uint8_t *data, size_t length);

/**
 * @brief Convert a message struct to an array of bytes.
 *
 * The fields of the message struct is written into buffer 
 * 
 * @param[in] buffer Array to write into.
 *
 * @param[in] msg The message.
 * 
 * @returns The number of bytes written.
 *
 */
size_t message_to_buffer(uint8_t *buffer, struct message *msg);

/**
 * @brief Free a message on the message heap.
 *
 * @param[in] msg The message.
 *
 */
void message_free(struct message *msg);