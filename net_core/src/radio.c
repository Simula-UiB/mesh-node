#include <logging/log.h>
#include <logging/log_ctrl.h>
#include <zephyr.h>

#include <hal/nrf_radio.h>
#include <nrfx.h>

#include <common.h>
#include <msg.h>
#include <radio.h>

LOG_MODULE_REGISTER(radio, GLOBAL_LOG_LEVEL);

/* Radio buffer offsets */
#define RF_BUFFER_LENGTH_OFFSET 0
#define RF_BUFFER_PAYLOAD_OFFSET 1

/* Thread definitions */
#define THREAD_STACK_SIZE 2048
#define THREAD_PRIORITY 5

K_HEAP_DEFINE(radio_heap, (sizeof(struct mesh_msg)+MAX_MESSAGE_SIZE)*10);

/* Radio transmission buffer. +1 byte for length byte. */
uint8_t rf_tx_buf[MAX_MESSAGE_SIZE + 1];
/* Radio receive buffer */
uint8_t rf_rx_buf[MAX_MESSAGE_SIZE + 1];

/* Binary semaphore for accessing radio for a transmission */
struct k_sem rf_tx_completed;
/* Semaphore for received packets */
struct k_sem rf_rx_sem;

/* Declaration of trigger functions */
void trigger_rx();
void trigger_tx();

/**
 * @brief Start radio transmission
 */
int radio_send(uint8_t *data, uint8_t length)
{
    int ret = 0;
    /* Fill radio transmission buffer */
    memcpy(rf_tx_buf + RF_BUFFER_PAYLOAD_OFFSET, data, length);
    /* Set length */
    rf_tx_buf[RF_BUFFER_LENGTH_OFFSET] = length + 1;

    LOG_HEXDUMP_DBG(rf_tx_buf, length + 1, "Data TX");

    /* Get radio state */
    nrf_radio_state_t state = nrf_radio_state_get(NRF_RADIO);
    switch (state)
    {
    case NRF_RADIO_STATE_RX:
        /* Stop RX, trigger TXEN task from RXIDLE state */
        nrf_radio_task_trigger(NRF_RADIO, NRF_RADIO_TASK_STOP);
        trigger_tx();
        break;
    case NRF_RADIO_STATE_RXIDLE:
    case NRF_RADIO_STATE_DISABLED:
        /* Radio already in compatible state, trigger TXEN task */
        trigger_tx();
        break;
    case NRF_RADIO_STATE_TXIDLE:
        nrf_radio_packetptr_set(NRF_RADIO, rf_tx_buf);
        nrf_radio_task_trigger(NRF_RADIO, NRF_RADIO_TASK_START);
        break;
    default:
        /* Radio in incompatible state. Drop packet, for now */
        LOG_ERR("TX Failed, incompatible state. State: %d", state);
        k_sem_give(&rf_tx_completed);
        ret = -EIO;
    }
    k_sem_take(&rf_tx_completed, K_FOREVER);
    return ret;
}

/**
 * @brief Radio RX thread
 *
 * Forwards messages from radio layer
 */
void radio_rx_thread(void *p1, void *p2, void *p3)
{
    k_msleep(500);
    LOG_INF("Radio RX thread started, ID: %04x", (uint32_t)k_current_get());

    while (true)
    {
        /* Wait for received frame from radio */
        k_sem_take(&rf_rx_sem, K_FOREVER);
        uint8_t length = rf_rx_buf[RF_BUFFER_LENGTH_OFFSET] - 1;
        if (length > MAX_MESSAGE_SIZE)
        {
            length = MAX_MESSAGE_SIZE;
        }
        struct mesh_msg *msg = (struct mesh_msg*) k_heap_alloc(&radio_heap, sizeof(struct mesh_msg), K_NO_WAIT);
        if (msg == NULL)
        {
            LOG_ERR("Cannot allocate heap memory");
            continue;
        }
        msg->data = (uint8_t*) k_heap_alloc(&radio_heap, length, K_NO_WAIT);
        if (msg->data == NULL)
        {
            LOG_ERR("Cannot allocate heap memory");
            k_heap_free(&radio_heap, msg);
            continue;
        }
        msg->len = length;
        memcpy(msg->data, rf_rx_buf + RF_BUFFER_PAYLOAD_OFFSET, length);

        LOG_HEXDUMP_DBG(msg->data, length, "Radio RX data");

        /* Callback function for received radio frame */
        radio_receive_cb(msg);

        k_heap_free(&radio_heap, msg->data);
        k_heap_free(&radio_heap, msg);
    }
}

/**
 * @brief Set packet pointer and trigger RXEN task
 */
