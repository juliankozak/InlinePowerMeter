// eeprom_memory.h

#ifndef EEPROM_MEMORY_H
	#define EEPROM_MEMORY_H
	
	
	
	void load_calibration(adc_data_t* adc_data, uint8_t memory_position);
	void delete_calibration(uint8_t pos);
	void delete_all_memory();
	
	
	
#endif