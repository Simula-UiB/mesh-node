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

struct message *message_from_buffer(uint8_t *data, size_t length);

size_t message_to_buffer(uint8_t *buffer, struct message *msg);

void message_free(struct message *msg);