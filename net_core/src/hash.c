#include <stdint.h>
#include <stdio.h>

#include <common.h>
#include <message.h>

#include <node.h>

#include <logging/log.h>
#include <logging/log_ctrl.h>
#include <zephyr.h>
LOG_MODULE_REGISTER(hash, GLOBAL_LOG_LEVEL);

#define NUM_BUCKETS (MAX_HASH_COUNT_LIMIT / 2)

struct node
{
    uint32_t data;
    struct node *next;
};

struct node *buckets[NUM_BUCKETS];
size_t hash_size = 0;

// TODO Find out why this is too small without the constant.
K_HEAP_DEFINE(hash_heap, sizeof(struct node) * MAX_HASH_COUNT_LIMIT * 8);

K_MUTEX_DEFINE(hash_mutex);

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

/* Based on djb2 */
uint32_t hash_packet(struct message *msg)
{
    uint32_t hash = 5381;

    for (size_t i = 0; i < MAC_LEN; i++)
    {
        hash += (hash << 5) + msg->src_mac[i];
    }
    for (size_t i = 0; i < MAC_LEN; i++)
    {
        hash += (hash << 5) + msg->dst_mac[i];
    }
    hash += (hash << 5) + msg->msg_number;
    hash += (hash << 5) + msg->payload_len;
    for (size_t i = 0; i < msg->payload_len; i++)
    {
        hash += (hash << 5) + msg->payload[i];
    }

    return hash;
}

void init_hash()
{
    for (size_t i = 0; i < NUM_BUCKETS; i++)
    {
        buckets[i] = NULL;
    }
}

void hash_remove(uint32_t hash_val)
{
    uint32_t bucket = hash_val % NUM_BUCKETS;
    if (buckets[bucket] == NULL)
        return;

    if (buckets[bucket]->data == hash_val)
    {
        buckets[bucket] = buckets[bucket]->next;
        return;
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
    k_mutex_lock(&hash_mutex, K_FOREVER);
    while (hash_size >= MAX_HASH_COUNT_LIMIT)
    {
        uint32_t old_hash = dequeue();
        hash_remove(old_hash);
    }
    enqueue(hash_val);
    uint32_t bucket = hash_val % NUM_BUCKETS;
    void *ptr = k_heap_alloc(&hash_heap, sizeof(struct node), K_NO_WAIT);
    if (ptr == NULL)
    {
        LOG_ERR("Failed to allocate memory in heap.");
        return;
    }
    struct node *new_node = (struct node *)ptr;
    new_node->data = hash_val;
    new_node->next = buckets[bucket];
    buckets[bucket] = new_node;
    hash_size++;
    k_mutex_unlock(&hash_mutex);
}

bool hash_contains(uint32_t hash_val)
{
    uint32_t bucket = hash_val % NUM_BUCKETS;
    for (struct node *x = buckets[bucket]; x != NULL; x = x->next)
    {
        if (x->data == hash_val)
        {
            return true;
        }
    }
    return false;
}