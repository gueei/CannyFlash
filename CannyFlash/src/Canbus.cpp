/*
 * Canbus.cpp
 *
 * Created: 10/19/2019 11:22:04 PM
 *  Author: andy
 */ 

#include "Canbus.h"
#include "SPI.h"

void Canbus::init(){
	// CS Pin OUTPUT
	MCP_CS_DDR	|= _BV(MCP_CS_PIN);
	
	SELECT;
	SPI::send(MCP_RESET);
	DESELECT;

	// Only possible to change bitrate in config mode
	setRegister(MCP_CANCTRL, MODE_CONFIG);
	setRegister(MCP_CNF1, MCP_CNF1_CFG);
	setRegister(MCP_CNF2, MCP_CNF2_CFG);
	setRegister(MCP_CNF3, MCP_CNF3_CFG);

	for(uint8_t i=0; i<3; i++){
		SELECT;
		SPI::send(MCP_WRITE);
		SPI::send(MCP_TXB0CTRL + (i*16) );
		for(uint8_t j=MCP_TXB0CTRL + (i*16); j<14; j++){
			SPI::send(0);
		}
		DESELECT;
	}
	
	// RX buffer 0 enable filter 0
	setRegister(MCP_RXB0CTRL, 0);
	
	// Filter and mask 0, accept only Canny Flash V1
	setRegister(MCP_RXF0SIDH, ACCEPT_ID_SIDH);
	setRegister(MCP_RXF0SIDL, ACCEPT_ID_SIDL);
	setRegister(MCP_RXM0SIDH, 0xFF);
	setRegister(MCP_RXM0SIDL, 0xE0);
	setRegister(MCP_RXM0EID8, 0x00);
	setRegister(MCP_RXM0EID0, 0x00);

	// Filter 1 and mask all 0xFF
	for (uint8_t i=0; i<6; i++){
		setRegister(MCP_RXF1SIDH+i, 0xFF);
	}
	
	// Disable all interrupt
	setRegister(MCP_CANINTE, 0);
	// Enter normal mode
	setRegister(MCP_CANCTRL, MODE_NORMAL);
}

void Canbus::setRegister(uint8_t address, uint8_t value){
	SELECT;
	SPI::send(MCP_WRITE);
	SPI::send(address);
	SPI::send(value);
	DESELECT;
}

uint8_t Canbus::readStatus(void){
	SELECT;
	SPI::send(MCP_READ_STATUS);
	uint8_t out = SPI::send(0);
	DESELECT;
	return out;
}

bool Canbus::msgAvailable(){
	return readStatus() & MCP_STAT_RX0IF;
}

// TX Buffer 1 for sending data packet
void Canbus::sendPacket(uint16_t id, uint8_t *buf, uint8_t len){
	SELECT;
	SPI::send(MCP_LOAD_TX0);
	// ID
	SPI::send(id>>3 & 0xFF);
	SPI::send(id<<5);
	SPI::send(0);
	SPI::send(0);
	// Len
	SPI::send(len);
	for(uint8_t i=0; i<len; i++){
		SPI::send(buf[i]);
	}
	DESELECT;
	// Request to SPI::send
	SELECT;
	SPI::send(MCP_RTS_TX0);
	DESELECT;

	// Wait until sent
	//SELECT;
	uint8_t status = readStatus();
	//DESELECT;
	while (!(status & MCP_STAT_TX0IF)){
		status = readStatus();
	}
}

void Canbus::readPacket(uint8_t *buf){
	SELECT;
	SPI::send(MCP_READ_RX0);
	for(uint8_t i=0; i<13; i++){
		buf[i] = SPI::send(0);
	}
	DESELECT;
	// uint8_t received = buf[4];
}