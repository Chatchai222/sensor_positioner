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

uint16_t PollMessage_get_personal_area_network_id(){
    return (uint16_t)_PollMessage_read_array(&_poll_message_array[POLL_MESSAGE_PERSONAL_AREA_NETWORK_ID_INDEX], POLL_MESSAGE_PERSONAL_AREA_NETWORK_ID_LENGTH);
}

uint16_t PollMessage_get_destination_address(){
    return (uint16_t)_PollMessage_read_array(&_poll_message_array[POLL_MESSAGE_DESTINATION_ADDRESS_INDEX], POLL_MESSAGE_DESTINATION_ADDRESS_LENGTH);
}

uint16_t PollMessage_get_source_address(){
    return (uint16_t)_PollMessage_read_array(&_poll_message_array[POLL_MESSAGE_SOURCE_ADDRESS_INDEX], POLL_MESSAGE_SOURCE_ADDRESS_LENGTH);
}

uint8_t PollMessage_get_function_code(){
    return (uint8_t)_PollMessage_read_array(&_poll_message_array[POLL_MESSAGE_FUNCTION_CODE_INDEX], POLL_MESSAGE_FUNCTION_CODE_LENGTH);
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







void setup() {
    UART_init();
    spiBegin(PIN_INTERRUPT_REQUEST, PIN_RESET);
    spiSelect(PIN_SLAVE_SELECT);
  
    // put your setup code here, to run once:

}

void loop() {
    // put your main code here, to run repeatedly:
    delay(1000);

    struct Responder my_responder;
    responder_initialize(&my_responder, 100);
    responder_set_frame_sequence_number(&my_responder, 66);
    responder_print(&my_responder);

    PollMessage_print();
    
    Serial.print("End of main loop \n");
}
