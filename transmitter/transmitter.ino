// transmitter
#include <VirtualWire.h> 
#include <EasyTransferVirtualWire.h> 
#include <Keeloq.h>
#include <LowPower.h>
#include <DHT.h>
//#include <MQ135.h>
#define DHTPIN 4     // to which contact the temperature sensor is connected
#define DHTTYPE DHT22
#define analogPin A0     // pin for reading random numbers
#define interruptPin INT0     // interrupt 0 = 2 pin on arduino pro mini
//#define CO2Pin A1     // to which contact the CO2 sensor is connected

EasyTransferVirtualWire ET;
Keeloq k(0x01320334,0x05063708);     // keys
DHT dht(DHTPIN, DHTTYPE);
//MQ135 gasSensor = MQ135(CO2Pin);

const byte pin_transmitter = 3;
const byte pin_power = 5;
const byte pin_led = 13;     // 13 - built-in led
int last_data[7];     // array with the latest data, count of cells = count of commands
unsigned int transmitter_id = 2221;     // this device id, max 65535

struct SEND_DATA_STRUCTURE {     // structure with information for transmit
  byte receiver_id = 1;     // which device receive the signal, max 255
  unsigned int packet_id;     // package number
  unsigned int random_number;     // it is necessary for the identical data after encryption to be different
  byte command;     // command number: 0 - supply voltage; 1 - temperature; etc...
  unsigned long data;     // unencrypted data
  unsigned long crypt;     // encrypted data
} mydata;     // structure variable

long readVcc() {
  // read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(200);     // wait for Vref to settle
  ADCSRA |= _BV(ADSC);     // start conversion
  while (bit_is_set(ADCSRA,ADSC));     // measuring
  byte low  = ADCL;     // must read ADCL first - it then locks ADCH  
  byte high = ADCH;     // unlocks both
  long result = (high<<8) | low;
  result = 1125300L / result;     // calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result;     // Vcc in millivolts
}

int Transmitting(byte command, int data) {
  if (last_data[command] != data || command == 1) {     // if the data is different from the latter, or a motion detection command
    last_data[command] = data;
    mydata.command = command;
    mydata.data = data;
    mydata.random_number = random(1, 42000);     // unsigned long can't make more than that!
    unsigned long raw = mydata.random_number * 100000 + transmitter_id;
    mydata.crypt = k.encrypt(raw);
    mydata.data = k.encrypt(mydata.data);
    digitalWrite(pin_led, HIGH);     // led on
    ET.sendData();
    digitalWrite(pin_led, LOW);     // led off
    mydata.packet_id++;
  }
  delay(random(1000, 2000));
}

void setup() { 
  //Serial.begin(9600);
  //Serial.println(data);
  dht.begin();
  pinMode(pin_led, OUTPUT);
  pinMode(pin_power, OUTPUT);     // pin for sensor and transmitter power supply
  vw_set_tx_pin(pin_transmitter);     // to which contact the transmitter is connected
  vw_setup(2000);     // speed bits per second
  ET.begin(details(mydata));
  analogReference(INTERNAL);
  randomSeed(analogRead(analogPin));
  digitalWrite(pin_power, HIGH);     // power up the sensor and transmitter
  for (byte i = 0; i < 3; i++) {     // cycle fo sincing transmitter and reciver
    Transmitting (0, i);     // send nothing
  }
  digitalWrite(pin_power, LOW);     // power down the sensor and transmitter
}

void wakeUp() {     // wake up the controller
}

void loop() {     // commands with data
  digitalWrite(pin_power, HIGH);     // power up the sensor and transmitter
  Transmitting (1, 1);     // motion detected
  Transmitting (2, round((float)readVcc()/1000*10));     // battery charge
  Transmitting (3, round((float)dht.readTemperature()));
  Transmitting (4, round((float)dht.readHumidity()));
  Transmitting (5, round((float)dht.computeHeatIndex((float)dht.readTemperature(), (float)dht.readHumidity(), false)));
//  Transmitting (6, round((float)gasSensor.getPPM()));
//  Transmitting (7, round((float)gasSensor.getRZero()));
  digitalWrite(pin_power, LOW);     // power down the sensor and transmitter
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  attachInterrupt(interruptPin, wakeUp, RISING);
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
  detachInterrupt(interruptPin);
}
