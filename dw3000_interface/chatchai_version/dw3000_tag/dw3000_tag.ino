/*
 * A format with white space and extra comments of the DW3000 example code by MakerFabs for "ex_06a_ss_twr_initiator"
 * Hopefully with the added comment and some formatting of the code will be easier to use since the library
 * for DW3000 has an interface for lower-level control
*/

#include "dw3000.h"


#define APP_NAME "SS TWR INIT v1.0"

// connection pins
const uint8_t PIN_RST = 27; // reset pin
const uint8_t PIN_IRQ = 34; // irq pin (interrupt request)
const uint8_t PIN_SS = 4; // spi select pin 


/* Default communication configuration. We use default non-STS DW mode. 
 *  I am too afraid to change anything here, so keeping it while explain
 *  The `dwt_config_t` is the config used by DW3000 chip for communication paramters
 *  STS: Short-term synchronization is a feature in DW3000 chip that allow more precise time sync
 *  at the cost of increase power consumption
 *  DW: Double word, menat fetch two "word" at once (in 32 bit system, meant it grab 64 bit)
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

/* Inter-ranging delay period, in milliseconds. */
/* The time in between sending ranging delay (AKA update every `RNG_DELAY_MS`)*/
#define RNG_DELAY_MS 1000

/* Default antenna delay values for 64 MHz PRF. See NOTE 2 below. 
 *  The delay here is used for making more precise calibration of the distance,
 *  since the anchor need time to process a response to the poll message sent by the tag
 *  PRF: Pulse repetition frequency
 *  TX: Transmit
 *  RX: Receive
 *  DLY: Delay
 *  ANT: Antenna
*/
#define TX_ANT_DLY 16385
#define RX_ANT_DLY 16385

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
/*                                0     1   2   3     4     5    6    7    8    9   10  11 12 13 14 15 16 17 18 19*/
static uint8_t tx_poll_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0xE0, 0, 0};
static uint8_t rx_resp_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0xE1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
/* Length of the common part of the message (up to and including the function code, see NOTE 3 below). */
#define ALL_MSG_COMMON_LEN 10
/* Indexes to access some of the fields in the frames defined above. */
#define ALL_MSG_SN_IDX 2 // all message sequence number index
#define RESP_MSG_POLL_RX_TS_IDX 10 // response message poll receive timestamp index
#define RESP_MSG_RESP_TX_TS_IDX 14 // response message response transmit timestamp index
#define RESP_MSG_TS_LEN 4 // response message timestamp length (timestamp is 32 bit)
/* Frame sequence number, incremented after each transmission. */
static uint8_t frame_seq_nb = 0;


/* Buffer to store received response message.
 * Its size is adjusted to longest frame that this example code is supposed to handle. */
#define RX_BUF_LEN 20
static uint8_t rx_buffer[RX_BUF_LEN];

/* Hold copy of status register state here for reference so that it can be examined at a debug breakpoint. */
static uint32_t status_reg = 0;

/* Delay between frames, in UWB microseconds. See NOTE 1 below.
 * POLL_TX_TO_RESP_RX_DLY_UUS is a constant that represents delay between frames in Ultra-Wideband microseconds
 * This is used for introducing delay between reception of a polling frame and transmission of a response frame
 * in a UWB communication protocol. 
 * To ensure the two frames don't interfere with each other [on the radio wave] and UWB radio can properly
 * distinguish them
*/
#ifdef RPI_BUILD
#define POLL_TX_TO_RESP_RX_DLY_UUS 240
#endif //RPI_BUILD
#ifdef STM32F429xx
#define POLL_TX_TO_RESP_RX_DLY_UUS 240
#endif //STM32F429xx
#ifdef NRF52840_XXAA
#define POLL_TX_TO_RESP_RX_DLY_UUS 240
#endif //NRF52840_XXAA
/* Receive response timeout. See NOTE 5 below. */
#ifdef RPI_BUILD
#define RESP_RX_TIMEOUT_UUS 270
#endif //RPI_BUILD
#ifdef STM32F429xx
#define RESP_RX_TIMEOUT_UUS 210
#endif //STM32F429xx
#ifdef NRF52840_XXAA
#define RESP_RX_TIMEOUT_UUS 400
#endif //NRF52840_XXAA

#define POLL_TX_TO_RESP_RX_DLY_UUS 240
#define RESP_RX_TIMEOUT_UUS 400


