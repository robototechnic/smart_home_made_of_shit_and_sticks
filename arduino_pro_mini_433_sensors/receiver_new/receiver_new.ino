#include <VirtualWire.h>
#include <EasyTransferVirtualWire.h>
#define USE_OTA
//#define USE_GetCheckSum
#define USE_ENCRYPTION
#define USE_DIMMER
#define USE_SERIAL
#define PIN_RECEIVER 2
#define PIN_POWER 5     // pin for sensor and transmitter power supply
#define PIN_LED 13
#define THIS_DEVICE_ID 216

EasyTransferVirtualWire ET;
byte transmitter_id[] = {111, 115};     // array with id's of known transmitters. It is better to put first identifier of transmitter, which sends information often
const size_t count_of_transmitters = sizeof(transmitter_id)/sizeof(transmitter_id[0]);     // count of elements in the array
unsigned int packet_id[count_of_transmitters];
bool sync_trigger = false;

#ifdef USE_OTA
  #include <OTA.h>
  #define pin_reset 17
  unsigned int timer_OTA = 0;     // how long in millis after start-up can you flash the board
#endif

#ifdef USE_ENCRYPTION
  #include <Keeloq.h>
  Keeloq k(0x34626326, 0x63934729);
#endif

#ifdef USE_DIMMER
  byte dimmer_pins[] = {6, 7};
  byte dimmer_level[sizeof(dimmer_pins)/sizeof(dimmer_pins[0])] = {};
  byte dimmer_variable[sizeof(dimmer_pins)/sizeof(dimmer_pins[0])] = {};
  const int dimmer_level_max = 100;
  unsigned long timer;
  unsigned long reset_timer;
  bool reset_trigger_1 = false;
  bool reset_trigger_2 = false;
  
  void dimmer(byte i) {     // Bresenham's line algorithm
    dimmer_variable[i] = dimmer_variable[i] + dimmer_level[i];
    if (dimmer_variable[i] >= dimmer_level_max) {
      dimmer_variable[i] = dimmer_variable[i] - dimmer_level_max;
      digitalWrite(dimmer_pins[i], HIGH);
    }
    else {
      digitalWrite(dimmer_pins[i], LOW);
    }
  }
#endif

#ifdef USE_GetCheckSum
  #include <GetCheckSum.h>
#endif

struct {
  byte receiver_id;     // which device receive the signal
  byte transmitter_id;     // this device id
  unsigned int packet_id;
  unsigned int random_number;      // discriminator it is necessary for the identical data after encryption to be different
  unsigned long command;     // command number: 0 - sleep bit or synchronization cycle; 1 - motion detect; 2 - battery charge data; 3 - temperature; 4 - humidity; 5 - compute temperature; 6 - CO2; 7 - lighting level; 10 - Dimmer_1 control; 11 - Dimmer_2 control
  unsigned long data;
  #ifdef USE_GetCheckSum
    byte checksum;     // sum of all structure objects
  #endif
} mydata;

#ifdef USE_SERIAL
  void data_human_readable(unsigned long data, byte i) {
    Serial.print("{\"transmitter_id\":");
    Serial.print(transmitter_id[i]);
    Serial.print(",\"packet_id\":");
    Serial.print(mydata.packet_id);
    Serial.print(",\"command\":");
    Serial.print(mydata.command);
    Serial.print(",\"data\":");
    if (mydata.command == 2) {      // battery charge data, float
      Serial.print((float)mydata.data / 1000, 3);
    }
    else Serial.print(mydata.data);      // other data, integer
    Serial.println("}");
  }
#endif

unsigned int synchronization(byte i) {
  if (sync_trigger == false) {     // первый шаг
    sync_trigger = true;     // взводим триггер
    #ifdef USE_SERIAL
      Serial.print("{\"transmitter_id\":");
      Serial.print(transmitter_id[i]);
      Serial.println(",\"Desync!\"}");
    #endif
  } else {      // второй шаг
    sync_trigger = false;     // сбрасываем триггер
    #ifdef USE_SERIAL
      Serial.print("{\"transmitter_id\":");
      Serial.print(transmitter_id[i]);
      Serial.println(",\"Synced\"}");
    #endif
    packet_id[i] = mydata.packet_id;
    packet_id[i]++;     // синхронизируем
  }
}

