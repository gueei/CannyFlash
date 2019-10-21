/*
 * CannyFlash_Boot.cpp
 *
 * Created: 10/19/2019 10:54:38 PM
 * Author : andy
 */ 

#include <avr/io.h>
#include <avr/boot.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "stk500.h"
#include "spi.h"
#include "Canbus.h"

#define CANCFG1		MCP_8MHz_1000kBPS_CFG1

#define CAN_ID		0x0B1
#define TXBUFSIZE	10
#define RXBUFSIZE	130
#define CANNYFLASH_MAJOR_VER	1
#define CANNYFLASH_MINOR_VER	0

int main(void); //__attribute__((section(".init9")));
void flush(bool ackRequired);
void putch(uint8_t ch);
void programStart();
void inSync();
void noSync();
uint8_t getch();
void getNch(uint8_t N);

uint8_t txbuf[TXBUFSIZE];
uint8_t txHead=0, txTail = 0;
uint8_t rbuf[RXBUFSIZE];
uint8_t rbufHead=0, rbufTail=0;

void putch(uint8_t ch){
	txbuf[txTail] = ch;
	txTail ++;
	txTail = (txTail)%TXBUFSIZE;
	
	bool canSend = (ch==STK_OK);
	if (txTail<txHead){
		canSend |= ((txTail + TXBUFSIZE)-txHead) >=8;
	}else{
		canSend |= (txTail - txHead) >=8;
	}
	if (canSend)  {
		flush(true);
	}
}

void flush(bool ackRequired){
	uint8_t buf[8];
	uint8_t len = 0;
	while( (txTail!=txHead) && (len<8)){
		buf[len] = txbuf[txHead];
		txHead ++;
		txHead = (txHead) % TXBUFSIZE;
		len++;
	}
	if (ackRequired)
		while(!Canbus::ackReceived());
	Canbus::sendPacket(CAN_ID, buf, len);
	Canbus::clearAck();
}

uint8_t getch(){
	// Buffer Empty, wait for message
	if (rbufTail==rbufHead){
		uint8_t buf[13];
		while(!Canbus::msgAvailable());
		Canbus::readPacket(buf);
		for(uint8_t i=0; i<buf[4]; i++){
			rbuf[rbufTail] = buf[i+5];
			rbufTail ++;
			if (rbufTail>=RXBUFSIZE)
				rbufTail = 0;
			// rbufTail = (rbufTail) % RXBUFSIZE;
		}
	}
	uint8_t c = rbuf[rbufHead];
	rbufHead++;
	// rbufHead = (rbufHead) % RXBUFSIZE;
	if (rbufHead>=RXBUFSIZE)
		rbufHead =0;
	return c;
}

void getNch(uint8_t N){
	for (uint8_t i=0; i<N; i++) getch();
}

void verifySpace() {
	if (getch() != CRC_EOP) {
//		watchdogConfig(WATCHDOG_16MS);    // shorten WD timeout
		while (1)			      // and busy-loop so that WD causes
		;				      //  a reset and app start.
	}
	putch(STK_INSYNC);
}

int main(void)
{
	uint16_t address = 0;
	uint8_t  length = 0;

	// uint8_t buf[8];
	SPI::init();
	Canbus::init();
	
	// Canbus::sendPacket(0x2AB, buf, 8);
	
	putch(MCUSR);
	putch(WDTCSR);
	putch(Canbus::msgAvailable());
	
	//wdt_enable(WDTO_4S);
	flush(false);
		
    for(;;)
    {
		uint8_t ch = getch();
		if (ch==STK_GET_SYNC) {
			verifySpace();
		} else if(ch == STK_GET_PARAMETER) {
			uint8_t which = getch();
			verifySpace();
			if (which == STK_SW_MINOR) {
				putch(CANNYFLASH_MINOR_VER);
			} else if (which == STK_SW_MAJOR) {
				putch(CANNYFLASH_MAJOR_VER);
			} else {
				/*
				* GET PARAMETER returns a generic 0x03 reply for
				* other parameters - enough to keep Avrdude happy
				*/
				putch(0x03);
			}
		} else if(ch == STK_SET_DEVICE) {
			// SET DEVICE is ignored
			getNch(20);
			verifySpace();
		} else if(ch == STK_SET_DEVICE_EXT) {
			// SET DEVICE EXT is ignored
			getNch(4);
			verifySpace();
		} else if(ch == STK_LOAD_ADDRESS) {
			// LOAD ADDRESS
			address = getch();
			address += getch() << 8;
			address <<= 1; // Convert from word address to byte address
			verifySpace();
		} else if(ch == STK_UNIVERSAL) {
			// Ignore Universal
			getNch(4);
			verifySpace();
			putch(0x00);
		} else if(ch == STK_PROG_PAGE) {
			// PROGRAM PAGE - we support flash programming only, not EEPROM
			
			// Data size should be 128, right?
			uint8_t dataBuffer[128];

			length = getch() << 8;
			length += getch();
			
			getch(); // Only writes to FLASH

			// read a page worth of contents
			//bufPtr = buff.bptr;
			for(uint8_t i=0; i<length; i++){
				dataBuffer[i] = getch();
			}
			
			// Start programming:
			// https://www.nongnu.org/avr-libc/user-manual/group__avr__boot.html#ga61add108c778273ff68233d044c3acca
			uint8_t *ptr = dataBuffer;
			eeprom_busy_wait();
			boot_page_erase (address);
			boot_spm_busy_wait ();      // Wait until the memory is erased.
			for (uint8_t i=0; i<length; i+=2)
			{
				// Set up little-endian word.
				uint16_t w = *ptr++;
				w += (*ptr++) << 8;
				  
				boot_page_fill (address + i, w);
			}
			boot_page_write (address);     // Store buffer in flash page.
			boot_spm_busy_wait();
			boot_rww_enable ();
			
			// Read command terminator, start reply
			verifySpace();
		} else if(ch == STK_READ_PAGE) {
			length = getch() << 8;
			length += getch();

			getch(); // Must be flash, ignore

			verifySpace();
			for(uint8_t i=0; i<length; i++){
				putch(pgm_read_byte_near(address+i));
			}
		} else if(ch == STK_READ_SIGN) {
			// READ SIGN - return what Avrdude wants to hear
			verifySpace();
			putch(SIGNATURE_0);
			putch(SIGNATURE_1);
			putch(SIGNATURE_2);
		}
		else if (ch == STK_LEAVE_PROGMODE) { 
			// Adaboot no-wait mod
			// watchdogConfig(WATCHDOG_16MS);
			verifySpace();
		} else if (ch==0xFF){
			while(getch()!=0x20);
		}
		else {
			// This covers the response to commands like STK_ENTER_PROGMODE
			verifySpace();
		}

		putch(STK_OK);
    }
}

// Program start
// Cleanup SPI and such
void programStart(){
	// SPI Disable
	// Clear watchdog timer
	// JUMP to 0x0000
	// Clear Reset bits
	//wdt_disable();
	//asm volatile("jmp 0x0000");
}

