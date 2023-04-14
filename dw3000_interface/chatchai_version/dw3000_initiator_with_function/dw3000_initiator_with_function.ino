/*
 * A refactored rewrite of the "ex_06a_ss_twr_initiator" example for ESP32 DW3000 Makerfabs
 * This time with using of function and more well defined name whenever its possible
 * to make the code easier to read
*/

#include "dw3000.h"

#define APP_NAME "SS TWR Init refactored with function v1.0"

// connections pins
const uint8_t PIN_RESET = 27;
const uint8_t PIN_INTERRUPT_REQUEST = 34;
const uint8_t PIN_SLAVE_SELECT = 4;


// I am too scared to change anything for communication protocol
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


#define RANGING_DELAY_MILLISECOND 1000


// ONE DEVICE TIME UNIT = (1.0/499.2e6/128.0) //!< = 15.65e-12 seconds
#define TRANSMIT_ANTENNA_DELAY_IN_DEVICE_TIME_UNIT 16385
#define RECEIVE_ANTENNA_DELAY_IN_DEVICE_TIME_UNIT 16385


/*
 *  * The frames used here are Decawave specific ranging frames, complying with the IEEE 802.15.4 standard data frame encoding. T
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
//                                           0     1     2  3     4      5    6    7    8   9     10 11 12 13 14 15 16 17 18 19                    
static uint8_t poll_message[]             = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0xE0, 0, 0};
static uint8_t receive_response_message[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0xE1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Index to access some of the fields in the frames involved 
#define ALL_MESSAGE_COMMON_LENGTH_IN_BYTE 10
#define ALL_MESSAGE_SEQUENCE_NUMBER_INDEX 10
#define RESPONSE_MESSAGE_POLL_RECEIVE_TIMESTAMP_INDEX 10
#define RESPONSE_MESSAGE_RESPONSE_TRANSMIT_TIMESTAMP_INDEX 14
#define RESPONSE_MESSAGE_TIMESTAMP_LENGTH_IN_BYTE 4

static uint8_t frame_sequence_number = 0;

#define RECEIVE_BUFFER_LENGTH_IN_BYTE 20
static uint8_t receive_buffer[RECEIVE_BUFFER_LENGTH_IN_BYTE];


// status register is for checking if frame is sent/receive properly, any error that occured will be showed here
static uint32_t status_register = 0;


// Delay between frames, in UWB microseconds, depending on the board type (although here it's all the same)
#ifdef RPI_BUILD
#define POLL_TRANSMIT_TO_RESPONSE_RECEIVE_DELAY_IN_UWB_MICROSECOND 240
#endif //RPI_BUILD
#ifdef STM32F429xx
#define POLL_TRANSMIT_TO_RESPONSE_RECEIVE_DELAY_IN_UWB_MICROSECOND 240
#endif //STM32F429xx
#ifdef NRF52840_XXAA
#define POLL_TRANSMIT_TO_RESPONSE_RECEIVE_DELAY_IN_UWB_MICROSECOND 240
#endif //NRF52840_XXAA
/* Receive response timeout. See NOTE 5 below. */
#ifdef RPI_BUILD
#define RESPONSE_RECEIVE_TIMEOUT_IN_UWB_MICROSECOND 270
#endif //RPI_BUILD
#ifdef STM32F429xx
#define RESPONSE_RECEIVE_TIMEOUT_IN_UWB_MICROSECOND 210
#endif //STM32F429xx
#ifdef NRF52840_XXAA
#define RESPONSE_RECEIVE_TIMEOUT_IN_UWB_MICROSECOND 400
#endif //NRF52840_XXAA

#define POLL_TRANSMIT_TO_RESPONSE_RECEIVE_DELAY_IN_UWB_MICROSECOND 240
#define RESPONSE_RECEIVE_TIMEOUT_IN_UWB_MICROSECOND 400


// holding copies of time of flight and distance
static double time_of_flight;
static double distance;



