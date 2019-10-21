/*
 * spi.cpp
 *
 * Created: 10/19/2019 11:08:58 PM
 *  Author: andy
 */ 

#include "SPI.h"
#include <avr/io.h>

void SPI::init(){
	DDRB |= _BV(2);
	SPCR |= _BV(MSTR) | _BV(SPE) | _BV(SPR0);
	DDRB |= _BV(3) | _BV(5);
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