/* Hold copies of computed time of flight and distance here for reference so that it can be examined at a debug breakpoint. */
static double tof; // time of flight
static double distance;

/* Values for the PG_DELAY and TX_POWER registers reflect the bandwidth and power of the spectrum at the current
 * temperature. These values can be calibrated prior to taking reference measurements. See NOTE 2 below. */
extern dwt_txconfig_t txconfig_options;

void setup() {
  UART_init();
  test_run_info((unsigned char *)APP_NAME);

  /* Configure SPI rate, DW3000 supports up to 38 MHz */
  /* Reset DW IC */
  spiBegin(PIN_IRQ, PIN_RST);
  spiSelect(PIN_SS);

  delay(2); // Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC, or could wait for SPIRDY event)

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
  * Antenna delay is used for calibration purposes for calculating distance
  */
  dwt_setrxantennadelay(RX_ANT_DLY);
  dwt_settxantennadelay(TX_ANT_DLY);

  /* Set expected response's delay and timeout. See NOTE 1 and 5 below.
   * As this example only handles one incoming frame with always the same delay and timeout, those values can be set here once for all.
   * setrxaftertxdelay: set delay between transmitting message and turning on the receiver to listen for a response
   * setrxtimeout: set timeout for receiver to listen for a response, if receiver does not 
   * receive a response within this time, it will stop listening and assume that
   * the response was not received
   */
  dwt_setrxaftertxdelay(POLL_TX_TO_RESP_RX_DLY_UUS);
  dwt_setrxtimeout(RESP_RX_TIMEOUT_UUS);

  /* Next can enable TX/RX states output on GPIOs 5 and 6 to help debug, and also TX/RX LEDs
   * Note, in real low power applications the LEDs should not be used. 
   * LNA: Low noise amplifier, which amplify weak signals received by antenna
   * PA: power amplifier, which amplify signal to higher power level so it can be transmitted by antenna
   */
  dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE);
}

