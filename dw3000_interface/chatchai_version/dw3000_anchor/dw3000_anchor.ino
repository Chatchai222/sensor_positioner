/*Written: start 2023 Apr 5
 * Modification with comments of the DW3000 example by MakerFabs for "ex_06b_ss_twr_responder" 
 * Hopefully with the some slight modification and extra comments it would make it a little easier to read 
 * and understand
*/

#include "dw3000.h"

#define APP_NAME "SS TWR RESP v1.0"

// connection pins
const uint8_t PIN_RST = 27; // reset pin
const uint8_t PIN_IRQ = 34; // irq pin
const uint8_t PIN_SS = 4; // spi select pin

/* Default communication configuration. We use default non-STS DW mode. 
 *  Too scared to change, so keeping it 
*/
static dwt_config_t config = {
        5,               /* Channel number. */
        DWT_PLEN_128,    /* Preamble length. Used in TX only. */
        DWT_PAC8,        /* Preamble acquisition chunk size. Used in RX only. */
        9,               /* TX preamble code. Used in TX only. */
        9,               /* RX preamble code. Used in RX only. */
        1,               /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
        DWT_BR_6M8,      /* Data rate. */
        DWT_PHRMODE_STD, /* PHY header mode. */
        DWT_PHRRATE_STD, /* PHY header rate. */
        (129 + 8 - 8),   /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
        DWT_STS_MODE_OFF, /* STS disabled */
        DWT_STS_LEN_64,/* STS length see allowed values in Enum dwt_sts_lengths_e */
        DWT_PDOA_M0      /* PDOA mode off */
};

/* Default antenna delay values for 64 MHz PRF. See NOTE 2 below. 
 * PRF: Pulse repetition frequency
 * TX: Transmit
 * RX: Receive
 * ANT: Antenna 
 * DLY: Delay
*/
#define TX_ANT_DLY 16385  // in device time unit
#define RX_ANT_DLY 16385  // in device time unit

/*
 * This part is absolutely hideous, instead of using placing the poll or response message
 * in some type of dataclass or struct and use getter methods, it straight up write the message
 * into the array and uses indexes to access each part of the message
 * 
 * The frames used here are Decawave specific ranging frames, complying with the IEEE 802.15.4 standard data frame encoding. T
 * Poll and Response message frame have the same starting format
 *  The first 10 bytes of those frame are common and are composed of the following fields:
 *     - byte 0/1: frame control (0x8841 to indicate a data frame using 16-bit addressing) 
 *                 (the reason it looked backward is because ESP32 is little endian).
 *                 (0x88 indicates the frame is a data frame)
 *                 (0x41 indicates 16-bit short addressing) (AKA uses 16 bit for addressing devices)
 *     - byte 2: sequence number, incremented for each new frame. (help detect and discard duplicate frame and identify missing frame during transmission)
 *     - byte 3/4: PAN ID (0xDECA). (Personal Area Network Identifier, uniquely identify particular wireless network in perhaps same physical location,
 *                 all device within a wireless network must share same PAN ID to communicate with each other)
 *     - byte 5/6: destination address, see NOTE 4 below.
 *     - byte 7/8: source address, see NOTE 4 below.
 *     - byte 9: function code (specific values to indicate which message it is in the ranging process).
 *    The remaining bytes are specific to each message as follows:
 *    Poll message:
 *     - no more data
 *    Response message:
 *     - byte 10 -> 13: poll message reception timestamp.
 *     - byte 14 -> 17: response message transmission timestamp.
 *    All messages end with a 2-byte checksum automatically set by DW IC.
*/
/* Frames used in the ranging process. See NOTE 3 below. */
static uint8_t rx_poll_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0xE0, 0, 0};
static uint8_t tx_resp_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0xE1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
/* Length of the common part of the message (up to and including the function code, see NOTE 3 below). */
#define ALL_MSG_COMMON_LEN 10
/* Index to access some of the fields in the frames involved in the process. */
#define ALL_MSG_SN_IDX 2
#define RESP_MSG_POLL_RX_TS_IDX 10
#define RESP_MSG_RESP_TX_TS_IDX 14
#define RESP_MSG_TS_LEN 4
/* Frame sequence number, incremented after each transmission. */
static uint8_t frame_seq_nb = 0;

