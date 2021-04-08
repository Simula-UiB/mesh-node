#include <stdint.h>
#include <stdio.h>

#include <common.h>
#include <message.h>
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
//uint8_t node_dst_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
uint8_t node_dst_addr[6] = {0, 0, 0, 0, 0, 0};

uint8_t node_broadcast_addr[6] = {0, 0, 0, 0, 0, 0};

void node_enqueue(struct message *msg)
{
    while (k_msgq_put(&node_msgq, &msg, K_NO_WAIT) != 0)
    {
        /* message queue is full: purge old data & try again */
        k_msgq_purge(&node_msgq);
    }
}

int node_radio_send(struct message *msg)
{
    size_t size = message_to_buffer(node_send_buf, msg);
    message_free(msg);
    return radio_send(node_send_buf, size);
}

void node_process_packet()
{
    struct message *msg;
    k_msgq_get(&node_msgq, &msg, K_FOREVER);

    uint32_t hash = hash_packet(msg);

    if (hash_contains(hash))
    {
        message_free(msg);
        return;
    }

    hash_add(hash);

    if (memcmp(msg->dst_mac, node_addr, sizeof(uint8_t) * 6) == 0)
    {
        node_receive(msg);
        message_free(msg);
        return;
    }

    if (memcmp(msg->dst_mac, node_broadcast_addr, sizeof(uint8_t) * 6) == 0 &&
        memcmp(msg->original_src_mac, node_addr, sizeof(uint8_t) * 6) != 0)
    {
        node_receive(msg);
    }

    if (msg->ttl <= 0)
    {
        message_free(msg);
        return;
    }

    msg->ttl--;
    memcpy(msg->src_mac, node_addr, sizeof(uint8_t) * 6);

    LOG_HEXDUMP_DBG(msg->payload, msg->payload_len, "Forwarding");
    node_radio_send(msg);
}

int node_send(struct message *msg)
{
    if (msg->payload_len > MAX_PAYLOAD_SIZE)
    {
        return -1;
    }
    memcpy(msg->src_mac, node_addr, sizeof(uint8_t) * 6);
    memcpy(msg->original_src_mac, node_addr, sizeof(uint8_t) * 6);
    memcpy(msg->dst_mac, node_dst_addr, sizeof(uint8_t) * 6);
    msg->msg_number = msg_count++;
    msg->ttl = MAX_HOP_COUNT;

    hash_add(hash_packet(msg));

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