// reciver
#include <VirtualWire.h>
#include <EasyTransferVirtualWire.h>

EasyTransferVirtualWire ET;

const byte led_pin = 13;      // built-in led
const byte receive_pin = 2;     // to which contact the reciver is connected

struct SEND_DATA_STRUCTURE {      // information package
  byte source_id;     // which device transmits the signal
  byte destination_id;     // which device will receive the signal
  unsigned int packet_id;
  byte command;     // command number
  int data;
} mydata;     // name of structure

void setup() {
    pinMode(led_pin, OUTPUT);
    Serial.begin(9600);
    ET.begin(details(mydata));
    vw_set_rx_pin(receive_pin);
    vw_setup(2000);
    vw_rx_start();
}

void loop() {
    if (ET.receiveData()) {
      if (mydata.destination_id == 0 || mydata.destination_id == 1) {     // 0 - broadcast, 1 - this device id
        digitalWrite(led_pin, HIGH);      // led on
        Serial.print("Got:");
        Serial.print(" Destination ID: ");
        Serial.print(mydata.destination_id);
        Serial.print(" Source ID: ");
        Serial.print(mydata.source_id);       
        Serial.print(" Packet ID: ");
        Serial.print(mydata.packet_id);
        Serial.print(" Command: ");
        Serial.print(mydata.command);
        Serial.print(" Data: ");
        Serial.print((float)mydata.data/10, 1);
        Serial.println();
        digitalWrite(led_pin, LOW);      // led off
      }
    }
}
