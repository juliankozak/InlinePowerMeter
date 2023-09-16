// display.c

#define F_CPU 2000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include "common.h"
#include "inlinePowermeter.h"
#include "debug.h"
#include "usart.h"
#include "menu.h"
#include "display.h"
#include <avr/eeprom.h>
#include "eeprom_memory.h"



extern calibration_data_t calibrations[10];
extern uint8_t last_calibration;


extern volatile uint8_t background_light;
extern volatile uint16_t background_light_last_timestamp;
extern volatile uint16_t timestamp;


void ini_display(display_data_t *display_data) {
	
	PORTA_DIR = 0xFF;
	PORTB_DIR = 0x07;
	
	SET_LCD_E();
	
	_delay_ms(15);
	PORTA_OUT = 0b00110000;
	CLEAR_LCD_RS();
	CLEAR_LCD_RW();
	CLEAR_LCD_E();		// neg. Flanke
	_delay_ms(1);
	SET_LCD_E();
	
	_delay_ms(5);
	PORTA_OUT = 0b00110000;
	CLEAR_LCD_E();
	_delay_ms(1);
	SET_LCD_E();
	
	_delay_ms(100);
	PORTA_OUT = 0b00110000;
	CLEAR_LCD_E();		//neg.  Flanke
	_delay_ms(1);
	SET_LCD_E();
	
	//RS,RW sind noch immer 0.
	PORTA_OUT = 0b00111000;		//SETTINGS: 8bit bus, 2 lines, 5x8 dots
	CLEAR_LCD_E();
	_delay_us(40);
	SET_LCD_E();
	
	PORTA_OUT = 0b00001000;		//DISPLAY OFF, cursor off, blink off
	CLEAR_LCD_E();
	_delay_us(40);
	SET_LCD_E();
	
	PORTA_OUT = 0b00000001;		//CLEAR DISPLAY 
	CLEAR_LCD_E();
	_delay_ms(2);
	SET_LCD_E();
	

	PORTA_OUT = 0b00000110;		//ENTRY MODE SET: DDRAM address auto increases by 1, shift off
	CLEAR_LCD_E();
	_delay_us(40);
	SET_LCD_E();
	
	PORTA_OUT = 0b00001100;		// DISPLAY ON, cursor off, blink off
	CLEAR_LCD_E();
	_delay_us(40);
	SET_LCD_E();
	
	/*
		comma_position_table:
		
		für die Anzeige des Ergebnisses mit 3 signifikanten Ziffern muss die Position des Dezimalpunktes aus der Anzahl
		der benötigten Ganzzahldivisionen, um die führende Null zu lokalisieren, bestimmt werden:
		
		Anzeige: 123 nW oder 32.4 nW oder 1.24 nW -> Komma an Stelle 0,1 oder 2
	*/
	 
	display_data->comma_position_table[0] = 0;
	display_data->comma_position_table[1] = 2;
	display_data->comma_position_table[2] = 1;
	display_data->comma_position_table[3] = 0;
	display_data->comma_position_table[4] = 2;
	display_data->comma_position_table[5] = 1;
	display_data->comma_position_table[6] = 0;
	display_data->comma_position_table[7] = 2;


	/* Display Beleuchtung: */
	
	PORTE_DIR |= 0x08;	
	LCD_LIGHT_ON();
	background_light = 1;
	cli();
	background_light_last_timestamp = timestamp;
	sei();
	
} // end ini_display()





