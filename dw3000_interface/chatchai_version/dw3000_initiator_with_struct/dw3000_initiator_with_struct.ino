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




typedef struct Responder {
    uint8_t _id;
    uint8_t _frame_sequence_number;
};

void Responder_initialize(Responder *self, uint8_t in_id){
    self->_id = in_id;
    self->_frame_sequence_number = 0;
}

uint8_t Responder_get_id(Responder *self){
    return self->_id;
}

uint8_t Responder_get_frame_sequence_number(Responder *self){
    return self->_frame_sequence_number;
}

uint8_t Responder_set_frame_sequence_number(Responder *self, uint8_t in_frame_seq_num){
    self->_frame_sequence_number = in_frame_seq_num;
}

void Responder_print(Responder *self){
    Serial.print("responder: \n");
    Serial.print("    id: "); Serial.println(self->_id);
    Serial.print("    frame_sequence_number: "); Serial.println(self->_frame_sequence_number);
}




typedef struct Initiator {
    uint8_t _id;
};

void Initiator_initialize(Initiator *self, uint8_t in_id){
    self->_id = in_id;
}

uint8_t Initiator_get_id(Initiator *self){
    return self->_id;
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

    Responder my_responder;
    Responder_initialize(&my_responder, 100);
    Responder_set_frame_sequence_number(&my_responder, 66);
    Responder_print(&my_responder);
    
    Serial.print("End of main loop \n");
}
