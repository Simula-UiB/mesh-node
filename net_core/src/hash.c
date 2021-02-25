#include <stdio.h>
#include <stdint.h>

#include <common.h>
#include <msg.h>
#include <node.h>

#include <zephyr.h>
#include <logging/log.h>
#include <logging/log_ctrl.h>
LOG_MODULE_REGISTER(hash, GLOBAL_LOG_LEVEL);

uint32_t hash_djb2(uint8_t *bytes, size_t len)
{
    uint32_t hash = 5381;

    for (size_t i = 0; i < len; i++)
    {
        hash = (hash << 5) + hash + bytes[i];
    }

    return hash;
}

uint32_t hash_packet(struct ipc_msg msg)
{
    uint32_t hash = 5381;

    // TODO This crashes, find out why
    // for (size_t i = 0; i < 6; i++)
    // {
    //     hash = (hash << 5) + hash + msg.data[ORIGINAL_SRC_MAC_POS + i];
    // }
    // for (size_t i = 0; i < 6; i++)
    // {
    //     hash = (hash << 5) + hash + msg.data[DST_MAC_POS + i];
    // }
    // hash = (hash << 5) + hash + msg.data[MSG_NUMBER_POS];
    // hash = (hash << 5) + hash + msg.data[PAYLOAD_LENGTH_POS];
    // for (size_t i = DATA_POS; i < msg.len; i++)
    // {
    //     hash = (hash << 5) + hash + msg.data[i];
    // }

    return hash;
}