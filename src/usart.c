// USART.c

#define F_CPU 2000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include "common.h"
#include "inlinePowermeter.h"
#include "usart.h"
#include "debug.h"
#include "eeprom_memory.h"
#include "adc.h"
#include "calibration.h"



extern calibration_data_t calibrations[10];
extern uint8_t last_calibration;

extern volatile rx_buffer_t rx_buffer;

/**********************************************************************/
/* Initialisierung: */
/**********************************************************************/

void ini_usart(usart_data_t* usart_data) {
/*
 *	Description: 
 *	9600 baud, 8bit, no parity, one stop bit
 *  nur SENDEN ohne interrupts, EMPFANGEN mit interrupts (medium level)
 */

	cli();
		
	PORTD_OUT |= (1<<PIN3_bp);		
	PORTD_DIR |= (1<<PIN3_bp);		
	USARTD0.BAUDCTRLA = 0x0c;		// BSEL = 12, BSCALE = 0
	USARTD0.BAUDCTRLB = 0x00;
	USARTD0.CTRLA = 0x20;			// Receive interrupt level = medium
	USARTD0.CTRLC = 0x03;			// Character Size = 8bit
	USARTD0.CTRLB = (1<<USART_TXEN_bp)|(1<<USART_RXEN_bp);	// enable Transmitter und Receiver
	
	PMIC.CTRL |= PMIC_MEDLVLEN_bm;	// enable medium level interrupts
	
	sei();
	
	usart_data->command = com_wait;
	
	
} // end ini_usart.


/**********************************************************************/
/* Steuerung per serieller Schnittstelle */
/**********************************************************************/



void usart(usart_data_t * data, adc_data_t * adc_data, new_calibration_t* new_calibration)
{
/*
 *	Description: 
 *	
 *  
 */
	
	if(rx_buffer.buffer_ready > 0)		// Daten wurden empfangen
	{
		
		handle_com_input(data, adc_data, new_calibration);
		handle_com_output(data, adc_data);		
	}	
	
}




void handle_com_input(usart_data_t* com_data, adc_data_t* adc_data, new_calibration_t* calibration)
{
	
	switch(com_data->command)
	{
		case com_wait:
			handle_com_wait(com_data);
			break;
		
		case com_measure:
			break;
			
		case com_set_wavelength:		
			switch(com_data->subcommand_set_wavelength)
			{
				case com_set_wavelength_start:
					break;
				case com_set_wavelength_choose:
		 			handle_com_set_wavelength(adc_data, com_data);
					break;
			}
			break;
			
		case com_delete_calibration:
			switch(com_data->subcommand_delete)
			{
				case com_delete_start:
					break;
				case com_delete_choose:
					handle_com_delete(com_data);
					break;	
			}
			break;				
							
		 case com_calibrate:
		 	handle_com_calibration(com_data, calibration, adc_data);
		 	break;
		default:
			com_data->command = com_wait;
	}
	
}

void handle_com_output(usart_data_t* com_data, adc_data_t* adc_data)
{
	switch(com_data->command)
	{
		case com_wait:
			break;
			
		case com_measure:
			show_opt_power(adc_data);
			com_data->command = com_wait;
			break;
			
		case com_set_wavelength:			
			switch(com_data->subcommand_set_wavelength)
			{
				case com_set_wavelength_start:
					sendUSARTs("** set wavelength: **\n");
					sendUSART(0x0D);
					show_saved_wavelength();
					com_data->subcommand_set_wavelength = com_set_wavelength_choose;
					break;
				case com_set_wavelength_choose:
					break;
			}			
			break;
		
		case com_delete_calibration:
			switch(com_data->subcommand_delete)
			{
				case com_delete_start:
					sendUSARTs("** delete calibration: **\n");
					sendUSART(0x0D);
					show_saved_wavelength();
					com_data->subcommand_delete = com_delete_choose;
					break;
				case com_delete_choose:
					break;
			}
			break;
			
		case com_calibrate:
			handle_com_show_calibration(com_data);
			break;
				
		case com_help:
			show_menu();
			com_data->command = com_wait;
			break;
			
			
	}
	
}

