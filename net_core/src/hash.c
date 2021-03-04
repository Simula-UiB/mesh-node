#include <stdint.h>
#include <stdio.h>

#include <common.h>
#include <msg.h>
#include <node.h>

#include <logging/log.h>
#include <logging/log_ctrl.h>
#include <zephyr.h>
LOG_MODULE_REGISTER(hash, GLOBAL_LOG_LEVEL);

#define BUCKETS (MAX_HASH_COUNT_LIMIT / 2)

struct node
{
    uint32_t data;
    struct node *next;
};

struct node *buckets[BUCKETS];
size_t hash_size = 0;

// TODO Find out why this is too small without the constant.
K_HEAP_DEFINE(hash_heap, sizeof(struct node) * MAX_HASH_COUNT_LIMIT * 8);

uint32_t queue[MAX_HASH_COUNT_LIMIT];
size_t queue_back = MAX_HASH_COUNT_LIMIT - 1;
size_t queue_front = 0;
size_t queue_size = 0;

void enqueue(uint32_t hash_val)
{
    if (queue_size == MAX_HASH_COUNT_LIMIT)
    {
        LOG_ERR("Queue Full!");
        return;
    }
    queue_back = (queue_back + 1) % MAX_HASH_COUNT_LIMIT;
    queue[queue_back] = hash_val;
    queue_size++;
}

uint32_t dequeue()
{
    if (queue_size == 0)
    {
        LOG_ERR("Nothing to dequeue");
        return 0;
    }
    uint32_t hash_val = queue[queue_front];
    queue_front = (queue_front + 1) % MAX_HASH_COUNT_LIMIT;
    queue_size--;
    return hash_val;
}

// Based on djb2
uint32_t hash_packet(struct mesh_msg *msg)
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
    if (buckets[bucket] != NULL && buckets[bucket]->data == hash_val)
    {
        buckets[bucket] = buckets[bucket]->next;
    }
    for (struct node *x = buckets[bucket]; x->next != NULL; x = x->next)
    {
        if (x->next->data == hash_val)
        {
            struct node *old_ptr = x->next;
            x->next = x->next->next;
            k_heap_free(&hash_heap, old_ptr);
            hash_size--;
            return;
        }
    }
}

/* Assumes that there isn't already an equal key in the set. This holds for the
 * mesh node since every packet is first checked if they're a member of the set.
 */
void hash_add(uint32_t hash_val)
{
    LOG_INF("hash_val=%u", hash_val);
    if (hash_size >= MAX_HASH_COUNT_LIMIT)
    {
        uint32_t old_hash = dequeue();
        LOG_HEXDUMP_INF(queue, MAX_HASH_COUNT_LIMIT, "Queue:");
        hash_remove(old_hash);
    }
    enqueue(hash_val);
    uint32_t bucket = hash_val % BUCKETS;
    void *ptr = k_heap_alloc(&hash_heap, sizeof(struct node), K_NO_WAIT);
    if (ptr == NULL)
    {
        LOG_ERR("Failed to allocate memory in heap.");
    }
    struct node *new_node = (struct node *)ptr;
    new_node->data = hash_val;
    new_node->next = buckets[bucket];
    buckets[bucket] = new_node;
    hash_size++;
    LOG_INF("hash_size=%u, queue_size=%u", hash_size, queue_size);
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

// TODO Something crashes somewhere, I don't know where yet...