// transmit configuration options, the variable name MUST be `txconfig_options` because of the extern keyword
// from the `dw3000_config_options.cpp`
extern dwt_txconfig_t txconfig_options;



// Functions for abstracting some info
void poll_message_initialize(){
  poll_message[ALL_MESSAGE_SEQUENCE_NUMBER_INDEX] = frame_sequence_number;
}

void poll_message_transmit_message(){
  
   /* TXFRS - Transmit frame sent
   * Write to status register that the transmit frame was sent */
  dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK); 
  
  dwt_writetxdata(sizeof(poll_message), poll_message, 0); /* Zero offset in TX buffer. */
  /* 
   *  Size of frame
   *  Zero offset in TX buffer, ranging. 
   *  Write to transmit frame control register the (size_of_frame, offset, transmit_data_rate_and_preamble_length)
   *  Which by default 1, should be [6.8 Mbps and 128 symbol preample length]? (by chatGPT)
  */
  dwt_writetxfctrl(sizeof(poll_message), 0, 1); 

  /* Start transmission, indicating that a response is expected so that reception is enabled automatically after the frame is sent and the delay
   * set by dwt_setrxaftertxdelay() has elapsed. */
  dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);

}

void poll_message_increment_sequence_number(){
  frame_sequence_number++;
  poll_message[ALL_MESSAGE_SEQUENCE_NUMBER_INDEX] = frame_sequence_number;
}


// status register 
bool status_register_is_receive_frame_control_good(){
  status_register = dwt_read32bitreg(SYS_STATUS_ID);
  return (status_register & SYS_STATUS_RXFCG_BIT_MASK);
}

bool status_register_is_receive_timeout(){
  status_register = dwt_read32bitreg(SYS_STATUS_ID);
  return (status_register & SYS_STATUS_ALL_RX_TO);
}

bool status_register_is_receive_error(){
  status_register = dwt_read32bitreg(SYS_STATUS_ID);
  return (status_register & SYS_STATUS_ALL_RX_ERR);
}

bool status_register_is_receive_done(){
  return status_register_is_receive_frame_control_good() ||
  status_register_is_receive_timeout() ||
  status_register_is_receive_error();
}

void status_register_clear_receive_frame_control_good(){
  dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);
}

void status_register_clear_receive_timeout(){
  dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO);
}

void status_register_clear_receive_error(){
  dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
}


// receive frame info register
uint32_t receive_frame_info_register_get_frame_length(){
  return dwt_read32bitreg(RX_FINFO_ID) & RXFLEN_MASK;
}


// receive_buffer 
void receive_buffer_read_receive_data(uint32_t frame_length){
  // read the receive frame data into 'receive_buffer' with length 'frame_length' with offset of 0
  dwt_readrxdata(receive_buffer, frame_length, 0); 
}

void receive_buffer_set_sequence_number(uint8_t in_seq_number){
  receive_buffer[ALL_MESSAGE_SEQUENCE_NUMBER_INDEX] = in_seq_number;
}

uint32_t receive_buffer_get_poll_receive_timestamp(){
  /*
   * Poll receive timestamp given by the responder is only the first 32 bit out of 40 bit
   * a timestamp in device time unit, (2 ** 32 * device time unit = roughly 67 millisecond) (device time unit = roughly 15.65 picosecond) 
   * (67 millisecond * speed of light = roughly 2 million meter)
   * (thus first 32 bit is all you really need to calculate distance less than the size of the earth)  
  */
  uint32_t output;
  resp_msg_get_ts(&receive_buffer[RESPONSE_MESSAGE_POLL_RECEIVE_TIMESTAMP_INDEX], &output);
  return output;
}

uint32_t receive_buffer_get_response_transmit_timestamp(){
  /*
   * response transmit timestamp given by the responder is only the first 32 bit out of 40 bit
   * a timestamp in device time unit, (2 ** 32 * device time unit = roughly 67 millisecond) (device time unit = roughly 15.65 picosecond) 
   * (67 millisecond * speed of light = roughly 2 million meter)
   * (thus first 32 bit is all you really need to calculate distance less than the size of the earth)  
  */
  uint32_t output;
  resp_msg_get_ts(&receive_buffer[RESPONSE_MESSAGE_RESPONSE_TRANSMIT_TIMESTAMP_INDEX], &output);
  return output;
}