void loop() {
        
        /* Write frame data to DW IC and prepare transmission. See NOTE 7 below. */
        tx_poll_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
        
        /*
         * TXFRS - Transmit frame sent
         * Write to status register that the transmit frame was sent
        */
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK); 
        
        dwt_writetxdata(sizeof(tx_poll_msg), tx_poll_msg, 0); /* Zero offset in TX buffer. */
        /* 
         *  Size of frame
         *  Zero offset in TX buffer, ranging. 
         *  Write to transmit frame control register the (size_of_frame, offset, transmit_data_rate_and_preamble_length)
         *  Which by default 1, should be [6.8 Mbps and 128 symbol preample length]? (by chatGPT)
        */
        dwt_writetxfctrl(sizeof(tx_poll_msg), 0, 1); 

        /* Start transmission, indicating that a response is expected so that reception is enabled automatically after the frame is sent and the delay
         * set by dwt_setrxaftertxdelay() has elapsed. */
        dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);

        /* We assume that the transmission is achieved correctly, poll for reception of a frame or error/timeout. See NOTE 8 below. 
         * Stay in the loop until a frame is received, an error occured, or timeout occurs
         * BITMASK:
         * `SYS_STATUS_RXFCG: Receive frame control good, check if good frame received and content available to read 
         * `SYS_STATUS_ALL_RX_TO`: All receive timeout, check if receive timeout has occured for any reason, including no frame received
         * `SYS_STATUS_ALL_RX_ERR`: All receive errors, check whether receive error occurs for any reason including CRC error, invalid frame format
        */
        while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
        { };

        /* Increment frame sequence number after transmission of the poll message (modulo 256). */
        frame_seq_nb++;
        
        if (status_reg & SYS_STATUS_RXFCG_BIT_MASK) // check if 'receive frame control good` AKA receive the response frame with no error
        {
            uint32_t frame_len;

            /* Clear good RX frame event in the DW IC status register.
             * Write to status register that receive the response frame sucessfully
            */
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);

            /* A frame has been received, read it into the local buffer.
             *  Read the receive frame information register which contain info like frame length and CRC status
             *  RXFLEN_MASK: receive frame length bitmask, to extract frame length from receive frame information register
            */
            frame_len = dwt_read32bitreg(RX_FINFO_ID) & RXFLEN_MASK;
            
            if (frame_len <= sizeof(rx_buffer))
            {   
                // read the received frame data into `rx_buffer` with length `frame_len` with offset of 0
                dwt_readrxdata(rx_buffer, frame_len, 0);

                /* Check that the frame is the expected response from the companion "SS TWR responder" example.
                 * As the sequence number field of the frame is not relevant, it is cleared to simplify the validation of the frame. */
                rx_buffer[ALL_MSG_SN_IDX] = 0;

                // if receive_buffer and receive_response_message is the same uptil the `ALL_MSG_COMMON_LEN` return true
                if (memcmp(rx_buffer, rx_resp_msg, ALL_MSG_COMMON_LEN) == 0) 
                {
                    uint32_t poll_tx_ts, resp_rx_ts, poll_rx_ts, resp_tx_ts;
                    int32_t rtd_init, rtd_resp;
                    float clockOffsetRatio ;

                    /* Retrieve poll transmission and response reception timestamps. See NOTE 9 below.
                     *  poll_tx_ts: poll transmit timestamp
                     *  resp_tx_ts: response receive timestamp
                    */
                    poll_tx_ts = dwt_readtxtimestamplo32();
                    resp_rx_ts = dwt_readrxtimestamplo32();

                    /* Read carrier integrator value and calculate clock offset ratio. See NOTE 11 below. 
                     *  SS-TWR: Symmetric double sided two-way ranging, a method for measuring time of flight of radio signal between 2 devices
                     *  PPM: Parts per million, a clock offset of N PPM mean the clock is running N/1_000_000 faster/slower than reference clock
                     *  Local initiator unit's clock: The sensor that sent the poll message clock 
                     *  `dwt_readclockoffset()`: reads the clockoffset register in the DW3000 chip, the clock offset
                     *  value is 21 bit signed integer that represents the local osciallator frequency of the chip
                     *  and the frequency of an ideal oscillator frequencies due to temperature or aging effect
                     *  (Recall that clocks depend on oscillator crystals using piezoelectirc effect)
                     *  `dwt_readclockoffset()` is called on the initiator side and it reads 
                     *  the clock offset value of the responder side. 
                     *  ?? Still confused how `dwt_readclockoffset()` get the offset of what initiator or responder???
                     *  2 ** 26 = roughly 67,100,000 
                     *   NOTE 11. The use of the clock offset value to correct the TOF calculation, significantly 
                     *   improves the result of the SS-TWR where the remote 
                     *   responder unit's clock is a number of PPM offset from the local initiator unit's clock. 
                    */
                    clockOffsetRatio = ((float)dwt_readclockoffset()) / (uint32_t)(1<<26);

                    /* Get timestamps embedded in response message. 
                     *  poll_rx_ts: poll receive timestamp
                     *  resp_tx_ts: response transmit timestamp
                    */
                    resp_msg_get_ts(&rx_buffer[RESP_MSG_POLL_RX_TS_IDX], &poll_rx_ts);
                    resp_msg_get_ts(&rx_buffer[RESP_MSG_RESP_TX_TS_IDX], &resp_tx_ts);

                    /* Compute time of flight and distance, using clock offset ratio to correct for differing local and remote clock rates */
                    rtd_init = resp_rx_ts - poll_tx_ts; // rtd_init: round trip delay initiate
                    rtd_resp = resp_tx_ts - poll_rx_ts; // rtd_resp: round trip delay respond

                    /*This uses two way ranging scheme?
                     * Timing diagram of poll and response message between initiator and responder
                     *  Initiator    Responder    Time
                     *          |    |             |
                     *      pt  |\   |             |   
                     *          | \  |             V
                     *          |  \ |
                     *          |   \| pr
                     *          |    |
                     *          |    |
                     *          |   /| rt
                     *          |  / | 
                     *          | /  |
                     *      rr  |/   |
                     *   pt: poll transmit timestamp = poll_tx_ts  
                     *   pr: poll receive timestamp = poll_rx_ts
                     *   rt: response transmit timestamp = resp_tx_ts
                     *   rr: response receive timestamp = resp_rx_ts
                     *   
                     *   rtd_init: round trip initiator = rr - pt
                     *   rtd_resp: round trip responder = rt - pr
                     *   
                     *   
                     *   time of flight = ( (rr - pt) - (rt - pr) ) / 2
                     *                  = ( rtd_init - rtd_resp ) / 2
                     *                  
                     *   clockOffSetRatio is used for calibrating the difference in ?? clock speed ?? since the timestamp
                     *   recording is done between by two different clocks (first clock is initiator's clock, second clock is 
                     *   responder clock)
                     *   
                     *   #define DWT_TIME_UNITS      (1.0/499.2e6/128.0) //!< = 15.65e-12 
                     *   
                     *   distance = time of flight * SPEED_OF_LIGHT
                     *   Since radio wave travel at the speed of light since it is an EM wave
                    */
                    tof = ( ( rtd_init - rtd_resp * (1 - clockOffsetRatio) ) / 2.0) * DWT_TIME_UNITS;
                    distance = tof * SPEED_OF_LIGHT;

                    /* Display computed distance on LCD. */
                    snprintf(dist_str, sizeof(dist_str), "DIST: %3.2f m", distance);
                    test_run_info((unsigned char *)dist_str);
                    
                }
            }
        }
        else
        {
            /* Clear RX error/timeout events in the DW IC status register. */
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
        }

        /* Execute a delay between ranging exchanges. */
        Sleep(RNG_DELAY_MS);
}

