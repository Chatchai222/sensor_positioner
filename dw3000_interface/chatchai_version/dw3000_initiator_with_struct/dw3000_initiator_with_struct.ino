/*
 * A refactored rewrite of the "ex_06a_ss_twr_initiator" example for ESP32 DW3000 Makerfabs
 * This time with using of struct and struct method hopefully to help make it easier to modify
 * and reason about how the f*** this works
 */

#include "dw3000.h"

#define APP_NAME "SS TWR Init refactored with struct v1.0"

// connections pins
const uint8_t PIN_RESET = 27;
const uint8_t PIN_INTERRUPT_REQUEST = 34;
const uint8_t PIN_SLAVE_SELECT = 4;




struct Responder {
    uint16_t _id;
    uint8_t _frame_sequence_number;
};

void responder_initialize(struct Responder *self, uint16_t in_id){
    self->_id = in_id;
    self->_frame_sequence_number = 0;
}

uint16_t responder_get_id(struct Responder *self){
    return self->_id;
}

uint8_t responder_get_frame_sequence_number(struct Responder *self){
    return self->_frame_sequence_number;
}

uint8_t responder_set_frame_sequence_number(struct Responder *self, uint8_t in_frame_seq_num){
    self->_frame_sequence_number = in_frame_seq_num;
}

uint8_t responder_increment_frame_sequence_number(struct Responder *self){
    self->_frame_sequence_number += 1;
}

void responder_print(struct Responder *self){
    Serial.print("responder: \n");
    Serial.print("    id: "); Serial.println(self->_id);
    Serial.print("    frame_sequence_number: "); Serial.println(self->_frame_sequence_number);
}

struct Initiator {
    uint16_t _id;
};

void initiator_initialize(struct Initiator *self, uint16_t in_id){
    self->_id = in_id;
}

uint16_t initiator_get_id(struct Initiator *self){
    return self->_id;
}




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
#define POLL_MESSAGE_FRAME_CONTROL_INDEX 0
#define POLL_MESSAGE_FRAME_CONTROL_LENGTH 2

#define POLL_MESSAGE_FRAME_SEQUENCE_NUMBER_INDEX 2
#define POLL_MESSAGE_FRAME_SEQUENCE_NUMBER_LENGTH 1

#define POLL_MESSAGE_PERSONAL_AREA_NETWORK_ID_INDEX 3
#define POLL_MESSAGE_PERSONAL_AREA_NETWORK_ID_LENGTH 2

#define POLL_MESSAGE_DESTINATION_ADDRESS_INDEX 5
#define POLL_MESSAGE_DESTINATION_ADDRESS_LENGTH 2

#define POLL_MESSAGE_SOURCE_ADDRESS_INDEX 7
#define POLL_MESSAGE_SOURCE_ADDRESS_LENGTH 2

#define POLL_MESSAGE_FUNCTION_CODE_INDEX 9
#define POLL_MESSAGE_FUNCTION_CODE_LENGTH 1
//                                      0     1     2  3     4      5    6    7    8   9    10 11
static uint8_t _poll_message_array[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0xE0, 0, 0};

uint16_t PollMessage_get_frame_control(){
    return (uint16_t)_PollMessage_read_array(&_poll_message_array[POLL_MESSAGE_FRAME_CONTROL_INDEX], POLL_MESSAGE_FRAME_CONTROL_LENGTH);
}

uint8_t PollMessage_get_frame_sequence_number(){
    return (uint8_t)_PollMessage_read_array(&_poll_message_array[POLL_MESSAGE_FRAME_SEQUENCE_NUMBER_INDEX], POLL_MESSAGE_FRAME_SEQUENCE_NUMBER_LENGTH);
}

void PollMessage_set_frame_sequence_number(uint8_t in_seq_num){
    _PollMessage_write_array(&_poll_message_array[POLL_MESSAGE_FRAME_SEQUENCE_NUMBER_INDEX], POLL_MESSAGE_FRAME_SEQUENCE_NUMBER_LENGTH, in_seq_num);
}

