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
K_MSGQ_DEFINE(node_msgq, sizeof(struct ipc_msg), 10, 4);

#define MAX_HOP_COUNT 2

uint8_t msg_count = 0;

uint8_t node_send_buf[MAX_MESSAGE_SIZE + HEADER_LENGTH];

uint8_t node_addr[6];

uint8_t node_dst_addr[6] = {0x84, 0xc8, 0xbd, 0xea, 0x64, 0x8a};
//uint8_t node_dst_addr[6] = {0xc6, 0xad, 0x75, 0x07, 0xd4, 0x50};

uint8_t node_broadcast_addr[6] = {0, 0, 0, 0, 0, 0};

void node_enqueue(struct ipc_msg msg)
{
    while (k_msgq_put(&node_msgq, &msg, K_NO_WAIT) != 0)
    {
        /* message queue is full: purge old data & try again */
        k_msgq_purge(&node_msgq);
    }
}

void node_process_packet()
{
    struct ipc_msg msg;
    k_msgq_get(&node_msgq, &msg, K_FOREVER);
    if (msg.len < HEADER_LENGTH || msg.len > MAX_MESSAGE_SIZE)
    {
        LOG_INF("Invalid packet");
        return;
    }

    uint32_t hash = hash_packet(&msg);

    if (msg.data[TTL_POS] <= 0 || hash_contains(hash))
    {
        return;
    }

    hash_add(hash);

    if (memcmp(msg.data + DST_MAC_POS, node_addr, 6) == 0)
    {
        node_receive(msg);
        return;
    }

    if (memcmp(msg.data + DST_MAC_POS, node_broadcast_addr, 6) == 0)
    {
        node_receive(msg);
    }

    msg.data[TTL_POS]--;
    memcpy(msg.data + SRC_MAC_POS, node_addr, 6);

    LOG_HEXDUMP_DBG(msg.data, msg.len, "Forwarding");
    radio_send(msg.data, msg.len);
}

int node_send(uint8_t *data, uint8_t length)
{
    // TODO max message size vs max payload size
    if (length > MAX_MESSAGE_SIZE + HEADER_LENGTH)
    {
        return -1;
    }
    memcpy(node_send_buf + SRC_MAC_POS, node_addr, 6);
    memcpy(node_send_buf + ORIGINAL_SRC_MAC_POS, node_addr, 6);
    memcpy(node_send_buf + DST_MAC_POS, node_dst_addr, 6);
    node_send_buf[MSG_NUMBER_POS] = msg_count++;
    node_send_buf[TTL_POS] = MAX_HOP_COUNT;
    node_send_buf[PAYLOAD_LENGTH_POS] = length;
    memcpy(node_send_buf + DATA_POS, data, length);

    return radio_send(node_send_buf, length + HEADER_LENGTH);
}

void node_thread(void *p1, void *p2, void *p3)
{
    LOG_INF("Node thread started");
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