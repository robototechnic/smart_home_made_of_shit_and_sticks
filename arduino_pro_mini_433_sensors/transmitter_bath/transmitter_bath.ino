// transmitter
#include <VirtualWire.h>
#include <EasyTransferVirtualWire.h>
#include <Keeloq.h>
#include <LowPower.h>
#include <DHT.h>
#define dhttype DHT22
#define pin_analog A3     // pin for reading random numbers
#define pin_interrupt INT0
#define pin_photoresistor A0
#define pin_motion_sensor 2
#define pin_transmitter 3
#define pin_dht 4
#define pin_power 5     // pin for sensor and transmitter power supply
#define pin_led 13
#define count_of_receivers 7

EasyTransferVirtualWire ET;
Keeloq k(0x34626326, 0x63934729);     // keys (0x01320334, 0x05063708);
DHT dht(pin_dht, dhttype);


int last_data[8];     // array with the latest data, count of cells = count of commands
unsigned int transmitter_id = 115;     // this device id, max 65535
byte counter;

struct send_data_structure {
  byte receiver_id = 1;     // which device receive the signal, max 255
  unsigned int packet_id = 1;
  unsigned int random_number;      // discriminator it is necessary for the identical data after encryption to be different
  byte command;     // command number: 0 - sleep bit or synchronization cycle; 1 - motion detect; 2 - battery charge data; 3 - temperature; 4 - humidity; 5 - compute temperature; 6 - CO2; 7 - lighting level
  unsigned long data;
  unsigned long service;     // service data
} mydata;

void transmitting(byte command, unsigned int data) {
  //digitalWrite(pin_led, HIGH);
  if (last_data[command] != data) {
    last_data[command] = data;
    mydata.command = command;
    mydata.data = data;
    mydata.random_number = random(1, 42000);     // unsigned long can't make more than that!
    mydata.service = mydata.random_number * 100000 + transmitter_id;
    mydata.service = k.encrypt(mydata.service);
    mydata.data = k.encrypt(mydata.data);
    ET.sendData();
    mydata.packet_id++;
  }
  digitalWrite(pin_led, LOW);
  delay(random(1000, 2000));
}

void flush_last_data () {
  size_t n = sizeof(last_data)/sizeof(last_data[0]);
  for (byte i = 0; i < n; i++) {
    last_data[i] = 0;
  }  
}

long read_vcc() {     // I do not understand what is happening here. Not my code.
  // read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(200);     // wait for Vref to settle
  ADCSRA |= _BV(ADSC);     // start conversion
  while (bit_is_set(ADCSRA, ADSC));    // measuring
  byte low  = ADCL;     // must read ADCL first - it then locks ADCH
  byte high = ADCH;     // unlocks both
  long result = (high << 8) | low;
  result = 1125300L / result;     // calculate Vcc (in mV); 1125300 = 1.1 * 1023 * 1000
  return result;     // Vcc in millivolts
}

void wake_up() {
}

ISR(PCINT1_vect) {
  counter = 0;
}

void setup() {
  Serial.begin(9600);
  pinMode(pin_power, OUTPUT);
  pinMode(pin_led, OUTPUT);
  ET.begin(details(mydata));
  dht.begin();
  vw_set_tx_pin(pin_transmitter);
  vw_setup(2000);
  analogReference(INTERNAL);
  randomSeed(analogRead(pin_analog));
  PCICR |= (1 << PCIE1);     // interrupt
  PCMSK1 |= (1 << PCINT8);
  for(int i = 0; i < 3; i++) {
    digitalWrite(pin_led, HIGH);
    delay(200);
    digitalWrite(pin_led, LOW);
    delay(200);
  }
  digitalWrite(pin_power, HIGH);
  for (byte i = 3; i > 0; i--) {     // cycle for syncing transmitter and receiver
    transmitting(1, i);
  }
  digitalWrite(pin_power, LOW);
}

void loop() {
  for (counter = 0; counter < 255; counter++) {
    digitalWrite(pin_power, HIGH);
    //transmitting(1, digitalRead(pin_motion_sensor));
    transmitting(2, round((float)read_vcc() / 100));
    transmitting(3, round((float)dht.readTemperature()));
    transmitting(4, round((float)dht.readHumidity()));
    transmitting(5, round((float)dht.computeHeatIndex((float)dht.readTemperature(), (float)dht.readHumidity(), false)));
    //transmitting(6, round(co2() / 10));
    transmitting(7, round((float)analogRead(pin_photoresistor) / 100));
    digitalWrite(pin_power, LOW);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
  
  digitalWrite(pin_power, HIGH);
  transmitting(0, 0);     // send "I sleep"
  digitalWrite(pin_power, LOW);

  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);     // sleep until motion appears
  
  digitalWrite(pin_power, HIGH);
  transmitting(0, 1);     // send "I woke up"
  digitalWrite(pin_power, LOW);
}
