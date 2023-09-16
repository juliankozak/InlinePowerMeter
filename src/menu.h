// menu.h


#ifndef MENU_H
#define MENU_H




void main_menu (adc_data_t* adc_data, menu_t* menu_data, new_calibration_t* calibration);

void ini_menu(menu_t* menu);

void submenu_measure(adc_data_t* adc_data, menu_t* menu_data);
void submenu_settings(adc_data_t* adc_data, menu_t* menu_data);
void submenu_com_settings(adc_data_t* adc_data, menu_t* menu_data);
void submenu_calibration(adc_data_t* adc_data, menu_t* menu_data, new_calibration_t* calibration);



#endif