void trigger_rx()
{
    nrf_radio_packetptr_set(NRF_RADIO, rf_rx_buf);
    nrf_radio_task_trigger(NRF_RADIO, NRF_RADIO_TASK_RXEN);
}

/**
 * @brief Set packet pointer and trigger TXEN task
 */
void trigger_tx()
{
    nrf_radio_packetptr_set(NRF_RADIO, rf_tx_buf);
    nrf_radio_task_trigger(NRF_RADIO, NRF_RADIO_TASK_TXEN);
}

/**
 * @brief Radio interrupt handler
 */
void radio_irq_handler(void *ctx)
{
    LOG_DBG("Radio IRQ");
    /* END event. Either TX or RX complete. */
    if (nrf_radio_event_check(NRF_RADIO, NRF_RADIO_EVENT_END))
    {
        nrf_radio_event_clear(NRF_RADIO, NRF_RADIO_EVENT_END);

        nrf_radio_state_t state = nrf_radio_state_get(NRF_RADIO);
        if (NRF_RADIO_STATE_RXIDLE == state)
        {
            /* RX Complete */
            k_sem_give(&rf_rx_sem);
            /* Start new RX, we are already ramped up */
            nrf_radio_task_trigger(NRF_RADIO, NRF_RADIO_TASK_START);
        }
        else if (NRF_RADIO_STATE_TXIDLE == state)
        {
            /* TX Complete */
            /* Return to RX mode */
            trigger_rx();
            /* Give radio tx semaphore */
            k_sem_give(&rf_tx_completed);
        }
        else
        {
            /* Really shouldn't happen! */
            LOG_ERR("Unknown radio state in IRQ: %d", state);
        }
    }
}

/**
 * @brief Enable radio interrupts
 */
void init_irq()
{
    int32_t ints_to_enable = 0;
    ints_to_enable |= NRF_RADIO_INT_END_MASK;
    nrf_radio_int_enable(NRF_RADIO, ints_to_enable);
    NVIC_ClearPendingIRQ(RADIO_IRQn);
    NVIC_EnableIRQ(RADIO_IRQn);
    IRQ_CONNECT(RADIO_IRQn, 7, radio_irq_handler, NULL, 0);
}

/**
 * @brief Initialize BLE radio
 */
void init_radio()
{
    LOG_INF("Initializing radio");
    k_sem_init(&rf_tx_completed, 0, 1);
    k_sem_init(&rf_rx_sem, 0, 1);

    /* Set modulation and bitrate */
    nrf_radio_mode_set(NRF_RADIO, NRF_RADIO_MODE_BLE_2MBIT);
    /* Enable fast ramp up, transmit center when idle */
    nrf_radio_modecnf0_set(NRF_RADIO, true, RADIO_MODECNF0_DTX_Center);

    /* Set channel */
    nrf_radio_frequency_set(NRF_RADIO, 2442); // 2442 Mhz

    /* Set radio transmission power */
    nrf_radio_txpower_set(NRF_RADIO, NRF_RADIO_TXPOWER_0DBM); // 0 dBm

    /* Set packet config */
    nrf_radio_packet_conf_t pkt_conf = {
        .lflen = 8UL, // Length field length in bits
        .s0len = 0UL,
        .s1len = 0UL,
        .s1incl = false,
        .plen = NRF_RADIO_PREAMBLE_LENGTH_16BIT,
        .crcinc = false, // CRC included in length field
        .statlen = 0UL,
        .balen = 4UL,
        .big_endian = true,
        .maxlen = MAX_MESSAGE_SIZE, // Max payload length
        .whiteen = false            // Packet whitening
    };
    nrf_radio_packet_configure(NRF_RADIO, &pkt_conf);

    /* Address */
    nrf_radio_base0_set(NRF_RADIO, 0xDEADBEEFUL);
    nrf_radio_prefix0_set(NRF_RADIO, 0x42U);
    nrf_radio_txaddress_set(NRF_RADIO, 0x00U);   // Transmit using logical address 0 (base0 and prefix0)
    nrf_radio_rxaddresses_set(NRF_RADIO, 0x01U); // Enable RX of logical address 0

    /* CRC Config */
    nrf_radio_crc_configure(NRF_RADIO,
                            RADIO_CRCCNF_LEN_Disabled, // Disable CRC
                            NRF_RADIO_CRC_ADDR_INCLUDE,
                            0);

    /* Shortcuts */
    nrf_radio_shorts_enable(NRF_RADIO, NRF_RADIO_SHORT_READY_START_MASK); // Ready -> Start shortcut

    /* Enable interrupts */
    init_irq();

    /* Start rx */
    trigger_rx();
}

/* Define and start radio rx thread */
K_THREAD_DEFINE(radio_rx, THREAD_STACK_SIZE, radio_rx_thread, NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);