void initialize_dw3000(){

  // RC = Resistor capacitor?
  delay(2); // Time needed for DW3000 to start up transition from INIT_RC to IDLE_RC or could wait for SPIRDY event (Serial peripheral interface ready)
  
  while(!dwt_checkidlerc()){ // make sure DW3000 integrated circuit is in IDLE_RC [state?] before proceeding
    UART_puts("INIT FAILED \r\n");
    while (1);
  }

  if(dwt_initialise(DWT_DW_INIT) == DWT_ERROR){ // initialize the DW3000 transreceiver
    UART_puts("INIT FAILED\r\n");
    while(1);
  }
  

}

void configure_dw3000(){
    /* Configure DW IC. See NOTE 6 below. */
  if(dwt_configure(&config)){ // if the dwt_configure returns DWT_ERROR either the PLL or RX calibration has failed the host should reset the device
    UART_puts("CONFIG FAILED\r\n");
    while (1) ;
  }
  
   /* Configure the TX spectrum parameters (power, PG delay and PG count) 
   *  PG delay: delay time when a singal is transmitted and time when power amplifier (PA) is turned on
   *  the purpose of PG delay to give enough time for PA enough time to stabilize
   *  PG count: number of delay cycles that are used to stabilize the power amplifier
  */
  dwt_configuretxrf(&txconfig_options);

  /*
   * Apply default antenna delay value. 
   * Used for calibration purposes 
  */
  dwt_setrxantennadelay(RECEIVE_ANTENNA_DELAY_IN_DEVICE_TIME_UNIT);
  dwt_settxantennadelay(TRANSMIT_ANTENNA_DELAY_IN_DEVICE_TIME_UNIT);

  /* Set expected response's delay and timeout. See NOTE 1 and 5 below.
   * As this example only handles one incoming frame with always the same delay and timeout, those values can be set here once for all.
   * setrxaftertxdelay: set delay between transmitting message and turning on the receiver to listen for a response
   * setrxtimeout: set timeout for receiver to listen for a response, if receiver does not 
   * receive a response within this time, it will stop listening and assume that
   * the response was not received 
   */
  dwt_setrxaftertxdelay(POLL_TRANSMIT_TO_RESPONSE_RECEIVE_DELAY_IN_UWB_MICROSECOND);
  dwt_setrxtimeout(RESPONSE_RECEIVE_TIMEOUT_IN_UWB_MICROSECOND);

  /* Next can enable TX/RX states output on GPIOs 5 and 6 to help debug, and also TX/RX LEDs
   * Note, in real low power applications the LEDs should not be used. 
   * LNA: Low noise amplifier, which amplify weak signals received by antenna
   * PA: power amplifier, which amplify signal to higher power level so it can be transmitted by antenna
   */
  dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE);
  
}


void setup() {
  UART_init();
  test_run_info((unsigned char *)APP_NAME);

  /* Configure SPI rate, DW3000 supports up to 38 MHz */
  /* Reset DW IC */
  spiBegin(PIN_INTERRUPT_REQUEST, PIN_RESET);
  spiSelect(PIN_SLAVE_SELECT);


  initialize_dw3000();


  // Enable LEDs here for debug so that for each transmit [of the poll message] the D1 LED will flash
  // on DW3000 red eval-shield boards.
  dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK);


  configure_dw3000();

  poll_message_initialize();

}

