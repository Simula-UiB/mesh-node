#include <stdio.h>

#include <common.h>
#include <node.h>

#include <zephyr.h>
#include <logging/log.h>
#include <logging/log_ctrl.h>
LOG_MODULE_REGISTER(node, GLOBAL_LOG_LEVEL);

/* Queue of incoming messages waiting to be processed */
K_MSGQ_DEFINE(node_msgq, sizeof(struct node_msg), 10, 4);

struct node_msg processing;

void enqueue(struct node_msg msg)
{
    while (k_msgq_put(&node_msgq, &msg, K_NO_WAIT) != 0) {
        /* message queue is full: purge old data & try again */
        k_msgq_purge(&node_msgq);
    }
}

void node_thread(void * p1, void * p2, void * p3)
{
    LOG_INF("Node thread started");
    k_msleep(500);

    struct node_msg msg;

    while (true)
    {
        k_msgq_get(&node_msgq, &msg, K_FOREVER);
        LOG_HEXDUMP_INF(msg.data, msg.len, "Node RX data");
    }
}