#include <rs485.h>

using namespace std;

void setupRS485(){
    Serial2.begin(9600, SERIAL_8N1, RS485_RXD2, RS485_TXD2);
    Serial.println("RS485 initialized"+String(TX));
    Serial.println("    9600baud, RX Pin:" + String(RS485_RXD2) + ", TX Pin:" + String(RS485_TXD2));
}