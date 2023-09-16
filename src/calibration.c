// calibration.c

#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <stdint.h>
#include <stdio.h>
#include "common.h"
#include "inlinePowermeter.h"
#include "menu.h"
#include "adc.h"
#include "usart.h"
#include "button.h"
#include "debug.h"
#include "calibration.h"
#include "display.h"

extern volatile enum button_e pushed_button;
extern calibration_data_t calibrations[10];
extern uint8_t last_calibration;


void reset_calibration_buffer(new_calibration_t* calibration, uint8_t setting)
{																			// setting mit enum-Wert überschreiben = nicht sauber!? 
		
		calibration->cursor_position = 0;
		
		switch(setting)
		{
			case set_first_point:
				/* Startwert der Eingabe: 100.000 nW  */
				calibration->new_value[0] = 1;		
				calibration->new_value[1] = 0; 
				calibration->new_value[2] = 0;		
				calibration->new_value[3] = 10;		// '.'  = Kommastelle an Position [3], deshalb ungültige Ziffer
				calibration->new_value[4] = 0; 
				calibration->new_value[5] = 0; 
				calibration->new_value[6] = 0;
				calibration->new_value_exponent = nW;
				
				calibration->setting = set_first_point;
				break;
			
			case set_second_point:
				/* Startwert der Eingabe: 001.000 uW  */
				calibration->new_value[0] = 0;
				calibration->new_value[1] = 0;
				calibration->new_value[2] = 1;
				calibration->new_value[3] = 10;		// '.'  = Kommastelle an Position [3], deshalb ungültige Ziffer
				calibration->new_value[4] = 0;
				calibration->new_value[5] = 0;
				calibration->new_value[6] = 0;
				calibration->new_value_exponent = uW;
				
				calibration->setting = set_second_point;
				break;
			
			case set_wavelength:
				/* Startwert der Eingabe: 600 nm  */
				calibration->new_value[0] = 6;
				calibration->new_value[1] = 0;
				calibration->new_value[2] = 0;
				calibration->new_value[3] = 10;		// es werden nur die 3 ersten Stellen benötigt
				calibration->new_value[4] = 10;
				calibration->new_value[5] = 10;
				calibration->new_value[6] = 10;
				calibration->new_value_exponent = uW;	// wird hier nicht beachtet. 
								
				calibration->setting = set_wavelength;
				break;
		}
		
		
		
} // end reset_calibration_buffer()





