// tasten.c

#define F_CPU 2000000UL


#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include "common.h"
#include "inlinePowermeter.h"
#include "debug.h"
#include "button.h"
#include "display.h"
#include "adc.h"
#include "eeprom_memory.h"
#include "usart.h"

extern calibration_data_t calibrations[10];

#if GET_KENNLINIE
extern adc_control_t adc_control;
#endif



void ini_button() {
	
	cli();						// disable interrupts
	
	PORTC_DIR &= 0x0F;			//  PC[5:7] als input setzten
	PORTC_INTCTRL |= 0x01;		// lo-level interrupt
	PORTC_INT0MASK |= 0xF0;		// die oberen PORTC bits als interrupt verwenden  -> Port interrupt 0 verwendet
	PORTC_PIN4CTRL |= 0x02;		// falling edge interrupt;
	PORTC_PIN5CTRL |= 0x02;		// falling edge interrupt;
	PORTC_PIN6CTRL |= 0x02;		// falling edge interrupt;
	PORTC_PIN7CTRL |= 0x02;		// falling edge interrupt;
	
	PMIC.CTRL |= PMIC_LOLVLEN_bm;
	
	sei();						// enable interrupts
	
	
} // end ini_button();




