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
    uint8_t id;
    uint8_t frame_sequence_number;
};

void Responder_initialize(Responder *self, uint8_t in_id){
    self->id = in_id;
    self->frame_sequence_number = 0;
}

uint8_t Responder_get_id(Responder *self){
    return self->id;
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
    Serial.print(Responder_get_id(&my_responder));
    
    Serial.print("End of main loop \n");
}
