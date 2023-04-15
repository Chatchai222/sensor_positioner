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
    uint8_t _id;
    uint8_t _frame_sequence_number;
};

void responder_initialize(struct Responder *self, uint8_t in_id){
    self->_id = in_id;
    self->_frame_sequence_number = 0;
}

uint8_t responder_get_id(struct Responder *self){
    return self->_id;
}

uint8_t responder_get_frame_sequence_number(struct Responder *self){
    return self->_frame_sequence_number;
}

uint8_t responder_set_frame_sequence_number(struct Responder *self, uint8_t in_frame_seq_num){
    self->_frame_sequence_number = in_frame_seq_num;
}

void responder_print(struct Responder *self){
    Serial.print("responder: \n");
    Serial.print("    id: "); Serial.println(self->_id);
    Serial.print("    frame_sequence_number: "); Serial.println(self->_frame_sequence_number);
}




struct Initiator {
    uint8_t _id;
};

void initiator_initialize(struct Initiator *self, uint8_t in_id){
    self->_id = in_id;
}

uint8_t initiator_get_id(struct Initiator *self){
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


void setup() {
    UART_init();
    spiBegin(PIN_INTERRUPT_REQUEST, PIN_RESET);
    spiSelect(PIN_SLAVE_SELECT);
  
    // put your setup code here, to run once:

}

void loop() {
    // put your main code here, to run repeatedly:
    delay(1000);
    Serial.print("Start of main loop \n");

    struct Responder my_responder;
    responder_initialize(&my_responder, 100);
    responder_set_frame_sequence_number(&my_responder, 66);
    responder_print(&my_responder);
    
    PollMessage_set_frame_sequence_number(66);
    PollMessage_set_destination_address(1000);
    PollMessage_set_source_address(2000);
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
