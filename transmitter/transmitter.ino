//transmitter
#include <VirtualWire.h> 
#include <EasyTransferVirtualWire.h> 
#include <LowPower.h>
#include <DHT.h>
#define DHTPIN 3
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
const int led_pin = 13;
const int transmit_pin = 2;

int inpin = 0;
double vcc = 5.0;
unsigned int count = 1;
int last_data_1;
int last_data_2;
int last_data_3;
int last_data_4;

EasyTransferVirtualWire ET;
struct SEND_DATA_STRUCTURE {
  unsigned int source_id = 2;
  unsigned int destination_id = 1;
  unsigned int packet_id; 
  byte command; 
  int data;
} mydata;

int Transmitting( byte command, int data) {
  switch (command) {
    case 1:
      if (last_data_1 != data) {
        last_data_1 = data;
        mydata.command = command;
        mydata.data = data;
        ET.sendData();
        mydata.packet_id = count++;
      }
      break;
    case 2:
      if (last_data_2 != data) {
        last_data_2 = data;
        mydata.command = command;
        mydata.data = data;
        ET.sendData();
        mydata.packet_id = count++;
      }
      break;
    case 3:
      if (last_data_3 != data) {
        last_data_3 = data;
        mydata.command = command;
        mydata.data = data;
        ET.sendData();
        mydata.packet_id = count++;
      }
      break;
    case 4:
      if (last_data_4 != data) {
        last_data_4 = data;
        mydata.command = command;
        mydata.data = data;
        ET.sendData();
        mydata.packet_id = count++;
      }
      break;
  }

  delay(random(500, 1500));
//  digitalWrite(led_pin, HIGH);
//  digitalWrite(led_pin, LOW);
}

void setup() { 
//  Serial.begin(9600);
//  Serial.println(data);
  dht.begin();
  pinMode(led_pin, OUTPUT);
  vw_set_tx_pin(transmit_pin);
  vw_setup(2000);
  ET.begin(details(mydata));
  analogReference(INTERNAL);
  randomSeed(analogRead(0));
}

  void loop() {
    Transmitting ( 1, round((double)dht.readTemperature()));
    Transmitting ( 2, round((double)dht.readHumidity()));
    Transmitting ( 3, round((double)dht.computeHeatIndex((double)dht.readTemperature(), (double)dht.readHumidity(), false)));
    Transmitting ( 4, (double)(analogRead(inpin) / 1023.0) * vcc * 100);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}