void handle_calibration_input(new_calibration_t* calibration, menu_t* menu_data)
/*	diese Funktion steuert das 'eintipseln' des Referenzwertes
 *
 *	Anzeigeformat: xxx.xxx uW 
 *	mit den Taste <> kann die Ziffer ausgewählt werden
 *	mit up/down wird der Wert der aktuellen Ziffer verändert
 *
 *	Aufbau: das Feld new_value speichert die einzelnen Ziffern, am Index [0] die höchstwertigste, an der Stelle [3] befindet sich der Dezimalpunkt
*/
{
	
	/*** TEIL 1 der Funktion -> eine Referenzleistung eingeben ***/
	
	if((calibration->setting == set_first_point) || (calibration->setting == set_second_point))
	{	
		
		if(pushed_button == button_confirm)
		{
			pushed_button = no_button;
			
			if(calibration->setting == set_first_point)
			{
				calculate_new_value(calibration);
				
				menu_data->state_calibration = calibration_second_reference;
			}
			else if(calibration->setting == set_second_point)
			{
				calculate_new_value(calibration);
				reset_calibration_buffer(calibration, set_wavelength);
				
				menu_data->state_calibration = calibration_set_wavelength;
			}
			else if(calibration->setting == set_wavelength)
			{
				calculate_new_wavelength(calibration);
				
				menu_data->state_calibration = calibration_done;
			}

		}
	
		if (pushed_button == button_right){
			pushed_button = no_button;
			if(calibration->cursor_position < 7 && calibration->cursor_position != 2){
				calibration->cursor_position = calibration->cursor_position + 1;
			}				
			else if(calibration->cursor_position == 2) {
				calibration->cursor_position = 4;
			}				
		}
	
		if (pushed_button == button_left) {
			pushed_button = no_button;
			if(calibration->cursor_position > 0 && calibration->cursor_position != 4){
				calibration->cursor_position = calibration->cursor_position - 1;
			}				
			else if(calibration->cursor_position == 4) {
				calibration->cursor_position = 2;
			}				
		
		}
	
		if(pushed_button == button_up){
			pushed_button = no_button;
			if(calibration->cursor_position < 7){
				calibration->new_value[calibration->cursor_position] = inc_10(calibration->new_value[calibration->cursor_position]);
			}
			else if (calibration->cursor_position == 7){
				calibration->new_value[calibration->cursor_position] = inc_4(calibration->new_value[calibration->cursor_position]);
				calibration->new_value_exponent = calibration->new_value[calibration->cursor_position];
			}
		
		}
	
		if(pushed_button == button_down) {
			pushed_button = no_button;
			if(calibration->cursor_position < 7){
				calibration->new_value[calibration->cursor_position] = dec_10(calibration->new_value[calibration->cursor_position]);
			}
			else if (calibration->cursor_position == 7){
				calibration->new_value[calibration->cursor_position] = dec_4(calibration->new_value[calibration->cursor_position]);
				calibration->new_value_exponent = calibration->new_value[calibration->cursor_position];
			}
		}
	
	}
	
	
	/*** TEIL 2 der Funktion -> die Wellenlänge eingeben ***/
	/* der Ablauf ist sehr ähnlich, allerdings werden nur die 3 ersten Einträge im Feld new_value verwendet */
	/* Format '675 nm' , die Einheit kann nicht verändert werden */
	
	else if (calibration->setting == set_wavelength)
	{
			if(pushed_button == button_confirm){
				pushed_button = no_button;
				calculate_new_wavelength(calibration);
				
				menu_data->state_calibration = calibration_done;
			}
			if(pushed_button == button_right){
				pushed_button = no_button;
				if(calibration->cursor_position < 2) {
					calibration->cursor_position = calibration->cursor_position + 1;
				}
			}
				
			else if(pushed_button == button_left){
				pushed_button = no_button;
				if(calibration->cursor_position > 0) {
					calibration->cursor_position = calibration->cursor_position - 1;
				}
			}
			else if(pushed_button == button_up) {
				pushed_button = no_button;
				calibration->new_value[calibration->cursor_position] = inc_10(calibration->new_value[calibration->cursor_position]);
			}
				
			else if(pushed_button == button_down) {
				pushed_button = no_button;
				calibration->new_value[calibration->cursor_position] = dec_10(calibration->new_value[calibration->cursor_position]);
			}


	}	
	
} // end handle_calibration_input()




void calculate_new_value(new_calibration_t* calibration){
/*
	Funktion berechnet aus dem 'eingetispelten' Referenzwert (im Feld new_value stehen die einzelnen Ziffern) den
	wirklichen Referenzwert als float, mit welchem dann die 2 Punkt Kalibrierung durchgeführt wird. 
	
*/
	
	float result = 0;
	uint32_t int_result = 0;
	
	int_result += calibration->new_value[6];
	int_result += calibration->new_value[5]*10;
	int_result += calibration->new_value[4]*100;
	int_result += calibration->new_value[2]*1000;
	int_result += calibration->new_value[1]*10000;
	int_result += calibration->new_value[0]*100000;
	

	// 1e-3: Dezimalpunkt erst jetzt einbeziehen, damit die vorigen Berechnungen ganzzahlig durchgeführt werden können.
	if(calibration->new_value_exponent == pW)
		result = (uint32_t)int_result * 1e-3 * 1e-12;
	else if(calibration->new_value_exponent == nW)
		result = (uint32_t)int_result * 1e-3 * 1e-9;
	else if(calibration->new_value_exponent == uW)
		result = (uint32_t)int_result * 1e-3 * 1e-6;
	else if(calibration->new_value_exponent == mW)
		result = (uint32_t)int_result * 1e-3 * 1e-3;
		
		
	if(calibration->setting == set_first_point)
	{
		calibration->new_ref_point[0] = result;
	}
	else if(calibration->setting == set_second_point)
	{
		calibration->new_ref_point[1] = result;
	}	
		
	
	
	
	#if 0	// eingestellte Punkte überprüfen, auch in calculate_intercept_slope in adc.c ausführlicher!
		char text[11] = {0};
		snprintf(text,11,"%ld",(uint32_t)(result*1e12));	
		/*snprintf(text,11,"%ld",(uint32_t)(calibration->new_ref_point[0]*1e12));*/
		sendUSARTs(text);
		sendUSARTs("\n");
		sendUSART(0x0D);		// an Zeilenbeginn springen
	#endif	
} // end calculate_new_value()




