// transmitter
//#include <EEPROM.h>
#include <VirtualWire.h> 
#include <EasyTransferVirtualWire.h> 
#include <Keeloq.h>
#include <LowPower.h>
#include <DHT.h>
#define DHTPIN 3      // to which contact the sensor is connected
#define DHTTYPE DHT22

EasyTransferVirtualWire ET;
Keeloq k(0x01320334,0x05063708);      // keys
DHT dht(DHTPIN, DHTTYPE);

int last_data[4];     // array with the latest data, count of cells = count of commands
unsigned int transmitter_id = 2222;     // this device id, max 65535

struct SEND_DATA_STRUCTURE {      // structure with information for transmit
  byte receiver_id = 1;     // which device receive the signal, max 255
  unsigned int packet_id;     // package number
  unsigned int random_number;      // it is necessary for the identical data after encryption to be different
  byte command;     // command number: 0 - supply voltage; 1 - temperature; etc...
  unsigned long data;      // unencrypted data
  unsigned long crypt;      // encrypted data
} mydata;     // structure variable

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
    mydata.random_number = random(1, 42000);     // unsigned long can't make more than that!
    unsigned long raw = mydata.random_number * 100000 + transmitter_id;
    mydata.crypt = k.encrypt(raw);
    mydata.data = k.encrypt(mydata.data);
    //digitalWrite(13, HIGH);      // 13 - built-in led, led on
    ET.sendData();
    //digitalWrite(13, LOW);      // 13 - built-in led, led off
    mydata.packet_id++;
  }
  delay(random(1000, 2000));
}

void setup() { 
  //Serial.begin(9600);
  //Serial.println(data);
  dht.begin();
  //pinMode(13, OUTPUT);      // 13 - built-in led
  vw_set_tx_pin(2);     // to which contact the transmitter is connected
  vw_setup(2000);     // speed bits per second 
  ET.begin(details(mydata));
  analogReference(INTERNAL);
  randomSeed(analogRead(0));
  for (byte i = 0; i < 3; i++) {      // cycle fo sincing transmitter and reciver
    Transmitting (0, i);      // send nothing
  }
}



void loop() {     // commands with data 
  Transmitting (0, round((float)readVcc()/1000*10));      // battery charge
  Transmitting (1, round((float)dht.readTemperature()));
  Transmitting (2, round((float)dht.readHumidity()));
  Transmitting (3, round((float)dht.computeHeatIndex((float)dht.readTemperature(), (float)dht.readHumidity(), false)));
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}