/******************************************************************/
/**** SHOW FUNKTIONEN außer calibration (später) ****/
/******************************************************************/

void clear_rx_buffer() 
{
	uint8_t i;
	
	for(i=0; i<BUFFER_LENGTH; i++)
	{
		rx_buffer.buffer[i] = 0;
	}
	rx_buffer.buffer_position = 0;
	rx_buffer.buffer_ready = 0;
	
}



void show_saved_wavelength()
{
	uint8_t i = 0;
calibration_data_t temp = {0};
	int8_t buffer[25] = {0};
	
	for(i=0; i<MEMORY_SIZE; i++)
	{
		eeprom_read_block(&temp, &calibrations[i],sizeof(calibration_data_t));
		
		if(temp.wavelength != 0)
		{
			snprintf(buffer, 25, "\t %d. %d nm \n", i+1, temp.wavelength);	
		}
		else
		{
			snprintf(buffer, 25, "\t %d. ** empty **\n", i+1);	
		}
		
		sendUSARTs(buffer);
		sendUSART(0x0D);		
	}
	sendUSARTs(">> ");
	
}

void show_menu()
{
	sendUSARTs("-------------------------------------\n");
	sendUSART(0x0D);
	sendUSARTs("Inline Powermeter - commands: \n");
	sendUSART(0x0D);
	sendUSARTs("m \t measure optical power\n");
	sendUSART(0x0D);
	sendUSARTs("s \t set wavelength\n");
	sendUSART(0x0D);
	sendUSARTs("d \t delete a calibration \n");
	sendUSART(0x0D);
	sendUSARTs("c \t calibrate\n");
	sendUSART(0x0D);
	sendUSARTs("help     show commands\n");
	sendUSART(0x0D);
	sendUSARTs("-------------------------------------\n\n");
	sendUSART(0x0D);
}


void show_opt_power(adc_data_t* adc_data)
{
	char buffer[50] = {0};
	int8_t vorkomma_zahl = 0;
	int8_t nachkomma_zahl = 0;
	
	if(adc_data->p_opt_dBm < 0)
	{
		vorkomma_zahl = -adc_data->p_opt_dBm / 100;
		nachkomma_zahl = -adc_data->p_opt_dBm % 100;
		snprintf(buffer,50,"%ld\tpW\t-%2d.%2d dBm\t%d nm\tMessbereich\t%d\tacd\t%d \n", adc_data->p_opt[0], vorkomma_zahl, nachkomma_zahl, adc_data->wavelength, GET_MB(),adc_data->adc_result);
	}
	else
	{
		vorkomma_zahl = adc_data->p_opt_dBm / 100;
		nachkomma_zahl = adc_data->p_opt_dBm % 100;
		snprintf(buffer,50,"%ld\tpW\t%2d.%2d dBm\t%d nm\tMessbereich\t%d\tacd\t%d \n", adc_data->p_opt[0], vorkomma_zahl, nachkomma_zahl, adc_data->wavelength, GET_MB(),adc_data->adc_result);
	}
	
	sendUSARTs(buffer);	
	sendUSARTs("\n");
	sendUSART(0x0D);
	
	
}


/*********************************************************************************/
/* HANDLE FUNKTIONEN außer calibration (später) */
/*********************************************************************************/


