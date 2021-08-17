// receiver
#include <VirtualWire.h>
#include <EasyTransferVirtualWire.h>
#include <Keeloq.h>
#include <RCSwitch.h>
#define this_device_id 1
#define count_of_transmitters 7
#define pin_receiver 2
#define pin_power 5     // pin for sensor and transmitter power supply
#define pin_dimmer 6
#define pin_dimmer_1 7
#define pin_led 13

EasyTransferVirtualWire ET;
Keeloq k(0x01320334, 0x05063708);     // keys
RCSwitch mySwitch = RCSwitch();

unsigned int transmitter_id[count_of_transmitters] = {1, 2, 3, 4, 5, 6, 7};     // array with id's of known transmitters. It is better to put first identifier of transmitter, which sends information often
unsigned int packet_id[count_of_transmitters];
bool sync_trigger = false;

byte dimmer_level = 0;
int dimmer_variable = 0;
int dimmer_level_max = 10;

struct send_data_structure {
  byte receiver_id;     // which device receive the signal, max 255
  unsigned int packet_id;
  unsigned int random_number;      // discriminator it is necessary for the identical data after encryption to be different
  byte command;     // command number: 0 - sleep bit or synchronization cycle; 1 - motion detect; 2 - battery charge data; 3 - temperature; 4 - humidity; 5 - compute temperature; 6 - CO2; 7 - lighting level
  unsigned long data;
  unsigned long service;      // service data
} mydata;

void dimmer(byte i) {     // Bresenham's line algorithm
  dimmer_variable = dimmer_variable + i;
  if (dimmer_variable >= dimmer_level_max) {
    dimmer_variable = dimmer_variable - dimmer_level_max;
    digitalWrite(pin_dimmer, HIGH);
  }
  else {
    digitalWrite(pin_dimmer, LOW);
  }
  delay(20);
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
  } else if (mydata.command == 7) {      // dimmer level
    Serial.print(mydata.data);
    dimmer_level = mydata.data;
    dimmer(dimmer_level);
  } else Serial.print(mydata.data);      // other data, integer
  Serial.println("}");
}

void synchronization(byte i) {
  if (sync_trigger == false) {     // first time, ignore packet
    sync_trigger = true;
    Serial.println("Desync!");
  } else {      // second time, syncing. Everything will be ok on the third time
    sync_trigger = false;
    Serial.println("Try to syncing...");
    packet_id[i] = mydata.packet_id;
    packet_id[i]++;
    return packet_id[i];
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Test");
  pinMode(pin_power, OUTPUT);
  pinMode(pin_led, OUTPUT);
  pinMode(pin_dimmer, OUTPUT);
  ET.begin(details(mydata));
  vw_set_rx_pin(pin_receiver);
  vw_setup(2000);
  vw_rx_start();
  mySwitch.enableReceive(0);
  mySwitch.enableTransmit(3);
  for(int i = 0; i < 3; i++) {
    digitalWrite(pin_led, HIGH);
    delay(200);
    digitalWrite(pin_led, LOW);
    delay(200);
  }
}

void loop() {
  //if (ET.receiveData()) {
    //if (mydata.receiver_id == 0 || mydata.receiver_id == this_device_id) {     // 0 = broadcast
      //digitalWrite(pin_led, HIGH);
      //mydata.service = k.decrypt(mydata.service);     // decrypt service data
      //unsigned int random_number = floor((mydata.service - mydata.receiver_id) / 100000);     // calculate a random number
      //if (random_number == mydata.random_number) {      // compare with an unencrypted number
        //unsigned int obtained_id = mydata.service - random_number * 100000;     // calculate obtained id of transmitter
        //for (byte i = 0; i < count_of_transmitters; i++) {     // verify transmitter id
          //if (transmitter_id[i] == obtained_id) {     // verify id of transmitter
            //if (packet_id[i] == mydata.packet_id) {     // verify synchronization
              //packet_id[i]++;
              //mydata.data = k.decrypt(mydata.data);
              //data_human_readable(mydata.data, i);
            //} else {      // synchronizing packets
              //synchronization(i);
            //}
            //break;
          //};
          //// else Serial.println("Transmitter ID mismatch!:" obtained_id)
        //}
      //} else Serial.println("Random number mismatch!");
      //digitalWrite(pin_led, LOW);
    //}
  //}

    if (mySwitch.available()) {
    digitalWrite(pin_led, HIGH);
    Serial.println( mySwitch.getReceivedValue() );
    mySwitch.resetAvailable();
    digitalWrite(pin_led, LOW);
  }

  if (Serial.available() > 0) {
    digitalWrite(pin_led, HIGH);
    digitalWrite(pin_power, HIGH);
    String readString = "";
    readString = Serial.readStringUntil('\n');
    Serial.println("readString: ");
    Serial.println(readString);
    long code = readString.toInt();
    Serial.println("code: ");
    Serial.println(code);
    mySwitch.send(code, 24);
    digitalWrite(pin_power, LOW);
    digitalWrite(pin_led, LOW);
  }
  delay(10);
}
