// transmitter
#include <VirtualWire.h>
#include <EasyTransferVirtualWire.h>
#include <Keeloq.h>
#define pin_analog A3     // pin for reading random numbers
#define pin_transmitter 3
#define pin_power 5     // pin for sensor and transmitter power supply
#define pin_led 13
#define count_of_receivers 7

EasyTransferVirtualWire ET;
Keeloq k(0x34626326, 0x63934729);     // keys (0x01320334, 0x05063708);

byte transmitter_id = 111;     // this device id, max 225
unsigned int packet_id[count_of_receivers] = {100, 2, 3, 4, 5, 6, 7};

struct send_data_structure {
  byte receiver_id;     // which device receive the signal, max 255
  unsigned int packet_id = 1;
  unsigned int random_number;     // discriminator it is necessary for the identical data after encryption to be different
  byte command;     // command number: 0 - sleep bit or synchronization cycle; 1 - motion detect; 2 - battery charge data; 3 - temperature; 4 - humidity; 5 - compute temperature; 6 - CO2; 7 - lighting level
  unsigned long data;
  unsigned long service;     // service data
} mydata;

void transmitting(byte receiver_id, byte command, unsigned int data) {
  digitalWrite(pin_led, HIGH);
  mydata.receiver_id = receiver_id;
  mydata.command = command;
  mydata.data = data;
  mydata.random_number = random(1, 42000);     // unsigned long can't make more than that!
  mydata.service = mydata.random_number * 100000 + transmitter_id;
  mydata.service = k.encrypt(mydata.service);
  mydata.data = k.encrypt(mydata.data);
  mydata.packet_id = packet_id[receiver_id];
  ET.sendData();
  packet_id[receiver_id]++;
  digitalWrite(pin_led, LOW);
  delay(random(1000, 2000));
}

void setup() {
  Serial.begin(9600);
  Serial.println("Test");
  Serial.println("Using: receiver_id command data");
  pinMode(pin_power, OUTPUT);
  pinMode(pin_led, OUTPUT);
  ET.begin(details(mydata));
  vw_set_tx_pin(pin_transmitter);
  vw_setup(2000);
  analogReference(INTERNAL);
  randomSeed(analogRead(pin_analog));
  for(int i = 0; i < 2; i++) {
    digitalWrite(pin_led, HIGH);
    delay(200);
    digitalWrite(pin_led, LOW);
    delay(200);
  }
  //for (byte x = 0; x = count_of_receivers; x++) {     // cycle for syncing transmitter and receiver
    //for (byte i = 3; i > 0; i--) {
      //transmitting(x, 0, i);
    //}
  //}
}

void loop() {
  String readString;
  byte receiver_id;
  unsigned int command;
  unsigned int data;
  if (Serial.available() > 0) {
    readString = Serial.readStringUntil(' ');
    receiver_id = readString.substring(0, 3).toInt();
    Serial.print("receiver_id:");
    Serial.print(receiver_id);
    
    readString = Serial.readStringUntil(' ');
    command = readString.substring(0, 3).toInt();
    Serial.print(" command:");
    Serial.print(command);
    
    readString = Serial.readStringUntil('\n');
    data = readString.substring(0, 3).toInt();
    Serial.print(" data:");
    Serial.println(data);
    
    digitalWrite(pin_power, HIGH);
    transmitting(receiver_id, command, data);
    digitalWrite(pin_power, LOW);
  }
}
