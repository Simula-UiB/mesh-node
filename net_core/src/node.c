#include <stdio.h>

#include <common.h>
#include <msg.h>
#include <node.h>
#include <ipc.h>
#include <radio.h>

#include <zephyr.h>
#include <logging/log.h>
#include <logging/log_ctrl.h>
LOG_MODULE_REGISTER(node, GLOBAL_LOG_LEVEL);

/* Queue of incoming messages waiting to be processed */
K_MSGQ_DEFINE(node_msgq, sizeof(struct ipc_msg), 10, 4);

#define HEADER_LENGTH 22

#define SRC_MAC_POS 0
#define ORIGINAL_SRC_MAC_POS 6
#define DST_MAC_POS 12
#define MSG_NUMBER_POS 18
#define HOP_COUNT_POS 20
#define PAYLOAD_LENGTH_POS 21
#define DATA_POS 22

#define MAX_HOP_COUNT 10
#define MAX_HASH_COUNT_LIMIT 100

uint8_t msg_count = 0;

uint8_t node_send_buf[MAX_MESSAGE_SIZE + HEADER_LENGTH];

uint8_t node_addr[6];

uint8_t node_dst_addr[6] = {0x84, 0xc8, 0xbd, 0xea, 0x64, 0x8a};
//uint8_t node_dst_addr[6] = {0xc6, 0xad, 0x75, 0x07, 0xd4, 0x50};

uint8_t node_broadcast_addr[6] = {0,0,0,0,0,0};

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
    if (msg.len < HEADER_LENGTH
        || msg.len > MAX_MESSAGE_SIZE
        || msg.data[HOP_COUNT_POS] > MAX_HOP_COUNT)
    {
        LOG_DBG("Packet discarded");
        return;
    }

    if (memcmp(msg.data + DST_MAC_POS, node_addr, 6) == 0)
    {
        node_receive(msg);
        return;
    }

    if (memcmp(msg.data + DST_MAC_POS, node_broadcast_addr, 6) == 0)
    {
        node_receive(msg);
    }

    // TODO TTL or Hop Count?
    msg.data[HOP_COUNT_POS]++;
    memcpy(msg.data + SRC_MAC_POS, node_addr, 6);

    LOG_HEXDUMP_DBG(msg.data, msg.len, "Forwarding");
    radio_send(msg.data, msg.len);
}

int node_send(uint8_t * data, uint8_t length) {
    // TODO max message size vs max payload size
    if (length > MAX_MESSAGE_SIZE + HEADER_LENGTH)
    {
        return -1;
    }
    memcpy(node_send_buf + SRC_MAC_POS, node_addr, 6);
    memcpy(node_send_buf + ORIGINAL_SRC_MAC_POS, node_addr, 6);
    memcpy(node_send_buf + DST_MAC_POS, node_dst_addr, 6);
    node_send_buf[MSG_NUMBER_POS] = msg_count++;
    node_send_buf[HOP_COUNT_POS] = 0;
    node_send_buf[PAYLOAD_LENGTH_POS] = length;
    memcpy(node_send_buf + DATA_POS, data, length);

    return radio_send(node_send_buf, length + HEADER_LENGTH);
}

void node_thread(void * p1, void * p2, void * p3)
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
        node_addr[i] = NRF_FICR->DEVICEADDR[i/4] >> ((i % 4) * 8);
    }
    LOG_HEXDUMP_INF(node_addr, 6, "Node Address");
}