/* Buffer to store received messages.
 * Its size is adjusted to longest frame that this example code is supposed to handle. */
#define RX_BUF_LEN 12//Must be less than FRAME_LEN_MAX_EX
static uint8_t rx_buffer[RX_BUF_LEN];

/* Hold copy of status register state here for reference so that it can be examined at a debug breakpoint. */
static uint32_t status_reg = 0;

/* Delay between frames, in UWB microseconds. See NOTE 1 below. */
#ifdef RPI_BUILD
#define POLL_RX_TO_RESP_TX_DLY_UUS 550
#endif //RPI_BUILD
#ifdef STM32F429xx
#define POLL_RX_TO_RESP_TX_DLY_UUS 450
#endif //STM32F429xx
#ifdef NRF52840_XXAA
#define POLL_RX_TO_RESP_TX_DLY_UUS 650
#endif //NRF52840_XXAA

#define POLL_RX_TO_RESP_TX_DLY_UUS 450

/* Timestamps of frames transmission/reception. */
static uint64_t poll_rx_ts; // poll receive timestamp in device time unit
static uint64_t resp_tx_ts; // response receive timestammp in device time unit

/* Values for the PG_DELAY and TX_POWER registers reflect the bandwidth and power of the spectrum at the current
 * temperature. These values can be calibrated prior to taking reference measurements. See NOTE 5 below.
 * txconfig_options: transmit configuration options
 */
extern dwt_txconfig_t txconfig_options;

void setup() {
  UART_init();
  test_run_info((unsigned char *)APP_NAME);

  /* Configure SPI rate, DW3000 supports up to 38 MHz */
  /* Reset DW IC */
  spiBegin(PIN_IRQ, PIN_RST);
  spiSelect(PIN_SS);

  delay(2); // Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC, or could wait for SPIRDY event)

  /*IDLE_RC: One of the possible state that DW3000 Integrated circuit could be in*/
  while (!dwt_checkidlerc()) // Need to make sure DW IC is in IDLE_RC before proceeding
  {
    UART_puts("IDLE FAILED\r\n");
    while (1) ;
  }

  if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR)
  {
    UART_puts("INIT FAILED\r\n");
    while (1) ;
  }

  // Enabling LEDs here for debug so that for each TX the D1 LED will flash on DW3000 red eval-shield boards.
  dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK);

  /* Configure DW IC. See NOTE 6 below. */
  if(dwt_configure(&config)) // if the dwt_configure returns DWT_ERROR either the PLL or RX calibration has failed the host should reset the device
  {
    UART_puts("CONFIG FAILED\r\n");
    while (1) ;
  }

    /* Configure the TX spectrum parameters (power, PG delay and PG count) 
     *  PG delay: delay time when a singal is transmitted and time when power amplifier (PA) is turned on
     *  the purpose of PG delay to give enough time for PA enough time to stabilize
     *  PG count: number of delay cycles that are used to stabilize the power amplifier
    */
    dwt_configuretxrf(&txconfig_options);

    /* Apply default antenna delay value. See NOTE 2 below.
     *  is used for calibration purposes for calculating distance from one DW3000 board to another
    */
    dwt_setrxantennadelay(RX_ANT_DLY);
    dwt_settxantennadelay(TX_ANT_DLY);

    /* Next can enable TX/RX states output on GPIOs 5 and 6 to help debug, and also TX/RX LEDs
     * Note, in real low power applications the LEDs should not be used. 
     * LNA: Low noise amplifier, which amplify weak signals received by antenna
     * PA: power amplifier, which amplify signal to higher power level so it can be transmitted by antenna
     */
    dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE); 
}

