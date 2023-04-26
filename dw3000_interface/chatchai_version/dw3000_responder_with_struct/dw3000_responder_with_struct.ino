/*Written: start 2023 Apr 5
 * Modification with comments of the DW3000 example by MakerFabs for "ex_06b_ss_twr_responder"
 * Hopefully with the some slight modification and extra comments it would make it a little easier to read
 * and understand
 */

#include "dw3000.h"

#define APP_NAME "SS TWR RESP with struct v1.0"

// THIS IS FOR ANCHOR
/******** START EASY CONFIGURE **********/
const uint16_t RESPONDER_ID = 1002;

/******** END EASY CONFIGURE ************/

// connection pins
const uint8_t PIN_RESET = 27; // reset pin
const uint8_t PIN_INTERRUPT_REQUEST = 34; // irq pin
const uint8_t PIN_SLAVE_SELECT = 4;   // spi select pin


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




/* Default communication configuration. We use default non-STS DW mode.
 *  Too scared to change, so keeping it
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

#define POLL_RECEIVE_TO_RESPONSE_TRANSMIT_DELAY_IN_ULTRAWIDEBAND_MICROSECOND 450

/* Hold copy of status register state here for reference so that it can be examined at a debug breakpoint. */
static uint32_t status_reg = 0;

/* Values for the PG_DELAY and TX_POWER registers reflect the bandwidth and power of the spectrum at the current
 * temperature. These values can be calibrated prior to taking reference measurements. See NOTE 5 below.
 * txconfig_options: transmit configuration options*/
extern dwt_txconfig_t txconfig_options;

void DW3000Chip_initialize_for_responder(){
    UART_init();

    /* Configure SPI rate, DW3000 supports up to 38 MHz */
    /* Reset DW IC */
    spiBegin(PIN_INTERRUPT_REQUEST, PIN_RESET);
    spiSelect(PIN_SLAVE_SELECT);
    
    delay(2); /// Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC, or could wait for SPIRDY event)

    while(!dwt_checkidlerc()){
        UART_puts("IDLE FAILED \r\n"); // Need to make sure DW IC is in IDLE_RC before proceeding
        while(1);
    }
    
    if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR){
        UART_puts("INIT FAILED \r\n");
        while(1);
    }

    // Enabling LEDs here for debug so that for each TX the D1 LED will flash on DW3000 red eval-shield boards.
    dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK);

    /* Configure DW IC. See NOTE 6 below. */
    if (dwt_configure(&config)){ // if the dwt_configure returns DWT_ERROR either the PLL or RX calibration has failed the host should reset the device 
        UART_puts("CONFIG FAILED\r\n");
        while (1);
    }

    /* Configure the TX spectrum parameters (power, PG delay and PG count)
    *  PG delay: delay time when a singal is transmitted and time when power amplifier (PA) is turned on
    *  the purpose of PG delay to give enough time for PA enough time to stabilize
    *  PG count: number of delay cycles that are used to stabilize the power amplifier*/
    dwt_configuretxrf(&txconfig_options);

    /* Apply default antenna delay value. See NOTE 2 below.
    *  is used for calibration purposes for calculating distance from one DW3000 board to another*/
    dwt_setrxantennadelay(RECEIVE_ANTENNA_DELAY_IN_DTU);
    dwt_settxantennadelay(RECEIVE_ANTENNA_DELAY_IN_DTU);

    /* Next can enable TX/RX states output on GPIOs 5 and 6 to help debug, and also TX/RX LEDs
    * Note, in real low power applications the LEDs should not be used.
    * LNA: Low noise amplifier, which amplify weak signals received by antenna
    * PA: power amplifier, which amplify signal to higher power level so it can be transmitted by antenna*/
    dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE);

}




uint16_t _RESPONDER_ID = RESPONDER_ID;

