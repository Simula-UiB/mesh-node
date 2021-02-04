#include <stdint.h>

struct node_msg {
    size_t len;
    uint8_t * data;
};

void enqueue(struct node_msg msg);

void process_packet();