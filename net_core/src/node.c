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

void node_enqueue(struct node_msg msg)
{
    LOG_INF("Message added to queue");
    while (k_msgq_put(&node_msgq, &msg, K_NO_WAIT) != 0) {
        /* message queue is full: purge old data & try again */
        k_msgq_purge(&node_msgq);
    }
}

void node_process_packet(struct node_msg msg)
{
    k_msgq_get(&node_msgq, &msg, K_FOREVER);
}