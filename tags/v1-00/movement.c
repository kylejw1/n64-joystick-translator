/*  
	File contains movement related functions.

	movement.c
	
    Copyright (C) Kyle Wagner 2010

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <system.h>
#include "movement.h"

/* 	The next two globals contain the positions of the vertical and horizontal movement ports
	in each of their respective state machines */
char stateVert=0;
char stateHoriz=0;

/* 	Really random delay function. I use it to add a delay for the ADCs
	but can't remember how it got all ugly like this...  Probably has 
	something to do with keeping it from being optimized out.  Should probably use some nop()s... */
void delay(unsigned char d) {
  unsigned int n;
  while (d--)
    for (n=0; n<1; n++) ;
}

/* 	Movement involves reading the ADC value, then moving within a state machine
	appropriately. 
	Counting up, the output ports should read:
	00
	01
	11
	10
	...

	Counting down:
	00
	10
	11
	01

	Counting up 3 then down 2:
	00
	10
	11
	10
	00
*/

void moveLeft(void) {
	switch(stateVert) {
		case 0:
			clear_bit(portc,3);
			stateVert = 1;
			break;
		case 1:
			clear_bit(portc,2);
			stateVert = 3;
			break;
		case 2:
			set_bit(portc,2);
			stateVert = 0;
			break;
		case 3:
			set_bit(portc,3);
			stateVert = 2;
			break;
	}
	return;
}

void moveRight(void) {
	switch(stateVert) {
		case 0:
			clear_bit(portc,2);
			stateVert = 2;
			break;
		case 1:
			set_bit(portc,3);
			stateVert = 0;
			break;
		case 2:
			clear_bit(portc,3);
			stateVert = 3;
			break;
		case 3:
			set_bit(portc,2);
			stateVert = 1;
			break;
	}
	return;
}

void moveUp(void) {
	switch(stateHoriz) {
		case 0:
			clear_bit(portc,0);
			stateHoriz = 2;
			break;
		case 1:
			set_bit(portc,1);
			stateHoriz = 0;
			break;
		case 2:
			clear_bit(portc,1);
			stateHoriz = 3;
			break;
		case 3:
			set_bit(portc,0);
			stateHoriz = 1;
			break;
	}
	return;
}

void moveDown(void) {
	switch(stateHoriz) {
		case 0:
			clear_bit(portc,1);
			stateHoriz = 1;
			break;
		case 1:
			clear_bit(portc,0);
			stateHoriz = 3;
			break;
		case 2:
			set_bit(portc,0);
			stateHoriz = 0;
			break;
		case 3:
			set_bit(portc,1);
			stateHoriz = 2;
			break;
	}
	return;
}

/*  Poll the ADC on the horizontal potentiometer port,
	summing four times then shifting (dividing) to get an average of four values.
	May be able to drop the extra polls and shift,
	to speed execution */
long returnHorizAvg(void){
	long horizAvg=0;
	int i=4;
	adcon0 = 0b00001001;

	while(i--){
		adcon0 = adcon0 | 0b00000010;
		delay(1);
		horizAvg += adresh;
	}
	
	return (horizAvg >> 2);
}

/*  Poll the ADC on the vertical potentiometer port,
	summing four times then shifting (dividing) to get an average of four values.
	May be able to drop the extra polls and shift,
	to speed execution */
long returnVertAvg(void){
	long vertAvg=0;
	int i=4;
	adcon0 = 0b00001101;

	while(i--){
		adcon0 = adcon0 | 0b00000010;
		delay(1);
		vertAvg += adresh;
	}
	
	return (vertAvg >> 2);
}






