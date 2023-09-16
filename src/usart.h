// usart.h

#ifndef USART_H 
	#define USART_H
	
	#include "inlinePowermeter.h"		// frag mich nicht warum sonst immer ein compiler fehler kommt... hat bisher immer so funktionniert !?
	
	void ini_usart();
	void clear_rx_buffer();
	void sendUSART(uint8_t data);
	void sendUSARTs(char* field);
	void sendUSARTb(uint8_t value);
	void sendUSARTb16(int value);
	
	void usart(usart_data_t * data, adc_data_t * adc_data, new_calibration_t* new_calibration);
	void show_saved_wavelength();
	void show_menu();
	void show_opt_power(adc_data_t* adc_data);
	
	void handle_com_input(usart_data_t* com_data, adc_data_t* adc_data, new_calibration_t* calibration);
	void handle_com_output(usart_data_t* com_data, adc_data_t* adc_data);
	
	void handle_com_set_wavelength(adc_data_t* adc_data, usart_data_t* com_data);
	
	void handle_com_wait(usart_data_t* data);
	void handle_com_delete(usart_data_t* com_data);
	
	void handle_com_show_calibration(usart_data_t* com_data);
	
	void handle_com_calibration_messbereich(usart_data_t* com_data, new_calibration_t* calibration, adc_data_t* adc_data);
	void handle_com_calibration_ref(usart_data_t* com_data, new_calibration_t* calibration, adc_data_t* adc_data, uint8_t point);
	void handle_com_calibration_value(usart_data_t* com_data, new_calibration_t* calibration, uint8_t point);
	void handle_com_calibration_set_wavelength(usart_data_t* com_data, new_calibration_t* calibration);
	
	void handle_com_calibration(com_data, calibration, adc_data);
	
#endif


