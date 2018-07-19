#include <VirtualWire.h> 
#include <EasyTransferVirtualWire.h> 
#include <LowPower.h>
#include <DHT.h>
#define DHTPIN 3
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
int inpin = 0;
double vcc = 5.0;
unsigned int count = 1;

EasyTransferVirtualWire ET;
struct SEND_DATA_STRUCTURE {
  unsigned int device_id = 2;
  unsigned int packet_id; 
  byte command; 
  int data;
} mydata;

int Transmitting( byte command, 
                  int data) {
  mydata.command = command;
  mydata.data = data;
  digitalWrite(13, HIGH); 
  ET.sendData();
  digitalWrite(13, LOW);
  mydata.packet_id = count++;
  //LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
  delay(2000);
}

void setup() { 
  //Serial.begin(9600);
  dht.begin();
  pinMode(13, OUTPUT);
  vw_set_tx_pin(2);
  vw_setup(2000);
  ET.begin(details(mydata));
  analogReference(INTERNAL);
}

  void loop() {
    Transmitting ( 1, 
                 (double)dht.readTemperature()*100 );
    Transmitting ( 2, 
                 (double)dht.readHumidity()*100 );
    Transmitting ( 3, 
                 (double)dht.computeHeatIndex((double)dht.readTemperature(), (double)dht.readHumidity(), false)*100 );
    Transmitting ( 4, 
                 //volt*100
                 (double)(analogRead(inpin) / 1023.0) * vcc * 100);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}