void handle_com_wait(usart_data_t* data)
{
	int8_t temp;
	int8_t anz = 0;

	anz = sscanf(rx_buffer.buffer,"%c", &temp );
	clear_rx_buffer();
	
	if(anz<0)	// Fehler beim einlesen von sscanf (-1)
	{
		data->command = com_wait;
		return;
	}
	
	switch(temp)
	{
		case 'm':
			data->command = com_measure;
			break;
		case 's':
			data->command = com_set_wavelength;
			data->subcommand_set_wavelength = com_set_wavelength_start; 
			break;
		case 'd':
			data->command = com_delete_calibration;
			break;
		case 'c':
			data->command = com_calibrate;
			data->subcommand_calibration = com_calibration_messbereich;
			break;
		case 'h':					// von der "help" Eingabe wird nur der erste Buchstabe genommen.
			data->command = com_help;
			break;
		default:
			data->command = com_wait;
	}
	
}


void handle_com_set_wavelength(adc_data_t* adc_data, usart_data_t* com_data)
{
	int8_t temp;
	int8_t anz = 0;

	anz = sscanf(rx_buffer.buffer,"%d", &temp );
	clear_rx_buffer();
		
	if((anz < 0) || (temp < 0) || (temp > MEMORY_SIZE+1))
	{
		sendUSARTs("invalid choice #1 \n");
		sendUSART(0x0D);
		com_data->command = com_wait;
		com_data->subcommand_set_wavelength = com_set_wavelength_start;			
		return;
	}
		
	load_calibration(adc_data, temp-1);	
	eeprom_write_byte(&last_calibration,temp-1);	// nach einem Neustart soll diese Einstellung wieder geladen werden.
	sendUSARTs("loading done... \n");
	sendUSART(0x0D);
		
	com_data->command = com_wait;
	com_data->subcommand_set_wavelength = com_set_wavelength_start;
}



void handle_com_delete(usart_data_t* com_data)
{
	int8_t temp;
	int8_t anz = 0;

	anz = sscanf(rx_buffer.buffer,"%d", &temp );
	clear_rx_buffer();
	
	if((anz < 0) || (temp < 0) || (temp > MEMORY_SIZE+1))	// Fehlerbehandlung von sscanf
	{
		sendUSARTs("invalid choice #2 \n");
		sendUSART(0x0D);
		com_data->command = com_wait;
		com_data->subcommand_delete = com_delete_start;
		return;
	}
	
	delete_calibration(temp-1);
	
	sendUSARTs("deleting done... \n");
	sendUSART(0x0D);
	
	com_data->command = com_wait;
	com_data->subcommand_delete = com_delete_start;
	
}



/*********************************************************************************/
/* KALIBRIERUNG PER SERIELLER SCHNITTSTELLE */
/*********************************************************************************/


void handle_com_calibration(usart_data_t* com_data, new_calibration_t* calibration, adc_data_t* adc_data)
{
	switch(com_data->subcommand_calibration)
	{
	
		case com_calibration_messbereich:
			handle_com_calibration_messbereich(com_data, calibration, adc_data);
			break;
		
		case com_calibration_first_ref:
			handle_com_calibration_ref(com_data,calibration,adc_data,1);	// erster Punkt
			break;
		
		case com_calibration_first_value:
			handle_com_calibration_value(com_data,calibration, 1);
			break;
		
		case com_calibration_second_ref:
			handle_com_calibration_ref(com_data,calibration,adc_data,2);	// zweiter Punkt
			break;
		
		case com_calibration_second_value:
			handle_com_calibration_value(com_data,calibration, 2);
			break;
		
		case com_calibration_set_wavelength:
			handle_com_calibration_set_wavelength(com_data, calibration);
			handle_save_com_calibration(calibration,adc_data);
			break;
	}
	
	
}