void PollMessage_increment_frame_sequence_number(){
    uint8_t frame_seq_num = PollMessage_get_frame_sequence_number();
    frame_seq_num = (frame_seq_num + 1) % 255;
    PollMessage_set_frame_sequence_number(frame_seq_num);
}

uint16_t PollMessage_get_personal_area_network_id(){
    return (uint16_t)_PollMessage_read_array(&_poll_message_array[POLL_MESSAGE_PERSONAL_AREA_NETWORK_ID_INDEX], POLL_MESSAGE_PERSONAL_AREA_NETWORK_ID_LENGTH);
}

uint16_t PollMessage_get_destination_address(){
    return (uint16_t)_PollMessage_read_array(&_poll_message_array[POLL_MESSAGE_DESTINATION_ADDRESS_INDEX], POLL_MESSAGE_DESTINATION_ADDRESS_LENGTH);
}

void PollMessage_set_destination_address(uint16_t in_dest){
    _PollMessage_write_array(&_poll_message_array[POLL_MESSAGE_DESTINATION_ADDRESS_INDEX], POLL_MESSAGE_DESTINATION_ADDRESS_LENGTH, in_dest);
}

uint16_t PollMessage_get_source_address(){
    return (uint16_t)_PollMessage_read_array(&_poll_message_array[POLL_MESSAGE_SOURCE_ADDRESS_INDEX], POLL_MESSAGE_SOURCE_ADDRESS_LENGTH);
}

void PollMessage_set_source_address(uint16_t in_source_address){
    _PollMessage_write_array(&_poll_message_array[POLL_MESSAGE_SOURCE_ADDRESS_INDEX], POLL_MESSAGE_SOURCE_ADDRESS_LENGTH, in_source_address);
}

uint8_t PollMessage_get_function_code(){
    return (uint8_t)_PollMessage_read_array(&_poll_message_array[POLL_MESSAGE_FUNCTION_CODE_INDEX], POLL_MESSAGE_FUNCTION_CODE_LENGTH);
}

uint16_t PollMessage_get_array_sizeof(){
    return sizeof(_poll_message_array);
}

uint8_t* PollMessage_get_array(){
    return _poll_message_array;
}

void PollMessage_print(){
    Serial.print("PollMessage: \n");
    Serial.print("    frame_control: 0x"); Serial.println(PollMessage_get_frame_control(), HEX);
    Serial.print("    frame_sequence_number: "); Serial.println(PollMessage_get_frame_sequence_number());
    Serial.print("    personal_area_network_id: 0x"); Serial.println(PollMessage_get_personal_area_network_id(), HEX);
    Serial.print("    destination_address: "); Serial.println(PollMessage_get_destination_address());
    Serial.print("    source_address: "); Serial.println(PollMessage_get_source_address());
    Serial.print("    function_code: "); Serial.println(PollMessage_get_function_code());
}

uint32_t _PollMessage_read_array(uint8_t *frame_field, uint8_t field_length){
    int i;
    uint32_t output = 0;
    for(i = 0; i < field_length; i++){
        output += (uint32_t)frame_field[i] << (i * 8);
    }
    return output;
}

void _PollMessage_write_array(uint8_t *frame_field, uint8_t field_length, uint32_t in_field){
    uint32_t num_to_write = in_field;
    int i;
    for(i = 0; i < field_length; i++){
        frame_field[i] = (uint8_t)num_to_write;
        num_to_write = num_to_write >> 8;
    }
}

#define RESPONSE_MESSAGE_FRAME_CONTROL_INDEX 0
#define RESPONSE_MESSAGE_FRAME_CONTROL_LENGTH 2

#define RESPONSE_MESSAGE_FRAME_SEQUENCE_NUMBER_INDEX 2
#define RESPONSE_MESSAGE_FRAME_SEQUENCE_NUMBER_LENGTH 1

