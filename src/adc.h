// adc.h

#ifndef ADC_H
	#define ADC_H
	
	
	void ini_adc(adc_data_t* adc_data);
	void adc(adc_data_t* data);
	uint16_t get_adc_result();
	uint32_t calcucalte_opt_power(adc_data_t adc_data );
	void calculate_intercept_slope(adc_data_t* adc_data, new_calibration_t* calibration);
	
	
	
	
	
	
	
#endif