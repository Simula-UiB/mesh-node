/**
 * @brief Initialize radio
 */
void init_radio();

/**
 * @brief Start radio transmission
 */
int radio_send(uint8_t *data, uint8_t length);

/**
 * @brief Radio receive callback function. 
 * 
 * @details This function must be implemented by the module making use of the radio. It will be called 
 *      when the radio has received a frame. The data buffer may be overwritten at any time after this
 *      function returns, so data should be copied to another buffer or processing should be completed
 *      before returning.
 */
void radio_receive_cb(uint8_t *data, uint8_t length);