void loop() {
        /* Activate reception immediately. */
        dwt_rxenable(DWT_START_RX_IMMEDIATE);

        /* Poll for reception of a frame or error/timeout. See NOTE 6 below. 
         *  `dwt_read32bitreg(SYS_STATUS_ID)`: get the value of status register 
         *  `SYS_STATUS_RXFCG_BIT_MASK`: bit mask for receive frame control good, check if frame is good
         *  `SYS_STATUS_ALL_RX_ERR`: bit mask for any receive error, check if CRC is wrong, incomplete frame message, etc for any error.
        */
        while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_ERR)))
        { };

        if (status_reg & SYS_STATUS_RXFCG_BIT_MASK) // check if frame is received and can be used
        {
            uint32_t frame_len;

            /* Clear good RX frame event in the DW IC status register. 
             *  Clear good receive frame event in the DW3000 integrated circuit status register
            */
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);

            /* A frame has been received, read it into the local buffer.
             *  Read the receive frame infromation regsiter which contain info like frame length and CRC status
             *  RXFLEN_MASK: receive frame length bitmask, used to extract frame length from receive frame information register
            */
            frame_len = dwt_read32bitreg(RX_FINFO_ID) & RXFLEN_MASK;
            if (frame_len <= sizeof(rx_buffer))
            {
                dwt_readrxdata(rx_buffer, frame_len, 0);

                /* Check that the frame is a poll sent by "SS TWR initiator" example.
                 * As the sequence number field of the frame is not relevant, it is cleared to simplify the validation of the frame. */
                rx_buffer[ALL_MSG_SN_IDX] = 0;
                if (memcmp(rx_buffer, rx_poll_msg, ALL_MSG_COMMON_LEN) == 0) // if receive_buffer and receive_response_message is same for length `ALL_MSG_COMMON_LENGTH`
                {
                    uint32_t resp_tx_time;
                    int ret;

                    /* Retrieve poll reception timestamp. 
                     *  `get_rx_timestamp_u64()` get the receive timestamp in u64 data type BUT it's only 40 bits long
                     *  uint64_t get_rx_timestamp_u64(void)
                      {
                          uint8_t ts_tab[5];
                          uint64_t ts = 0;
                          int8_t i;
                          dwt_readrxtimestamp(ts_tab);
                          for (i = 4; i >= 0; i--)
                          {
                              ts <<= 8;
                              ts |= ts_tab[i];
                          }
                          return ts;
                      }
                    */
                    poll_rx_ts = get_rx_timestamp_u64();

                    /* Compute response message transmission time. See NOTE 7 below. 
                     *  POLL_RX_TO_RESP_TX_DLY_UUS: delay between frame of poll receive to response transmit in UWB microseconds
                     *  UUS_TO_DWT_TIME: A constant conversion factor to convert UWB microseconds into DW3000 time?
                     *  
                     *  Delayed transmission time refers to ability of a wireless communication device to schedule a 
                     *  transmission at a specific time.
                     *  2 ** 8 = 256
                     *  2 ** 9 = 512
                     *  
                     *  "As we want to send final TX timestamp in final message, we have to compute it in advance
                     *  instead of relying on reading of DW IC register..." (because if you are reading from DW IC register
                     *  for transmission time it won't be accurate so you need to compute ahead of time)
                     *  "Timestamps and delayed transmission time are both expressed(represented) in device time units, so we just
                     *  have to add the desired response delay to response RX timestamp"
                     *  
                     *  `UUS`: Ultra wideband microsecond
                     *  `DWT`: Device time unit
                     *  Delayed tranmission time resolution
                     *  
                     *  From `dw3000_device_api.cpp`
                     *  void dwt_setdelayedtrxtime(uint32_t starttime)
                        {
                            dwt_write32bitoffsetreg(DX_TIME_ID, 0, starttime); // Note: bit 0 of this register is ignored
                        } // end dwt_setdelayedtrxtime()
                        
             
                     *  From `dw_shared_functions_defines.h`
                     *  UWB microsecond (uus) to device time unit (dtu, around 15.65 ps) conversion factor.
                     *  1 uus = 512 / 499.2 µs and 1 µs = 499.2 * 128 dtu. 
                     *  #define UUS_TO_DWT_TIME 63898
                     *  
                     *  poll_rx_ts: poll receive timestamp in 40 bit with u64 type in device time unit
                     *  POLL_RX_TO_RESP_TX_DLY_UUS: delay between frame of poll receive to response transmit in UWB microseconds
                     *  UUS_TO_DWT_TIME: constant to convert Ultrawide band microsecond to device time unit
                     *  resp_tx_time = response transmit time in 512 device time units represented with 32 unsigned bit
                     *  dwt_setdelayedtrxtime(uint32_t startime): decawave transceiver toolset set dealyed transmission time
                     *  
                     *  poll receive to response transmit delay in ultrawideband microsecond:
                     *  POLL_RX_TO_RESP_TX_DLY_UUS;
                     *  
                     *  poll receive to response transmit delay in device time unit:
                     *  POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME;
                     *  
                     *  response transmit [date]time (in which the response message will be transmitted)
                     *  represented with 40 bit with type u64 in resoultion of 1 device time unit:
                     *  poll_rx_ts + (POLL_RX_TORESP_TX_DLY_UUS * UUS_TO_DWT_TIME);
                     *  
                     *  response transmit [date]time (in which the reponse message will be transmitted)
                     *  represented with 32 bit with type uint64_t in resolution of 256 device time unit:
                     *  ( poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME) ) >> 8;
                     *  
                     *  assign response transmit [date]time represented with 32 bit with uint64_t in resolution of
                     *  256 device time unit to variable `resp_tx_time` with type uint32_t:
                     *  resp_tx_time = (poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
                     *  
                     *  set the response transmit [date]time which have a resolution of 512 device time units, 
                     *  which are given resp_tx_time which has resolution of 256 device time unit with ignoring the
                     *  0th bit which makes it 512 device time units
                     *  dwt_setdelayedtrxtime(resp_tx_time);
                    */
                    resp_tx_time = (poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
                    dwt_setdelayedtrxtime(resp_tx_time);

                    /* Response TX timestamp is the transmission time we programmed plus the antenna delay.
                     *  0xFFFFFFFEUL  , the `UL` part is supposed to mean unsigned long (32 bit unsigned integer)
                     *  From `dw_shared_functions_defines.h`
                     *  UWB microsecond (uus) to device time unit (dtu, around 15.65 ps) conversion factor.
                     *  1 uus = 512 / 499.2 µs and 1 µs = 499.2 * 128 dtu. 
                     *  
                     *  (2 ** 32) * (15.65 * (10 ** -12)) = 0.067216 
                     *  2 ** 9 = 512
                     *  
                     *  resp_tx_ts: response transmit timestamp represented with 40 bits in u64 type in device time unit
                     *  #define DWT_TIME_UNITS      (1.0/499.2e6/128.0) //!< = 15.65e-12 s
                     *  
                     *  `resp_tx_time`: reponse transmit time represented in 32 bit with uint32_t with unit as (512 device time unit)
                     *  
                     *  response transmit time in 32 bit with uint32_t with unit as (256 device time unit): 
                     *  resp_tx_time & 0xFFFFFFFEUL;
                     *  
                     *  response transmit time in 32 bit with uint64_t with unit as (256 device time unit):
                     *  (uint64_t)(resp_tx_time & 0xFFFFFFFEUL);
                     *  
                     *  response transmit time in 40 bit with uint64_t with unit as (1 device time unit):
                     *  ((uint64_t)(resp_tx_time & 0xFFFFFFFEUL) << 8);
                     *  
                     *  response trnasmit time in 40 bit with uint64_t in (1 device time unit) with transmit antenna delay:
                     *  (((uint64_t)(resp_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;
                     *  
                    */
                    resp_tx_ts = (((uint64_t)(resp_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;

                    /* Write all timestamps in the final message. See NOTE 8 below.
                     *  
                     * set the poll receive timestamp part of response message with the first 32 bit 
                     * of the 40 bit poll_rx_ts in uint64_t with unit as (1 device time unit)
                     * response_message.set_poll_receive_timestamp(poll_rx_ts[0:32+1])
                     * resp_msg_set_ts(&tx_resp_msg[RESP_MSG_POLL_RX_TS_IDX], poll_rx_ts);  
                     * 
                     * set the response transmit timestamp part of response message with the first 32 bit 
                     * of the 40 bit resp_tx_ts in uint64_t with unit as (1 device time unit)
                     * response_message.set_response_transmit_timestamp(resp_tx_ts[0:32+1])
                     * resp_msg_set_ts(&tx_resp_msg[RESP_MSG_RESP_TX_TS_IDX], poll_rx_ts); 
                     * 
                     * 
                    */
                    resp_msg_set_ts(&tx_resp_msg[RESP_MSG_POLL_RX_TS_IDX], poll_rx_ts);
                    resp_msg_set_ts(&tx_resp_msg[RESP_MSG_RESP_TX_TS_IDX], resp_tx_ts);

                    /* Write and send the response message. See NOTE 9 below. */
                    tx_resp_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
                    // dwt_writetxdata = write transmit data
                    dwt_writetxdata(sizeof(tx_resp_msg), tx_resp_msg, 0); /* Zero offset in TX buffer. */
                    // dwt_writetxfctrl = write to transmit frame control register  
                    dwt_writetxfctrl(sizeof(tx_resp_msg), 0, 1); /* Zero offset in TX buffer, ranging. */

                    /*
                     ------------------------------------------------------------------------------------------------------------------
                       * @brief This call initiates the transmission, input parameter indicates which TX mode is used see below
                       *
                       * input parameters:
                       * @param mode - if mode = DWT_START_TX_IMMEDIATE - immediate TX (no response expected)
                       *               if mode = DWT_START_TX_DELAYED - delayed TX (no response expected)  at specified time (time in DX_TIME register)
                       *               if mode = DWT_START_TX_DLY_REF - delayed TX (no response expected)  at specified time (time in DREF_TIME register + any time in DX_TIME register)
                       *               if mode = DWT_START_TX_DLY_RS  - delayed TX (no response expected)  at specified time (time in RX_TIME_0 register + any time in DX_TIME register)
                       *               if mode = DWT_START_TX_DLY_TS  - delayed TX (no response expected)  at specified time (time in TX_TIME_LO register + any time in DX_TIME register)
                       *               if mode = DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED - immediate TX (response expected - so the receiver will be automatically turned on after TX is done)
                       *               if mode = DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED - delayed TX (response expected - so the receiver will be automatically turned on after TX is done)
                       *               if mode = DWT_START_TX_CCA - Send the frame if no preamble detected within PTO time
                       *               if mode = DWT_START_TX_CCA  | DWT_RESPONSE_EXPECTED - Send the frame if no preamble detected within PTO time and then enable RX
                       * output parameters
                       *
                       * returns DWT_SUCCESS for success, or DWT_ERROR for error (e.g. a delayed transmission will be cancelled if the delayed time has passed)
                      int dwt_starttx(uint8_t mode);
                    */
                    ret = dwt_starttx(DWT_START_TX_DELAYED);

                    /* If dwt_starttx() returns an error, abandon this ranging exchange and proceed to the next one. See NOTE 10 below. */
                    if (ret == DWT_SUCCESS)
                    {
                        /* Poll DW IC until TX frame sent event set. See NOTE 6 below. */
                        while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS_BIT_MASK))
                        { };

                        /* Clear TXFRS event. transmit frame sent event from status register */
                        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);

                        /* Increment frame sequence number after transmission of the poll message (modulo 256). */
                        frame_seq_nb++;
                    }
                }
            }
        }
        else
        {
            /* Clear RX error events in the DW IC status register. clear receive frame error from status register */
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
        }
}

