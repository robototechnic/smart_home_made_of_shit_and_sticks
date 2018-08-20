// reciver
#include <VirtualWire.h>
#include <EasyTransferVirtualWire.h>
#include <Keeloq.h>

EasyTransferVirtualWire ET;
Keeloq k(0x01320334,0x05063708);      //keys

const unsigned int this_device_id = 1111; //unsigned int

struct SEND_DATA_STRUCTURE {      // structure with information for transmit
  byte source_id;     // which device transmits the signal, max 255 devices
  unsigned int destination_id;     // which device receive the signal
  unsigned int packet_id;     // package number
  unsigned int random_count;      // it is necessary for the identical data after encryption to be different
  byte command;     // command number
  unsigned int data;      // unencrypted data
  unsigned long crypt;      // encrypted data
} mydata;     // structure variable

void setup() {
  pinMode(13, OUTPUT);      // 13 - built-in led
  Serial.begin(9600);
  ET.begin(details(mydata));
  vw_set_rx_pin(2);     // to which contact the reciver is connected
  vw_setup(2000);     // speed bits per second
  vw_rx_start();      // start the receiver
}

void loop() {
  if (ET.receiveData()) {
    //if (mydata.destination_id == 0 || mydata.destination_id == 1) {     // 0 - broadcast, 1 - this device id
    if (mydata.destination_id == this_device_id) {
      digitalWrite(13, HIGH);      // 13 - built-in led, led on
//      Serial.print("Encrypted data: ");
//      Serial.print(mydata.crypt);
      mydata.crypt = k.decrypt(mydata.crypt);     // decrypt the data
//      Serial.print(" Decrypted data: ");
//      Serial.println(mydata.crypt);
      unsigned int random_count = floor((mydata.crypt - mydata.destination_id) / 100000);
      if (random_count == mydata.random_count) {
        unsigned int received_id = mydata.crypt - random_count * 100000;
        if (received_id == this_device_id) {
          Serial.print("Destination ID: ");
          Serial.print(received_id);
          Serial.print(" Source ID: ");
          Serial.print(mydata.source_id); 
          Serial.print(" Packet ID: ");
          Serial.print(mydata.packet_id);
          Serial.print(" Command: ");
          Serial.print(mydata.command);
          Serial.print(" Data: ");
          if (mydata.command == 0) {      // battery charge data
            Serial.println((float)mydata.data / 10, 1);     // float
          } else Serial.println(mydata.data);      // other data, integer
        }
        else {
          Serial.println("Decrypted ID mismatch!");
//          Serial.print("This device id: "); 
//          Serial.println(this_device_id);
//          Serial.print("Calculated destination id: ");
//          Serial.println(received_id);
//          Serial.println();          
        }
      }
      else {
        Serial.println("Random count mismatch!");
//        Serial.print("Recived random count: "); 
//        Serial.println(mydata.random_count);
//        Serial.print("Calculated random count: ");
//        Serial.println(random_count);
//        Serial.println();
      }
      digitalWrite(13, LOW);      // 13 - built-in led, led off
    }
  }
}
