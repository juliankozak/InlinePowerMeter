// adc.c

#define F_CPU 2000000UL

#include <avr/io.h>
#include <avr/eeprom.h>
#include <math.h>
#include <util/delay.h>
#include <stdio.h>

#include "common.h"
#include "usart.h"
#include "inlinePowermeter.h"
#include "adc.h"
#include "eeprom_memory.h"
#include "debug.h"
#include "display.h"

extern calibration_data_t calibrations[10];
extern uint8_t last_calibration;

extern volatile uint16_t timestamp;

#if GET_KENNLINIE
extern adc_control_t adc_control;
#endif

void ini_adc(adc_data_t* adc_data) 
{
	/*
	 *	Description: 
	 *	I2C Konfiguration ohne Interrupts, Master Mode, 100kHz, PORTC (PC0, PC1)
	 *  ADC: 16 bit, continous conversion -> 3.75 Samples/s
	 *
	 *	Messbereiche : PE1,PE2 als Ausgang setzten
	 */	
	uint8_t last_calibration_buffer;
	
	
	TWIC_CTRL = 0x00;						
	TWIC_MASTER_CTRLA = 0x40;
	TWIC_MASTER_CTRLB = 0x00;
	TWIC_MASTER_CTRLC = 0x02;
	TWIC_MASTER_BAUD = 0x05;
	TWIC_MASTER_CTRLA |= 0x08;		// enable
	TWIC_MASTER_STATUS = 0xCD;
	
	TWIC_MASTER_ADDR = 0xD0;		// master write mode

		while((TWIC_MASTER_STATUS & 0x40) == 0) {;}		// auf WIF warten
		
		TWIC_MASTER_STATUS &= 0xBF;	//clear WIF
		TWIC_MASTER_DATA = 0x98;	// conf reg. adc 16bit mode, continous conversion
		
		while((TWIC_MASTER_STATUS & 0x40) == 0) {;}		// auf WIF warten
		
		TWIC_MASTER_CTRLC = 0x03; // stop condition	
	
	
	PORTE_DIR |= (PIN1_bm|PIN2_bm);		// Messbereiche aktivieren (Steuerleitung für Analogschalter)	

	adc_data->range = automatic;
	adc_data->new_MB = 1;

	last_calibration_buffer = eeprom_read_byte(&last_calibration);
	load_calibration(adc_data, last_calibration_buffer);

#if 0
	sendUSARTs("ini adc -> last calibration = ");
	sendUSARTb(last_calibration_buffer);
	sendUSARTs("\n");
	sendUSART(0x0D);		// an Zeilenbeginn springen
#endif					
	
} // end ini_ADC



uint16_t get_adc_result() {			// [Dauer 390us]
/*
 *	Description: 
 *	16 bit result:
 *  vom ADC empfangen: | hi byte | lo byte | conf. byte |  
 */
uint8_t hi_byte = 0;
uint8_t lo_byte = 0;
uint8_t conf = 0;
uint16_t result = 0;

uint16_t timeout_start = 0;
	
	
	TWIC_MASTER_ADDR = 0xD1;		// master read mode
	
		#if DEBUG_I2C
		sendUSARTs("I2C status nach adresse: "); sendUSARTb(TWIC_MASTER_STATUS);	
		#endif
	
	timeout_start = timestamp;	
	while((TWIC_MASTER_STATUS & 0x70) == 0)		// auf RIF warten, allerdings maximal 100ms
	{	
		if(timestamp-timeout_start > 100 )		//FEHLERMELDUNG
		{
			sendLCDs("  ERROR-I2C BUS",1);
			sendUSARTs("ERROR-I2C BUS");
		}				
	}
			
	hi_byte = TWIC_MASTER_DATA;
	TWIC_MASTER_CTRLC = 0x02;		// ACK
	
		#if DEBUG_I2C
		sendUSARTs("I2C status nach 1.Byte: "); sendUSARTb(TWIC_MASTER_STATUS);		
		#endif
	
	timeout_start = timestamp;
	while((TWIC_MASTER_STATUS & 0x70) == 0)		// auf RIF warten, allerdings maximal 100ms
	{
		if(timestamp-timeout_start > 100 )		//FEHLERMELDUNG
		{
			sendLCDs("  ERROR-I2C BUS",1);
			sendUSARTs("ERROR-I2C BUS");
		}
	}
	lo_byte = TWIC_MASTER_DATA;	
	TWIC_MASTER_CTRLC = 0x02;		// ACK	
	
		#if DEBUG_I2C
		sendUSARTs("I2C status nach 2.Byte: "); sendUSARTb(TWIC_MASTER_STATUS);			
		#endif
	
	timeout_start = timestamp;
	while((TWIC_MASTER_STATUS & 0x70) == 0)		// auf RIF warten, allerdings maximal 100ms
	{
		if(timestamp-timeout_start > 100 )		//FEHLERMELDUNG
		{
			sendLCDs("  ERROR-I2C BUS",1);
			sendUSARTs("ERROR-I2C BUS");
		}
	}
	
	conf = TWIC_MASTER_DATA;
	TWIC_MASTER_CTRLC = 0x07;		// NACK + stop condition

		#if DEBUG_I2C
		sendUSARTs("I2C status nach Konfigurations-Byte: "); sendUSARTb(TWIC_MASTER_STATUS);
		#endif

	result = result | (int)lo_byte;
	
	result = result | ( ((int)hi_byte) << 8 );
	
		#if DEBUG_I2C
		sendUSARTs("I2C status nach NACK + Stop Bit: "); sendUSARTb(TWIC_MASTER_STATUS);		
		sendUSARTs("\n");
		#endif
	
	return result;	

} // end adc_result.






