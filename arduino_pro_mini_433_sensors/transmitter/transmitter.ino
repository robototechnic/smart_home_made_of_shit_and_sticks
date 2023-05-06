// transmitter







#define pin_meter 6     // pin for connecting a gas/water/light/etc  meter 



unsigned long meter;





void transmitting(byte command, unsigned int data) {
  digitalWrite(pin_led, HIGH);
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
    
    result = mydata.packet_id / 30;
    if (result != old_result) {
      old_result = result;
      flush_last_data();
    }
  }
  digitalWrite(pin_led, LOW);
  delay(random(1000, 2000));
}

void flush_last_data() {
  size_t n = sizeof(last_data)/sizeof(last_data[0]);
  for (byte i = 0; i < n; i++) {
    last_data[i] = 0;
  }
}



unsigned int co2() {
  while (Serial.available() > 0) {     // Read the entire buffer before sending the request. Solves communication problems with the sensor.
    Serial.read();
  }
  // MH-Z19 sensor commands
  byte request[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
  //byte calib[9] = {0xFF,0x01,0x87,0x00,0x00,0x00,0x00,0x00,0x78};
  //byte abcon[9] = {0xFF,0x01,0x79,0xA0,0x00,0x00,0x00,0x00,0xE6};
  //byte abcoff[9] = {0xFF,0x01,0x79,0x00,0x00,0x00,0x00,0x00,0x86};
  
  unsigned char response[9];
  Serial.write(request, 9);
  memset(response, 0, 9);
  Serial.readBytes(response, 9);
  byte crc = 0;
  for (byte i = 1; i < 8; i++) {
    crc += response[i];
  }
  crc = 255 - crc;
  crc++;
  if (!(response[0] == 0xFF && response[1] == 0x86 && response[8] == crc)) {
    return 10;
  } else {
    unsigned int responseHigh = (unsigned int) response[2];
    unsigned int responseLow = (unsigned int) response[3];
    unsigned int ppm = (256 * responseHigh) + responseLow;
    return ppm;
  }
}

void setup() {
  Serial.begin(9600);
  ET.begin(details(mydata));
  vw_set_tx_pin(pin_transmitter);
  vw_setup(2000);

  pinMode(pin_meter, INPUT_PULLUP);
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
  for (byte i = 0; i < 3; i++) {
    digitalWrite(pin_power, HIGH);
    if (digitalRead(pin_motion_sensor) == 1) {
      i = 0;
    }
    
    transmitting(6, round(co2() / 10));
    
    digitalWrite(pin_power, LOW);
  }

}
