// reciver
#include <VirtualWire.h>
#include <EasyTransferVirtualWire.h>
#include <Keeloq.h>

EasyTransferVirtualWire ET;
Keeloq k(0x01320334, 0x05063708);     // keys

const byte this_device_id = 1;
const byte count_of_transmitters = 7;
unsigned int transmitter_id[count_of_transmitters] = {2222, 2221, 3, 4, 5, 6, 7};     // array with id's of known transmitters. It is better to put first identifier of transmitter, which sends information often
unsigned int packet_id[count_of_transmitters];
bool b = false;       // trigger for synchronization
const byte pin_power = 5;     // pin for sensor and transmitter power supply

struct SEND_DATA_STRUCTURE {      // structure with information for transmit
  byte receiver_id;     // which device receive the signal, max 255
  unsigned int packet_id;     // package number
  unsigned int random_number;      // discriminator it is necessary for the identical data after encryption to be different
  byte command;     // command number
  unsigned long data;      // useful data
  unsigned long service;      // service data
} mydata;     // structure variable

void setup() {
  pinMode(13, OUTPUT);      // 13 - built-in led
  pinMode(pin_power, OUTPUT);
  digitalWrite(pin_power, LOW);
  Serial.begin(9600);
  Serial.println(" ");
  ET.begin(details(mydata));
  vw_set_rx_pin(2);     // to which contact the reciver is connected
  vw_setup(2000);     // speed bits per second
  vw_rx_start();      // start the receiver
}

void loop() {
  if (ET.receiveData()) {
    if (mydata.receiver_id == 0 || mydata.receiver_id == this_device_id) {     // 0 - broadcast, or this device id
      digitalWrite(13, HIGH);      // 13 - built-in led, led on
      mydata.service = k.decrypt(mydata.service);     // decrypt service data
      unsigned int random_number = floor((mydata.service - mydata.receiver_id) / 100000);     // calculate a random number
      if (random_number == mydata.random_number) {      // compare with an unencrypted number
        unsigned int obtained_id = mydata.service - random_number * 100000;     // calculate obtained id of transmitter
        for (byte i = 0; i < count_of_transmitters; i++) {     // verify transmitter id
          if (transmitter_id[i] == obtained_id) {     // verify id of transmitter
            if (packet_id[i] == mydata.packet_id) {     // verify synchronization
              packet_id[i]++;
              mydata.data = k.decrypt(mydata.data);
              Serial.print("Receiver ID: ");
              Serial.print(mydata.receiver_id);
              Serial.print(" Transmitter ID: ");
              Serial.print(transmitter_id[i]);
              Serial.print(" Packet ID: ");
              Serial.print(mydata.packet_id);
              Serial.print(" Command: ");
              Serial.print(mydata.command);
              Serial.print(" Data: ");
              if (mydata.command == 2) {      // battery charge data
                Serial.println((float)mydata.data / 10, 1);     // float
              } else if (mydata.command == 6) {      // co2 data
                Serial.println(mydata.data * 10);     // float
              } else Serial.println(mydata.data);      // other data, integer
            } else {      // synchronizing packets
              if (b == false) {     // first time, ignore packet
                b = true;     // trigger
                //Serial.println("Desync!");
              } else {      // second time, syncing. Everything will be ok on the third time
                b = false;
                //Serial.println("Try to syncing...");
                packet_id[i] = mydata.packet_id;
                packet_id[i]++;
              }
            }
            break;
          } //else Serial.println("Transmitter ID mismatch!");
        }
      } //else Serial.println("Random number mismatch!");
      digitalWrite(13, LOW);      // 13 - built-in led, led off
    }
  }
}
