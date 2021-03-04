#include <stdlib.h>
#include <stdint.h>

/**
 * @brief Mesh message struct
 */
struct mesh_msg
{
    size_t len;
    uint8_t *data;
};
