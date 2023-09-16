// common.h

#ifndef COMMON_H
	#define COMMON_H	
	
	
	/*	DEBUG_LED
		Die beiden LED's werden als Ausgang gesetzt (die dazugeh�rige Initialisierungsroutine wird nur aufgerufen, wenn DEBUG_LED 1 ist)
		DEGUG_LED_1_ON / OFF (und LED 2) bleiben auch dann definiert, wenn DEBUG_LED 0 ist, nur haben die Befehle dann keinen Effekt.	
	*/	#define  DEBUG_LED 1
	
	
	/*	DEBUG_I2C:
		W�hrend des Lesevorganges am I�C Bus wird das Status Register in regelm��igen Abst�nden angezeigt. Dadurch kann unter anderem
		der Busstatus beobachtet werden: bus idle, bus owner oder bus busy wenn sich der I2C Bus aufgeh�ngt hat.	
	*/	#define  DEBUG_I2C 0
	
	
	/*	DEBUG_ADC:
		Nach jedem Sampling Vorgang des ADC's wird das 16Bit Ergebnis bin�r formattiert per serieller Schnittstelle ausgegeben.
		Anschlie�end wird der berechnete Fotostrom angezeigt.
	*/  #define  DEBUG_ADC 0
	
	
	/*	GET_KENNLINIE Kennlinie aufnehmen:
		Auf Tastendruck 10 Samples durchf�hren und die ADC Ergebnisse per serieller Schnittstelle ausgeben
		keinerlei Berechnungen, das ergebnis ist dezimal formattiert
		
		linker Taster: MB 0 (kleiner Messbereich)
		zweiter Taster: MB 1 (gro�er Messbereich)		einmal dr�cken, dann wird der MB umgestellt
		rechteste Taste : 10 Samples durchf�hren	
	
	*/	#define  GET_KENNLINIE 0	
	
	
	
	
#endif