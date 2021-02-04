#include <stdint.h>

struct radio_msg {
    size_t len;
    uint8_t * data;
};

void init_void();

void handle_receive(struct radio_msg msg);

void process_packet();