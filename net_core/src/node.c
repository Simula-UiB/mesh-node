#include <common.h>
#include <node.h>

#include <zephyr.h>
#include <logging/log.h>
#include <logging/log_ctrl.h>
LOG_MODULE_REGISTER(node, GLOBAL_LOG_LEVEL);

/* Queue of incoming messages waiting to be processed */
K_MSGQ_DEFINE(message_queue, MAX_MESSAGE_SIZE, 10, 4);

void init_void()
{
}

void handle_receive(uint8_t * data, uint8_t length)
{
    LOG_HEXDUMP_INF(data, length, "Node RX data");
}

void process_packet()
{

}