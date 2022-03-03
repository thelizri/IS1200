#include <stdint.h>
#include <pic32mx.h>
#include "mipslab.h"

int getsw( void );
int getbtns(void);

//Switches 1-4 are connect to bits 8-11 in PORTD
int getsw( void ){
	volatile int* switches;
	switches = (int *) PORTD;
	return (0xF&(*switches>>8));
}

//Button 2-4 are connected to bit 5-7 in PORTD
int getbtns(void){
	volatile int* buttons;
	buttons = (int *) PORTD;
	return (0x7&(*buttons>>5));
}

//Button 1 is connect to bit 1 in PORTF
int getbtn1(){
	volatile int* buttons;
	buttons = (int *) PORTF;
	return ((*buttons>>1) & 0x1);
}