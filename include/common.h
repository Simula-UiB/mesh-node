#ifndef GLOBAL_LOG_LEVEL
#define GLOBAL_LOG_LEVEL LOG_LEVEL_DBG
#endif

#define MAX_MESSAGE_SIZE 255

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