// eeprom_memroy.c

#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdio.h>

#include "common.h"
#include "usart.h"
#include "inlinePowermeter.h"
#include "adc.h"

#include "debug.h"

extern calibration_data_t calibrations[10];
extern uint8_t last_calibration;


void load_calibration(adc_data_t* adc_data, uint8_t memory_position) 
{
	
	calibration_data_t calibration = {0};
	if (memory_position < MEMORY_SIZE)
	{
		eeprom_read_block(&calibration, &calibrations[memory_position], sizeof(calibration_data_t));

		adc_data->intercept_MB0 = calibration.intercept_MB0;
		adc_data->intercept_MB1 = calibration.intercept_MB1;
		adc_data->slope_MB0 = calibration.slope_MB0;
		adc_data->slope_MB1 = calibration.slope_MB1;
		adc_data->wavelength = calibration.wavelength;
	}



	#if 0
		{
		char text[50] = {0};
		_delay_ms(1000);		// warten, dass TeraTerm bereit ist nachdem der Stecker aus und wieder eingesteckt wurde
	
		sendUSARTs("read wavelength: ");
		snprintf(text,50,"%d\n", adc_data->wavelength);
		sendUSARTs(text);
		sendUSART(0x0D);		// an Zeilenbeginn springen
		sendUSARTs("read intercept MB0: ");
		snprintf(text,50,"%ld\n", (uint32_t)(adc_data->intercept_MB0*1e12));
		sendUSARTs(text);
		sendUSART(0x0D);		// an Zeilenbeginn springen
		sendUSARTs("read intercept MB1: ");
		snprintf(text,50,"%ld\n", (uint32_t)(adc_data->intercept_MB1*1e12));
		sendUSARTs(text);
		sendUSART(0x0D);		// an Zeilenbeginn springen
		sendUSARTs("read slope MB0: ");
		snprintf(text,50,"%ld\n", (uint32_t)(adc_data->slope_MB0));
		sendUSARTs(text);
		sendUSART(0x0D);		// an Zeilenbeginn springen
		sendUSARTs("read slope MB1: ");
		snprintf(text,50,"%ld\n", (uint32_t)(adc_data->slope_MB1));
		sendUSARTs(text);
		sendUSART(0x0D);		// an Zeilenbeginn springen


		}	
	#endif
	
	
}; // end load_last_calibration()











void delete_calibration(uint8_t pos){
	calibration_data_t empty_calibration = {0};
	
	if (pos<MEMORY_SIZE) {		
		eeprom_write_block(&empty_calibration, &calibrations[pos], sizeof(calibration_data_t));
	}
}

void delete_all_memory() {
	uint8_t i;
	
	for(i=0; i<MEMORY_SIZE; i++) {
		delete_calibration(i);
	}
}

