#include "nRF24L01.H"
#include "Arduino.H"
#include <SPI.h>

void NRF_Config(void){
  Serial.begin(115200);
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  
  pinMode(CSN, OUTPUT);
  pinMode(CE, OUTPUT);
  digitalWrite(CSN, HIGH);
  digitalWrite(CE, LOW);
  
  digitalWrite(CSN, LOW);
  SPI.transfer(CONFIG|W_REGISTER);
  SPI.transfer(0b01111111); //disable mask interupt, 2 byte crc 
  digitalWrite(CSN, HIGH); //power up, PRX
  
  digitalWrite(CSN, LOW);
  SPI.transfer(EN_AA|W_REGISTER);
  SPI.transfer(0b00000000); //disable auto acknowledgment
  digitalWrite(CSN, HIGH);
  
  digitalWrite(CSN, LOW);
  SPI.transfer(EN_RXADDR|W_REGISTER);
  SPI.transfer(0b00000001); //enable data pipe 0
  digitalWrite(CSN, HIGH);
  
  digitalWrite(CSN, LOW);
  SPI.transfer(SETUP_AW|W_REGISTER); 
  SPI.transfer(0b00000001); //3 byte adress width
  digitalWrite(CSN, HIGH);
  
  digitalWrite(CSN, LOW);
  SPI.transfer(SETUP_RETR|W_REGISTER); 
  SPI.transfer(0b00000000); //disable automatic retransmission
  digitalWrite(CSN, HIGH);
  
  digitalWrite(CSN, LOW);
  SPI.transfer(0x05|W_REGISTER);
  SPI.transfer(RF_CH); //set rf channel
  digitalWrite(CSN, HIGH);
  
  digitalWrite(CSN, LOW);
  SPI.transfer(0x06|W_REGISTER);
  SPI.transfer(0b00000110);//rf setup
  digitalWrite(CSN, HIGH);
  
  digitalWrite(CSN, LOW);
  SPI.transfer(0x0A|W_REGISTER); //data pipe 0 rx adress
  SPI.transfer(0x56);//lsb first
  SPI.transfer(0x34);
  SPI.transfer(0x12);
  digitalWrite(CSN, HIGH);
  
  digitalWrite(CSN, LOW);
  SPI.transfer(0x11|W_REGISTER);
  SPI.transfer(0b00000100); //4 bytes in rx payload pipe 0
  digitalWrite(CSN, HIGH);
}

boolean NRF_RX_read(byte *data){ //todo: is this correct pointer syntax?
  byte NRFstatus;
  boolean result;
  
  digitalWrite(CE, LOW); //disable rx during check
  
  digitalWrite(CSN, LOW);
  NRFstatus=SPI.transfer(NOP);
  digitalWrite(CSN, HIGH);
  
  if (NRFstatus&0b00001110){ //no data in rx register
    result=false;
  }else{
    digitalWrite(CSN, LOW);
    NRFstatus=SPI.transfer(R_RX_PAYLOAD); //read LSByte first
    *(data+3)=SPI.transfer(0x00);
    *(data+2)=SPI.transfer(0x00);
    *(data+1)=SPI.transfer(0x00);
    *data=SPI.transfer(0x00);
    digitalWrite(CSN, HIGH);
    
    digitalWrite(CSN, LOW);
    SPI.transfer(FLUSH_RX);
    digitalWrite(CSN, HIGH);
    result=true;
    
    digitalWrite(CE, HIGH); //re-enable RX
  }
  
  digitalWrite(CSN, LOW);
  NRFstatus=SPI.transfer(FLUSH_RX);//clear any extra data
  digitalWrite(CSN, HIGH);
  
  digitalWrite(CE, HIGH); //re-enable receive
  return result;
}

void NRF_RX_enable(void){
  digitalWrite(CE, HIGH);
}

void NRF_RX_disable(void){
  digitalWrite(CE, LOW);
}

void NRF_TX(byte address[], byte data){
  digitalWrite(CE, LOW); //disable
  //enable TX
  digitalWrite(CSN, LOW);
  SPI.transfer(CONFIG|W_REGISTER);
  SPI.transfer(0b01111110); //disable mask interupt, 2 byte crc 
  digitalWrite(CSN, HIGH); //power up, TRX
  
  digitalWrite(CSN, LOW);
  SPI.transfer(TX_ADDR|W_REGISTER);
  SPI.transfer(address[2]); //set tx adress
  SPI.transfer(address[1]); //set tx adress
  SPI.transfer(address[0]); //set tx adress
  digitalWrite(CSN, HIGH);
  
  digitalWrite(CSN, LOW); 
  SPI.transfer(W_TX_PAYLOAD);  //send tx payload
  SPI.transfer(data);
  digitalWrite(CSN, HIGH);
  
  digitalWrite(CE, HIGH);//pulse CE line to TX
  delay(1);
  digitalWrite(CE, LOW);
  
  //restore RX configuration
  digitalWrite(CSN, LOW);
  SPI.transfer(CONFIG|W_REGISTER);
  SPI.transfer(0b01111111); //disable mask interupt, 2 byte crc 
  digitalWrite(CSN, HIGH); //power up, PRX
}

void NRF_RF_Channel(byte rf_channel){
  digitalWrite(CSN, LOW);
  SPI.transfer(RF_CH|W_REGISTER);
  SPI.transfer(rf_channel); //disable mask interupt, 2 byte crc 
  digitalWrite(CSN, HIGH); //power up, PRX
  
}

void NRF_Status(void){
  Serial.print(SPI.transfer(NOP));
  Serial.print('\n');
}