void handle_com_show_calibration(usart_data_t* com_data)
/*	handle für die Anzeige in der Konsole während der Kalibrierung, da sonst
	handle_com_output zu unübersichtlich werden würde.
*/
{
	
		switch(com_data->subcommand_calibration)
		{
			
			case com_calibration_messbereich:
			sendUSARTs("0.\t Messbereich bis 2uW \n");
			sendUSART(0x0D);
			sendUSARTs("1.\t Messbereich bis 10mW \n");
			sendUSART(0x0D);
			sendUSARTs("Messbereich >> ");
			break;
			
			case com_calibration_first_ref:
			 	sendUSARTs("Press 'r' when ready to save first reference. >> ");
				break;
			
			case com_calibration_first_value:
				sendUSARTs("Enter corresponding power in pW >>");
				break;
			
			case com_calibration_second_ref:
				sendUSARTs("Press 'r' when ready to save second reference. >> ");
				break;
			
			case com_calibration_second_value:
				sendUSARTs("Enter corresponding power in pW >>");
				break;
			
			case com_calibration_set_wavelength:
				sendUSARTs("Set wavelength in nm >>");
				break;
		}
	
}

/* Unterprogramme dür die Kalibrierung */

void handle_com_calibration_messbereich(usart_data_t* com_data, new_calibration_t* calibration, adc_data_t* adc_data)
{
	int8_t temp;
	int8_t anz = 0;

	anz = sscanf(rx_buffer.buffer,"%d", &temp );
	clear_rx_buffer();
	
	if((anz < 0) || (temp < 0) || (temp > 1))	// Fehlerbehandlung von sscanf
	{
		sendUSARTs("invalid choice #3\n");
		sendUSART(0x0D);
		com_data->command = com_wait;
		com_data->subcommand_calibration = com_calibration_messbereich;
		return;
	}
	
	adc_data->range = manual;	// Messbereich kann nicht mehr automatisch verstellt werden.
	adc_data->new_MB = temp;	// Messbereich einstellen
	calibration->messbereich = temp;

 	com_data->subcommand_calibration = com_calibration_first_ref;
		
}

void handle_com_calibration_ref(usart_data_t* com_data, new_calibration_t* calibration, adc_data_t* adc_data, uint8_t point)
{
	int8_t temp;
	int8_t anz = 0;

	anz = sscanf(rx_buffer.buffer,"%c", &temp );
	clear_rx_buffer();
	
	if((anz < 0) || (temp != 'r'))	// Fehlerbehandlung von sscanf
	{
		sendUSARTs("invalid choice #4\n");
		sendUSART(0x0D);
		com_data->command = com_wait;
		com_data->subcommand_calibration = com_calibration_messbereich;
		return;
	}

		calibration->new_adc_point[point-1] = adc_data->adc_result;
	
	if(point == 1) 
	{

		com_data->subcommand_calibration = com_calibration_first_value;
	}
	else if(point == 2)
	{
		com_data->subcommand_calibration = com_calibration_second_value;
	}
	
		
}


void handle_com_calibration_value(usart_data_t* com_data, new_calibration_t* calibration, uint8_t point)
{
	uint32_t temp;
	int8_t anz = 0;

	anz = sscanf(rx_buffer.buffer,"%ld", &temp );
	clear_rx_buffer();
	
	if(anz < 0)	// Fehlerbehandlung von sscanf
	{
		sendUSARTs("invalid value #5\n");
		sendUSART(0x0D);
		com_data->command = com_wait;
		com_data->subcommand_calibration = com_calibration_messbereich;
		return;
	}
	
	calibration->new_ref_point[point-1] = ((float)temp)*1e-12;
	
	if(point == 1)
	{
		com_data->subcommand_calibration = com_calibration_second_ref;	
	}
	else if(point == 2)
	{
		com_data->subcommand_calibration = com_calibration_set_wavelength;
	}		
		
	
	
}

void handle_com_calibration_set_wavelength(usart_data_t* com_data, new_calibration_t* calibration)
{
	uint16_t temp;
	int8_t anz = 0;

	anz = sscanf(rx_buffer.buffer,"%d", &temp );
	clear_rx_buffer();
	
	if(anz < 0)	// Fehlerbehandlung von sscanf
	{
		sendUSARTs("invalid value #6\n");
		sendUSART(0x0D);
		com_data->command = com_wait;
		com_data->subcommand_calibration = com_calibration_messbereich;
		return;
	}
	
	calibration->wavelength = temp;
	
	com_data->command = com_wait;
	com_data->subcommand_calibration = com_calibration_messbereich;
}