void lcd(menu_t *menu_data, adc_data_t* adc_data, display_data_t* display_data, new_calibration_t* calibration_data) {
/*
	Hauptfunktion der Display-Anzeige.
	je nach Menü- und Untermenüposition wird ein bestimmter Text und ev. Wert angezeigt
*/
	
	
// MEASURE:	
	if(menu_data->menu == menu_measure) {
		if(menu_data->state_measure == measure_start) {
			sendLCDs("    MEASURE    >",1);
			sendLCDs(" ",2);
		}
		else if(menu_data->state_measure == measure_value) {
			
			if(adc_data->scale == lin)
			{
				display_result(display_data,adc_data);
			}
			else if(adc_data->scale == dB)
			{
				display_result_dBm(adc_data);
			}
			
		}
	}

// SETTINGS:
	else if(menu_data->menu == menu_settings) {
		if(menu_data->state_settings == settings_start) {
			sendLCDs("<   SETTINGS   >",1);
			sendLCDs(" ",2);
		}
		else if(menu_data->state_settings == settings_display_saved_calibrations)
		{
			sendLCDs("select:    ok:<>",1);
			display_saved_calibration(adc_data);
		}
		else if(menu_data->state_settings == settings_delete_entry)
		{
			sendLCDs("delete:   del=<>",1);
			display_saved_calibration(adc_data);
		}
		else if(menu_data->state_settings == settings_background_light)
		{
			sendLCDs("Background Light",1);
			sendLCDs("< OFF       ON >",2);
		}
		else if(menu_data->state_settings == settings_unit)
		{
			sendLCDs("     Scale      ",1);
			sendLCDs("< lin      log >",2);
		}
	}
	

// CALIBRATION:
	else if(menu_data->menu == menu_calibration) {
		if(menu_data->state_calibration == calibration_start) {
			sendLCDs("<  CALIBRATION  ",1);
			sendLCDs(" ",2);
		}
		else if(menu_data->state_calibration == calibration_confirm) {
			sendLCDs("new calibration",1);
			sendLCDs("  confirm < >",2);
			/*adc_data->new_mb_calibration = 0;*/		// per default MB0
		}
		else if(menu_data->state_calibration == calibration_messbereich) {
			sendLCDs(" Messbreich? ",1);
			if(calibration_data->messbereich == 0){
				sendLCDs("   bis  2uW    >",2);
			}
			else {
				sendLCDs("<  bis 10mW     ",2);
			}
		}
		else if(menu_data->state_calibration == calibration_first_reference) {
			sendLCDs("first reference ",1);
			sendLCDs("press any key",2);
		}
		else if(menu_data->state_calibration == calibration_set_first_point) {
			sendLCDs("set:       ok:<>",1);
			display_new_value(calibration_data);
			if(calibration_data->cursor_position == 7) {					//Leerzeichen vor Einheit auslassen
				blink_digit(calibration_data->cursor_position + 4 +1);		
			}
			else {
			blink_digit(calibration_data->cursor_position + 4);
			}			
		}
		else if(menu_data->state_calibration == calibration_second_reference) {
			blink_digit_off();
			sendLCDs("second point",1);
			sendLCDs("press any key",2);
		}
		else if(menu_data->state_calibration == calibration_set_second_point) {
			sendLCDs("set:       ok:<>",1);
			display_new_value(calibration_data);
			if(calibration_data->cursor_position == 7) {					//Leerzeichen vor Einheit auslassen
				blink_digit(calibration_data->cursor_position + 4 +1);
			}
			else {
				blink_digit(calibration_data->cursor_position + 4);
			}
		}
		else if(menu_data->state_calibration == calibration_set_wavelength) {
			sendLCDs("set wavelength:",1);
			display_new_wavelength(calibration_data);
			blink_digit(calibration_data->cursor_position);
		}
		else if(menu_data->state_calibration == calibration_done) {
			blink_digit_off();
			sendLCDs("calibration done...",1);
			sendLCDs("press any key",2);
		}
		
		
	}
	
} // end lcd()





void sendLCDs ( char* text, uint8_t line) {
/*
	sendLCDs "string" 
	zeigt einen string am Display an. Maximallänge, die angezeigt werden kann, ist 16 Zeichen. 
*/
	
	uint8_t i,j;
	
	SET_LCD_E();
	
	CLEAR_LCD_RS();
	CLEAR_LCD_RW();
	
	if(line == 1) 
		PORTA_OUT = 0b10000000;		// DDRAM Address
	else if(line == 2)
		PORTA_OUT = 0b11000000;		// DDRAM Address 	
	else 
		PORTA_OUT = 0b11010000;		// ungültige Adresse, die nicht angezeigt wird
	
	CLEAR_LCD_E();
	_delay_us(40);
	
	SET_LCD_E();
	SET_LCD_RS();		
	

	for(i=0; i<16;i++) {
		
		if(text[i] != '\0'){
			
			PORTA_OUT = text[i];
		}
		else {
			for(j=i; j <16; j++){
				PORTA_OUT = ' ';
				CLEAR_LCD_E();
				_delay_us(40);
				SET_LCD_E();		
			}
			break;				
		}
		CLEAR_LCD_E();
		_delay_us(40);
		SET_LCD_E();
	}
	
	
	
	
}// end sendLCDs()




void clearLCD() 
{
	SET_LCD_E();
	
	CLEAR_LCD_RS();
	CLEAR_LCD_RW();
	PORTA_OUT = 0x01;
	CLEAR_LCD_E();
	_delay_ms(1);
	_delay_us(600);
	SET_LCD_E();
	
} // end clearLCD()




