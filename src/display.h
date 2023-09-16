// display.h

#ifndef DISPLAY_H
	#define DISPLAY_H


	void ini_display(display_data_t *display_data);
	void lcd(menu_t *menu_data, adc_data_t* adc_data, display_data_t* display_data, new_calibration_t* calibration_data);
	
	void sendLCDs ( char* text, uint8_t line); 
	void clearLCD(); 
	
	void display_result(display_data_t* display_data, adc_data_t* adc_data);
	void display_result_dBm(adc_data_t* adc_data); 
	
	void display_new_value(new_calibration_t* calibration);	
	void display_new_wavelength(new_calibration_t* calibration);	
	
	void display_saved_calibration(adc_data_t* adc_data);

	
 	void blink_digit(uint8_t position);
 	void blink_digit_off();

	uint8_t inc_10(uint8_t input );
	uint8_t inc_4(uint8_t input );
	uint8_t dec_10(uint8_t input );
	uint8_t dec_4(uint8_t input );
	
	












#endif