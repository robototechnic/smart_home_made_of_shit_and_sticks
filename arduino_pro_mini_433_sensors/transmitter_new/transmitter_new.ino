#include <VirtualWire.h>
#include <EasyTransferVirtualWire.h>
//#define USE_OTA
#define USE_VCCRead
#define USE_DHT
#define USE_LOWPOWER_WAKEUP_BY_PHOTORESISTOR
//#define USE_LOWPOWER_WAKEUP_BY_MOTION_SENSOR
#define USE_FLUSH_LAST_DATA
//#define USE_GetCheckSum
#define USE_ENCRYPTION
//#define USE_SERIAL
#define PIN_POWER 5     // pin for sensor and transmitter power supply
#define PIN_LED 13
#define THIS_DEVICE_ID 115
#define RECEIVER_ID 216
#define PIN_TRANSMITTER 3
#define PIN_ANALOG 27     // pin for reading random numbers
#define DATA_ARRAY_SIZE 8
#define SLEEP_COUNTER 30

EasyTransferVirtualWire ET;

struct {
  byte receiver_id = RECEIVER_ID;     // which device receive the signal
  byte transmitter_id = THIS_DEVICE_ID;     // this device id
  unsigned int packet_id;
  unsigned int random_number;      // discriminator it is necessary for the identical data after encryption to be different
  unsigned long command;     // command number: 0 - sleep bit or synchronization cycle; 1 - motion detect; 2 - battery charge data; 3 - temperature; 4 - humidity; 5 - compute temperature; 6 - CO2; 7 - lighting level
  unsigned long data;
  #ifdef USE_GetCheckSum
    byte checksum;     // sum of all structure objects 
  #endif
} mydata;

#ifdef USE_OTA
  #include <OTA.h>
  #define pin_reset 17
  unsigned int timer_OTA = 10000;     // how long in millis after start-up can you flash the board
#endif

#ifdef USE_VCCRead
  #include <VCCRead.h>
#endif

#ifdef USE_DHT
  #include <DHT.h>
  #define dhttype DHT22
  #define pin_dht 4
  DHT dht(pin_dht, dhttype);
#endif

#ifdef USE_LOWPOWER_WAKEUP_BY_PHOTORESISTOR
  #include <LowPower.h>
  #define pin_photoresistor A0     // 5528 LDR + 4.7K Ohm resistor. For illumination readings
  volatile byte counter_before_sleep = SLEEP_COUNTER;
  ISR(PCINT1_vect) {
    counter_before_sleep = SLEEP_COUNTER;
  }
#endif

#ifdef USE_LOWPOWER_WAKEUP_BY_MOTION_SENSOR
  #include <LowPower.h>
  #define pin_motion_sensor 2
  #define pin_interrupt INT0
  void wake_up() {     // interrupt
  }
#endif

#ifdef USE_FLUSH_LAST_DATA
  //int last_data[] = {0, 1, 2, 3, 4, 5, 6, 7};     // array with the latest data, count of cells = count of commands
  int last_data[DATA_ARRAY_SIZE] = {};
  void flush_last_data() {
  byte i = mydata.packet_id % 30;     // остаток от деления на 30. сбрасываем сохраненные значения после каждых 30 отправлений
  if (i == 29) {
    for (byte i = 0; i < DATA_ARRAY_SIZE; i++) {
      last_data[i] = 0;
    }
    
    
    //const size_t n = sizeof(last_data)/sizeof(last_data[0]);
    //for (byte i = 0; i < n; i++) {
      //last_data[i] = 0;
    //}
  }
}
#endif

#ifdef USE_GetCheckSum
  #include <GetCheckSum.h>
#endif

#ifdef USE_ENCRYPTION
  #include <Keeloq.h>
  Keeloq k(0x34626326, 0x63934729);
#endif

void transmitting(byte command, unsigned long data) {
  #ifdef USE_FLUSH_LAST_DATA
    if (last_data[command] == data && last_data[4] <= 99) {     // ХУЙНЯ!
      return;
    }
    last_data[command] = data;
    flush_last_data();
  #endif
  
  unsigned long seed;
  seed = millis();
  seed *= analogRead(PIN_ANALOG);
  randomSeed(seed);
  
  mydata.random_number = random(1, 65535);
  mydata.command = command;
  mydata.data = data;
  
  #ifdef USE_GetCheckSum
    mydata.checksum = 0;
    mydata.checksum  = GetCheckSum((byte*)&mydata, sizeof(mydata));
  #endif
  
  #ifdef USE_SERIAL
    Serial.println(mydata.receiver_id);
    Serial.println(mydata.transmitter_id);
    Serial.println(mydata.packet_id);
    Serial.println(mydata.random_number);
    Serial.println(mydata.command);
    Serial.println(mydata.data);
    //Serial.println(mydata.checksum);
    Serial.println("");
  #endif
  
  #ifdef USE_ENCRYPTION
    mydata.command = k.encrypt(mydata.command);
    mydata.data = k.encrypt(mydata.data);
  #endif
  
  ET.sendData();
  mydata.packet_id++;
  counter_before_sleep++;
  delay(random(2000, 3000));
}

void setup() {
  #ifdef USE_SERIAL
    Serial.begin(57600);     // the speed of this board when write a sketch, you can see it in "boards.txt"
  #endif
  
  pinMode(PIN_POWER, OUTPUT);
  digitalWrite(PIN_POWER, HIGH);
  pinMode(PIN_LED, OUTPUT);  
  analogReference(INTERNAL);     // for voltage measurement
  ET.begin(details(mydata));
  vw_set_tx_pin(PIN_TRANSMITTER);
  vw_setup(400);
  for (byte i = 0; i < 2; i++) {     // cycle for syncing transmitter and receiver
    transmitting(0, i);
    delay(random(2000, 3000));
  }

  #ifdef USE_OTA
    pinMode(pin_reset,OUTPUT);
    OTA(timer_OTA, pin_reset);
  #endif

  #ifdef USE_DHT
    dht.begin();
  #endif

  #ifdef USE_LOWPOWER_WAKEUP_BY_PHOTORESISTOR
    PCICR |= (1 << PCIE1);     // interrupt
    PCMSK1 |= (1 << PCINT8);
  #endif

}

void loop() {
  #ifdef USE_VCCRead
    transmitting(2, VCCRead());
  #endif

  #ifdef USE_DHT
    transmitting(3, round((float)dht.readTemperature()));
    transmitting(4, round((float)dht.readHumidity()));
    transmitting(5, round((float)dht.computeHeatIndex((float)dht.readTemperature(), (float)dht.readHumidity(), false)));
  #endif

  #ifdef USE_LOWPOWER_WAKEUP_BY_PHOTORESISTOR
  transmitting(7, round((float)analogRead(pin_photoresistor) / 100));
  if (counter_before_sleep > 0) {
    counter_before_sleep--;
  }
  if (counter_before_sleep != 0) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    return;
  } else {
    transmitting(0, 0);     // send "I sleep"
    while (counter_before_sleep != SLEEP_COUNTER) {     // sleep until light appears
      LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    }
    transmitting(0, 1);     // send "I woke up"
  }
  #endif

  #ifdef USE_LOWPOWER_WAKEUP_BY_MOTION_SENSOR //outdated, needs rewrite
    transmitting(1, digitalRead(pin_motion_sensor));
    transmitting(0, 0);     // send "I sleep"
    attachInterrupt(pin_interrupt, wake_up, RISING);
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);     // sleep until motion appears
    detachInterrupt(pin_interrupt);
    transmitting(0, 1);     // send "I woke up"
  #endif

}
