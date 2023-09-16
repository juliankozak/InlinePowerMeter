// calibration.h

#ifndef CALIBRATION_H
#define CALIBRATION_H

void handle_calibration_input(new_calibration_t* calibration, menu_t* menu_data);

void reset_calibration_buffer(new_calibration_t* calibration, uint8_t setting);

void calculate_new_value(new_calibration_t* calibration);
void calculate_new_wavelength(new_calibration_t* calibration);

void save_new_calibration(adc_data_t* adc, new_calibration_t* new_calibration); // save to EEPROM





#endif