void wait_for_poll_message_and_transmit_back_response_message(){
    
    /* Activate reception immediately. */
    dwt_rxenable(DWT_START_RX_IMMEDIATE);

    /* Poll for reception of a frame or error/timeout. See NOTE 6 below.
    *  `dwt_read32bitreg(SYS_STATUS_ID)`: get the value of status register
    *  `SYS_STATUS_RXFCG_BIT_MASK`: bit mask for receive frame control good, check if frame is good
    *  `SYS_STATUS_ALL_RX_ERR`: bit mask for any receive error, check if CRC is wrong, incomplete frame message, etc for any error.*/
    while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_ERR))){};

    if (!(status_reg & SYS_STATUS_RXFCG_BIT_MASK)){ // if not receive frame control good
        /* Clear RX error events in the DW IC status register. clear receive frame error from status register */
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
        Serial.println("wait_poll_transmit_response error: Not receive frame control good");
        return;
    }

    /* Clear good RX frame event in the DW IC status register.
    *  Clear good receive frame event in the DW3000 integrated circuit status register*/
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);

    bool is_write_rx_buffer_to_poll_message_success = _write_rx_buffer_to_poll_message();
    if (!is_write_rx_buffer_to_poll_message_success){
        Serial.println("wait_poll_transmit_response error: Failed to write rx buffer to poll message");
        return;
    }

    if (!_is_poll_message_destination_match_responder_id()){
        Serial.println("wait_poll_transmit_response error: Poll message destination don't match responder id");
        return;
    }

    _update_response_message_from_poll_message_and_timestamp();
    
    int dwt_status = _transmit_response_message();
    if (dwt_status != DWT_SUCCESS){
        Serial.println("wait_poll_transmit_response error: transmit response message failed");
        return;
    }

    /* Poll DW IC until TX frame sent event set. See NOTE 6 below. */
    while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS_BIT_MASK)){};
    
    // Clear TXFRS event. transmit frame sent event
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);

    ResponseMessage_increment_frame_sequence_number();
    Serial.println("wait_poll_transmit_response: success!!");
    
}

bool _write_rx_buffer_to_poll_message(){
    bool is_operation_success = true;

    // Read the receive frame infromation regsiter which contain info like frame length and CRC status
    // in this case frame length
    uint32_t frame_len = dwt_read32bitreg(RX_FINFO_ID) & RXFLEN_MASK;
    
    if(frame_len > PollMessage_get_array_sizeof()){
        is_operation_success = false;
        return is_operation_success;
    }

    /*Zero is the offset buffer*/
    dwt_readrxdata(PollMessage_get_array(), frame_len, 0);

    return is_operation_success;
}

bool _is_poll_message_destination_match_responder_id(){
    uint16_t poll_msg_dest_addr = PollMessage_get_destination_address();
    return RESPONDER_ID == poll_msg_dest_addr;
}

void _update_response_message_from_poll_message_and_timestamp(){
    uint32_t response_transmit_time;
    uint64_t poll_receive_timestamp;
    uint64_t response_transmit_timestamp;
    int transmit_returned;
    uint16_t poll_message_source_address = PollMessage_get_source_address();

    // get 40 bit timestamp in type unsigned64int in device time unit
    poll_receive_timestamp = get_rx_timestamp_u64();

    // dwt_setdelayedtrxtime(tx_time): takes the higher 32 bit of timestamp(whic is 40 bit)
    // which is in 256 device time unit, the function then ignore the least significant bit
    // of `tx_time` which effectively make the resolution of 512 device time unit, in delayed transmit time 
    // NOTE: this part and onward is f***ed up
    response_transmit_time = (poll_receive_timestamp + (POLL_RECEIVE_TO_RESPONSE_TRANSMIT_DELAY_IN_ULTRAWIDEBAND_MICROSECOND * UUS_TO_DWT_TIME)) >> 8;
    dwt_setdelayedtrxtime(response_transmit_time);
    
    // response_transmit_timestamp in 40 bit, with type u64 in device time unit
    // `(((uint64_t)(response_transmit_time & 0xFFFFFFFEUL)) << 8)` 
    // ^^^^ Converts the `response_transmit_time` 32 bit, u32, 512 device time unit into `40 bit, u64, 1 device time unit`
    //                                                                    01234567
    response_transmit_timestamp = (((uint64_t)(response_transmit_time & 0xFFFFFFFEUL)) << 8) + TRANSMIT_ANTENNA_DELAY_IN_DTU;

    ResponseMessage_set_poll_message_receive_timestamp(poll_receive_timestamp);
    ResponseMessage_set_response_message_transmit_timestamp(response_transmit_timestamp);
    ResponseMessage_set_source_address(_RESPONDER_ID);
    ResponseMessage_set_destination_address(poll_message_source_address);
}

int _transmit_response_message(){
    int dwt_status;

    // Write transmit data
    // Zero offset in TX buffer
    dwt_writetxdata(ResponseMessage_get_array_sizeof(), ResponseMessage_get_array(), 0);
    
    // Write to transmit frame control
    // Zero offset in TX buffer, ranging
    dwt_writetxfctrl(ResponseMessage_get_array_sizeof(), 0, 1);

    dwt_status = dwt_starttx(DWT_START_TX_DELAYED);
    return dwt_status;
}




void setup(){
    DW3000Chip_initialize_for_responder();

}

void loop(){
    Serial.println("start main loop of responder with struct");

    wait_for_poll_message_and_transmit_back_response_message();
    PollMessage_print();
    ResponseMessage_print();

    Serial.println("end main loop of responder with struct");
    
    
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
