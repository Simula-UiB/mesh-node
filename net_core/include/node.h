#include <stdint.h>

struct node_msg {
    size_t len;
    uint8_t * data;
};

void node_enqueue(struct node_msg msg);

void node_process_packet();