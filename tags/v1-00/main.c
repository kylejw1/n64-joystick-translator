/*  
	File contains main functionality of the gray code simulator
	
	main.c
	
    Copyright (C) Kyle Wagner 2010

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <system.h>

#include "movement.h"

#define REGULAR_SWING_LIMIT 80
#define LARGE_SWING_LIMIT 120

#pragma DATA _CONFIG, _BOR_OFF & _IOSCFS_8MHZ & _CP_OFF & _MCLRE_OFF & _PWRTE_OFF & _WDT_OFF & _INTOSCIO



/* 	Since I'm greedy and inefficient, I'll use longs everywhere.
	From what I understant, PICs are much faster with unsigned ints, but I don't 
	want to deal with correcting everything to 0-255... */


/****************** function declarations **************/
/**/	void initPorts(void);
/**/	void makeHorizEqual( long *current, long target );
/**/	void makeVertEqual( long *current, long target );
/**/	char setAdvanced( long *max_swing, long *deadzone );
/**/	void loopWithDZ(void);
/**/	void loopNoDZ(void);
/*******************************************************/

/********* Maximum swing limit / Deadzone vars *********/
/**/	long horizLimitLow = 0;
/**/	long horizLimitHigh = 0;
/**/	long vertLimitLow = 0;
/**/	long vertLimitHigh = 0;
/**/	long max_swing_per_direction = 85;
/**/	long deadzone=0;
/*******************************************************/


/************ Position vars (read from ADC) ************/
/**/	long horizInit = 0;
/**/	long vertInit = 0;
/**/	long horizCurrent = 0;
/**/	long vertCurrent = 0;
/**/	long horizNew = 0;
/**/	long vertNew = 0;
/*******************************************************/

int main(void)
{	

	char useDeadzone = 0;

	/* Init I/O and ADCs */
	initPorts();

	/* Determine if the user would like to implement a deadzone, or change
		the maximum swing limits.  If not, the deadzone 
		code is not used and thus cannot slow execution of regular code. */
	useDeadzone = setAdvanced( &max_swing_per_direction, &deadzone );

	/* Grab initial potentiometer values, set the maximums */
	horizInit = returnHorizAvg();
	vertInit = returnVertAvg();

	delay(100);

	/* Do it again for no reason :)... Actually, this was originally done before the 
		deadzone was implemented, due to issues with sampling the ADCs too
		soon after startup??? Not 100% sure what was happening, this code could
		probably be removed now but it isn't hurting anyone.*/
	horizInit = returnHorizAvg();
	vertInit = returnVertAvg();

	horizCurrent = horizInit;
	vertCurrent = vertInit;

	/* Set the maximum swing limit for each axis */
	horizLimitLow = horizCurrent - max_swing_per_direction;
	horizLimitHigh = horizCurrent + max_swing_per_direction;
	vertLimitLow = vertCurrent - max_swing_per_direction;
	vertLimitHigh = vertCurrent + max_swing_per_direction;

	/* If user has selected deadzone, go to DZ loop.  Otherwise
		hit the noDZ loop, which is much simpler and faster
		to run. */
	if(useDeadzone){
		loopWithDZ();
	} else {
		loopNoDZ();
	}


	return 0;
}

/* Initialize the I/O and set up the ADCs */
void initPorts(void){
    trisa = 0b00010100;

	trisc = 0x00;
	//adcon1 = 0x50;/* FOR 8 MHz this should be set, but for some reason I have problems... */
	adcon1 = 0x10;
	ansel = 0b00001010;

    trisc = 0;
	porta = 0;

	/* For no reason... */
	delay(100);

	return;
}

/* Our desired value is different from the current position index, so move in the direction of the desired value! */
void makeHorizEqual( long *current, long target ){
	if( (*current) == target ){
		return;
	}
	while( ((*current) > target) && ((*current) > horizLimitLow) ) {
		moveLeft();
		(*current)--;
	}
	while( ((*current) < target) && ((*current) < horizLimitHigh) ){
		moveRight();
		(*current)++;
	}
	return;		
}

/* Our desired value is different from the current position index, so move in the direction of the desired value! */
void makeVertEqual( long *current, long target ){
	if( (*current) == target ){
		return;
	}
	while( ((*current) > target) && ((*current) > vertLimitLow) ){
		moveDown();
		(*current)--;
	}
	while( ((*current) < target) && ((*current) < vertLimitHigh) ){
		moveUp();
		(*current)++;
	}
	return;		
}

/* Set advanced settings if user desires */
char setAdvanced( long *max_swing, long *deadzone )
{
	char useDeadzone = 0;
	char needDelay = 0;

	/* If the controller is turned on (plugged in) with the analog stick held down, the
		max swing is increased to 120, which may allow you to move faster in some games.
		However this can have unpredictable results in other games. */
	while( returnVertAvg() > 160 ){
		(*max_swing) = 120;
		
		needDelay = 1;
	}

	/* If the controller is turned on (plugged in) with the analog stick held up, the
		max swing is decreased to 40, which will limit the speed or motion.  I don't
		know of a use for this, but maybe someone will find one. */
	while( returnVertAvg() < 97 ){
		(*max_swing) = 40;
		
		needDelay = 1;
	}

	/* If the controller is turned on (plugged in) with the analog stick held left, the
		deadzone is set to 2. */
	while( returnHorizAvg() > 160 ){
		(*deadzone) = 2;
		(*max_swing) += 2;
		
		useDeadzone = 1;
		needDelay = 1;
	}

	/* If the controller is turned on (plugged in) with the analog stick held right, the
		deadzone is set to 10. */
	while( returnHorizAvg() < 97 ){
		(*deadzone) = 10;
		(*max_swing) += 10;
		
		useDeadzone = 1;
		needDelay = 1;
	}
	
	if(needDelay) {
		/* Delay for the stick to return to neutral */
		delay_ms(3000);
	}

	return useDeadzone;
}

void loopWithDZ(void)
{
	while(1) {
		horizNew = returnHorizAvg(); 
		/* 	If we are HIGHER than our starting point, and we are STILL HIGHER after subtracting the 
			deadzone, then we move to that position.  Otherwise, we are IN the deadzone and want
			to be centered (horizInit is considered the center) */
		if(horizNew > horizInit){
			if(horizNew - deadzone > horizInit){
				makeHorizEqual( &horizCurrent, horizNew-deadzone );	
			} else{
				makeHorizEqual( &horizCurrent, horizInit );
			}
		} else if(horizNew < horizInit){
			if(horizNew + deadzone < horizInit){
				makeHorizEqual( &horizCurrent, horizNew+deadzone );	
			} else{
				makeHorizEqual( &horizCurrent, horizInit );
			}
		}

		vertNew = returnVertAvg(); 
		if(vertNew > vertInit){
			if(vertNew - deadzone > vertInit){
				makeVertEqual( &vertCurrent, vertNew-deadzone );	
			} else{
				makeVertEqual( &vertCurrent, vertInit );
			}
		} else if(vertNew < vertInit){
			if(vertNew + deadzone < vertInit){
				makeVertEqual( &vertCurrent, vertNew+deadzone );	
			} else{
				makeVertEqual( &vertCurrent, vertInit );
			}
		}
	}

	return;
}

void loopNoDZ(void)
{
	/* Infinite loop */
	while(1) {
		makeHorizEqual( &horizCurrent, returnHorizAvg() );	
		makeVertEqual( &vertCurrent, returnVertAvg() );	
	}
}