void setup() {
  //#ifdef USE_SERIAL
    Serial.begin(57600);
    Serial.println("Started!");
  //#endif
  pinMode(PIN_POWER, OUTPUT);
  digitalWrite(PIN_POWER, HIGH);
  pinMode(PIN_LED, OUTPUT);
  ET.begin(details(mydata));
  vw_set_rx_pin(PIN_RECEIVER);
  vw_setup(400);
  vw_rx_start();
  
  #ifdef USE_OTA
    pinMode(pin_reset,OUTPUT);
  #endif

  #ifdef USE_DIMMER
    for(int i = 0; i < sizeof(dimmer_pins)/sizeof(dimmer_pins[0]); i++) {
      pinMode(dimmer_pins[i], OUTPUT);
    }
    timer = millis();
  #endif
  for(int i = 0; i < 3; i++) {
    digitalWrite(PIN_LED, HIGH);
    delay(200);
    digitalWrite(PIN_LED, LOW);
    delay(200);
  }
}

void loop() {
  
  #ifdef USE_OTA
    OTA(timer_OTA, pin_reset);
  #endif
  
  if (ET.receiveData()) {
    if (mydata.receiver_id != THIS_DEVICE_ID) {
      return;
    }
    digitalWrite(PIN_LED, HIGH);
    
    #ifdef USE_ENCRYPTION
      mydata.command = k.decrypt(mydata.command);
      mydata.data = k.decrypt(mydata.data);
    #endif
    
    #ifdef USE_GetCheckSum
      if (GetCheckSum((byte*)&mydata, sizeof(mydata) - 1) != mydata.checksum) {
        return;
      }
    #endif
    
    for (byte i = 0; i < count_of_transmitters; i++) {     // verify transmitter id
      if (transmitter_id[i] != mydata.transmitter_id) {      // validity control
        continue;
      }
      if (packet_id[i] == mydata.packet_id) {     // verify synchronization
        packet_id[i]++;
        #ifdef USE_SERIAL
          data_human_readable(mydata.data, i);
        #endif
        
        #ifdef USE_DIMMER
          if (mydata.command == 4) {      // Dimmer_1 level from humidity sensor
            if (mydata.data < 95) {
              mydata.data = 0;
              digitalWrite(dimmer_pins[0], LOW);
            } else if (mydata.data >= 95) {
              mydata.data = 100;
            }
            dimmer_level[0] = mydata.data;
          }
          
          if (mydata.command == 10) {      // Dimmer_1 level from main controller
            if (mydata.data < 0) {
              mydata.data = 0;
            } else if (mydata.data > 100) {
              mydata.data = 100;
            }
            dimmer_level[0] = mydata.data;
          } else if (mydata.command == 11) {      // Dimmer_2 level from main controller
            if (mydata.data < 0) {
              mydata.data = 0;
              digitalWrite(dimmer_pins[0], LOW);
            } else if (mydata.data > 100) {
              mydata.data = 100;
            }
            dimmer_level[1] = mydata.data;
          }
        #endif
  
      } else {      // synchronizing packets
          synchronization(i);
        }
        
    }
    
    digitalWrite(PIN_LED, LOW);
    
  }

  #ifdef USE_DIMMER
    if (millis() - timer >= 20) {
      timer = millis();
      for(int i = 0; i < sizeof(dimmer_pins)/sizeof(dimmer_pins[0]); i++) {
        if (dimmer_level[i] == 0) {
          continue;
        }
        reset_trigger_1 = true;
        dimmer(i);
      }


      if (reset_trigger_1 == true && reset_trigger_2 == false) {
        reset_trigger_1 = false;
        reset_trigger_2 = true;
        reset_timer = millis();
        Serial.println("timer");
      }
      
      if (reset_trigger_2 == true && millis() - reset_timer >= 1200000) {      // 20 minets
        reset_trigger_1 = false;
        reset_trigger_2 = false;
        Serial.println("reset");
        for(int i = 0; i < sizeof(dimmer_pins)/sizeof(dimmer_pins[0]); i++) {
          dimmer_level[i] = 0;
          dimmer(i);
        }
      }
    }
  #endif

}
