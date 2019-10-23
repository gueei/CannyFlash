/*
 * spi.h
 *
 * Created: 10/19/2019 11:04:28 PM
 *  Author: andy
 */ 


#ifndef SPI_H_
#define SPI_H_

#include <inttypes.h>
#include <Configuration.h>

class SPI{
	
	public:
		static void init();
				
		static int8_t send(uint8_t data);
};



#endif /* SPI_H_ */