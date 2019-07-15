static const byte crc_table[256] = {
  0x00, 0x1D, 0x3A, 0x27, 0x74, 0x69, 0x4E, 0x53, 0xE8, 0xF5, 0xD2, 0xCF, 0x9C, 0x81, 0xA6, 0xBB,
  0xCD, 0xD0, 0xF7, 0xEA, 0xB9, 0xA4, 0x83, 0x9E, 0x25, 0x38, 0x1F, 0x02, 0x51, 0x4C, 0x6B, 0x76,
  0x87, 0x9A, 0xBD, 0xA0, 0xF3, 0xEE, 0xC9, 0xD4, 0x6F, 0x72, 0x55, 0x48, 0x1B, 0x06, 0x21, 0x3C,
  0x4A, 0x57, 0x70, 0x6D, 0x3E, 0x23, 0x04, 0x19, 0xA2, 0xBF, 0x98, 0x85, 0xD6, 0xCB, 0xEC, 0xF1,
  0x13, 0x0E, 0x29, 0x34, 0x67, 0x7A, 0x5D, 0x40, 0xFB, 0xE6, 0xC1, 0xDC, 0x8F, 0x92, 0xB5, 0xA8,
  0xDE, 0xC3, 0xE4, 0xF9, 0xAA, 0xB7, 0x90, 0x8D, 0x36, 0x2B, 0x0C, 0x11, 0x42, 0x5F, 0x78, 0x65,
  0x94, 0x89, 0xAE, 0xB3, 0xE0, 0xFD, 0xDA, 0xC7, 0x7C, 0x61, 0x46, 0x5B, 0x08, 0x15, 0x32, 0x2F,
  0x59, 0x44, 0x63, 0x7E, 0x2D, 0x30, 0x17, 0x0A, 0xB1, 0xAC, 0x8B, 0x96, 0xC5, 0xD8, 0xFF, 0xE2,
  0x26, 0x3B, 0x1C, 0x01, 0x52, 0x4F, 0x68, 0x75, 0xCE, 0xD3, 0xF4, 0xE9, 0xBA, 0xA7, 0x80, 0x9D,
  0xEB, 0xF6, 0xD1, 0xCC, 0x9F, 0x82, 0xA5, 0xB8, 0x03, 0x1E, 0x39, 0x24, 0x77, 0x6A, 0x4D, 0x50,
  0xA1, 0xBC, 0x9B, 0x86, 0xD5, 0xC8, 0xEF, 0xF2, 0x49, 0x54, 0x73, 0x6E, 0x3D, 0x20, 0x07, 0x1A,
  0x6C, 0x71, 0x56, 0x4B, 0x18, 0x05, 0x22, 0x3F, 0x84, 0x99, 0xBE, 0xA3, 0xF0, 0xED, 0xCA, 0xD7,
  0x35, 0x28, 0x0F, 0x12, 0x41, 0x5C, 0x7B, 0x66, 0xDD, 0xC0, 0xE7, 0xFA, 0xA9, 0xB4, 0x93, 0x8E,
  0xF8, 0xE5, 0xC2, 0xDF, 0x8C, 0x91, 0xB6, 0xAB, 0x10, 0x0D, 0x2A, 0x37, 0x64, 0x79, 0x5E, 0x43,
  0xB2, 0xAF, 0x88, 0x95, 0xC6, 0xDB, 0xFC, 0xE1, 0x5A, 0x47, 0x60, 0x7D, 0x2E, 0x33, 0x14, 0x09,
  0x7F, 0x62, 0x45, 0x58, 0x0B, 0x16, 0x31, 0x2C, 0x97, 0x8A, 0xAD, 0xB0, 0xE3, 0xFE, 0xD9, 0xC4
};

byte fast_crc(byte data[], int len){
  byte crc = 0;

  crc ^= 0xff;
  while (len--){
    crc = crc_table[*data++ ^ crc];
  }
  crc &= 0xff;
  crc ^= 0xff;
  return crc;
};

unsigned int rx_message_count = 0;
unsigned int tx_message_count = 0;
unsigned int crc_error_count = 0;
unsigned int short_sof_count = 0;
unsigned int bus_contention_count = 0;
unsigned int download_underrun_count = 0;