#define RESPONSE_MESSAGE_PERSONAL_AREA_NETWORK_ID_INDEX 3
#define RESPONSE_MESSAGE_PERSONAL_AREA_NETWORK_ID_LENGTH 2

#define RESPONSE_MESSAGE_DESTINATION_ADDRESS_INDEX 5
#define RESPONSE_MESSAGE_DESTINATION_ADDRESS_LENGTH 2

#define RESPONSE_MESSAGE_SOURCE_ADDRESS_INDEX 7
#define RESPONSE_MESSAGE_SOURCE_ADDRESS_LENGTH 2

#define RESPONSE_MESSAGE_FUNCTION_CODE_INDEX 9
#define RESPONSE_MESSAGE_FUNCTION_CODE_LENGTH 1

#define RESPONSE_MESSAGE_POLL_MESSAGE_RECEIVE_TIMESTAMP_INDEX 10
#define RESPONSE_MESSAGE_POLL_MESSAGE_RECEIVE_TIMESTAMP_LENGTH 4

#define RESPONSE_MESSAGE_RESPONSE_MESSAGE_TRANSMIT_TIMESTAMP_INDEX 14
#define RESPONSE_MESSAGE_RESPONSE_MESSAGE_TRANSMIT_TIMESTAMP_LENGTH 4
//                                          0     1     2  3     4      5    6    7    8   9    10 11 12 13 14 15 16 17 18 19
static uint8_t _response_message_array[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0xE1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

uint16_t ResponseMessage_get_frame_control(){
    return (uint16_t)_ResponseMessage_read_array(&_response_message_array[RESPONSE_MESSAGE_FRAME_CONTROL_INDEX], RESPONSE_MESSAGE_FRAME_CONTROL_LENGTH);
}

uint8_t ResponseMessage_get_frame_sequence_number(){
    return (uint8_t)_ResponseMessage_read_array(&_response_message_array[RESPONSE_MESSAGE_FRAME_SEQUENCE_NUMBER_INDEX], RESPONSE_MESSAGE_FRAME_SEQUENCE_NUMBER_LENGTH);
}

void ResponseMessage_set_frame_sequence_number(uint8_t in_seq_num){
    _ResponseMessage_write_array(&_response_message_array[RESPONSE_MESSAGE_FRAME_SEQUENCE_NUMBER_INDEX], RESPONSE_MESSAGE_FRAME_SEQUENCE_NUMBER_LENGTH, in_seq_num);
}

void ResponseMessage_increment_frame_sequence_number(){
    uint8_t frame_seq_num = ResponseMessage_get_frame_sequence_number();
    frame_seq_num = (frame_seq_num + 1) % 255;
    ResponseMessage_set_frame_sequence_number(frame_seq_num);
}

uint16_t ResponseMessage_get_personal_area_network_id(){
    return (uint16_t)_ResponseMessage_read_array(&_response_message_array[RESPONSE_MESSAGE_PERSONAL_AREA_NETWORK_ID_INDEX], RESPONSE_MESSAGE_PERSONAL_AREA_NETWORK_ID_LENGTH);
}

uint16_t ResponseMessage_get_destination_address(){
    return (uint16_t)_ResponseMessage_read_array(&_response_message_array[RESPONSE_MESSAGE_DESTINATION_ADDRESS_INDEX], RESPONSE_MESSAGE_DESTINATION_ADDRESS_LENGTH);
}

void ResponseMessage_set_destination_address(uint16_t in_dest_addr){
    _ResponseMessage_write_array(&_response_message_array[RESPONSE_MESSAGE_DESTINATION_ADDRESS_INDEX], RESPONSE_MESSAGE_DESTINATION_ADDRESS_LENGTH, in_dest_addr);
}

uint16_t ResponseMessage_get_source_address(){
    return (uint16_t)_ResponseMessage_read_array(&_response_message_array[RESPONSE_MESSAGE_SOURCE_ADDRESS_INDEX], RESPONSE_MESSAGE_SOURCE_ADDRESS_LENGTH);
}

void ResponseMessage_set_source_address(uint16_t in_src_addr){
    _ResponseMessage_write_array(&_response_message_array[RESPONSE_MESSAGE_SOURCE_ADDRESS_INDEX], RESPONSE_MESSAGE_SOURCE_ADDRESS_LENGTH, in_src_addr);
}

uint8_t ResponseMessage_get_function_code(){
    return (uint16_t)_ResponseMessage_read_array(&_response_message_array[RESPONSE_MESSAGE_FUNCTION_CODE_INDEX], RESPONSE_MESSAGE_FUNCTION_CODE_LENGTH);
}

uint32_t ResponseMessage_get_poll_message_receive_timestamp(){
    return (uint32_t)_ResponseMessage_read_array(&_response_message_array[RESPONSE_MESSAGE_POLL_MESSAGE_RECEIVE_TIMESTAMP_INDEX], RESPONSE_MESSAGE_POLL_MESSAGE_RECEIVE_TIMESTAMP_LENGTH);
}

void ResponseMessage_set_poll_message_receive_timestamp(uint32_t poll_msg_receive_ts){
    _ResponseMessage_write_array(&_response_message_array[RESPONSE_MESSAGE_POLL_MESSAGE_RECEIVE_TIMESTAMP_INDEX], RESPONSE_MESSAGE_POLL_MESSAGE_RECEIVE_TIMESTAMP_LENGTH, poll_msg_receive_ts);
}

uint32_t ResponseMessage_get_response_message_transmit_timestamp(){
    return (uint32_t)_ResponseMessage_read_array(&_response_message_array[RESPONSE_MESSAGE_RESPONSE_MESSAGE_TRANSMIT_TIMESTAMP_INDEX], RESPONSE_MESSAGE_RESPONSE_MESSAGE_TRANSMIT_TIMESTAMP_LENGTH);
}

void ResponseMessage_set_response_message_transmit_timestamp(uint32_t resp_transmit_ts){
    _ResponseMessage_write_array(&_response_message_array[RESPONSE_MESSAGE_RESPONSE_MESSAGE_TRANSMIT_TIMESTAMP_INDEX], RESPONSE_MESSAGE_RESPONSE_MESSAGE_TRANSMIT_TIMESTAMP_LENGTH, resp_transmit_ts);
}

uint16_t ResponseMessage_get_array_sizeof(){
    return sizeof(_response_message_array);
}

uint8_t* ResponseMessage_get_array(){
    return _response_message_array;
}

void ResponseMessage_print(){
    Serial.print("ResponseMessage: \n");
    Serial.print("    frame_control: 0x"); Serial.println(ResponseMessage_get_frame_control(), HEX);
    Serial.print("    frame_sequence_number: "); Serial.println(ResponseMessage_get_frame_sequence_number());
    Serial.print("    personal_area_network_id: 0x"); Serial.println(ResponseMessage_get_personal_area_network_id(), HEX);
    Serial.print("    destination_address: "); Serial.println(ResponseMessage_get_destination_address());
    Serial.print("    source_address: "); Serial.println(ResponseMessage_get_source_address());
    Serial.print("    function_code: "); Serial.println(ResponseMessage_get_function_code());
    Serial.print("    poll_message_receive_timestamp: "); Serial.println(ResponseMessage_get_poll_message_receive_timestamp());
    Serial.print("    response_message_transmit_timestamp: "); Serial.println(ResponseMessage_get_response_message_transmit_timestamp());
}

uint32_t _ResponseMessage_read_array(uint8_t *frame_field, uint8_t field_length){
    int i;
    uint32_t output = 0;
    for(i = 0; i < field_length; i++){
        output += (uint32_t)frame_field[i] << (i * 8);
    }
    return output;
}

void _ResponseMessage_write_array(uint8_t *frame_field, uint8_t field_length, uint32_t in_field){
    uint32_t num_to_write = in_field;
    int i;
    for(i = 0; i < field_length; i++){
        frame_field[i] = (uint8_t)num_to_write;
        num_to_write = num_to_write >> 8;
    }
}




// DW Chip
/* Default communication configuration. We use default non-STS DW mode.
 *  I am too afraid to change anything here, so keeping it
 */
static dwt_config_t config = {
    5,                /* Channel number. */
    DWT_PLEN_128,     /* Preamble length. Used in TX only. */
    DWT_PAC8,         /* Preamble acquisition chunk size. Used in RX only. */
    9,                /* TX preamble code. Used in TX only. */
    9,                /* RX preamble code. Used in RX only. */
    1,                /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
    DWT_BR_6M8,       /* Data rate. */
    DWT_PHRMODE_STD,  /* PHY header mode. */
    DWT_PHRRATE_STD,  /* PHY header rate. */
    (129 + 8 - 8),    /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
    DWT_STS_MODE_OFF, /* STS disabled */
    DWT_STS_LEN_64,   /* STS length see allowed values in Enum dwt_sts_lengths_e */
    DWT_PDOA_M0       /* PDOA mode off */
};
// DTU - Device time unit
// ONE DEVICE TIME UNIT = (1.0/499.2e6/128.0) //!< = 15.65e-12 seconds
#define TRANSMIT_ANTENNA_DELAY_IN_DTU 16385
#define RECEIVE_ANTENNA_DELAY_IN_DTU 16385

#define POLL_TRANSMIT_TO_RESPONSE_RECEIVE_DELAY_IN_ULTRAWIDEBAND_MICROSECOND 240
#define RESPONSE_RECEIVE_TIMEOUT_IN_ULTRAWIDEBAND_MICROSECOND 400

/* Values for the PG_DELAY and TX_POWER registers reflect the bandwidth and power of the spectrum at the current
 * temperature. These values can be calibrated prior to taking reference measurements*/
extern dwt_txconfig_t txconfig_options;

/* Hold copy of status register state here for reference so that it can be examined at a debug breakpoint. */
static uint32_t status_reg = 0;

void DW3000Chip_initialize_for_initiator(){
    UART_init();

    /* Configure SPI rate, DW3000 supports up to 38 MHz, baud rate 115200 */
    /* Reset DW IC */
    spiBegin(PIN_INTERRUPT_REQUEST, PIN_RESET);
    spiSelect(PIN_SLAVE_SELECT);

    delay(2);  // Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC, or could wait for SPIRDY event)

    while(!dwt_checkidlerc()){  // Need to make sure DW IC is in IDLE_RC before proceeding
        UART_puts("IDLE FAILED \r\n");
        while (1);
    }

    if(dwt_initialise(DWT_DW_INIT) == DWT_ERROR){
        UART_puts("INIT FAILED \r\n");
    }

    // Enabling LEDs here for debug so that for each TX the D1 LED will flash on DW3000 red eval-shield board
    dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK);

    // Configure DW IC. See NOTE 6 below. */
    if(dwt_configure(&config)){  // if the dwt_configure returns DWT_ERROR either the PLL or RX calibration has failed the host should reset the device
        UART_puts("CONFIG FAILED \r\n");
        while (1);
    }

    /* Configure the TX spectrum parameters (power, PG delay and PG count)
     *  PG delay: delay time when a singal is transmitted and time when power amplifier (PA) is turned on
     *  the purpose of PG delay to give enough time for PA enough time to stabilize
     *  PG count: number of delay cycles that are used to stabilize the power amplifier
     */
    dwt_configuretxrf(&txconfig_options);

    /* Apply default antenna delay value. See NOTE 2 below.
     * Antenna delay is used for calibration purposes for calculating distance */
    dwt_setrxantennadelay(RECEIVE_ANTENNA_DELAY_IN_DTU);
    dwt_settxantennadelay(TRANSMIT_ANTENNA_DELAY_IN_DTU);

    /* Set expected response's delay and timeout. See NOTE 1 and 5 below.
     * As this example only handles one incoming frame with always the same delay and timeout, those values can be set here once for all.
     * setrxaftertxdelay: set delay between transmitting message and turning on the receiver to listen for a response
     * setrxtimeout: set timeout for receiver to listen for a response, if receiver does not
     * receive a response within this time, it will stop listening and assume that
     * the response was not received
     */
    dwt_setrxaftertxdelay(POLL_TRANSMIT_TO_RESPONSE_RECEIVE_DELAY_IN_ULTRAWIDEBAND_MICROSECOND);
    dwt_setrxtimeout(RESPONSE_RECEIVE_TIMEOUT_IN_ULTRAWIDEBAND_MICROSECOND);

    /* Next can enable TX/RX states output on GPIOs 5 and 6 to help debug, and also TX/RX LEDs
     * Note, in real low power applications the LEDs should not be used.
     * LNA: Low noise amplifier, which amplify weak signals received by antenna
     * PA: power amplifier, which amplify signal to higher power level so it can be transmitted by antenna
     */
    dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE);
}

