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
K_MSGQ_DEFINE(node_msgq, sizeof(struct mesh_msg), 10, 4);

#define MAX_HOP_COUNT 2

uint8_t msg_count = 0;

uint8_t node_send_buf[MAX_MESSAGE_SIZE + HEADER_LENGTH];

uint8_t node_addr[6];

//uint8_t node_dst_addr[6] = {0xe2, 0xbb, 0x82, 0x7f, 0x52, 0x8f}; /* 960180795 */
//uint8_t node_dst_addr[6] = {0xbf, 0x2b, 0xca, 0x94, 0xcc, 0x30}; /* 960150638 */
uint8_t node_dst_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

uint8_t node_broadcast_addr[6] = {0, 0, 0, 0, 0, 0};

K_HEAP_DEFINE(node_heap, (sizeof(struct mesh_msg) + MAX_MESSAGE_SIZE) * 10);

void node_enqueue(struct mesh_msg *in_msg)
{
    struct mesh_msg *msg = (struct mesh_msg *)k_heap_alloc(&node_heap, sizeof(struct mesh_msg), K_NO_WAIT);
    if (msg == NULL)
    {
        LOG_ERR("Cannot allocate heap memory");
        return;
    }
    msg->data = (uint8_t *)k_heap_alloc(&node_heap, in_msg->len, K_NO_WAIT);
    if (msg->data == NULL)
    {
        LOG_ERR("Cannot allocate heap memory");
        return;
    }
    msg->len = in_msg->len;
    memcpy(msg->data, in_msg->data, in_msg->len);
    while (k_msgq_put(&node_msgq, &msg, K_NO_WAIT) != 0)
    {
        /* message queue is full: purge old data & try again */
        k_msgq_purge(&node_msgq);
    }
}

void node_process_packet()
{
    struct mesh_msg *msg;
    k_msgq_get(&node_msgq, &msg, K_FOREVER);
    if (msg->len < HEADER_LENGTH || msg->len > MAX_MESSAGE_SIZE)
    {
        LOG_INF("Invalid packet");
        k_heap_free(&node_heap, msg->data);
        k_heap_free(&node_heap, msg);
        return;
    }

    uint32_t hash = hash_packet(msg);

    if (msg->data[TTL_POS] <= 0 || hash_contains(hash))
    {
        k_heap_free(&node_heap, msg->data);
        k_heap_free(&node_heap, msg);
        return;
    }

    hash_add(hash);

    if (memcmp(msg->data + DST_MAC_POS, node_addr, 6) == 0)
    {
        node_receive(msg);
        k_heap_free(&node_heap, msg->data);
        k_heap_free(&node_heap, msg);
        return;
    }

    if (memcmp(msg->data + DST_MAC_POS, node_broadcast_addr, 6) == 0)
    {
        node_receive(msg);
    }

    msg->data[TTL_POS]--;
    memcpy(msg->data + SRC_MAC_POS, node_addr, 6);

    LOG_HEXDUMP_DBG(msg->data, msg->len, "Forwarding");
    radio_send(msg->data, msg->len);
    k_heap_free(&node_heap, msg->data);
    k_heap_free(&node_heap, msg);
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