void calculate_intercept_slope(adc_data_t* adc_data, new_calibration_t* calibration)
{
/*
	2-Punkt-Kalibrierung durchführen und daraus auf intercept und slope für den bestimmten, neu kalibrierten Messbereich schließen.
	anschließend das Ergebnis im adc Datenstruct abspeichern, damit dieses bei den künftigen Messungen verwendet wird.
*/
	
//	slope = (y2-y1) / (log10(x2) - log10(x1));			%MATLAB wobei y_i die codewörter und x_i die Leistungen sind

//	intercept = 10^(log10(x1)-(y1/slope));

	if(calibration->messbereich == 0)
	{
		adc_data->slope_MB0 = (calibration->new_adc_point[1] - calibration->new_adc_point[0]) / (log10(calibration->new_ref_point[1])-log10(calibration->new_ref_point[0]));
			
		adc_data->intercept_MB0 = pow(10, log10(calibration->new_ref_point[0]) - (calibration->new_adc_point[0]/adc_data->slope_MB0));
	}
	else
	{
		adc_data->slope_MB1 = (calibration->new_adc_point[1] - calibration->new_adc_point[0]) / (log10(calibration->new_ref_point[1])-log10(calibration->new_ref_point[0]));
			
		adc_data->intercept_MB1 = pow(10, log10(calibration->new_ref_point[0]) - (calibration->new_adc_point[0]/adc_data->slope_MB1));
	}
	
	adc_data->wavelength = calibration->wavelength;
	

	
	
#if 0	
	char text[50] = {0};
		snprintf(text,50,"PUNKT 1: ADC = %d ; Ref.Wert = %ld pW \n", calibration->new_adc_point[0], (uint32_t)(calibration->new_ref_point[0]*1e12));
		sendUSARTs(text);
		sendUSART(0x0D);		// an Zeilenbeginn springen
		snprintf(text,50,"PUNKT 2: ADC = %d ; Ref.Wert = %ld pW \n", calibration->new_adc_point[1], (uint32_t)(calibration->new_ref_point[1]*1e12));
		sendUSARTs(text);
		sendUSART(0x0D);		// an Zeilenbeginn springen
		
		if(calibration->messbereich == 0)
		{
			snprintf(text,50,"Intercept MB0 = %ld pW \n", (uint32_t)(adc_data->intercept_MB0*1e12));
			sendUSARTs(text);
			sendUSART(0x0D);		// an Zeilenbeginn springen
			snprintf(text,50,"Slope MB0 = %ld \n", (uint32_t)(adc_data->slope_MB0));
			sendUSARTs(text);
			sendUSART(0x0D);		// an Zeilenbeginn springen
		}
// 		else
// 		{
// 			snprintf(text,50,"Intercept MB 1 = %ld pW \n", (uint32_t)(adc_data->intercept_MB1*1e12));
// 			sendUSARTs(text);
// 			sendUSART(0x0D);		// an Zeilenbeginn springen
// 			snprintf(text,50,"Slope MB 1 = %ld \n", (uint32_t)(adc_data->slope_MB1));
// 			sendUSARTs(text);
// 			sendUSART(0x0D);		// an Zeilenbeginn springen	
// 		}

#endif		
	

	

} // end calculate_intercept_slope();