void display_result(display_data_t* display_data, adc_data_t* adc_data) 
{ 
/* Anzeige der aktuell gemessenen Leistung am Display.
 *
 * die gemessene Leistung wird als uint32 Wert so skaliert abgespeichert, dass der Wert "1" einem Picowatt entspricht.
 *
 * angezeigt soll dieser Wert allergigs, je nach Ergebnis in folgenden Formaten
 * 123 nW oder 32.4 nW oder 1.24 nW, wobei die Einheit mW, uW, nW oder pW betragen kann
 *
 * braucht 5-20ms
 */	
	uint16_t formatted_result = 0;
	uint8_t i;
	char text [5] = {0};
	char text_ausgabe[11] = {0};
		uint8_t offset = 0;
	
	/* als erstes muss die Position der führenden Null gefunden werden und anschließend das Ergebnis als Wert zwischen 001 und 999
	 in formatted Result geschrieben werden. 
	 die Position der Kommastelle ist in der Tabelle comma_position_table festgelegt.
	*/
	for(i=3;i<20;i++) 
	{
		if((adc_data->p_opt[0])/((uint32_t)pow(10, i)) > 0 ) {
			continue;			
		}
		else {
			formatted_result = (uint32_t)((adc_data->p_opt[0]) / (uint32_t)pow(10,i-3)) ;	
			break;
		}		
	}
	
	
	
		
		
 	snprintf(text,5,"%3d",formatted_result);

	// Dezimalpunkt in den String hinzufügen:
	if(display_data->comma_position_table[i] == 1) {
		text[4] = text[3];
		text[3] = text[2];
		text[2] = '.';
	}
	else if(display_data->comma_position_table[i] == 2){
			text[4] = text[3];
			text[3] = text[2];
			text[2] = text[1];
			text[1] = '.';
	}
	
	text_ausgabe[0] = ' ';
	text_ausgabe[1] = 'P';
	text_ausgabe[2] = '=';
	text_ausgabe[3] = text[0];
	text_ausgabe[4] = text[1];
	text_ausgabe[5] = text[2];
	
	if(display_data->comma_position_table[i] != 0) {		// ein punkt wird zusätzlich angezeigt
		offset = 1;
		text_ausgabe[6] = text[3];
	}
				
	// Die Einheit anhängen 
	// (dabei muss berücksichtigt werden, dass das Ergebnis um eine Stelle kürzer ist, wenn kein Punkt angezeigt wird (123 nW) -> offset)
		
	if(i-3 == 0){										// i entspricht der Postion der führenden Null;
		text_ausgabe[6+offset] = ' ';					// auf die Position der letzten Ziffer zurückrechnen,
		text_ausgabe[7+offset] = 'p';					// bei 3 Ziffern Anzeigebreite -> i-3.
		text_ausgabe[8+offset] = 'W';
		
		
	}
	else if(i-3<4) {
		text_ausgabe[6+offset] = ' ';
		text_ausgabe[7+offset] = 'n';
		text_ausgabe[8+offset] = 'W';
	}
	else if(i-3<7) {
		text_ausgabe[6+offset] = ' ';
		text_ausgabe[7+offset] = 'u';
		text_ausgabe[8+offset] = 'W';
	}
	else if(i-3 == 7){
		text_ausgabe[6+offset] = ' ';
		text_ausgabe[7+offset] = 'm';
		text_ausgabe[8+offset] = 'W';
	}	
	text_ausgabe[9+offset] = '\0';	
	

	
	sendLCDs(text_ausgabe,1);
	
	
	// Wellenlänge in der 2.Zeile anzeigen
	snprintf(text_ausgabe,11," %d nm", adc_data->wavelength);
	sendLCDs(text_ausgabe,2); 		
					
	
} // end display_result()



void display_result_dBm(adc_data_t* adc_data)
{
	char text[16] = {0};
	int8_t vorkomma_zahl = 0;
	int8_t nachkomma_zahl = 0;
	
	char text_ausgabe[11] = {0};
	
	if(adc_data->p_opt_dBm < 0)
	{
		vorkomma_zahl = -adc_data->p_opt_dBm / 100;
		nachkomma_zahl = -adc_data->p_opt_dBm % 100;
		snprintf(text,11,"-%2d.%2d dBm",vorkomma_zahl,nachkomma_zahl);
	}
	else
	{
		vorkomma_zahl = adc_data->p_opt_dBm / 100;
		nachkomma_zahl = adc_data->p_opt_dBm % 100;
		snprintf(text,11," %2d.%2d dBm",vorkomma_zahl,nachkomma_zahl);		
	}
	
	sendLCDs(text,1);
	
	// Wellenlänge in der 2.Zeile anzeigen
	snprintf(text_ausgabe,11," %d nm", adc_data->wavelength);
	sendLCDs(text_ausgabe,2);
	
	
} // end display_result();





