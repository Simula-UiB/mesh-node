#include <stdlib.h>
#include <stdint.h>

/**
 * @brief IPC Message struct
 */
struct ipc_msg {
    size_t len;
    uint8_t * data;
};