// Ranger
double Ranger_get_distance_or_null(struct Initiator *initiator_ptr, struct Responder *responder_ptr) {
    // single attempt get distance?
    _Ranger_update_poll_message(initiator_ptr, responder_ptr);
    responder_increment_frame_sequence_number(responder_ptr);

    _Ranger_send_poll_message_and_wait_til_receive_response_message_done();
    if(status_reg & SYS_STATUS_RXFCG_BIT_MASK){  // if receive frame control good
        /* Clear good RX frame event in the DW IC status register.
         * Write to status register that receive the response frame sucessfully*/
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);
    } else {
        /* Clear RX error/timeout events in the DW IC status register. */
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
        return NULL;
    }

    bool is_write_rx_buffer_to_response_message_success = _Ranger_write_rx_buffer_to_response_message();
    if(!is_write_rx_buffer_to_response_message_success){
        return NULL;
    }

    if(!_Ranger_is_response_message_the_expected_message()){
        return NULL;
    }

    double distance = _Ranger_calculate_distance();
    return distance;
}

void _Ranger_update_poll_message(struct Initiator *initiator_ptr, struct Responder *responder_ptr){
    uint16_t initiator_id = initiator_get_id(initiator_ptr);

    uint16_t responder_id = responder_get_id(responder_ptr);
    uint8_t responder_frame_sequence_num = responder_get_frame_sequence_number(responder_ptr);

    PollMessage_set_source_address(initiator_id);
    PollMessage_set_destination_address(responder_id);
    PollMessage_set_frame_sequence_number(responder_frame_sequence_num);
}

