#include <stdint.h>
#include <stdio.h>

#include <common.h>
#include <msg.h>
#include <node.h>

#include <logging/log.h>
#include <logging/log_ctrl.h>
#include <zephyr.h>
LOG_MODULE_REGISTER(hash, GLOBAL_LOG_LEVEL);

struct node
{
    uint32_t data;
    struct node *next;
};

#define BUCKETS (MAX_HASH_COUNT_LIMIT / 2)

struct node *buckets[BUCKETS];

uint32_t queue[MAX_HASH_COUNT_LIMIT];

K_HEAP_DEFINE(hash_heap, sizeof(struct node) * MAX_HASH_COUNT_LIMIT);

size_t size = 0;

// Based on djb2
uint32_t hash_packet(struct ipc_msg *msg)
{
    uint32_t hash = 5381;

    for (size_t i = 0; i < 6; i++)
    {
        hash += (hash << 5) + msg->data[ORIGINAL_SRC_MAC_POS + i];
    }
    for (size_t i = 0; i < 6; i++)
    {
        hash += (hash << 5) + msg->data[DST_MAC_POS + i];
    }
    hash += (hash << 5) + msg->data[MSG_NUMBER_POS];
    hash += (hash << 5) + msg->data[PAYLOAD_LENGTH_POS];
    for (size_t i = DATA_POS; i < msg->len; i++)
    {
        hash += (hash << 5) + msg->data[i];
    }

    return hash;
}

void init_hash()
{
    for (size_t i = 0; i < BUCKETS; i++)
    {
        buckets[i] = NULL;
    }
}

void hash_remove(uint32_t hash_val)
{
    uint32_t bucket = hash_val % BUCKETS;
    for (struct node *x = buckets[bucket]; x != NULL; x = x->next)
    {
        if (x->data == hash_val)
        {
            struct node *old_ptr = x;
            x = x->next;
            k_heap_free(&hash_heap, old_ptr);
            size--;
        }
    }
}

void enqueue(uint32_t hash_val)
{
    // TODO Add the value to the queue at the end
}

uint32_t dequeue()
{
    // TODO pop the oldest element from the start
}

/* Assumes that there isn't already an equal key in the set. This holds for the
 * mesh node since every packet is first checked if they're a member of the set.
 */
void hash_add(uint32_t hash_val)
{
    if (size >= MAX_HASH_COUNT_LIMIT)
    {
        uint32_t old_hash = dequeue();
        hash_remove(old_hash);
    }
    enqueue(hash_val);
    uint32_t bucket = hash_val % BUCKETS;
    struct node *new_node =
        (struct node *)k_heap_alloc(&hash_heap, sizeof(struct node), K_NO_WAIT);
    new_node->data = hash_val;
    new_node->next = buckets[bucket];
    buckets[bucket] = new_node;
    size++;
}

bool hash_contains(uint32_t hash_val)
{
    uint32_t bucket = hash_val % BUCKETS;
    for (struct node *x = buckets[bucket]; x != NULL; x = x->next)
    {
        if (x->data == hash_val)
        {
            return true;
        }
    }
    return false;
}