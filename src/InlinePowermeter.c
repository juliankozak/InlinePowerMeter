/*
 * main.c
 *
 * Created: 11.03.2013 13:07:20
 *  Author: juli-admin
 */ 


#define F_CPU 2000000UL


#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <stdint.h>
#include "common.h"
#include "inlinePowermeter.h"
#include "timestamp.h"
#include "menu.h"
#include "adc.h"
#include "usart.h"
#include "button.h"
#include "debug.h"
#include "display.h"
#include "calibration.h"

#if DEBUG_LED
	#include "debug.h"
#endif




/* *** GLOBALE VARIABLEN: ***/

volatile uint16_t timestamp;

volatile enum button_e pushed_button = no_button;
volatile uint16_t timestamp_button_entprellen;

volatile uint8_t background_light = 0;
volatile uint16_t background_light_last_timestamp; 

	/* USART Empfangsbuffer */
volatile rx_buffer_t rx_buffer = {0};

	/* EEPROM */
calibration_data_t calibrations[MEMORY_SIZE] EEMEM;
uint8_t last_calibration EEMEM;		// letzte ausgewählte Kalibrierung speichern, damit diese beim Neustart eingestellt werden kann

#if	GET_KENNLINIE
	adc_control_t adc_control = {0};
#endif



/* *** INTERRUPT ROUTINEN ***/

ISR(TCC0_OVF_vect)		// Timestamp inkrementieren - interrupt (inkl. Tasten Entprellung)
{		
	
	timestamp ++;

	// Tasten entprellen:
	if((pushed_button != no_button) && (timestamp - timestamp_button_entprellen > 200 )) 
	{		
		PORTC_INTFLAGS |= 0x01;		// button Interrupt Flag löschen
	}	
	
	// Backgroundlight abdrehen:
	if((timestamp - background_light_last_timestamp) > 60000 && background_light == 1)
	{
		LCD_LIGHT_OFF();
	}
}

ISR(USARTD0_RXC_vect)		// USART Receive complete interrupt (medium priority)
{

	if(rx_buffer.buffer_ready != 0)
	{
		rx_buffer.buffer_temp = USARTD0_DATA;		// Daten verwerfen wenn der Buffer noch nicht geleert wurde	
		return;	
	}
	
	if(rx_buffer.buffer_position < BUFFER_LENGTH) 
	{
		rx_buffer.buffer[rx_buffer.buffer_position] = USARTD0_DATA;
		
		if(rx_buffer.buffer[rx_buffer.buffer_position] == 0x0D)	// auf CR der Eingabe warten. CR = 0x0D ASCII
		{
			rx_buffer.buffer_ready = 1;
		}
		rx_buffer.buffer_position ++;
	}
	
	if(rx_buffer.buffer_position == BUFFER_LENGTH)
	{
		rx_buffer.buffer[BUFFER_LENGTH-1] = 0x0D;	// CR an die letzte Stelle schreiben, damit sich das Programm nicht aufhängt und der buffer geleert werden kann
		rx_buffer.buffer_ready = 1;
	}
	
	
	USARTD0_STATUS |= USART_RXCIF_bm;	// Interrupt Flag löschen
}


ISR(PORTC_INT0_vect)		// button (low priority)
{
	cli();
	timestamp_button_entprellen = timestamp;	//in 100ms soll das Interrupt Flag vom Prozessor gelöscht werden (vgl. ISR Timer)
	
	
	if(background_light == 1)
	{
		background_light_last_timestamp = timestamp;	
	}
	sei();
	
	
	
	if ((BUTTON_RIGHT == PUSHED) && (BUTTON_LEFT == PUSHED))
	{
		pushed_button = button_confirm;
	}
	else if(BUTTON_UP == PUSHED)
	{						
		pushed_button = button_up;
	}	
	else if (BUTTON_DOWN == PUSHED)
	{
		pushed_button = button_down;
	}		
	else if (BUTTON_LEFT == PUSHED)
	{	
		pushed_button = button_left;
	}	
	else if (BUTTON_RIGHT == PUSHED)
	{			
		pushed_button = button_right;
	}
	
	// Licht wieder aufdrehen:
	if(LCD_LIGHT_STATUS == 0 && background_light == 1)
	{
		DEBUG_1_ON;
		LCD_LIGHT_ON();
		pushed_button = no_button;		// Tastendruck verwerfen.				
		DEBUG_1_OFF;
	}

}




/* *** MAIN ***/

int main(void)
{
	adc_data_t adc_data = {0};
		adc_data.new_MB = 0;	// Achtung auf Bitposition MB
	usart_data_t usart_data = {0};
	button_data_t button_data = {0};
	display_data_t display_data = {0};
	new_calibration_t new_calibration = {0}; // wenn eine neue Kalibrierung durchgeführt wird, werden die Zwischenergebnisse der Eingabe hier gespeichert
	new_calibration_t new_com_calibration = {0}; // eigenen Zwischenspeicher für die Kalibrierung per serieller Schnittstelle damit die Werte durch 
												 // Tastendruck nicht verändert/zerstört werden können.
		
	menu_t menu_data = {0};	
	
	#if DEBUG_LED
		ini_LED();	
		DEBUG_1_OFF;
	#endif	
	
	ini_timestamp();
	ini_usart();
	ini_adc(&adc_data);
	ini_button();
	ini_display(&display_data);
	ini_menu(&menu_data);

	
    while(1)
    {
			if(check_timing(adc_data.last_call, 100))
			{
				adc_data.last_call = timestamp;					
				adc(&adc_data);
			}

			if(check_timing(button_data.last_call, 250 ))
			{
				button_data.last_call = timestamp;
				main_menu(&adc_data, &menu_data, &new_calibration);
			}
		
			if(check_timing(display_data.last_call, 250))
			{
				display_data.last_call = timestamp;
				lcd(&menu_data, &adc_data, &display_data, &new_calibration);				
			}
	
			if(check_timing(usart_data.last_call, 250))
			{
				usart_data.last_call = timestamp;
				usart(&usart_data, &adc_data, &new_com_calibration);	
			}				
        

   }
	
	
	
} //end main.