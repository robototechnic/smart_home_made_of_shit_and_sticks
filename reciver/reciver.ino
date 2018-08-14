//reciver
#include <VirtualWire.h>
#include <EasyTransferVirtualWire.h>

const int led_pin = 13;
const int receive_pin = 2;

EasyTransferVirtualWire ET;
struct SEND_DATA_STRUCTURE {
  unsigned int source_id;
  unsigned int destination_id;
  unsigned int packet_id;
  byte command;
  int data;
} mydata;

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
      if (mydata.destination_id == 0 || mydata.destination_id == 1) {
        digitalWrite(led_pin, HIGH);
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
        Serial.print(mydata.data);
        Serial.println();
        digitalWrite(led_pin, LOW);
      }
    }
}
