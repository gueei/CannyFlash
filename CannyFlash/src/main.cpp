/*
 * CannyFlash_Boot.cpp
 *
 * Created: 10/19/2019 10:54:38 PM
 * Author : andy
 */ 

#include <avr/io.h>
#include <avr/boot.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "stk500.h"
#include "spi.h"
#include "Canbus.h"

/* Watchdog settings */
#define WATCHDOG_OFF    (0)
#define WATCHDOG_16MS   (_BV(WDE))
#define WATCHDOG_32MS   (_BV(WDP0) | _BV(WDE))
#define WATCHDOG_64MS   (_BV(WDP1) | _BV(WDE))
#define WATCHDOG_125MS  (_BV(WDP1) | _BV(WDP0) | _BV(WDE))
#define WATCHDOG_250MS  (_BV(WDP2) | _BV(WDE))
#define WATCHDOG_500MS  (_BV(WDP2) | _BV(WDP0) | _BV(WDE))
#define WATCHDOG_1S     (_BV(WDP2) | _BV(WDP1) | _BV(WDE))
#define WATCHDOG_2S     (_BV(WDP2) | _BV(WDP1) | _BV(WDP0) | _BV(WDE))
#define WATCHDOG_4S     (_BV(WDP3) | _BV(WDE))
#define WATCHDOG_8S     (_BV(WDP3) | _BV(WDP0) | _BV(WDE))

static const char password[] 
	__attribute__((used, section(".password"))) 
	 = PASSWORD;
	
int main(void) ; //__attribute__((OS_main)) ; //__attribute__((section(".init9")));
void flush();
void putch(uint8_t ch);
inline void programStart();
uint8_t getch();
void getNch(uint8_t N);
void watchdogReset();
void watchdogConfig(uint8_t x);

uint8_t txbuf[TXBUFSIZE];
uint8_t txHead=0, txTail = 0;
uint8_t rbuf[RXBUFSIZE];
uint8_t rbufHead=0, rbufTail=0;

void putch(uint8_t ch){
	txbuf[txTail] = ch;
	txTail ++;
	if (txTail>=8 || ch==STK_OK){
		flush();
	}
	// txTail = (txTail)%TXBUFSIZE;
	/*
	bool canSend = (ch==STK_OK) || (txTail==8);
	if (canSend)  {
		flush(false);
	}
	*/
}

void flush(){
	//if (txTail == 0) return;
	Canbus::sendPacket(CAN_ID, txbuf, txTail);
	txTail = 0;
	for(int i=0;i<1000; i++){
		asm("nop");
	}
	/*
	if (ackRequired)
		while(!Canbus::ackReceived());
	Canbus::clearAck();
	*/
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
		watchdogConfig(WATCHDOG_16MS);   // shorten WD timeout
		while (1)			      // and busy-loop so that WD causes
		;						  //  a reset and app start.
	}
	putch(STK_INSYNC);
}

int main(void)
{
	uint8_t MCUSR_Initial = MCUSR;
	/* Clear WDRF in MCUSR */
	//MCUSR &= ~(1<<WDRF);
	cli();
	watchdogReset();
	MCUSR &= ~(1<<WDRF);
	WDTCSR |= (1<<WDCE) | (1<<WDE);
	WDTCSR = 0x00;
	sei();
	
	
	if ((MCUSR_Initial & (1<<WDRF)) && (pgm_read_word(0) != 0xFFFF)){
		/* 
		* save the reset flags in the designated register
		* This can be saved in a main program by putting code in .init0 (which
		* executes before normal c init code) to save R2 to a global variable.
		*/
		watchdogConfig(WATCHDOG_OFF);
		wdt_reset();
		__asm__ __volatile__ ("mov r2, %0\n" :: "r" (MCUSR_Initial));
		programStart();
	}
	
	register uint16_t address = 0;
	register uint8_t  length = 0;

	SPI::init();
	Canbus::init();
	/*
	putch(0xFF);
	putch(MCUSR_Initial);
	putch(MCUSR_Initial & (1<<WDRF));
	putch(0xFF);
	flush();
	putch(pgm_read_word(0) & 0xFF);
	putch(MCUSR_Initial);
	putch(MCUSR);
	putch(WDTCSR);
	putch(WDRF);
	putch(Canbus::msgAvailable());
	
	flush();
	*/

	watchdogConfig(WATCHDOG_2S);
	
	/*
	// If no ack received, watchdog will timeout and cause reset
	while(!Canbus::ackReceived()){
		asm volatile("nop; \n");
	}
	Canbus::clearAck();
	*/
	
	putch('C');
	putch('A');
	putch('N');
	putch('N');
	putch('Y');
	putch('v');
	putch('1');
	putch('!');
	
	flush();
	
	address = PASSWORD_ADDRESS;
	/*
	for(uint8_t i=0;i<8; i++){
		putch(pgm_read_byte_near(address+i));
	}
	*/
	
	uint8_t pwBuffer[8];
	// Enter Password
	for(uint8_t i=0; i<8; i++){
		pwBuffer[i] = getch();
	}
	
	for (uint8_t i=0; i<8; i++){
		if (pgm_read_byte_near(address+i) != pwBuffer[i]) {
			putch(STK_NOSYNC);
			putch(STK_OK);
			// This code would never sent, just to trick the compiler
			putch(password[i]);
			for(;;);
		}
	}
	
	// Password accepted
	putch(STK_INSYNC);
	putch(STK_OK);
	
	watchdogConfig(WATCHDOG_OFF);
	wdt_reset();
	
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
			getch();
			//length = getch() << 8;
			length = getch();

			getch(); // Must be flash, ignore
			verifySpace();
			for(uint8_t i=0; i<length; i++){
				//putch(txTail);
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
			watchdogConfig(WATCHDOG_16MS);
			verifySpace();
		} else if (ch==0xFF){
			while(getch()!=0x20);
		} else if (ch=='z'){
			// Change password - Extended method from standard STK500
			for(uint8_t i=0;i<8; i++){
				pwBuffer[i] = getch();
			}
			verifySpace();
			
			eeprom_busy_wait();
			boot_page_erase (address);
			boot_spm_busy_wait ();      // Wait until the memory is erased.
			for (uint8_t i=0; i<8; i+=2)
			{
				uint16_t w = (pwBuffer[i+1] << 8) + pwBuffer[i];
				boot_page_fill (address + i, w);
			}
			boot_page_write (address);     // Store buffer in flash page.
			boot_spm_busy_wait();
			boot_rww_enable ();
		} else {
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
	asm volatile("jmp 0x0000");
}

// Watchdog functions. These are only safe with interrupts turned off.
void watchdogReset() {
	__asm__ __volatile__ (
	"wdr\n"
	);
}

void watchdogConfig(uint8_t x) {
	cli();
	#ifdef WDCE //does it have a Watchdog Change Enable?
	#ifdef WDTCSR
	WDTCSR |= _BV(WDCE) | _BV(WDE);
	#else
	WDTCR |= _BV(WDCE) | _BV(WDE);
	#endif
	#else //then it must be one of those newfangled ones that use CCP
	CCP=0xD8; //so write this magic number to CCP
	#endif
	#ifdef WDTCSR
	WDTCSR = x;
	#else
	WDTCR= x;
	#endif
	sei();
}