/*****************************************************************************************************************************************************
 * NOTES:
 *
 * 1. The single-sided two-way ranging scheme implemented here has to be considered carefully as the accuracy of the distance measured is highly
 *    sensitive to the clock offset error between the devices and the length of the response delay between frames. To achieve the best possible
 *    accuracy, this response delay must be kept as low as possible. In order to do so, 6.8 Mbps data rate is used in this example and the response
 *    delay between frames is defined as low as possible. The user is referred to User Manual for more details about the single-sided two-way ranging
 *    process.  NB:SEE ALSO NOTE 11.
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
 * 2. The sum of the values is the TX to RX antenna delay, this should be experimentally determined by a calibration process. Here we use a hard coded
 *    value (expected to be a little low so a positive error will be seen on the resultant distance estimate). For a real production application, each
 *    device should have its own antenna delay properly calibrated to get good precision when performing range measurements.
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
 * 5. This timeout is for complete reception of a frame, i.e. timeout duration must take into account the length of the expected frame. Here the value
 *    is arbitrary but chosen large enough to make sure that there is enough time to receive the complete response frame sent by the responder at the
 *    6.8M data rate used (around 200 µs).
 * 6. In a real application, for optimum performance within regulatory limits, it may be necessary to set TX pulse bandwidth and TX power, (using
 *    the dwt_configuretxrf API call) to per device calibrated values saved in the target system or the DW IC OTP memory.
 * 7. dwt_writetxdata() takes the full size of the message as a parameter but only copies (size - 2) bytes as the check-sum at the end of the frame is
 *    automatically appended by the DW IC. This means that our variable could be two bytes shorter without losing any data (but the sizeof would not
 *    work anymore then as we would still have to indicate the full length of the frame to dwt_writetxdata()).
 * 8. We use polled mode of operation here to keep the example as simple as possible but all status events can be used to generate interrupts. Please
 *    refer to DW IC User Manual for more details on "interrupts". It is also to be noted that STATUS register is 5 bytes long but, as the event we
 *    use are all in the first bytes of the register, we can use the simple dwt_read32bitreg() API call to access it instead of reading the whole 5
 *    bytes.
 * 9. The high order byte of each 40-bit time-stamps is discarded here. This is acceptable as, on each device, those time-stamps are not separated by
 *    more than 2**32 device time units (which is around 67 ms) which means that the calculation of the round-trip delays can be handled by a 32-bit
 *    subtraction.
 * 10. The user is referred to DecaRanging ARM application (distributed with EVK1000 product) for additional practical example of usage, and to the
 *     DW IC API Guide for more details on the DW IC driver functions.
 * 11. The use of the clock offset value to correct the TOF calculation, significantly improves the result of the SS-TWR where the remote
 *     responder unit's clock is a number of PPM offset from the local initiator unit's clock.
 *     As stated in NOTE 2 a fixed offset in range will be seen unless the antenna delay is calibrated and set correctly.
 * 12. In this example, the DW IC is put into IDLE state after calling dwt_initialise(). This means that a fast SPI rate of up to 20 MHz can be used
 *     thereafter.
 * 13. Desired configuration by user may be different to the current programmed configuration. dwt_configure is called to set desired
 *     configuration.
 ****************************************************************************************************************************************************/
