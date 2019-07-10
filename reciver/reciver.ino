// reciver
#include <VirtualWire.h>
#include <EasyTransferVirtualWire.h>
#include <Keeloq.h>

EasyTransferVirtualWire ET;
Keeloq k(0x01320334, 0x05063708);     // keys

const byte this_device_id = 1;
const byte pin_receiver = 2;
//const byte pin_transmitter = 3;     // to which contact the transmitter is connected
const byte pin_power = 5;     // pin for sensor and transmitter power supply
const byte count_of_transmitters = 7;
unsigned int transmitter_id[count_of_transmitters] = {2222, 2221, 3, 4, 5, 6, 7};     // array with id's of known transmitters. It is better to put first identifier of transmitter, which sends information often
unsigned int packet_id[count_of_transmitters];
bool sync_trigger = false;


struct SEND_DATA_STRUCTURE {
  byte receiver_id;     // which device receive the signal, max 255
  unsigned int packet_id;
  unsigned int random_number;      // discriminator it is necessary for the identical data after encryption to be different
  byte command;     // command number: 0 - sleep bit or synchronization cycle; 1 - motion detect; 2 - battery charge data; 3 - temperature; 4 - humidity; 5 - compute temperature; 6 - CO2; 7 - lighting level
  unsigned long data;
  unsigned long service;      // service data
} mydata;

void setup() {
  pinMode(13, OUTPUT);
  pinMode(pin_power, OUTPUT);
  digitalWrite(pin_power, LOW);
  Serial.begin(9600);
  Serial.println(" ");
  ET.begin(details(mydata));
  vw_set_rx_pin(pin_receiver);
  vw_setup(2000);
  vw_rx_start();
}

void loop() {
  if (ET.receiveData()) {
    if (mydata.receiver_id == 0 || mydata.receiver_id == this_device_id) {     // 0 = broadcast
      digitalWrite(13, HIGH);
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
              if (mydata.command == 2) {      // battery charge data, float
                Serial.println((float)mydata.data / 10, 1);
              } else if (mydata.command == 6) {      // co2 data, float
                Serial.println(mydata.data * 10);
              } else Serial.println(mydata.data);      // other data, integer
            } else {      // synchronizing packets
              if (sync_trigger == false) {     // first time, ignore packet
                sync_trigger = true;
                Serial.println("Desync!");
              } else {      // second time, syncing. Everything will be ok on the third time
                sync_trigger = false;
                Serial.println("Try to syncing...");
                packet_id[i] = mydata.packet_id;
                packet_id[i]++;
              }
            }
            break;
          } else Serial.println("Transmitter ID mismatch!");
        }
      } else Serial.println("Random number mismatch!");
      digitalWrite(13, LOW);
    }
  }
}
