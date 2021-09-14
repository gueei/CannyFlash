/*
 * Configuration.h
 *
 * Created: 10/21/2019 12:33:46 PM
 *  Author: andy
 */ 


#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#define MCP2515_CLOCK_8MHz
// #define MCP2515_CLOCK_16MHz


#if defined (__AVR_ATmega328P__)
#define SPI_DDR		DDRB
#define SPI_PORT	PORTB
#define SS			2
#define SCK			5
#define MOSI		3
#endif

#if defined (__AVR_ATmega32U4__)
#define SPI_DDR		DDRB
#define SPI_PORT	PORTB
#define SS			0
#define SCK			1
#define MOSI		2
#define PRR			PRR0
#endif

// Digital Pin 10 on UNO and Leonardo
#if defined (__AVR_ATmega328P__)
#define MCP_CS_DDR		DDRB
#define MCP_CS_PORT		PORTB
#define MCP_CS_PIN		2
#endif

#if defined (__AVR_ATmega32U4__)
#define MCP_CS_DDR		DDRB
#define MCP_CS_PORT		PORTB
#define MCP_CS_PIN		6
#endif

#define CAN_ID		0x0B1
#define TXBUFSIZE	8
#define RXBUFSIZE	130
#define CANNYFLASH_MAJOR_VER	1
#define CANNYFLASH_MINOR_VER	1
#define ACCEPT_ID	0x1CF
#define ACCEPT_ID_SIDH (ACCEPT_ID >> 3) & 0xFF
#define ACCEPT_ID_SIDL (ACCEPT_ID << 5) & 0xFF

#define PASSWORD			"12345678"
#define PASSWORD_ADDRESS	0x7f80 // Note: 3Fc0 * 2

#endif /* CONFIGURATION_H_ */