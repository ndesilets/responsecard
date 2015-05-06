#include <nRF24L01.h>
#include <SPI.h>

#define CE 9
#define CSN 10
#define CHANNEL 69
#define PACKET_SIZE 4
#define MAX_SIZE 512

uint8_t **packet_array;			//Rows: MAX_SIZE, Cols: PACKET_SIZE
uint16_t packets_idx = 0;
uint8_t raw_packet[PACKET_SIZE];

uint16_t answers[10];
uint8_t answers_idx = 0;

uint8_t rx_status;

void setup(){
	/* Setup serial */
	Serial.begin(9600);		//Baud rate does not matter w/ Teensy 3.1
  
	/* Set pins */
	pinMode(CE, OUTPUT);
	pinMode(CSN, OUTPUT);
  
	/* Setup SPI */
	SPI.begin();
	SPI.setBitOrder(MSBFIRST);
	SPI.setClockDivider(SPI_CLOCK_DIV2);
	SPI.setDataMode(SPI_MODE0);
	
	/* Init packet array 
	 * Dynamic allocation is not a good idea but fuck it yolo */
	packet_array = new uint8_t *[MAX_SIZE];
	for(uint16_t i = 0; i < MAX_SIZE; i++){
		packet_array[i] = new uint8_t[PACKET_SIZE];
	}

	/* Setup nRF24L01 */
	init_rx(); 
	delay(1000);
	read_registers();
}

void loop(){
	/* Get status of rx pipe */
	get_rx_status(&rx_status);
  
	/* If RX pipe contains data */
	if(rx_status < 0x07){
		/* Get raw packet data */
		get_packet(raw_packet);
		
		/* Check if raw packet is unique */
		if(packet_unique(packet_array, raw_packet)){
			/* Display raw packet info + packets_idx */
			Serial.println();
			print_packet(raw_packet);
			Serial.print("PCK IDX: ");
			Serial.print(packets_idx);
			Serial.println();
			/* Add packet to array + add answer */
			add_packet(packet_array, raw_packet);
			add_answer(answers, raw_packet);
			print_answers(answers);
		} 
	}	
	
	/* This delay can probably be removed */
	delay(10);
}

/* Get status of rx pipe */
void get_rx_status(uint8_t *rx_status){
	*rx_status = rf24_read(0, 0);
	*rx_status &= 0x0C;   //Mask RX_P_NO bits
}

/* Get packet from rx pipe */
void get_packet(uint8_t *packet){
	rf24_read(R_RX_PAYLOAD, packet, PACKET_SIZE);
	rf24_write(FLUSH_RX);
}

/* Check if packet is unique, return 1 if true, 0 if false */
uint8_t packet_unique(uint8_t **packet_array, uint8_t *packet){
	for(uint16_t i = 0; i < packets_idx; i++){
		if(!memcmp(packet_array[i], packet, PACKET_SIZE)){
			return 0;
		}
	}
	
	return 1;
}

/* Add packet to packet array */
void add_packet(uint8_t **packet_array, uint8_t *packet){
	for(uint16_t i = 0; i < 3; i++){
		packet_array[packets_idx][i] = packet[i];
	}
	
	packet_array[packets_idx][3] = packet[3];
	
	packets_idx++;
}

/* Print packet details */
void print_packet(uint8_t *packet){
	Serial.print("[MAC ADDR: ");
	for(uint8_t i = 0; i < PACKET_SIZE - 1; i++){
		Serial.print(packet[i], HEX);
		Serial.print(" ");
	}
  
	Serial.print("] ");
	Serial.print("[ANSWER: ");
	Serial.print((packet[3] - 97)); //Subtract by 'A'
	Serial.print("]\n");
}

/* Add answer to packet array */
void add_answer(uint16_t *answers, uint8_t *packet){
	uint8_t idx = packet[PACKET_SIZE - 1] - 98;
	answers[idx]++;
}

/* Print list of all answers w/ frequency */
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

/* --- Code below is adapted from Taylor Killian --- */

/* Read status of R_REGISTERS */
void read_registers(){
 	uint8_t buffer[10];
    
 	Serial.print("Registers: \n");
 	for(uint8_t i = 0; i < 10; i++){
		rf24_read(R_REGISTER | i, buffer, 1);
		Serial.print(i, HEX);
		Serial.print(",");
		Serial.println(buffer[0], BIN);
	}
}

/* Init RF24 to receive */
void init_rx(){
	digitalWrite(CE, LOW); //Select device
  
	rf24_write(W_REGISTER | CONFIG, 0x0A);      //Power on, enable CRC
	delay(5);                                   //Wait for rf24 to power up
	rf24_write(W_REGISTER | CONFIG, 0x3F);      //2-byte CRC, 0 to jam
	rf24_write(W_REGISTER | EN_RXADDR, 0x01);   //Receive on pipe 1
	rf24_write(W_REGISTER | RX_PW_P0, 0x04);    //4-byte packet size
	rf24_write(CONFIG, 0x00);                   //Unset prim rx for recv
	rf24_write(W_REGISTER | EN_AA, 0x00);       //Disable auto-ack
	rf24_write(W_REGISTER | RF_CH, CHANNEL);    //Set channel
	rf24_write(W_REGISTER | SETUP_AW, 0x01);    //3-byte MAC addr
	rf24_write(W_REGISTER | RF_SETUP, 0x06);    //1MBPS data rate, high pwr
	rf24_write(FLUSH_RX);                       //Clear receiver buffer
  
	digitalWrite(CE, HIGH); //Deselect device
}

/* --- nRF24L01 SPI r/w functions --- */

void rf24_read(uint8_t cmd, uint8_t* result, uint8_t length){
	digitalWrite(CSN, LOW);  //Select device
  
	SPI.transfer(cmd);
	for(uint8_t i = 0; i < length; i++){
		result[i] = SPI.transfer(0);
	}

	digitalWrite(CSN, HIGH); //Deselect device
}

void rf24_read(uint8_t cmd, uint8_t result){
	digitalWrite(CSN, LOW);  //Select device
  
	SPI.transfer(cmd);
	result = SPI.transfer(0);

	digitalWrite(CSN, HIGH); //Deselect device
}

void rf24_write(uint8_t cmd, uint8_t* val, uint8_t length){
	digitalWrite(CSN, LOW);  //Select device

	SPI.transfer(cmd);
	for(uint8_t i = 0; i < length; i++){
		SPI.transfer(val[i]);
	}

	digitalWrite(CSN, HIGH);  //Deselect device
}

void rf24_write(uint8_t cmd, uint8_t val){
	digitalWrite(CSN, LOW);  //Select device
  
	SPI.transfer(cmd);
	SPI.transfer(val);

	digitalWrite(CSN, HIGH);  //Deselect device
}

void rf24_write(uint8_t cmd){
	digitalWrite(CSN, LOW);  //Select device
  
	SPI.transfer(cmd);

	digitalWrite(CSN, HIGH);  //Deselect device
}