void calculate_new_wavelength(new_calibration_t* calibration)
{
/*
	ähnlich wie calculate_new_value wird die der Wert der Wellenlänge aus den einzelnen Ziffern berechnet	
	
*/
	
	uint16_t wavelength = 0;
	
	wavelength = wavelength + calibration->new_value[0] * 100;
	wavelength = wavelength + calibration->new_value[1] * 10;
	wavelength = wavelength + calibration->new_value[2];
	
	calibration->wavelength = wavelength;
	
#if 0
	char text[4] = {0};
	snprintf(text,4, "%d", wavelength);
	sendUSARTs("wavelength calc : ");
	sendUSARTs(text);
	sendUSARTs("\n");
	sendUSART(0x0D);		// an Zeilenbeginn springen
#endif

}// end calculate new wavelength 



// ins EEPROM speichern:

void save_new_calibration(adc_data_t* adc, new_calibration_t* calibration) 
{
	
	calibration_data_t new_calibration = {0};	// wird als Block in den EEPROM geschireben (nachdem es befüllt wurde)
	calibration_data_t buffer;
	uint8_t i;
	int8_t position = -1;
	

	new_calibration.wavelength = adc->wavelength;


	/* einen bereits vorhandenen Eintrag mit derselben Wellenlänge suchen: */
	for(i=0; i<MEMORY_SIZE ; i++)
	{
		eeprom_read_block(&buffer, &calibrations[i], sizeof(calibration_data_t));
		if(buffer.wavelength == new_calibration.wavelength)
		{
			position = i; 
			break;
		}
	}

	/* wenn keiner gefunden, dann einen leeren Speicherplatz suchen */
	if(position < 0)
	{
		for(i=0; i < MEMORY_SIZE ; i++)
		{
			eeprom_read_block(&buffer, &calibrations[i], sizeof(calibration_data_t));
			if(buffer.wavelength == 0)
			{
				position = i;
				break;
			}
		}					
	}

	/* wenn kein Speicherplatz mehr frei ist -> Fehlermeldung und ohne Speichern abbrechen */
	if(position < 0)
	{	
		sendLCDs("no free memory!",1);
		return;
	}

	eeprom_read_block(&buffer, &calibrations[position], sizeof(calibration_data_t));	
			// den aktuellen Block auslesen, damit der andere MB nicht überschrieben wird
		
	if(calibration->messbereich == 0)							// aufpassen nicht beide Messbereiche zu überschreiben
	{																// obwohl nur einer kalibriert wurde!
		new_calibration.intercept_MB0 = adc->intercept_MB0;
		new_calibration.slope_MB0 = adc->slope_MB0;
	
		new_calibration.intercept_MB1 = buffer.intercept_MB1;
		new_calibration.slope_MB1 = buffer.slope_MB1;
	}
	else
	{
		new_calibration.intercept_MB1 = adc->intercept_MB1;
		new_calibration.slope_MB1 = adc->slope_MB1;
	
		new_calibration.intercept_MB0 = buffer.intercept_MB0;
		new_calibration.slope_MB0 = buffer.slope_MB0;
	}

	eeprom_write_block(&new_calibration, &calibrations[position], sizeof(calibration_data_t));
	
	eeprom_write_byte(&last_calibration,position);
	/*last_calibration = position;	*/
	

#if 0
	{
	char text[50] = {0};
	snprintf(text,50,"%ld\n", (uint32_t)(new_calibration.intercept_MB0*1e12));
	sendUSARTs("write intercept MB 0 to eeprom: ");
	sendUSARTs(text);
	sendUSART(0x0D);
	
	snprintf(text,50,"%ld\n", (uint32_t)(new_calibration.intercept_MB1*1e12));
	sendUSARTs("write intercept MB1 to eeprom: ");
	sendUSARTs(text);
	sendUSART(0x0D);

	snprintf(text,4, "%d", new_calibration.wavelength);
	sendUSARTs("write wavelength to eeprom : ");
	sendUSARTs(text);
	sendUSARTs("\n");
	sendUSART(0x0D);		// an Zeilenbeginn springen

	}
#endif
	
	
} // end save_new_calibration()