/*****************************************************************************************************************************************************
 * NOTES:
 *
 * 1. The single-sided two-way ranging scheme implemented here has to be considered carefully as the accuracy of the distance measured is highly
 *    sensitive to the clock offset error between the devices and the length of the response delay between frames. To achieve the best possible
 *    accuracy, this response delay must be kept as low as possible. In order to do so, 6.8 Mbps data rate is used in this example and the response
 *    delay between frames is defined as low as possible. The user is referred to User Manual for more details about the single-sided two-way ranging
 *    process.
 *
 *    Initiator: |Poll TX| ..... |Resp RX|
 *    Responder: |Poll RX| ..... |Resp TX|
 *                   ^|P RMARKER|                    - time of Poll TX/RX
 *                                   ^|R RMARKER|    - time of Resp TX/RX
 *
 *                       <--TDLY->                   - POLL_TX_TO_RESP_RX_DLY_UUS (RDLY-RLEN)
 *                               <-RLEN->            - RESP_RX_TIMEOUT_UUS   (length of response frame)
 *                    <----RDLY------>               - POLL_RX_TO_RESP_TX_DLY_UUS (depends on how quickly responder can turn around and reply)
 *
 *
 * 2. The sum of the values is the TX to RX antenna delay, experimentally determined by a calibration process. Here we use a hard coded typical value
 *    but, in a real application, each device should have its own antenna delay properly calibrated to get the best possible precision when performing
 *    range measurements.
 * 3. The frames used here are Decawave specific ranging frames, complying with the IEEE 802.15.4 standard data frame encoding. The frames are the
 *    following:
 *     - a poll message sent by the initiator to trigger the ranging exchange.
 *     - a response message sent by the responder to complete the exchange and provide all information needed by the initiator to compute the
 *       time-of-flight (distance) estimate.
 *    The first 10 bytes of those frame are common and are composed of the following fields:
 *     - byte 0/1: frame control (0x8841 to indicate a data frame using 16-bit addressing).
 *     - byte 2: sequence number, incremented for each new frame.
 *     - byte 3/4: PAN ID (0xDECA).
 *     - byte 5/6: destination address, see NOTE 4 below.
 *     - byte 7/8: source address, see NOTE 4 below.
 *     - byte 9: function code (specific values to indicate which message it is in the ranging process).
 *    The remaining bytes are specific to each message as follows:
 *    Poll message:
 *     - no more data
 *    Response message:
 *     - byte 10 -> 13: poll message reception timestamp.
 *     - byte 14 -> 17: response message transmission timestamp.
 *    All messages end with a 2-byte checksum automatically set by DW IC.
 * 4. Source and destination addresses are hard coded constants in this example to keep it simple but for a real product every device should have a
 *    unique ID. Here, 16-bit addressing is used to keep the messages as short as possible but, in an actual application, this should be done only
 *    after an exchange of specific messages used to define those short addresses for each device participating to the ranging exchange.
 * 5. In a real application, for optimum performance within regulatory limits, it may be necessary to set TX pulse bandwidth and TX power, (using
 *    the dwt_configuretxrf API call) to per device calibrated values saved in the target system or the DW IC OTP memory.
 * 6. We use polled mode of operation here to keep the example as simple as possible but all status events can be used to generate interrupts. Please
 *    refer to DW IC User Manual for more details on "interrupts". It is also to be noted that STATUS register is 5 bytes long but, as the event we
 *    use are all in the first bytes of the register, we can use the simple dwt_read32bitreg() API call to access it instead of reading the whole 5
 *    bytes.
 * 7. As we want to send final TX timestamp in the final message, we have to compute it in advance instead of relying on the reading of DW IC
 *    register. Timestamps and delayed transmission time are both expressed in device time units so we just have to add the desired response delay to
 *    response RX timestamp to get final transmission time. The delayed transmission time resolution is 512 device time units which means that the
 *    lower 9 bits of the obtained value must be zeroed. This also allows to encode the 40-bit value in a 32-bit words by shifting the all-zero lower
 *    8 bits.
 * 8. In this operation, the high order byte of each 40-bit timestamps is discarded. This is acceptable as those time-stamps are not separated by
 *    more than 2**32 device time units (which is around 67 ms) which means that the calculation of the round-trip delays (needed in the
 *    time-of-flight computation) can be handled by a 32-bit subtraction.
 * 9. dwt_writetxdata() takes the full size of the message as a parameter but only copies (size - 2) bytes as the check-sum at the end of the frame is
 *    automatically appended by the DW IC. This means that our variable could be two bytes shorter without losing any data (but the sizeof would not
 *    work anymore then as we would still have to indicate the full length of the frame to dwt_writetxdata()).
 * 10. When running this example on the DW3000 platform with the POLL_RX_TO_RESP_TX_DLY response delay provided, the dwt_starttx() is always
 *     successful. However, in cases where the delay is too short (or something else interrupts the code flow), then the dwt_starttx() might be issued
 *     too late for the configured start time. The code below provides an example of how to handle this condition: In this case it abandons the
 *     ranging exchange and simply goes back to awaiting another poll message. If this error handling code was not here, a late dwt_starttx() would
 *     result in the code flow getting stuck waiting subsequent RX event that will will never come. The companion "initiator" example (ex_06a) should
 *     timeout from awaiting the "response" and proceed to send another poll in due course to initiate another ranging exchange.
 * 11. The user is referred to DecaRanging ARM application (distributed with EVK1000 product) for additional practical example of usage, and to the
 *     DW IC API Guide for more details on the DW IC driver functions.
 * 12. In this example, the DW IC is put into IDLE state after calling dwt_initialise(). This means that a fast SPI rate of up to 20 MHz can be used
 *     thereafter.
 * 13. Desired configuration by user may be different to the current programmed configuration. dwt_configure is called to set desired
 *     configuration.
 ****************************************************************************************************************************************************/
