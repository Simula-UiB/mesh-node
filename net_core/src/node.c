#include <stdint.h>
#include <stdio.h>

#include <common.h>
#include <msg.h>

#include <hash.h>
#include <ipc.h>
#include <node.h>
#include <radio.h>

#include <logging/log.h>
#include <logging/log_ctrl.h>
#include <zephyr.h>
LOG_MODULE_REGISTER(node, GLOBAL_LOG_LEVEL);

/* Thread definitions */
#define THREAD_STACK_SIZE 2048
#define THREAD_PRIORITY 5

/* Queue of incoming messages waiting to be processed */
K_MSGQ_DEFINE(node_msgq, sizeof(struct message *), 10, 4);

#define MAX_HOP_COUNT 2

uint8_t msg_count = 0;

uint8_t node_send_buf[MAX_MESSAGE_SIZE];

uint8_t node_addr[6];

// TODO Find a better way to handle destination MACs
//uint8_t node_dst_addr[6] = {0xe2, 0xbb, 0x82, 0x7f, 0x52, 0x8f}; /* 960180795 */
//uint8_t node_dst_addr[6] = {0xbf, 0x2b, 0xca, 0x94, 0xcc, 0x30}; /* 960150638 */
//uint8_t node_dst_addr[6] = {0xe6, 0x09, 0xb2, 0x18, 0x41, 0x23}; /* 960131836 */
uint8_t node_dst_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
//uint8_t node_dst_addr[6] = {0, 0, 0, 0, 0, 0};

uint8_t node_broadcast_addr[6] = {0, 0, 0, 0, 0, 0};

K_HEAP_DEFINE(node_heap, (sizeof(struct message) + MAX_MESSAGE_SIZE) * 10);

void node_enqueue(uint8_t *data, size_t length)
{
    if (length < HEADER_LENGTH ||
        length > MAX_MESSAGE_SIZE ||
        length != data[PAYLOAD_LENGTH_POS] + HEADER_LENGTH)
    {
        LOG_DBG("Invalid packet");
        return;
    }
    struct message *msg = (struct message *)k_heap_alloc(&node_heap, sizeof(struct message), K_NO_WAIT);
    if (msg == NULL)
    {
        LOG_ERR("Cannot allocate heap memory");
        return;
    }
    memcpy(msg->src_mac, data + SRC_MAC_POS, sizeof(uint8_t) * 6);
    memcpy(msg->original_src_mac, data + ORIGINAL_SRC_MAC_POS, sizeof(uint8_t) * 6);
    memcpy(msg->dst_mac, data + DST_MAC_POS, sizeof(uint8_t) * 6);
    msg->msg_number = data[MSG_NUMBER_POS];
    msg->ttl = data[TTL_POS];
    msg->payload_len = data[PAYLOAD_LENGTH_POS];
    msg->payload = (uint8_t *)k_heap_alloc(&node_heap, msg->payload_len, K_NO_WAIT);
    if (msg->payload == NULL)
    {
        LOG_ERR("Cannot allocate heap memory");
        k_heap_free(&node_heap, msg);
        return;
    }
    memcpy(msg->payload, data + DATA_POS, msg->payload_len);
    while (k_msgq_put(&node_msgq, &msg, K_NO_WAIT) != 0)
    {
        /* message queue is full: purge old data & try again */
        k_msgq_purge(&node_msgq);
    }
}

int node_radio_send(struct message *msg)
{
    memcpy(node_send_buf + SRC_MAC_POS, msg->src_mac, sizeof(uint8_t) * 6);
    memcpy(node_send_buf + ORIGINAL_SRC_MAC_POS, msg->original_src_mac, sizeof(uint8_t) * 6);
    memcpy(node_send_buf + DST_MAC_POS, msg->dst_mac, sizeof(uint8_t) * 6);
    node_send_buf[MSG_NUMBER_POS] = msg->msg_number;
    node_send_buf[TTL_POS] = msg->ttl;
    node_send_buf[PAYLOAD_LENGTH_POS] = msg->payload_len;
    memcpy(node_send_buf + DATA_POS, msg->payload, msg->payload_len);
    return radio_send(node_send_buf, msg->payload_len + HEADER_LENGTH);
}

void node_process_packet()
{
    struct message *msg;
    k_msgq_get(&node_msgq, &msg, K_FOREVER);

    uint32_t hash = hash_packet(msg);

    if (hash_contains(hash))
    {
        k_heap_free(&node_heap, msg->payload);
        k_heap_free(&node_heap, msg);
        return;
    }

    hash_add(hash);

    if (memcmp(msg->dst_mac, node_addr, sizeof(uint8_t) * 6) == 0)
    {
        node_receive(msg);
        k_heap_free(&node_heap, msg->payload);
        k_heap_free(&node_heap, msg);
        return;
    }

    if (memcmp(msg->dst_mac, node_broadcast_addr, sizeof(uint8_t) * 6) == 0 &&
        memcmp(msg->original_src_mac, node_addr, sizeof(uint8_t) * 6) != 0)
    {
        node_receive(msg);
    }

    if (msg->ttl <= 0)
    {
        k_heap_free(&node_heap, msg->payload);
        k_heap_free(&node_heap, msg);
        return;
    }

    msg->ttl--;
    memcpy(msg->src_mac, node_addr, sizeof(uint8_t) * 6);

    LOG_HEXDUMP_DBG(msg->payload, msg->payload_len, "Forwarding");
    node_radio_send(msg);
    k_heap_free(&node_heap, msg->payload);
    k_heap_free(&node_heap, msg);
}

int node_send(struct message *msg)
{
    if (msg->payload_len + HEADER_LENGTH > MAX_MESSAGE_SIZE)
    {
        return -1;
    }
    memcpy(msg->src_mac, node_addr, sizeof(uint8_t) * 6);
    memcpy(msg->original_src_mac, node_addr, sizeof(uint8_t) * 6);
    memcpy(msg->dst_mac, node_dst_addr, sizeof(uint8_t) * 6);
    msg->msg_number = msg_count++;
    msg->ttl = MAX_HOP_COUNT;

    return node_radio_send(msg);
}

void node_thread(void *p1, void *p2, void *p3)
{
    LOG_INF("Node thread started with id %04x", (uint32_t)k_current_get());
    k_msleep(500);

    while (true)
    {
        node_process_packet();
    }
}

void init_node()
{
    for (size_t i = 0; i < 6; i++)
    {
        node_addr[i] = NRF_FICR->DEVICEADDR[i / 4] >> ((i % 4) * 8);
    }
    LOG_HEXDUMP_INF(node_addr, 6, "Node Address");

    init_hash();
}

K_THREAD_DEFINE(node, THREAD_STACK_SIZE, node_thread, NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);