void send_stats(){
  Serial.write(rx_message_count >> 8);
  Serial.write(rx_message_count & 0x00FF);
  Serial.write(tx_message_count >> 8);
  Serial.write(tx_message_count & 0x00FF);
  Serial.write(crc_error_count >> 8);
  Serial.write(crc_error_count & 0x00FF);
  Serial.write(short_sof_count >> 8);
  Serial.write(short_sof_count & 0x00FF);
  Serial.write(bus_contention_count >> 8);
  Serial.write(bus_contention_count & 0x00FF);
  Serial.write(download_underrun_count >> 8);
  Serial.write(download_underrun_count & 0x00FF);
}

unsigned long download_start_time;

bool bus_contention=false;
#define LONG_PULSE true
#define SHORT_PULSE false

#define RX_PIN 7
#define TX_PIN 8
#define CTS_PIN 10
#define TRIGGER_PIN 11

byte lo_bit_times[128];
byte hi_bit_times[128];
byte bt_len=0;

byte rx_buffer[32];
byte rx_buf_len=0;
byte tx_buffer[32];
byte tx_buffer_len=0;

byte sof_length;

void upload_data(){
  Serial.write(rx_buf_len-1); // count
  for (int i = 0; i < (rx_buf_len - 1); i++){
    Serial.write(rx_buffer[i]);
  }
}

void output_data()
{
    // RAW DEBUG:
    // for (int i = 0; i < (rx_buf_len - 1); i++)
    // {
    //   Serial.print("0x");
    //   Serial.print(rx_buffer[i] < 16 ? "0" : "");
    //   Serial.print(rx_buffer[i], HEX);
    //   Serial.print(" ");
    // }
    // Serial.println();


  byte header = rx_buffer[0];
  byte src = rx_buffer[2];
  if ((src & 0xF0) == 0x80) {
  //if (true) {
    Serial.print("H:0x");
    Serial.print(header < 16 ? "0" : "");
    Serial.print(header, HEX);
    byte priority = (header & 0xE0) >> 5;
    Serial.print("|P");
    Serial.print(priority, HEX);
    if (header & 0x10)
    {
      Serial.print("|H1");
    }
    else
    {
      Serial.print("|H3");
    }
    if (header & 0x04)
    {
      Serial.print("|PA");
    }
    else
    {
      Serial.print("|FA");
    }
    Serial.print("|Z:0x");
    Serial.print(header & 0x03, HEX);
    Serial.print("|dst:0x");
    Serial.print(rx_buffer[1], HEX);
    Serial.print("|src:0x");
    Serial.print(rx_buffer[2], HEX);

    Serial.print("|Data: ");
    for (int i = 3; i < (rx_buf_len - 1); i++)
    {
      Serial.print("0x");
      Serial.print(rx_buffer[i] < 16 ? "0" : "");
      Serial.print(rx_buffer[i], HEX);
      Serial.print(" ");
    }

    if (sof_length < 150)
    {
      Serial.print(" (Short SOF: ");
      Serial.print(sof_length, DEC);
      Serial.print(" uSec)");
    }
    byte crc = fast_crc(rx_buffer, rx_buf_len - 1);
    if (crc != rx_buffer[rx_buf_len - 1])
    {
      crc_error_count++;
      Serial.print(" (**** Bad CRC!! *** ");
      Serial.print(crc_error_count);
      Serial.print(" so far) ");
    }
    Serial.println();
  }

  // for(int i=0; i<bt_len; i++){
  //   Serial.print(lo_bit_times[i], DEC);
  //   Serial.print("   ");
  //   Serial.print(hi_bit_times[i], DEC);
  //   Serial.println();
  // }
}

void rx_msg(){
  byte rx_byte = 0x00;
  byte rx_bit = 0x80;
  unsigned long sof_start;
  unsigned long bit_start;
  byte duration;
  bool eof = false;

  bt_len = 0;
  rx_buf_len = 0;
  digitalWrite(LED_BUILTIN, HIGH);

  sof_start=micros();
  while (digitalRead(RX_PIN) == HIGH)
  { // SOF
  }
  sof_length = micros() - sof_start;
  if (sof_length < 150)
  {
    short_sof_count++;
  }

  while (!eof) {
    digitalWrite(LED_BUILTIN, LOW);
    bit_start = micros();
    while (digitalRead(RX_PIN) == LOW) { // passive bit
      if ((micros() - bit_start) > 200){
        eof = true;
        return;
      }
    }
    // end of passive bit
    duration = micros() - bit_start;
    // lo_bit_times[bt_len] = duration;
    if (duration > 80){
      // long passive = 1
      rx_byte |= rx_bit;
    }
    rx_bit = rx_bit >> 1;

    digitalWrite(LED_BUILTIN, HIGH);
    bit_start = micros();
    while (digitalRead(RX_PIN) == HIGH) {                                      // active_bit
      if ((micros() - bit_start) > 1000){  // RESET
        // Serial.println("RESET");
        rx_buf_len=0;
        return;
      }
    }
    // end of active bit
    duration = micros() - bit_start;
    // hi_bit_times[bt_len++] = duration;
    if (duration < 80) {
      // short active = 1
      rx_byte |= rx_bit;
    }
    rx_bit = rx_bit >> 1;
    if (rx_bit == 0){
      rx_buffer[rx_buf_len++] = rx_byte;
      rx_byte = 0x00;
      rx_bit = 0x80;
    }
  }
}


