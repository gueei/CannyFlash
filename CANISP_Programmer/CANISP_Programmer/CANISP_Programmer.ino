#include <SPI.h>
#include <mcp2515.h>
#define MP_PIN  3
#define LED_PIN 13

struct can_frame msg, rxmsg;
uint8_t buf[8];
uint8_t bufTail = 0;
MCP2515 mcp2515(10);

char magicPacket[] = "CANNYv1!";
char password[] = "12345678";

void setup() {
  pinMode(MP_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  // while (!Serial);
  Serial.begin(57600);
  
  while(mcp2515.reset()!= MCP2515::ERROR_OK){
    Serial.println("Error in reset");
    delay(1000);
  }
  mcp2515.setBitrate(CAN_500KBPS, MCP_16MHZ);
  mcp2515.setNormalMode();
}

void waitMagicPacket(){
  if (mcp2515.readMessage(&rxmsg) == MCP2515::ERROR_OK) {
    bool correct = false;
    if (rxmsg.can_dlc == 8){
      correct = true;
      for (int i=0; i<8; i++){
        if (rxmsg.data[i] != magicPacket[i]) correct = false;
      }
    }
    if (correct){
      for(int i=0; i<8; i++){
        cansend(password[i]);
      }
      canFlush();
      serial_flush_buffer();
    }
  }
}

void serial_flush_buffer()
{
  while (Serial.read() >= 0)
   ; // do nothing
}

void loop() {
  if (digitalRead(MP_PIN)==LOW){
    waitMagicPacket();
    digitalWrite(LED_PIN, HIGH);
  }else{
    digitalWrite(LED_PIN, LOW);
  }
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
