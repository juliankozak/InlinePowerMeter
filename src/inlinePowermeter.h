/*
	INLINE POWERMETER
	inlinePowermeter.h
*/

#ifndef INLINEPOWERMETER
	#define INLINEPOWERMETER
	

	/*  ** LCD DISPLAY ** 
 
			D0:D7 = PORTA PA0:PA7
			E     = PB0
			RW    = PB1
			RS    = PB2						
			LIGHT = PE3	
	*/

		#define SET_LCD_E() PORTB_OUT |= 0x01
		#define CLEAR_LCD_E() PORTB_OUT &= ~0x01
	
		#define SET_LCD_RW() PORTB_OUT |= 0x02
		#define CLEAR_LCD_RW() PORTB_OUT &= ~0x02
	
		#define SET_LCD_RS() PORTB_OUT |= 0x04
		#define CLEAR_LCD_RS() PORTB_OUT &= ~0x04
	
		#define SET_LCD_LIGHT_ON() PORTE_OUT |= 0x04
		#define SET_LCD_LIGHT_OFF() PORTE_OUT &= ~0x04
	
	
	/*  ** Analog Verstärker - Messbereiche  **

			PE1 = 1 und PE2 = 0  => Messbereich 0 (klein)
			PE1 = 0 und PE2 = 1  => Messbereich 1 (groß)
		
			(PE1 = ~PE2)	
	*/	

		#define SET_MB_0() (PORTE_OUT = ((PORTE_OUT | 0x02) & 0xFB))
		#define SET_MB_1() (PORTE_OUT = ((PORTE_OUT | 0x04) & 0xFD))
		
		#define GET_MB() (PORTE_OUT & 0x04)			// für if Abfragen, PE2 prüfen
		
		#define LCD_LIGHT_ON() (PORTE_OUT |= 0x08)
		#define LCD_LIGHT_OFF() (PORTE_OUT &= 0xF7)
		#define LCD_LIGHT_STATUS (PORTE_OUT & (0x08))
		
				
		
	/*  ** Taster  (lo-aktiv)   **
			
			BUTTON UP    = PC5
			BUTTON DOWN  = PC4
			BUTTON LEFT  = PC6
			BUTTON RIGHT = PC7	
	*/	
	
		#define BUTTON_DOWN (PORTC_IN & PIN4_bm)
		#define BUTTON_UP (PORTC_IN & PIN5_bm)
		#define BUTTON_LEFT (PORTC_IN & PIN6_bm)
		#define BUTTON_RIGHT (PORTC_IN & PIN7_bm)
		
		#define PUSHED 0
		
	/* ** EEPROM Kalibrierung speichern:
	
	*/
	
		#define MEMORY_SIZE 10
		
	/* ** USART RX BUFFER:
	
	*/	
		#define BUFFER_LENGTH 25
		
	
	
	
/**********************************************************************************************
			*** DATENSTRUKTUREN: ***
**********************************************************************************************/	


typedef struct 
{	
		uint16_t last_call;
		uint16_t adc_result;
		uint32_t p_opt[10];
		int16_t p_opt_dBm;
		uint8_t  sample_number;
		enum 
		{	lin,
			dB
		} scale;
		enum 
		{	automatic,
			manual
		} range;		
		uint8_t new_MB;		// wenn der Messbereich geändert werden soll, den neuen MB in new_MB schreiben,
							// damit das aktuelle Ergebnis und der MB konsistent bleiben. 
		float intercept_MB0;
		float intercept_MB1;
		float slope_MB0;
		float slope_MB1;
		uint16_t wavelength;
		
		uint8_t eeprom_entry_position; // fürs durchblättern der vorhandenene Kalibrierungen im eeprom. 
		
} adc_data_t;


typedef struct 
{										// EEPROM Speicher-Block
		uint16_t wavelength;
		float intercept_MB0;
		float intercept_MB1;
		float slope_MB0;
		float slope_MB1;	
} calibration_data_t;

typedef struct				// Zwischenspeicher für die Erstellung einer neuen 2-Punkt-Kalibrierung
{
	uint8_t messbereich;
	enum								// welcher Messbereich soll kalibriert werden
	{	set_first_point,
		set_second_point,
		set_wavelength
	} setting;			
	uint8_t new_value[8];				// Feld mit Ziffern 0-9 um den neuen MW einzustellen + letzte ziffer symbol für den exponenten
	uint8_t cursor_position;
	enum 
	{	pW,
		nW,
		uW,
		mW
	} new_value_exponent;
	float new_ref_point[2];			// 2 referenzpunkte als float für die berechnung von intercept und slope
	uint16_t new_adc_point[2];		// die dazugehörigen adc codes abspeichern	(= 2 Punkte in der Kennline)
	uint16_t wavelength;
		
} new_calibration_t;



typedef struct 
{
		uint16_t last_call;
		uint8_t level;
		uint8_t background_light;	
		uint8_t comma_position_table[8];	
} display_data_t;



typedef struct 
{
		uint16_t last_call;
		uint16_t time_intervall;
		enum 
		{	stop,
			start
		} transmission;
		
		enum
		{
			com_wait,
			com_measure,
			com_set_wavelength,
			com_delete_calibration,
			com_calibrate,
			com_help			
		} command;
		
		enum
		{
			com_set_wavelength_start,
			com_set_wavelength_choose			
		}subcommand_set_wavelength;
		
		enum
		{
			com_delete_start,
			com_delete_choose
		}subcommand_delete;
		
		enum
		{			
			com_calibration_messbereich,
			com_calibration_first_ref,
			com_calibration_first_value,
			com_calibration_second_ref,
			com_calibration_second_value,
			com_calibration_set_wavelength	
		}subcommand_calibration;
		
		
		
}usart_data_t;	

typedef struct		// wird global angelegt.
{
	int8_t buffer[BUFFER_LENGTH];
	int8_t buffer_position;
	uint8_t buffer_ready;
	int8_t buffer_temp;
}rx_buffer_t;
	
	

typedef struct 
{
		uint16_t last_call;
} button_data_t;


enum button_e
{
	no_button,
	button_right,
	button_left,
	button_up,
	button_down,
	button_confirm			// links und rechts gleichzeitig gedrückt	
};

typedef struct
{
	enum
	{	menu_measure,
		menu_settings,
	//	menu_com_settings,
		menu_calibration,
	} menu;
	
	enum
	{	measure_start,
		measure_value
	} state_measure;
	
	enum
	{
		settings_start,
		settings_display_saved_calibrations,
		settings_delete_entry,
		settings_background_light,
		settings_unit
	} state_settings;
	
// 	enum
// 	{
// 		com_settings_start
// 	} state_com_settings;
	
	enum
	{	calibration_start,
		calibration_confirm,
		calibration_messbereich,
		calibration_first_reference,
		calibration_set_first_point,
		calibration_second_reference,
		calibration_set_second_point,
		calibration_set_wavelength,
		calibration_done
	} state_calibration;
			
} menu_t;


#if GET_KENNLINIE

typedef struct 
{
	enum 
	{
		adc_stop,
		adc_sample
	} adc_status;
	
	int8_t counter;
} adc_control_t;

#endif // end GET_KENNLINIE
	
	
	
	
#endif