void _Ranger_send_poll_message_and_wait_til_receive_response_message_done(){
    // Write to status register that transmit frame sent
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);

    // Zero offset in TX buffer
    dwt_writetxdata(PollMessage_get_array_sizeof(), PollMessage_get_array(), 0);

    /*
     *  Size of frame
     *  Zero offset in TX buffer, ranging.
     *  Write to transmit frame control register the (size_of_frame, offset, transmit_data_rate_and_preamble_length)
     *  Which by default 1, should be [6.8 Mbps and 128 symbol preample length]? (by chatGPT)
     */
    dwt_writetxfctrl(PollMessage_get_array_sizeof(), 0, 1);

    /* Start transmission, indicating that a response is expected so that reception is enabled automatically after the frame is sent and the delay
     * set by dwt_setrxaftertxdelay() has elapsed. */
    dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);

    /* We assume that the transmission is achieved correctly, poll for reception of a frame or error/timeout. See NOTE 8 below.
     * Stay in the loop until a frame is received, an error occured, or timeout occurs
     * BITMASK:
     * `SYS_STATUS_RXFCG: Receive frame control good, check if good frame received and content available to read
     * `SYS_STATUS_ALL_RX_TO`: All receive timeout, check if receive timeout has occured for any reason, including no frame received
     * `SYS_STATUS_ALL_RX_ERR`: All receive errors, check whether receive error occurs for any reason including CRC error, invalid frame format*/
    while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR))) {};

}

