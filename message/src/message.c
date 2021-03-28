#include <stdint.h>
#include <stdio.h>

#include <logging/log.h>
#include <logging/log_ctrl.h>
#include <zephyr.h>

#include <common.h>
#include <message.h>

LOG_MODULE_REGISTER(message, GLOBAL_LOG_LEVEL);

K_HEAP_DEFINE(message_heap, (sizeof(struct message) + MAX_MESSAGE_SIZE) * 10);

struct message *message_from_buffer(uint8_t *data, size_t length)
{
    if (length < HEADER_LENGTH ||
        length > MAX_MESSAGE_SIZE ||
        length != data[PAYLOAD_LENGTH_POS] + HEADER_LENGTH)
    {
        LOG_DBG("Invalid packet");
        return NULL;
    }
    struct message *msg = (struct message *)k_heap_alloc(&message_heap, sizeof(struct message), K_NO_WAIT);
    if (msg == NULL)
    {
        LOG_ERR("Cannot allocate heap memory");
        return NULL;
    }
    memcpy(msg->src_mac, data + SRC_MAC_POS, sizeof(uint8_t) * 6);
    memcpy(msg->original_src_mac, data + ORIGINAL_SRC_MAC_POS, sizeof(uint8_t) * 6);
    memcpy(msg->dst_mac, data + DST_MAC_POS, sizeof(uint8_t) * 6);
    msg->msg_number = data[MSG_NUMBER_POS];
    msg->ttl = data[TTL_POS];
    msg->payload_len = data[PAYLOAD_LENGTH_POS];
    msg->payload = (uint8_t *)k_heap_alloc(&message_heap, msg->payload_len, K_NO_WAIT);
    if (msg->payload == NULL)
    {
        LOG_ERR("Cannot allocate heap memory");
        k_heap_free(&message_heap, msg);
        return NULL;
    }
    memcpy(msg->payload, data + DATA_POS, msg->payload_len);
    return msg;
}

size_t message_to_buffer(uint8_t *buffer, struct message *msg)
{
    memcpy(buffer + SRC_MAC_POS, msg->src_mac, sizeof(uint8_t) * 6);
    memcpy(buffer + ORIGINAL_SRC_MAC_POS, msg->original_src_mac, sizeof(uint8_t) * 6);
    memcpy(buffer + DST_MAC_POS, msg->dst_mac, sizeof(uint8_t) * 6);
    buffer[MSG_NUMBER_POS] = msg->msg_number;
    buffer[TTL_POS] = msg->ttl;
    buffer[PAYLOAD_LENGTH_POS] = msg->payload_len;
    memcpy(buffer + DATA_POS, msg->payload, msg->payload_len);
    return msg->payload_len + HEADER_LENGTH;
}

void message_free(struct message *msg)
{
    k_heap_free(&message_heap, msg->payload);
    k_heap_free(&message_heap, msg);
}