void loop() {

  Sleep(RANGING_DELAY_MILLISECOND);

  Serial.print("poll_send_message \n");
  poll_message_transmit_message();

  while( !status_register_is_receive_done() ) {}; // receive sucessful, timeout or error

  Serial.print("poll_increment_sequence_number \n");
  poll_message_increment_sequence_number();
  

  if( !status_register_is_receive_frame_control_good() ){
    Serial.print("receive frame control is NOT good \n");
    if ( status_register_is_receive_timeout() ){
      Serial.print("receive timeout flag \n");
    }
    if ( status_register_is_receive_error() ){
      Serial.print("receive error flag \n");
    }
    status_register_clear_receive_timeout();
    status_register_clear_receive_error();
    
    return;
  }
  

  uint32_t frame_length;
  frame_length = receive_frame_info_register_get_frame_length();

  if( frame_length > sizeof(receive_buffer) ){
    Serial.print("frame_length > receive_buffer size \n");
    return;
  }


  // receive_buffer read receive frame data up until frame_length;
  receive_buffer_read_receive_data(frame_length);

  /* Check that the frame is the expected response from the companion "SS TWR responder" example.
   *  (which could be called dw3000_responder_with_function)
   * As the sequence number field of the frame is not relevant, 
   it is cleared to simplify the validation of the frame. */ 
  receive_buffer_set_sequence_number(0);
  // if receive_buffer and receive_response_message is the same uptil the `ALL_MSG_COMMON_LENNGTH_LENGTH_IN_BYTE` return true
//  if (memcmp(receive_buffer, receive_response_message, ALL_MESSAGE_COMMON_LENGTH_IN_BYTE) != 0){
//    Serial.print("memcmp, receive_buffer, receive_response_message, message_common_length != 0 \n");
//    return;
//  }

  // calculating distance
  uint32_t poll_transmit_timestamp_in_device_time_unit;
  uint32_t poll_receive_timestamp_in_device_time_unit;
  uint32_t response_transmit_timestamp_in_device_time_unit;
  uint32_t response_receive_timestamp_in_device_time_unit;

  int32_t round_trip_delay_initiator_in_device_time_unit;
  int32_t round_trip_delay_responder_in_device_time_unit;

  float clock_offset_ratio;

  /*
  The high order byte of each 40-bit time-stamps is discarded here. This is acceptable as, on each device,
  those time-stamps are not separated by more than 2**32 device time units (which is around 67 ms)
  which means that the calculation of the round-trip delays can be handled by a 32-bit subtraction.
  */
  poll_transmit_timestamp_in_device_time_unit = dwt_readtxtimestamplo32();
  response_receive_timestamp_in_device_time_unit = dwt_readrxtimestamplo32();

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
  *  ?? Still confused how `dwt_readclockoffset()` get the offset of what!? initiator or responder???
  *  2 ** 26 = roughly 67,100,000 
  *   NOTE 11. The use of the clock offset value to correct the TOF calculation, significantly 
  *   improves the result of the SS-TWR where the remote 
  *   responder unit's clock is a number of PPM offset from the local initiator unit's clock. 
  */
  clock_offset_ratio = ((float)dwt_readclockoffset()) / (uint32_t)(1<<26);

  poll_receive_timestamp_in_device_time_unit = receive_buffer_get_poll_receive_timestamp();
  response_transmit_timestamp_in_device_time_unit = receive_buffer_get_response_transmit_timestamp();

  round_trip_delay_initiator_in_device_time_unit = response_receive_timestamp_in_device_time_unit - poll_transmit_timestamp_in_device_time_unit;
  round_trip_delay_responder_in_device_time_unit = response_transmit_timestamp_in_device_time_unit - poll_receive_timestamp_in_device_time_unit;

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
  time_of_flight = ( ( round_trip_delay_initiator_in_device_time_unit - round_trip_delay_responder_in_device_time_unit * (1 - clock_offset_ratio) ) / 2.0) * DWT_TIME_UNITS;
  distance = time_of_flight * SPEED_OF_LIGHT;

  Serial.print(distance);
  Serial.print("m fuck yeah! \n");
  snprintf(dist_str, sizeof(dist_str), "DIST: %3.2f m", distance);
  test_run_info((unsigned char *)dist_str);
  

}
