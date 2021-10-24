// receiver
#include <VirtualWire.h>
#include <EasyTransferVirtualWire.h>
#include <Keeloq.h>
#define pin_receiver 2
#define pin_power 5     // pin for sensor and transmitter power supply
#define pin_dimmer_0 10
#define pin_dimmer_1 11
#define pin_led 13
#define this_device_id 1
#define count_of_transmitters 7

EasyTransferVirtualWire ET;
Keeloq k(0x34626326, 0x63934729);     // keys (0x01320334, 0x05063708);

byte transmitter_id[count_of_transmitters] = {111, 115, 1, 2, 3, 4, 5};     // array with id's of known transmitters. It is better to put first identifier of transmitter, which sends information often
unsigned int packet_id[count_of_transmitters];
bool sync_trigger = false;

byte pin_dimmer[2] = {pin_dimmer_0, pin_dimmer_1};
byte dimmer_level[2] = {}; 
byte dimmer_variable[2] = {}; 
const int dimmer_level_max = 50;

struct send_data_structure {
  byte receiver_id;     // which device receive the signal, max 255
  unsigned int packet_id;
  unsigned int random_number;      // discriminator it is necessary for the identical data after encryption to be different
  byte command;     // command number: 0 - sleep bit or synchronization cycle; 1 - motion detect; 2 - battery charge data; 3 - temperature; 4 - humidity; 5 - compute temperature; 6 - CO2; 7 - lighting level; 10 - dimmer_1 control; 11 - dimmer_2 control
  unsigned long data;
  unsigned long service;      // service data
} mydata;

void dimmer(byte i) {     // Bresenham's line algorithm
  dimmer_variable[i] = dimmer_variable[i] + dimmer_level[i];
  if (dimmer_variable[i] >= dimmer_level_max) {
    dimmer_variable[i] = dimmer_variable[i] - dimmer_level_max;
    digitalWrite(pin_dimmer[i], HIGH);
  }
  else {
    digitalWrite(pin_dimmer[i], LOW);
  }
  delay(10);
}

void data_human_readable(unsigned long data, byte i) {
  Serial.print("{\"transmitter_id\":");
  Serial.print(transmitter_id[i]);
  Serial.print(",\"packet_id\":");
  Serial.print(mydata.packet_id);
  Serial.print(",\"command\":");
  Serial.print(mydata.command);
  Serial.print(",\"data\":");
  if (mydata.command == 2) {      // battery charge data, float
    Serial.print((float)mydata.data / 10, 1);
  } else if (mydata.command == 6) {      // co2 data, float
    Serial.print(mydata.data * 10);
  } else if (mydata.command == 10) {      // dimmer_1 level
      if (mydata.data <= 0) {
        dimmer_level[0] = 0;
      } else if (mydata.data >= 50) {
        dimmer_level[0] = 50;
      } else {
        dimmer_level[0] = mydata.data;
      }
    Serial.print(dimmer_level[0]);
  } else if (mydata.command == 11) {      // dimmer_2 level
      if (mydata.data <= 0) {
        dimmer_level[1] = 0;
      } else if (mydata.data >= 50) {
        dimmer_level[1] = 50;
      } else {
        dimmer_level[1] = mydata.data;
      }
    Serial.print(dimmer_level[1]);
  }
  else Serial.print(mydata.data);      // other data, integer
  Serial.println("}");
}

unsigned int synchronization(byte i) {
  if (sync_trigger == false) {     // first time, ignore packet
    sync_trigger = true;
    Serial.print("{\"transmitter_id\":");
    Serial.print(transmitter_id[i]);
    Serial.println(",\"Desync!\"}");
  } else {      // second time, syncing. Everything will be ok on the third time
    sync_trigger = false;
    Serial.print("{\"transmitter_id\":");
    Serial.print(transmitter_id[i]);
    Serial.println(",\"Try to syncing...\"}");
    packet_id[i] = mydata.packet_id;
    packet_id[i]++;
    return packet_id[i];
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(pin_power, OUTPUT);
  pinMode(pin_led, OUTPUT);
  pinMode(pin_dimmer[0], OUTPUT);
  pinMode(pin_dimmer[1], OUTPUT);
  ET.begin(details(mydata));
  vw_set_rx_pin(pin_receiver);
  vw_setup(2000);
  vw_rx_start();
  for(int i = 0; i < 3; i++) {
    digitalWrite(pin_led, HIGH);
    delay(200);
    digitalWrite(pin_led, LOW);
    delay(200);
  }
}

void loop() {
  if (ET.receiveData()) {
    if (mydata.receiver_id == this_device_id) {
      digitalWrite(pin_led, HIGH);
      mydata.service = k.decrypt(mydata.service);     // decrypt service data
      unsigned int random_number = floor((mydata.service - mydata.receiver_id) / 100000);     // calculate a random number
      if (random_number == mydata.random_number) {      // compare with an unencrypted number
        unsigned int obtained_id = mydata.service - random_number * 100000;     // calculate obtained id of transmitter
        for (byte i = 0; i <= count_of_transmitters; i++) {     // verify transmitter id
          if (transmitter_id[i] == obtained_id) {     // verify id of transmitter
            if (packet_id[i] == mydata.packet_id) {     // verify synchronization
              packet_id[i]++;
              mydata.data = k.decrypt(mydata.data);
              data_human_readable(mydata.data, i);
            } else {      // synchronizing packets
              synchronization(i);
            }
            break;
          };
          // else Serial.println("Transmitter ID mismatch!:" obtained_id)
        }
      } else Serial.println("Random number mismatch!");
      digitalWrite(pin_led, LOW);
    }
  }
  dimmer(0);
  dimmer(1);
}