void handle_save_com_calibration(new_calibration_t* calibration, adc_data_t* adc_data)
{
	calculate_intercept_slope(adc_data, calibration);
	save_new_calibration(adc_data,calibration);	// save to EEPROM	
	
	adc_data->range = automatic;	// Messbereich wieder automatisch an Messgröße anpassen
	
	
	sendUSARTs("calibration done...\n");
	sendUSART(0x0D);
}




/*********************************************************************************/
/* USART-Kommunikation Sendefunktionen */
/*********************************************************************************/ 


void sendUSART(uint8_t data)
/*
 *	Description: 
 *	Funktion sendet ein einzelnes Byte per serieller Schnittstelle
 *  
 */
{
	while(1)
	{
		if((USARTD0.STATUS & USART_DREIF_bm) != 0)
		{
			USARTD0.DATA = data;
			break;
		}
	}
	
} // end sendUSART.





void sendUSARTs(char* field)	
/*
 *	Description: - sendUSARTs (string)
 *	
 *	Eingabe: - zeiger auf ein Feld von chars (8 bit breit), muss '\0' terminiert sein 			 
 *	
 *	sendet die Elemente des Feldes der Reihe nach über die UART Schnittstelle
 *	verwendet polling, wartet bis das byte gesendet wurde bevor das nächste Byte gesendet wird.
 *	 
 */
	
{
	uint8_t i;
	uint8_t length = 0;
	uint8_t max_length = 100;			// längste Zeichenkette, die in einer Zeile ausgegeben werden kann.
	
	while( length < max_length ) {
		
		if(field[length] == '\0') {
			break;
		}
		length ++;
	}
	
		
	for(i=0; i<length; i++)
	{
		while(1) 
		{
			if((USARTD0.STATUS & USART_DREIF_bm) != 0)
			{
				USARTD0.DATA = *(field + i);
				break;
			}
		}
	}
	
} //end sendUARTs.



void sendUSARTb(uint8_t value) 
/*
 *	Description - USARTb (binary)
 *		
 *	Funktion sendet ein Byte binär formattiert ('xxxx xxxx') + neue Zeile + eine Zeile auslassen
 *  Funktion eignet sich, um Register auf der seriellen Schnittstelle für debug Zwecke auszugeben 
 */

{
	unsigned char i = 0;
	uint8_t temp = 0;
	
	for(i=8 ; i>4 ; i--) {
			
		temp = value & (1 << (i-1));
		
		if(temp == 0) {
			 sendUSART('0');
		}
		else {
			sendUSART('1');
		}
				
	}
	
	sendUSART(' ');		// i = 4
	
	for(i=4 ; i > 0 ; i--) {
		
		temp = value & (1 << (i-1));
		
		if(temp == 0) {
			sendUSART('0');
		}
		else {
			sendUSART('1');
		}
	}
	

		sendUSART(0x0A);		//neue Zeile
		sendUSART(0x0A);		//neue Zeile
		sendUSART(0x0D);
	

	
} //end sendUSARTb.




void sendUSARTb16(int value) 
/*
 *	Description - USARTb16 (binary 16 bit)
 *		
 *	Funktion sendet einen 16 Bit breiten Wert binär formattiert
 *  Hilfsfunktion zum Debuggen
 */

{
	unsigned char i = 0;
	int temp = 0;
	
	for(i=16 ; i>0 ; i--) {
			
		temp = value & (1 << (i-1));
		
		if(temp == 0) {
			 sendUSART('0');
		}
		else {
			sendUSART('1');
		}
				
	}	

// 		sendUSART(0x0A);		//neue Zeile
// 		sendUSART(0x0A);		//neue Zeile
// 		sendUSART(0x0D);
	

	
} //end sendUSARTb16.