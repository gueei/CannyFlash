/*
 * spi.cpp
 *
 * Created: 10/19/2019 11:08:58 PM
 *  Author: andy
 */ 

#include "SPI.h"
#include <avr/io.h>
#include "Configuration.h"

void SPI::init(){
    //disable power reduction to allow SPI:
	PRR &= ~_BV(PRSPI);
	SPI_DDR |= _BV(SS) | _BV(SCK) | _BV(MOSI);
	SPCR = _BV(MSTR) | _BV(SPE) | _BV(SPR0);
	SPI_PORT |= _BV(SCK) | _BV(MOSI);
}
		
int8_t SPI::send(uint8_t data){
	SPDR = data;
	/*
	* The following NOP introduces a small delay that can prevent the wait
	* loop form iterating when running at the maximum speed. This gives
	* about 10% more speed, even if it seems counter-intuitive. At lower
	* speeds it is unnoticed.
	*/
	asm volatile("nop");
	while (!(SPSR & _BV(SPIF))) ; // wait
	return SPDR;
}