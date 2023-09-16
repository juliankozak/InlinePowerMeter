// menu.c


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
#include "menu.h"
#include "calibration.h"

extern calibration_data_t calibrations[10];
extern uint8_t last_calibration;

#if GET_KENNLINIE
extern adc_control_t adc_control;
#endif

extern volatile enum button_e pushed_button;

extern volatile uint8_t background_light;
extern volatile uint16_t background_light_last_timestamp;
extern volatile uint16_t timestamp;



void ini_menu(menu_t* menu)
{
	menu->menu = menu_measure;
	menu->state_measure = measure_start;
	menu->state_settings = settings_start;
	menu->state_calibration = calibration_start;
} // end ini_menu()





void main_menu (adc_data_t* adc_data, menu_t* menu_data, new_calibration_t* calibration)
{
	
	/* 
		wenn eine Kennlinie aufgenommen werden soll, ist die Tastenbelegung anders. (vgl. Kommentar in common.h)  
	*/
	
	#if GET_KENNLINIE			// nicht interruptgesteuert, direkt die pins abfragen! Tasten müssen länger gedrückt werden.
	
		if(BUTTON_UP == PUSHED){
			adc_data->new_MB = 0;
		}
		else if (BUTTON_DOWN == PUSHED) {
			adc_data->new_MB = 4;			// wegen Bitposition von MB
		}
	
	
	
		if(BUTTON_RIGHT == PUSHED) {				// Wenn die rechteste Taste gedrückt wird, sollen der ADC 10 Mal samplen.

		if (adc_control.adc_status == adc_stop) {
			DEBUG_1_ON;
			_delay_us(10);
			DEBUG_1_OFF;
			adc_control.counter = 10;
			adc_control.adc_status = adc_sample;
		}
	}
	#else



	if(pushed_button ==  no_button) {
		return;
	}



	switch(menu_data->menu)
	{
		case menu_measure:
		submenu_measure(adc_data, menu_data);
		break;
	
		case menu_settings:
		submenu_settings(adc_data, menu_data);
		break;
	
		case menu_calibration:
		submenu_calibration(adc_data, menu_data, calibration);
		break;

	}


#endif
} // end main_menu()






void submenu_measure(adc_data_t* adc_data, menu_t* menu_data) 
{
		switch (menu_data->state_measure){
			
			case measure_start:
				if(pushed_button == button_right) {
					pushed_button = no_button;
					menu_data->menu = menu_settings;
				}
				else if(pushed_button == button_down){
					pushed_button = no_button;
					menu_data->state_measure = measure_value;
				}
				break;
				
			case measure_value:
				if(pushed_button == button_up){
					pushed_button = no_button;
					menu_data->state_measure = measure_start;
				}
				break;
			
		}
	
	
} // end submenu_measure()







void submenu_settings(adc_data_t* adc_data, menu_t* menu_data)
{
	
		switch (menu_data->state_settings){
			
			case settings_start:
				if(pushed_button == button_right) 
				{
					pushed_button = no_button;
					menu_data->menu = menu_calibration;
				}
				else if (pushed_button == button_left)
				{
					pushed_button = no_button;
					menu_data->menu = menu_measure;
				}
				else if(pushed_button == button_down) 
				{
					pushed_button = no_button;
					menu_data->state_settings = settings_display_saved_calibrations;
					adc_data->eeprom_entry_position = 0;
				}
				break;
				
			case settings_display_saved_calibrations:
				if(pushed_button == button_right)
				{
					pushed_button = no_button;
					if(adc_data->eeprom_entry_position < MEMORY_SIZE - 1)
					{
						adc_data->eeprom_entry_position ++;
					}
				}
				else if(pushed_button == button_left)
				{
					pushed_button = no_button;
					if(adc_data->eeprom_entry_position > 0)
					{
						adc_data->eeprom_entry_position --;
					}					
				}
				else if(pushed_button == button_up)
				{
					pushed_button = no_button;
					menu_data->state_settings = settings_start;
				}
				else if(pushed_button == button_down)
				{
					pushed_button = no_button;
					menu_data->state_settings = settings_delete_entry;
				}	
				else if(pushed_button == button_confirm)
				{
					pushed_button = no_button;
					load_calibration(adc_data, adc_data->eeprom_entry_position);
					eeprom_write_byte(&last_calibration,adc_data->eeprom_entry_position);
					/*last_calibration = adc_data->eeprom_entry_position;*/
					
					menu_data->state_settings = settings_start;
				#if 0	
					sendUSARTs("settings->load calibration, last calibration = ");
					sendUSARTb(last_calibration);
					sendUSARTs("\n");
					sendUSART(0x0D);		// an Zeilenbeginn springen
				#endif
					
				}		
				break;
		
			case settings_delete_entry:
				if(pushed_button == button_right)
				{
					pushed_button = no_button;
					if(adc_data->eeprom_entry_position < MEMORY_SIZE - 1)
					{
						adc_data->eeprom_entry_position ++;
					}
				}
				else if(pushed_button == button_left)
				{
					pushed_button = no_button;
					if(adc_data->eeprom_entry_position > 0)
					{
						adc_data->eeprom_entry_position --;
					}
				}
				else if(pushed_button == button_up)
				{
					pushed_button = no_button;
					menu_data->state_settings = settings_display_saved_calibrations;
				}
				else if (pushed_button == button_confirm)
				{
					pushed_button = no_button;
					delete_calibration(adc_data->eeprom_entry_position);	
				}	
				else if(pushed_button == button_down)
				{
					pushed_button = no_button;
					menu_data->state_settings = settings_background_light;
				}				
				break;
			
			case settings_background_light:
				if(pushed_button == button_up)
				{
					pushed_button = no_button;
					menu_data->state_settings = settings_delete_entry;									
				}
				else if (pushed_button == button_right)
				{
					pushed_button = no_button;
					LCD_LIGHT_ON();
					cli();
					background_light_last_timestamp = timestamp;
					sei();
					background_light = 1;
				}
				else if(pushed_button == button_left)
				{
					pushed_button = no_button;
					LCD_LIGHT_OFF();
					background_light = 0;
				}
				else if(pushed_button == button_down)
				{
					pushed_button = no_button;
					menu_data->state_settings = settings_unit;
				}
				break;
				
			case settings_unit:
				if(pushed_button == button_up)
				{
					pushed_button = no_button;
					menu_data->state_settings = settings_background_light;
				}
				else if(pushed_button == button_left)
				{
					pushed_button = no_button;
					// linear:
					adc_data->scale = lin;
					menu_data->state_settings=settings_start;					
				}
				else if(pushed_button == button_right)
				{
					pushed_button = no_button;
					//dB:
					adc_data->scale = dB;
					menu_data->state_settings=settings_start;
				}
			
		}
	
	
} // end submenu_settings()





