#define MAX_MESSAGE_SIZE 255

#define SRC_MAC_POS 0
#define ORIGINAL_SRC_MAC_POS 6
#define DST_MAC_POS 12
#define MSG_NUMBER_POS 18
#define TTL_POS 20
#define PAYLOAD_LENGTH_POS 21
#define DATA_POS 22

#define HEADER_LENGTH 22

#define MAX_PAYLOAD_SIZE (MAX_MESSAGE_SIZE - HEADER_LENGTH)

struct message
{
    uint8_t src_mac[6];
    uint8_t original_src_mac[6];
    uint8_t dst_mac[6];
    uint8_t msg_number;
    uint8_t ttl;
    uint8_t payload_len;
    uint8_t *payload;
};

struct message *message_from_buffer(uint8_t *data, size_t length);

size_t message_to_buffer(uint8_t *buffer, struct message *msg);

void message_free(struct message *msg);