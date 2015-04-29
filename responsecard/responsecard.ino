#include <nRF24L01.h>
#include <SPI.h>

#define CE 9
#define CSN 10
#define CHN 40
#define PACKET_SIZE 4
#define MAX_SIZE 512

uint8_t master_MAC[3] = {0x56, 0x34, 0x12};
uint8_t my_MAC[3] = {0x4E, 0x03, 0xAD};

uint8_t rx_packet[4];  //3 bytes MAC addr, 1 byte answer
uint8_t tx_packet[4];  //3 bytes MAC addr, 1 byte answer

void setup(){
  /* Setup serial */
  Serial.begin(9600);
  
  /* Set pins */
  pinMode(CE, OUTPUT);
  pinMode(CSN, OUTPUT);
  
  /* Setup SPI */
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setDataMode(SPI_MODE0);

  /* Setup nRF24L01 */
  init_rx(); 
  delay(1000);
  read_all_registers();
}

uint8_t raw_packet[4];
uint8_t prev_packet[4];
uint8_t status;

uint16_t answers[10];
uint8_t answers_idx = 0;

void loop(){
  get_status(&status);
  
  /* If RX pipe contains data */
  if(status < 0x07){
    get_data(raw_packet);
    
    /* If packet not a repeat */
    if(memcmp(prev_packet, raw_packet, PACKET_SIZE)){
      //print_packet(raw_packet);
      add_answer(answers, raw_packet);
      print_answers(answers);
    }
      
    memcpy(prev_packet, raw_packet, PACKET_SIZE);
  }

  delay(50);
}

void read_all_registers(){
  uint8_t buffer[10];
  int tmp;
    
  Serial.print("Registers: \n");
  for(uint8_t i = 0; i < 10; i++){
    rf24_read(R_REGISTER | i, buffer, 1);
    Serial.print(i, HEX);
    Serial.print(",");
    tmp = buffer[0];
    Serial.println(tmp, BIN);
  }
}

void get_status(uint8_t *status){
  *status = rf24_read(0, 0);
  *status &= 0x0C;   //Mask RX_P_NO bits
}

void get_data(uint8_t *raw_packet){
  uint8_t buffer[PACKET_SIZE];
  
  rf24_read(R_RX_PAYLOAD, buffer, PACKET_SIZE);
  rf24_write(FLUSH_RX);
  
  memcpy(raw_packet, buffer, PACKET_SIZE);
}

void print_packet(uint8_t *packet){
  Serial.print("[MAC ADDR: ");
  for(uint8_t i = 0; i < sizeof(packet) - 1; i++){
    Serial.print(packet[i], HEX);
    Serial.print(" ");
  }
  
  Serial.print("] ");
  Serial.print("[ANSWER: ");
  Serial.print((packet[3] - 97));
  Serial.print("]\n");
}

void add_answer(uint16_t *answers, uint8_t *packet){
  uint8_t pos = packet[3] - 98;
  answers[pos] += 1;
}

void print_answers(uint16_t *answers){
  Serial.print("[ANSWERS]\n");
  for(uint8_t i = 0; i < 10; i++){
    Serial.print(i + 1);
    Serial.print(":\t");
    Serial.print(answers[i]);
    Serial.print("\n");
  }
  Serial.print("\n");
}

/* --- */

/* Init RF24 to receive */
void init_rx(){
  digitalWrite(CE, LOW); //Select device
  rf24_write(W_REGISTER | CONFIG, 0x0A);      //Power on, enable CRC
  delay(5);
  rf24_write(W_REGISTER | CONFIG, 0x3F);      //2-byte CRC, 0 to jam
  rf24_write(W_REGISTER | EN_RXADDR, 0x01);   //Receive on pipe 1
  rf24_write(W_REGISTER | RX_PW_P0, 0x04);    //4-byte addr width
  rf24_write(CONFIG, 0x00);                   //Unset prim rx for recv
  rf24_write(W_REGISTER | EN_AA, 0x00);       //Disable auto-ack
  rf24_write(W_REGISTER | RF_CH, CHN);        //Set channel
  rf24_write(W_REGISTER | SETUP_AW, 0x01);    //3-byte MAC addr
  rf24_write(W_REGISTER | RF_SETUP, 0x06);    //1MBPS data rate, high pwr
  
  rf24_write(FLUSH_RX);                       //Clear receiver buffer
  rf24_write(W_REGISTER | STATUS, 0x70);      //Clear interrupts
  rf24_write(W_REGISTER | RX_ADDR_P0, master_MAC, 3);  //Set MAC to listen for
  digitalWrite(CE, HIGH); //Deselect device
}

/* Init RF24 to transmit */
void init_tx(){
  
}
    
/* --- */

uint8_t rf24_read(uint8_t cmd, uint8_t* result, uint8_t length){
  uint8_t status;
  
  digitalWrite(CSN, LOW);  //Select device
  
  status = SPI.transfer(cmd);
  for(int i = 0; i < length; i++){
    result[i] = SPI.transfer(0);
  }

  digitalWrite(CSN, HIGH); //Deselect device
  return status;
}

uint8_t rf24_read(uint8_t cmd, uint8_t result){
  uint8_t status;
  
  digitalWrite(CSN, LOW);  //Select device
  
  status = SPI.transfer(cmd);
  result = SPI.transfer(0);

  digitalWrite(CSN, HIGH); //Deselect device
  return status;
}

uint8_t rf24_write(uint8_t cmd, uint8_t* value, uint8_t length){
  uint8_t status;
  
  digitalWrite(CSN, LOW);  //Select device

  status = SPI.transfer(cmd);
  for(int i = 0; i < length; i++){
    SPI.transfer(value[i]);
  }

  digitalWrite(CSN, HIGH);  //Deselect device
  return status;
}

uint8_t rf24_write(uint8_t cmd, uint8_t value){
  uint8_t status;
  
  digitalWrite(CSN, LOW);  //Select device
  
  status = SPI.transfer(cmd);
  SPI.transfer(value);

  digitalWrite(CSN, HIGH);  //Deselect device
  return status;
}

uint8_t rf24_write(uint8_t cmd){
  uint8_t status;
  
  digitalWrite(CSN, LOW);  //Select device
  
  status = SPI.transfer(cmd);

  digitalWrite(CSN, HIGH);  //Deselect device
  return status;
}