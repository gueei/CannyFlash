/*
 * Canbus.h
 *
 * Created: 10/19/2019 11:17:34 PM
 *  Author: andy
 */ 


#ifndef CANBUS_H_
#define CANBUS_H_

#include <inttypes.h>
#include <avr/io.h>
#include "mcp2515_dfs.h"
#include "Configuration.h"

#if defined (MCP2515_CLOCK_8MHz)
#define MCP_CNF1_CFG	MCP_8MHz_500kBPS_CFG1
#define MCP_CNF2_CFG	MCP_8MHz_500kBPS_CFG2
#define MCP_CNF3_CFG	MCP_8MHz_500kBPS_CFG3
#endif

#if defined (MCP2515_CLOCK_16MHz)
#define MCP_CNF1_CFG	MCP_16MHz_500kBPS_CFG1
#define MCP_CNF2_CFG	MCP_16MHz_500kBPS_CFG2
#define MCP_CNF3_CFG	MCP_16MHz_500kBPS_CFG3
#endif

#define SELECT		MCP_CS_PORT &= ~_BV(MCP_CS_PIN)
#define DESELECT	MCP_CS_PORT |= _BV(MCP_CS_PIN)

#define ACK_ID			0x1AC
#define ACK_SIDH		(ACK_ID >> 3) & 0xFF
#define ACK_SIDL		(ACK_ID << 5) & 0xFF
#define ACCEPT_ACK_ID	0x2AC
#define ACCEPT_ACK_SIDH	(ACCEPT_ACK_ID >> 3) & 0xFF
#define ACCEPT_ACK_SIDL	(ACCEPT_ACK_ID << 5) & 0xFF

class Canbus{
	public:
		static void init();
		static bool msgAvailable();	
		static void setRegister(uint8_t address, uint8_t value);
		static uint8_t readStatus(void);
		static void sendPacket(uint16_t id, uint8_t *buf, uint8_t len);
		static void readPacket(uint8_t *buf);
		static bool ackReceived();
		static void clearAck();
		static void sendAck();
};



#endif /* CANBUS_H_ */