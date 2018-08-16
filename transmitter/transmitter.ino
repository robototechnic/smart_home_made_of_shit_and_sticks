// transmitter
#include <VirtualWire.h> 
#include <EasyTransferVirtualWire.h> 
#include <LowPower.h>
#include <DHT.h>
#define DHTPIN 3      // to which contact the sensor is connected
#define DHTTYPE DHT22

EasyTransferVirtualWire ET;
DHT dht(DHTPIN, DHTTYPE);

//const byte led_pin = 13;      // built-in led
const byte transmit_pin = 2;      // to which contact the transmitter is connected
int last_data[4];     // count of commands

struct SEND_DATA_STRUCTURE {      // information package
  byte source_id = 2;     // this device id
  byte destination_id = 1;     // which device will receive the signal
  unsigned int packet_id; 
  byte command;     // command number
  int data;
} mydata;     // name of structure

long readVcc() {
  // read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(200);     // wait for Vref to settle
  ADCSRA |= _BV(ADSC);      // start conversion
  while (bit_is_set(ADCSRA,ADSC));      // measuring
  byte low  = ADCL;     // must read ADCL first - it then locks ADCH  
  byte high = ADCH;     // unlocks both
  long result = (high<<8) | low;
  result = 1125300L / result;     // calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result;      // Vcc in millivolts
}

int Transmitting(byte command, int data) {
  if (last_data[command] != data) {
    last_data[command] = data;
    mydata.command = command;
    mydata.data = data;
    //digitalWrite(led_pin, HIGH);      // led on
    ET.sendData();
    //digitalWrite(led_pin, LOW);      // led off
    mydata.packet_id++;
  }
  delay(random(1000, 2000));
}

void setup() { 
  //Serial.begin(9600);
  //Serial.println(data);
  dht.begin();
  //pinMode(led_pin, OUTPUT);
  vw_set_tx_pin(transmit_pin);
  vw_setup(2000);
  ET.begin(details(mydata));
  analogReference(INTERNAL);
  randomSeed(analogRead(0));
}

void loop() {
  Transmitting (0, round((float)readVcc()/1000*10));
  Transmitting (1, round((float)dht.readTemperature()*10));
  Transmitting (2, round((float)dht.readHumidity()*10));
  Transmitting (3, round(((float)dht.computeHeatIndex((float)dht.readTemperature(), (float)dht.readHumidity(), false))*10));
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}