void adc(adc_data_t* data) {
/*
 *	Description: 
 *
 */	

float p_opt = 0;
float p_opt_dBm = 0;

	if(GET_MB() != data->new_MB) 
	{
			if(data->new_MB == 0) 
			{
				SET_MB_0();
			}
			else 
			{
				SET_MB_1();			
			}
	}
		
	
	#if GET_KENNLINIE
		if(adc_control.adc_status == adc_stop) 
		{
			return;
		}
		else if (adc_control.counter < 1) 
		{
			adc_control.adc_status = stop;
			return;
		}
		else 
		{
			adc_control.counter--;
		}
	#endif
	
	/*  ADC ERGEBNIS per I²C auslesen */
	data->adc_result = get_adc_result();

	/*  Optische Leistung je nach Messbereich berechnen: 	
		Das Array p_opt war vorgesehen, um eine Mittelung der Ergbnisse durchführen zu können.
	*/
	if(GET_MB() == 0)
	{
		p_opt = data->intercept_MB0 * pow(10,data->adc_result/data->slope_MB0);
		p_opt_dBm = 10*log10(p_opt/1e-3);
		
		data->p_opt[0] = (uint32_t)(p_opt*1e12) ;		// 1e12 Offset in der Fix-Komma-Darstellung
		data->p_opt_dBm = (int16_t)(p_opt_dBm*1e2);		// 1e2 Offset, Vorzeichen -> signed!
		
	}
	else
	{
		p_opt = data->intercept_MB1 * pow(10,data->adc_result/data->slope_MB1);
		p_opt_dBm = 10*log10(p_opt/1e-3);
				
		data->p_opt[0] = (uint32_t)(p_opt*1e12) ;		// 1e12 Offset in der Fix-Komma-Darstellung
		data->p_opt_dBm = (int16_t)(p_opt_dBm*1e2);		// 1e2 Offset, Vorzeichen -> signed!
	}
	
	/*	Überprüfen, ob der aktuell eingestellte Messbereich auch angemessen ist.
		Sonst muss dieser geändert werden (new_MB) und wird automatisch vor dem nächsten Sampling (get_adc_result) umgestellt.
		Ziel ist es, dass die Ergebnisse und der Messbereich zeitlich konsitent bleiben. Der MB wird erst dann umgeschaltet, wenn auch
		neu gemessen wird. 
	*/
	
	if(data->range == automatic)			// manual wäre während der Kalibrierung eines bestimmten Messbereiches. 
	{
		if((GET_MB() == 0) && (data->adc_result > 28000))		// Schwellwert bei ca. 2uW optischer Leistung. 
		{
			data->new_MB = 1;
		}
		else if ((GET_MB() != 0) && data->adc_result < 1950)
		{
			data->new_MB = 0;
		}
	}
	
	#if GET_KENNLINIE	
		data->range = manual;
		char adc_result_string[7] = {0};
		snprintf(adc_result_string, 7, "%d", data->adc_result);

		sendUSARTs("MB ");
		if(GET_MB() == 0) 
		{
			sendUSARTs("0 ");
		}
		else 
		{
			sendUSARTs("1 ");
		}
		sendUSARTs(adc_result_string);
		sendUSARTs("\n");
		sendUSART(0x0D);		// an Zeilenbeginn springen
	#endif
	
	
	#if DEBUG_ADC		
		char adc_result_string[7] = {0};
		char result_string[11] = {0};
				
		snprintf(adc_result_string, 7, "%d", data->adc_result);
		snprintf(result_string, 11, "%ld ", data->p_opt[0]);
			
		sendUSARTs("MB ");
		if(MB == 0) 
		{
			sendUSARTs("0 ");
		}
		else 
		{
			sendUSARTs("1 ");
		}
			
		sendUSARTs("ADC Result: ");
		sendUSARTs(adc_result_string);
		sendUSARTs("\t");
		
		sendUSARTs("P_opt = ");
		sendUSARTs(result_string);
		sendUSARTs("\n");
		sendUSART(0x0D);		// an Zeilenbeginn springen
	#endif
	
	return;

} // end adc.
