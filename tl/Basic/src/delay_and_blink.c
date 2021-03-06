/*
 * delay_and_blink.c
 *
 *  Created on: 6 Nov 2018
 *      Author: s1886282
 */

/*
 * File Name: delay_and_blink.c
 * Project Name: Basic Traffic Light
 * Target Device/Platform: Basys3 Board (with MICROBLAZE processor on the ARTIX-7 FPGA)
 * Tool Version: XILINX SDK 2015.2
 * Author: Yunfan Jiang (s1886282)
 * Company: School of Engineering, The University of Edinburgh
 * Created on: 6 Nov 2018
 * Last edited on: 15 Nov 2018
 * Version: 2.1.2
 *
 * Description:
 * Delay_and_Blink_Module.
 * This file contains interface variable "output_state" and time-related functions, including "hwTimerISR ()", "delay_timer ()", and "blink_PD ()".
 *
 * ISR function will be called every 0.004s.
 * Every time the ISR is called, it will output the current state of the state machine through interface variable "output_state" to display module.
 * Meanwhile, function "displayDigit ()" is called within ISR for the strobe of 7-segment display.
 *
 * Function "delay_timer ()" can output a state for certain time.
 * In other words, with assistance of other modules, traffic light can be displayed for specified time.
 * A loop is used for counting. Within the loop, function "read_btn ()" is called to detect the press action precisely, and function "displayNumber ()" is used to display the remaining time for each state.
 *
 * Function "blink_PD ()" is designed for pedestrian light to turn on and blink.
 * A "for" loop is used, in which ISR is called to count time. The whole period is divided into different intervals.
 * "If...else if" is used to determine the pedestrian light to turn on or off in different intervals.
 * Hence, blink can be realized.
 */

#include "xil_types.h"		/* Added for type definition */
#include "read_PD_btn.h"	/* Added for function "read_btn ()" */
#include "seg7_display.h"	/* Added for functions for 7-segment displays */
#include "gpio_init.h"		/* Added for XGpio objects */

/* Declaration for interface variable */
/* This interface variable will be accessed by Display_Module */
u16 output_state;

/* Declaration for ISR function */
void hwTimerISR(void *CallbackRef);

/* Declaration for within-module global variables */
/*
 * Within-module global variables will only be accessed and changed by the functions within this file
 */
static u8 interruptServiced = FALSE;
static u16 state_in;

/* Definition for interface function */
/*
 * This interface function will be called by the State_Machine_Module.
 * This function can delay the display of traffic light for a certain period.
 * Input "period" is with the unit of second. Input "state" is the state that will be delayed.
 */
void delay_timer (int period, u8 state)
{
	state_in = state;

	int i;									/* Loop counter */
	for (i = (period * 250); i > 0; i--)	/* Since ISR will be called every 0.004s, in one second, it will be called 250 times */
	{
		read_btn ();						/* Detect the press action */
		displayNumber (i / 250);			/* Display the remaining time */

		interruptServiced = FALSE;
		while (interruptServiced == FALSE);	/* Wait for ISR being serviced */
	}
}

/* Definition for interface function */
/*
 * This interface function will be called by the State_Machine_Module.
 * This function determines the behaviour of pedestrian light. It can control the pedestrian light to turn on or blink.
 */
void blink_PD ()
{
	PD_Pressed_Flag = 0;	/* Deactivate the flag when pedestrians start to go */

	int i;					/* Loop counter */
	for (i = 1250; i > 0; i--)
	{
		interruptServiced = FALSE;
		while (interruptServiced == FALSE);	/* Wait for ISR being serviced */

		displayNumber (i / 250);			/* Show the remaining time for pedestrians to go */

		/* Logic to blink the pedestrian light */
		if (((i >= 500) & (i <= 1250)) | ((i >= 400) & (i <= 450)) | ((i >= 300) & (i <= 350)) | ((i >= 200) & (i <= 250)) | ((i >= 100) & (i <= 150)) | ((i >= 0) & (i <= 50)))
		{
			XGpio_DiscreteWrite (&PD_LIGHT, 1, 0x0F0);
			XGpio_DiscreteWrite (&TR1_YELLOW, 1, 0xFFF);
			XGpio_DiscreteWrite (&LED_OUT, 1, 0x8084);
		}
		else if ( ((i >= 450) & (i <= 500)) | ((i >= 350) & (i <= 400)) | ((i >= 250) & (i <= 300)) | ((i >= 150) & (i <= 200)) | ((i >= 50) & (i <= 100)))
		{
			XGpio_DiscreteWrite (&PD_LIGHT, 1, 0xFFF);
			XGpio_DiscreteWrite (&TR1_YELLOW, 1, 0xFFF);
			XGpio_DiscreteWrite (&LED_OUT, 1, 0x8004);
		}
	}

}

/* Definition for ISR function */
/* The ISR function will be called every 0.004s */
void hwTimerISR(void *CallbackRef)
{
	interruptServiced = FALSE;

	displayDigit ();			/* Strobe for 7-segment display */
	output_state = state_in;	/* Output the state */

	interruptServiced = TRUE;
}