bool _Ranger_write_rx_buffer_to_response_message() {
    bool is_operation_success = false;

    /* A frame has been received, read it into the local buffer.
     *  Read the receive frame information register which contain info like frame length and CRC status
     *  RXFLEN_MASK: receive frame length bitmask, to extract frame length from receive frame information register*/
    uint32_t frame_len = dwt_read32bitreg(RX_FINFO_ID) & RXFLEN_MASK;

    if (frame_len > ResponseMessage_get_array_sizeof()) {
        return is_operation_success;
    }

    // read the received frame data into `rx_buffer` with length `frame_len` with offset of 0
    dwt_readrxdata(ResponseMessage_get_array(), frame_len, 0);

    is_operation_success = true;
    return is_operation_success;
}

bool _Ranger_is_response_message_the_expected_message(){
    uint16_t poll_src_addr = PollMessage_get_source_address();
    uint16_t poll_des_addr = PollMessage_get_destination_address();
    
    uint16_t resp_src_addr = ResponseMessage_get_source_address();
    uint16_t resp_des_addr = ResponseMessage_get_destination_address();

    return (poll_des_addr == resp_src_addr) && (poll_src_addr == resp_des_addr);
}

double _Ranger_calculate_distance(){
    uint32_t poll_transmit_ts = dwt_readtxtimestamplo32();
    uint32_t poll_receive_ts = ResponseMessage_get_poll_message_receive_timestamp();
    
    uint32_t resp_transmit_ts = ResponseMessage_get_response_message_transmit_timestamp();
    uint32_t resp_receive_ts = dwt_readrxtimestamplo32();

    /*
     * 11. The use of the clock offset value to correct the TOF calculation, significantly improves the result of the SS-TWR where the remote
     *     responder unit's clock is a number of PPM offset from the local initiator unit's clock.
     *     As stated in NOTE 2 a fixed offset in range will be seen unless the antenna delay is calibrated and set correctly.
     */
    float clock_offset_ratio = ((float)dwt_readclockoffset()) / (uint32_t)(1 << 26);

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
    int32_t rtd_initiator = resp_receive_ts - poll_transmit_ts;
    int32_t rtd_responder = resp_transmit_ts - poll_receive_ts;
    
    double time_of_flight = ((rtd_initiator - rtd_responder * (1 - clock_offset_ratio)) / 2.0) * DWT_TIME_UNITS;
    double distance = time_of_flight * SPEED_OF_LIGHT;
    return distance;
}

