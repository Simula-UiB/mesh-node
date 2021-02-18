#include <stdint.h>

/**
 * @brieg Initialize radio
 */
void init_radio();

/**
 * @brief Start radio transmission
 */
int radio_send(uint8_t *data, uint8_t length);

/**
 * @brief Receive data from radio
 */
int radio_receive(uint8_t *data, uint8_t max_length);
