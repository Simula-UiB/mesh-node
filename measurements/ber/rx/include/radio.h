/**
 * @brief Initialize radio
 */
void init_radio();

/**
 * @brief Start radio transmission
 */
int radio_send(uint8_t *data, size_t length);

/**
 * @brief Radio receive callback function. 
 */
void radio_receive_cb(uint32_t ones, uint32_t length);
