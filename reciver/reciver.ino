#include <VirtualWire.h>
#include <EasyTransferVirtualWire.h>

//const int led_pin = 13;
//const int receive_pin = 2;

EasyTransferVirtualWire ET;

struct SEND_DATA_STRUCTURE {
  unsigned int device_id;
  unsigned int packet_id;
  byte command;
  int data;
} mydata;

void setup()
{
    pinMode(13, OUTPUT);
    Serial.begin(9600);
    ET.begin(details(mydata));
    vw_set_rx_pin(2);
    vw_setup(2000);
    vw_rx_start();
}

void loop()
{
    if(ET.receiveData()) // получили пакет данных, обрабатываем
    {
        digitalWrite(13, HIGH);
        Serial.print("Got: ");
        Serial.print("Device ID: ");
        Serial.print(mydata.device_id);       
        Serial.print(" Packet ID: ");
        Serial.print(mydata.packet_id);
        Serial.print(" Command: ");
        Serial.print(mydata.command);
        Serial.print(" Data: ");
        Serial.print(mydata.data);
        Serial.println();
        digitalWrite(13, LOW);
    }
}
