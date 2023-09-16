// timestamp.c

#include <avr/io.h>
#include <avr/interrupt.h>
#include "common.h"
#include "timestamp.h"



extern volatile uint16_t timestamp;

void ini_timestamp() 
{
/*
 *	Description: 
 *	Timer PORT C 0
 *  no precaler, 1ms, interrupt priority hi.
 */	
	
	cli();
	
	TCC0.CTRLA = TC_CLKSEL_DIV1_gc;
	TCC0.CTRLB = 0x00;
	TCC0.PER = 2000;
	TCC0.CNT = 0x00;
	TCC0.INTCTRLA = 0x03;
	
	PMIC.CTRL |= PMIC_HILVLEN_bm;
	
	
	sei();
	
	
	
	
} // end ini_timestamp


uint8_t check_timing(uint16_t last_call, uint16_t timing )
{
	uint8_t result = 0;
	
	// timer interrupt ausschalten
	cli();
	
	if((timestamp-last_call) > timing) 
	{
		result = 1;	
	}
	
	// timer interrupt einschalten
	sei();
	
	return result;	
}