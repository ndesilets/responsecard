#include <nRF24L01.h>
#include <SPI.h>

#define CE 9
#define CSN 10
#define CHN 40
#define MAX_SIZE 512

uint8_t master_MAC[3] = {0x56, 0x34, 0x12};
uint8_t my_MAC[3] = {0x4E, 0x03, 0xAD};

uint8_t rx_packet[4];  //3 bytes MAC addr, 1 byte answer
uint8_t tx_packet[4];  //3 bytes MAC addr, 1 byte answer

struct{
  uint8_t MAC_addr[3];
  uint8_t data[1];
} transmission[MAX_SIZE];

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
  //init_RF24(); 
}

void loop(){
  init_rx(); 
  delay(1000);
  
  read_all_registers();
  delay(1000);
  
  while(1){
    record_data();
  }
}

uint8_t rf24_SPI_read(uint8_t cmd, uint8_t* result, uint8_t length){
  uint8_t status;
  
  digitalWrite(CSN, LOW);  //Select device
  
  status = SPI.transfer(cmd);
  for(int i = 0; i < length; i++){
    result[i] = SPI.transfer(0);
  }

  digitalWrite(CSN, HIGH); //Deselect device
  return status;
}

uint8_t rf24_SPI_read(uint8_t cmd, uint8_t result){
  uint8_t status;
  
  digitalWrite(CSN, LOW);  //Select device
  
  status = SPI.transfer(cmd);
  result = SPI.transfer(0);

  digitalWrite(CSN, HIGH); //Deselect device
  return status;
}

uint8_t rf24_SPI_write(uint8_t cmd, uint8_t* value, uint8_t length){
  uint8_t status;
  
  digitalWrite(CSN, LOW);  //Select device

  status = SPI.transfer(cmd);
  for(int i = 0; i < length; i++){
    SPI.transfer(value[i]);
  }

  digitalWrite(CSN, HIGH);  //Deselect device
  return status;
}

uint8_t rf24_SPI_write(uint8_t cmd, uint8_t value){
  uint8_t status;
  
  digitalWrite(CSN, LOW);  //Select device
  
  status = SPI.transfer(cmd);
  SPI.transfer(value);

  digitalWrite(CSN, HIGH);  //Deselect device
  return status;
}

uint8_t rf24_SPI_write(uint8_t cmd){
  uint8_t status;
  
  digitalWrite(CSN, LOW);  //Select device
  
  status = SPI.transfer(cmd);

  digitalWrite(CSN, HIGH);  //Deselect device
  return status;
}

/* Init RF24 to read data */
void init_rx(){
  digitalWrite(CE, LOW); //Select device
  rf24_SPI_write(W_REGISTER | CONFIG, 0x0A);  //Power on
  delay(5);
  rf24_SPI_write(W_REGISTER | CONFIG, 0x3F);  //2-byte CRC, 0 to jam
  rf24_SPI_write(W_REGISTER | EN_RXADDR, 0x01);  //Receive on pipe 1
  rf24_SPI_write(W_REGISTER | RX_PW_P0, 0x04);  //MAC(3) + answer(1)
  rf24_SPI_write(CONFIG, 0x00);                                 //Unset prim rx for recv
  rf24_SPI_write(W_REGISTER | EN_AA, 0x00);   //Disable auto-ack
  rf24_SPI_write(W_REGISTER | RF_CH, CHN);    //Set Channel
  rf24_SPI_write(W_REGISTER | SETUP_AW, 0x01);  //3-byte MAC
  rf24_SPI_write(W_REGISTER | RF_SETUP, 0x06);  //1MBPS data rate, high pwr
  
  rf24_SPI_write(FLUSH_RX);                                        //Clear receiver buffer
  rf24_SPI_write(W_REGISTER | STATUS, 0x70);    //Clear interrupts
  rf24_SPI_write(W_REGISTER | RX_ADDR_P0, master_MAC, 3);  //Set MAC to listen for
  digitalWrite(CE, HIGH); //Deselect device
}

void init_tx(){
  
}

void read_all_registers(){
  byte readByte[10];
  int tmp;
    
  Serial.print("Registers: \n");
  for(int i = 0; i < 10; i++){
    rf24_SPI_read(R_REGISTER | i, readByte, 1);
    Serial.print(i, HEX);
    Serial.print(",");
    tmp = readByte[0];
    Serial.println(tmp, BIN);
  }
}

void record_data(){
  uint8_t buffer[4];
  
  rf24_SPI_read(R_RX_PAYLOAD, buffer, 4);
  rf24_SPI_write(W_REGISTER | (REGISTER_MASK & STATUS), 0x70); //clr interrupts
  for(int i = 0; i < 4; i++){
    Serial.print(buffer[i]);
    Serial.print(", ");
  }
  
  Serial.println();
}
    