void display_new_value(new_calibration_t* calibration)
{
/*
	Anzeige des neu eingestellten Referenzwerts während der Kalibrierung
	
	in dem Feld new_value stehen die einzelnen Ziffern
*/
	
	char text[16] = {0};
	char text_unit_pW[3] = "pW";
	char text_unit_nW[3] = "nW";
	char text_unit_uW[3] = "uW";
	char text_unit_mW[3] = "mW";

	snprintf(text,13, "    %d%d%d.%d%d%d ",calibration->new_value[0],calibration->new_value[1],calibration->new_value[2],
	calibration->new_value[4],calibration->new_value[5],calibration->new_value[6]);
	if(calibration->new_value_exponent == pW)
	strncat(text,text_unit_pW,3);
	else if(calibration->new_value_exponent == nW)
	strncat(text,text_unit_nW,3);
	else if(calibration->new_value_exponent == uW)
	strncat(text,text_unit_uW,3);
	else if(calibration->new_value_exponent == mW)
	strncat(text,text_unit_mW,3);

	sendLCDs(text,2);
	
} // end display_new_value()





void display_new_wavelength(new_calibration_t* calibration) {
/*
	Anzeige der eingestellten Wellenlänge während der Kalibrierung
	
	es sind nur die 3 ersten Einträge von new_value benützt, da die Wellenlänge hier maximal 999 nm betragen kann.

*/	
	char text[17] = {0};
	char text_unit[] = " nm     ok:<>";
	snprintf(text,4,"%d%d%d", calibration->new_value[0],calibration->new_value[1],calibration->new_value[2]);
	strncat(text,text_unit,14);
	
	sendLCDs(text,2);
	
} // end display_new_wavelength();



void display_saved_calibration(adc_data_t* adc_data)
{
	/*
		
	*/
	calibration_data_t calibration = {0};
	char buffer[17] = {0};

	eeprom_read_block(&calibration, &calibrations[adc_data->eeprom_entry_position], sizeof(calibration_data_t));


	if(calibration.wavelength == 0)
	{
		if(adc_data->eeprom_entry_position < MEMORY_SIZE - 1)	// mit Pfeil nach rechts
		{
			snprintf(buffer,17,"%d. **empty**   ~", adc_data->eeprom_entry_position+1);
		}										// '~' = Pfeil nach rechts am display
		else  // ohne Pfeil nach rechts
		{
			snprintf(buffer,17,"%d. **empty** ", adc_data->eeprom_entry_position+1);
		}			
				
		
	}
	else
	{
		if(adc_data->eeprom_entry_position < MEMORY_SIZE -1) //mit Pfeil nach rechts
		{
			// Anzeigen, falls nicht beide Messbereiche (bei einer Wellenlänge)  kalibriert wurden! 
			
			if(calibration.slope_MB0 < 1e-5) // slope ist ein float und kann somit nicht auf Null abgefragt werden.  
			{
				snprintf(buffer,17,"%d. %d nm >2uW ~",adc_data->eeprom_entry_position+1, calibration.wavelength);
			}
			
			else if(calibration.slope_MB1 < 1e-5)		// slope ist ein float und kann somit nicht auf Null abgefragt werden.
			{
				snprintf(buffer,17,"%d. %d nm <2uW ~",adc_data->eeprom_entry_position+1, calibration.wavelength);
			}
			else
			{
				snprintf(buffer,17,"%d. %d nm      ~",adc_data->eeprom_entry_position+1, calibration.wavelength);
			}
			
			
		}
		else  // ohne Pfeil nach rechts
		{
			snprintf(buffer,17,"%d.  %d nm",adc_data->eeprom_entry_position+1, calibration.wavelength);
		}	
	}
	
	
	sendLCDs(buffer,2);
	
	
}




 














#if 1
void blink_digit(uint8_t position) {
	
	SET_LCD_E();
	CLEAR_LCD_RS();
	CLEAR_LCD_RW();

	PORTA_OUT = 0b11000000 + position;
	CLEAR_LCD_E();
	_delay_us(40);
	SET_LCD_E();
	
	PORTA_OUT = 0b00001110;
	CLEAR_LCD_E();
	_delay_us(40);
	SET_LCD_E();
}

void blink_digit_off() {
	SET_LCD_E();
	_delay_us(20);
	PORTA_OUT = 0b00001100;
	CLEAR_LCD_E();
	_delay_us(40);
	SET_LCD_E();
}

#endif




/* Hilfsfunktionen für die Kalibrierung, werden zum 'eintipseln' benötigt */

uint8_t inc_10(uint8_t input ) {
	
	if(input < 9) {
		input ++;
	}
	else{
		input = 0;
	}
	
	return input;
	
}


uint8_t inc_4(uint8_t input ) {
	
	if(input < 3) {
		input ++;
	}
	else{
		input = 0;
	}
	
	return input;
	
}


uint8_t dec_10(uint8_t input ) {
	
	if(input > 0) {
		input --;
	}
	else{
		input = 9;
	}
	
	return input;
	
}


uint8_t dec_4(uint8_t input ) {
	
	if(input > 0) {
		input --;
	}
	else{
		input = 3;
	}
	
	return input;
	
}