void my_debug() {
    delay(1000);
    Serial.print("Start of main loop \n");

    struct Responder my_responder;
    responder_initialize(&my_responder, 100);
    responder_set_frame_sequence_number(&my_responder, 66);
    responder_print(&my_responder);

    PollMessage_set_frame_sequence_number(66);
    // PollMessage_set_destination_address(1000);
    // PollMessage_set_source_address(2000);
    PollMessage_print();

    ResponseMessage_set_frame_sequence_number(13);
    ResponseMessage_set_destination_address(60);
    ResponseMessage_set_source_address(33);
    ResponseMessage_set_poll_message_receive_timestamp(69);
    ResponseMessage_set_response_message_transmit_timestamp(420);
    ResponseMessage_get_array()[RESPONSE_MESSAGE_PERSONAL_AREA_NETWORK_ID_INDEX] = 15;
    ResponseMessage_print();

    Serial.print("End of main loop \n");
}

void setup() {
    DW3000Chip_initialize_for_initiator();

    // put your setup code here, to run once:
}

void loop() {
    // put your main code here, to run repeatedly:
    struct Initiator my_initiator;
    initiator_initialize(&my_initiator, 17750);
    
    struct Responder my_responder;
    responder_initialize(&my_responder, 16727);

    double distance = Ranger_get_distance_or_null(&my_initiator, &my_responder);
    if (distance == NULL){
        Serial.println("ranger failed to get distance");
    } else {
        Serial.print("distance is: "); Serial.println(distance);
    }

    Sleep(200);
}
