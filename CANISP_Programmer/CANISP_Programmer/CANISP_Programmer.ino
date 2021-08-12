#include <SPI.h>
#include <mcp2515.h>

#define SHORT 167, 333
#define LONG  333, 167

#define MP_PIN  7
#define LED_PIN 13

struct can_frame msg, rxmsg;
uint8_t buf[8];
uint8_t bufTail = 0;
MCP2515 mcp2515(10);

char magicPacket[] = "CANNYv1!";
char password[] = "12345678";

int pat_enter_packet[] = {LONG, LONG, LONG};
// Moose code AC
int pat_mp_accepted[] = {SHORT, LONG, LONG, SHORT, LONG, SHORT};

int pat_error[] = {SHORT, LONG};

int state = 0;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  
  while(mcp2515.reset()!= MCP2515::ERROR_OK){
    blinkPattern(pat_error, 2);
  }
  mcp2515.setBitrate(CAN_500KBPS, MCP_16MHZ);
  mcp2515.setNormalMode();

  //blinkPattern(pat_enter_packet, 6);
  digitalWrite(LED_PIN, HIGH);
}

void loop() {
  if (state==0)
    state_magicPacket();
  else
    state_isp();
}

void state_magicPacket(){
  bool mp = waitMagicPacket();
  if (mp){
    for(int i=0; i<8; i++){
      cansend(password[i]);
    }
    canFlush();
    serial_flush_buffer();
    digitalWrite(LED_PIN, LOW);
    blinkPattern(pat_mp_accepted, 12);
    state = 1;
    while(!Serial){}
    Serial.begin(19200);
  }
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  delay(100);
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  delay(100);
}

bool waitMagicPacket(){
  if (mcp2515.readMessage(&rxmsg) == MCP2515::ERROR_OK) {
    bool correct = false;
    if (rxmsg.can_dlc == 8){
      canFlush();
      correct = true;
      for (int i=0; i<8; i++){
        if (rxmsg.data[i] != magicPacket[i]) correct = false;
      }
    }
    if (correct){
      return true;
    }
  }
  return false;
}

void serial_flush_buffer()
{
  while (Serial.read() >= 0)
   ; // do nothing
}

void state_isp(){
  if (Serial.available()){
    uint8_t data = Serial.read();
    cansend(data);
  }
  while (mcp2515.readMessage(&rxmsg) == MCP2515::ERROR_OK) {
    Serial.write(rxmsg.data, rxmsg.can_dlc);
  }
}


void cansend(uint8_t c){
  buf[bufTail] = c;
  bufTail++;
  if ((bufTail>=8)||c==0x20){
    canFlush();
  }
}

void canFlush(){
  if (bufTail==0) return;
  msg.can_id = 0x1CF;
  msg.can_dlc = bufTail;
  for(int i=0;i<bufTail; i++){
    msg.data[i] = buf[i];
  }
  mcp2515.sendMessage(&msg);
  bufTail = 0;
}

void blinkPattern(int pat[], int len){
  for (int i=0; i<len; i++){
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(pat[i]);
  }
}
