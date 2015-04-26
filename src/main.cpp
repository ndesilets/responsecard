#include <Arduino.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

const uint8_t num_channels = 128;
uint8_t values[num_channels];

byte rx_packet[4];  //3 bytes MAC addr, 1 byte answer
byte tx_packet[4];  //3 bytes MAC addr, 1 byte answer
RF24 radio(9, 10);

void setup(){
    /* Setup serial */
    Serial.begin(9600);
    Serial.print("[BEGIN]\n");
    
    /* Setup nRF24L01 */
    //radio.setCRCLength(RF24_CRC_2);
    digitalWrite(10, LOW);
    SPI.transfer(CONFIG|W_REGISTER);
    SPI.transfer(0b01111111); //disable mask interupt, 2 byte crc 
    digitalWrite(10, HIGH); //power up, PRX
    radio.setPALevel(RF24_PA_LOW);
    radio.setAutoAck(false);
    radio.setAddressWidth(3);
    radio.setDataRate(RF24_1MBPS);
    radio.begin();
    radio.startListening();
    radio.stopListening();          //Go into standby mode
    
    /* Test */
    // Print out header, high then low digit
  int i = 0;
  while ( i < num_channels )
  {
    printf("%x",i>>4);
    ++i;
  }
  printf("\n\r");
  i = 0;
  while ( i < num_channels )
  {
    printf("%x",i&0xf);
    ++i;
  }
  printf("\n\r");
}

/* 4 byte packet: 3 bytes addr, 1 byte answer */

const int num_reps = 100;


void loop(){
// Clear measurement values
  memset(values,0,sizeof(values));

  // Scan all channels num_reps times
  int rep_counter = num_reps;
  while (rep_counter--)
  {
    int i = num_channels;
    while (i--)
    {
      // Select this channel
      radio.setChannel(i);

      // Listen for a little
      radio.startListening();
      delayMicroseconds(225);
      

      // Did we get a carrier?
      if ( radio.testCarrier() ){
        ++values[i];
      }
      radio.stopListening();
    }
  }

  // Print out channel measurements, clamped to a single hex digit
  int i = 0;
  while ( i < num_channels )
  {
    printf("%x",min(0xf,values[i]&0xf));
    ++i;
  }
  Serial.print("SHIT!\n\r");
}