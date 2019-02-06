// transmitter
#include <VirtualWire.h>
#include <EasyTransferVirtualWire.h>
#include <Keeloq.h>
#include <LowPower.h>
#include <DHT.h>
#define DHTPIN 4     // to which contact the temperature sensor is connected
#define DHTTYPE DHT22
#define analogPin A0     // pin for reading random numbers
#define interruptPin INT0     // interrupt 0 = 2 pin on arduino pro mini

EasyTransferVirtualWire ET;
Keeloq k(0x01320334, 0x05063708);    // keys
DHT dht(DHTPIN, DHTTYPE);

byte pin_hc_sr501 = 2;     // to which contact the motion sensor is connected
const byte pin_transmitter = 3;     // to which contact the transmitter is connected
const byte pin_power = 5;     // pin for sensor and transmitter power supply
const byte pin_led = 13;     // 13 - built-in led
int last_data[8];     // array with the latest data, count of cells = count of commands
unsigned int transmitter_id = 2221;     // this device id, max 65535

byte request[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
byte calib[9] = {0xFF,0x01,0x87,0x00,0x00,0x00,0x00,0x00,0x78};
byte abcon[9] = {0xFF,0x01,0x79,0xA0,0x00,0x00,0x00,0x00,0xE6};
byte abcoff[9] = {0xFF,0x01,0x79,0x00,0x00,0x00,0x00,0x00,0x86};
unsigned char response[9];

struct SEND_DATA_STRUCTURE {     // structure with information for transmit
  byte receiver_id = 1;     // which device receive the signal, max 255
  unsigned int packet_id = 1;     // package number
  unsigned int random_number;     // it is necessary for the identical data after encryption to be different
  byte command;     // command number: 0 - supply voltage; 1 - temperature; etc...
  unsigned long data;     // useful data
  unsigned long service;     // service data
} mydata;     // structure variable

long readVcc() {
  // read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(200);     // wait for Vref to settle
  ADCSRA |= _BV(ADSC);     // start conversion
  while (bit_is_set(ADCSRA, ADSC));    // measuring
  byte low  = ADCL;     // must read ADCL first - it then locks ADCH
  byte high = ADCH;     // unlocks both
  long result = (high << 8) | low;
  result = 1125300L / result;     // calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result;     // Vcc in millivolts
}

void Transmitting(byte command, unsigned int data) {
//  if (last_data[command] != data || command == 1) {     // if data is different from the last, or a motion detection command
    last_data[command] = data;
    mydata.command = command;
    mydata.data = data;
    mydata.random_number = random(1, 42000);     // unsigned long can't make more than that!
    unsigned long raw = mydata.random_number * 100000 + transmitter_id;
    mydata.service = k.encrypt(raw);
    mydata.data = k.encrypt(mydata.data);
//    digitalWrite(pin_led, HIGH);     // led on
    ET.sendData();
//    digitalWrite(pin_led, LOW);     // led off
    mydata.packet_id++;
//  }
  delay(random(1000, 2000));
}

void setup() {
  Serial.begin(9600);
  //Serial.println(mydata.data);
  dht.begin();
  pinMode(pin_led, OUTPUT);
  pinMode(pin_power, OUTPUT);
  vw_set_tx_pin(pin_transmitter);
  vw_setup(2000);     // speed bits per second
  ET.begin(details(mydata));
  analogReference(INTERNAL);
  randomSeed(analogRead(analogPin));
  digitalWrite(pin_power, HIGH);
  for (byte i = 0; i < 3; i++) {     // cycle for syncing transmitter and receiver
    Transmitting(0, i);     // send nothing
  }
  digitalWrite(pin_power, LOW);
}

void wakeUp() {     // wake up the controller
}

unsigned int co2() {
  Serial.write(request, 9);
  memset(response, 0, 9);
  Serial.readBytes(response, 9);
  byte i;
  byte crc = 0;
  for (i = 1; i < 8; i++) crc+=response[i];
  crc = 255 - crc;
  crc++;

  if ( !(response[0] == 0xFF && response[1] == 0x86 && response[8] == crc) ) {
    return 1;
  } else {
    unsigned int responseHigh = (unsigned int) response[2];
    unsigned int responseLow = (unsigned int) response[3];
    unsigned int ppm = (256*responseHigh) + responseLow;
    return ppm;
  }
}

void loop() {     // commands with data
  for (byte i = 0; i <= 2; i++) {
    digitalWrite(pin_power, HIGH);
    Transmitting(1, digitalRead(pin_hc_sr501));     // motion detected
    Transmitting(2, round((float)readVcc() / 100)); // battery charge
    Transmitting(3, round((float)dht.readTemperature()));
    Transmitting(4, round((float)dht.readHumidity()));
    Transmitting(5, round((float)dht.computeHeatIndex((float)dht.readTemperature(), (float)dht.readHumidity(), false)));
    Transmitting(6, round(co2() / 10));
    digitalWrite(pin_power, LOW);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
  digitalWrite(pin_power, HIGH);
  Transmitting(0, 10);
  digitalWrite(pin_power, LOW);
  attachInterrupt(interruptPin, wakeUp, RISING);
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
  detachInterrupt(interruptPin);
  digitalWrite(pin_power, HIGH);
  Transmitting(7, 01);
  digitalWrite(pin_power, LOW);
}