void submenu_calibration(adc_data_t* adc_data, menu_t* menu_data, new_calibration_t* calibration)
{
	switch (menu_data->state_calibration)
	{
		
		case calibration_start:
			if(pushed_button == button_left) {
				pushed_button = no_button;
				menu_data->menu = menu_settings;
			}
			if(pushed_button == button_down) {
				pushed_button = no_button;
				menu_data->state_calibration = calibration_confirm;
			}
			if(pushed_button == button_right) {
				pushed_button = no_button;							// Tastendruck ignorieren und Flag löschen
			}
			break;
			
		case calibration_confirm:
			if(pushed_button == button_up) 
			{
				pushed_button = no_button;
				menu_data->state_calibration = calibration_start;
			}
			if(pushed_button == button_confirm)
			{
				pushed_button = no_button;
				adc_data->range = manual;	// der Messbereich soll bis zum Ende der Kalibrierung nicht mehr automatisch umgeschaltet werden. 
				menu_data->state_calibration = calibration_messbereich;
			}
			else 
			{								
				pushed_button = no_button;			
			}
			break;
			
		case calibration_messbereich:
			if(pushed_button == button_right) 
			{
				pushed_button = no_button;
				calibration->messbereich = 1;
				adc_data->new_MB = 1;	// Messbereich umschalten
			}
			if (pushed_button == button_left) 
			{
				pushed_button = no_button;
				calibration->messbereich = 0;
				adc_data->new_MB = 0;	// Messbereich umschalten
			}
			if(pushed_button == button_up) {
				pushed_button = no_button;
				menu_data->state_calibration = calibration_start;		// doch noch abbrechen können..
			}
			if(pushed_button ==  button_down) {
				pushed_button = no_button;
				menu_data->state_calibration = calibration_first_reference;
			}
			break;
	
	
		case calibration_first_reference:
			if(pushed_button != no_button)
			{
				pushed_button = no_button;
			
				reset_calibration_buffer(calibration, set_first_point);
				calibration->new_adc_point[0] = adc_data->adc_result;	// adc ergebnis abspeichern
				menu_data->state_calibration = calibration_set_first_point;
			}
			break;
	
	
		case calibration_set_first_point:		// Referenzwert eintippen
			handle_calibration_input(calibration, menu_data);
			break;
	
		case calibration_second_reference:
			if(pushed_button != no_button)
			{
				pushed_button = no_button;
				reset_calibration_buffer(calibration, set_second_point);
				calibration->new_adc_point[1] = adc_data->adc_result;		 // adc ergebnis abspeichern
				menu_data->state_calibration = calibration_set_second_point;
			}
			break;
	
		case calibration_set_second_point:		// zweiten Referenzwert eintippen
			handle_calibration_input(calibration, menu_data);
			break;		// nach 'confirm' wird im handle_calibration_input() automatisch auf set_wavelength gestellt und der buffer geleert
	
		case calibration_set_wavelength:
			handle_calibration_input(calibration, menu_data);
			break;
		
		
		case calibration_done:
			if(pushed_button != no_button )
			{
				pushed_button = no_button;
			
				/* alles in adc_data abspeichern..
				   der struct calibration diente nur als zwischenspeicher  
				*/				
				calculate_intercept_slope(adc_data,calibration);	// hier wird auch in adc_data gespeichert
				
				save_new_calibration(adc_data, calibration);		// save to EEPROM
			
				adc_data->range = automatic;	// Messbereiche wieder automatisch auswählen lassen.
			
				menu_data->state_calibration = calibration_start;	// Kalibrierung beendet, wieder an den Anfang springen.
			}
			break;
		
		
		default:
			menu_data->state_calibration = calibration_start;
	
	}
	
	
	
}