void bus_high(bool long_pulse){
  digitalWrite(TX_PIN, HIGH);
  if (long_pulse) {
    delayMicroseconds(124);
  } else {
    delayMicroseconds(60);
  }
}

void bus_low(bool long_pulse){
  digitalWrite(TX_PIN, LOW);
  unsigned long low_start = micros();
  if (long_pulse){
    while ((micros() - low_start) < 116){
      if (digitalRead(RX_PIN) == HIGH) {
        bus_contention_count++;
        bus_contention = true;
        break;
      }
    }
  }else{
    while ((micros() - low_start) < 56){
      if (digitalRead(RX_PIN) == HIGH) {
        bus_contention_count++;
        bus_contention = true;
        break;
      }
    }
  }
}

void send_msg(){
  byte msg_len = tx_buffer[0];
  byte *msg = &tx_buffer[1];
  byte contention_retries = 0;


  msg[msg_len] = fast_crc(msg, msg_len);
  msg_len++;

  do {
    bus_contention = false;

    bus_high(LONG_PULSE); // SOF ~= 200 uS
    bus_high(SHORT_PULSE); // Long + Short = 128+64 = 192

    for (byte b=0; b<msg_len; b++){
      byte mask = 0x80;
      for (byte d=0; d<4; d++){
        if (msg[b] & mask){
          bus_low(LONG_PULSE);
        }else{
          bus_low(SHORT_PULSE);
        }

        if (bus_contention){
          break;
        }

        mask = mask >> 1;

        if (msg[b] & mask){
          bus_high(SHORT_PULSE);
        }else{
          bus_high(LONG_PULSE);
        }
        mask = mask >> 1;
      }
      if (bus_contention){
        break;
      }
    }
  } while (bus_contention & contention_retries++ < 2);

  bus_low(LONG_PULSE);  // EOF
  bus_low(LONG_PULSE);
  tx_buffer_len = 0;
}

void setup(){
  pinMode(CTS_PIN, OUTPUT);
  pinMode(TX_PIN, OUTPUT);
  pinMode(RX_PIN, INPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  Serial.begin(115200);
  // Serial.println("Hello.  J1850 Serial Receive.");
  digitalWrite(CTS_PIN, HIGH);
}

byte msg[7] = {0x05, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x00};

void loop() {
  // tx_buffer_len = 6;
  // memcpy(tx_buffer, msg, 6);
  // send_msg();
  // delay(5000);

  if (digitalRead(RX_PIN) == HIGH) {
    digitalWrite(CTS_PIN, LOW);
    rx_msg();
    rx_message_count++;
    digitalWrite(CTS_PIN, HIGH);
    byte crc = fast_crc(rx_buffer, rx_buf_len - 1);
    if (crc == rx_buffer[rx_buf_len - 1]){
      upload_data();
    }
    else{
      digitalWrite(TRIGGER_PIN, HIGH);
      digitalWrite(TRIGGER_PIN, LOW);
      crc_error_count++;
    }
  }


  if (Serial.available()){
    if (Serial.peek() == 0x66){
      Serial.read();
      send_stats();   
    } else {
      if (tx_buffer_len == 0){
        download_start_time = millis();
      }
      while(Serial.available()){
        tx_buffer[tx_buffer_len++] = Serial.read();
        if (tx_buffer_len == tx_buffer[0] + 1) {
          digitalWrite(CTS_PIN, LOW);
          send_msg();
          tx_message_count++;
          digitalWrite(CTS_PIN, HIGH);
        }
      }
    }
  }

  if (tx_buffer_len > 0){
    if ((millis() - download_start_time) > 200){
      download_underrun_count ++;
      tx_buffer_len = 0;
    }
  }
}
