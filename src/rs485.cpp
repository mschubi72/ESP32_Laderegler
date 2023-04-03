#include <rs485.h>

using namespace std;

void setupRS485(){
    Serial2.begin(9600, SERIAL_8N1, RS485_RXD2, RS485_TXD2);
    Serial.println("RS485 initialized...");
    Serial.printf("\t9600baud, RX Pin: %i, TX Pin: %i \n", RS485_RXD2, RS485_TXD2);
}