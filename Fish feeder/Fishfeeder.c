/******************************************************************************************************************************
 *                                                                                                                            *
 *                                         COPYRIGHT (C) 2015    Tallinn University of Technology                             *
 *                                                                                                                            *
 ******************************************************************************************************************************
 *                                                                                                                            *
 *                                                                                                                            *
 *                                              CCCCC   FFFFFF    AAA                                                         *
 *                                             C        F        A   A                                                        *
 *                                             C        FFFF    A     A                                                       *
 *                                             C        F       AAAAAAA                                                       *
 *                                             C        F       A     A                                                       *
 *                                              CCCCC   F       A     A                                                       *
 *                                                                                                                            *
 *                                           Common firmware application SW                                                   *
 *                                                                                                                            *
 *                                                                                                                            *
 $                 $Workfile::   main.c                                                                                       $
 $                 $Revision::   1.0                                                                                          $
 $                     $Date::   13.10.2015                                                                                   $
 *                                                        								      								  *
 *                    Manager:   Yuanxu Xu                                                                                    *
 *                 Developers:   Martin Grosberg									      *
 *                  		 Aivar Koodi										      *
 *				 Marek Aare										      *
 *		                 Sander Sinijärv									      *
 *		                 Kätlin Lahtvee                                                                               *
 *                     Target:   Fishfeeder Project 2015                                                                      *
 *                                                                                                                            *
 *****************************************************************************************************************************/

/***************************************************************************************************
 *	main.c  - Template for the MSP430 Launchpad project   								   		   *
 ***************************************************************************************************/

/***************************************************************************************************
 *	        Include section					                       		   					       *
 ***************************************************************************************************/
#include <msp430.h>


/***************************************************************************************************
 *	        Define section					                       		   					       *
 ***************************************************************************************************/

/***************************************************************************************************
 *	        Prototype section					                       							   *
 ***************************************************************************************************/


/***************************************************************************************************
 *	        Global Variable section  				                            				   *
 ***************************************************************************************************/


/***************************************************************************************************
 *         Main section                                                                            *
 ***************************************************************************************************/
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	
	return 0;
}

void timerA(){

}

void timerB(){

}

void temperature(){

}

int lighting(int LightLevel){
	
	/*initialize ADC*/
	ADC10CTL0 = ADC10IE + ADC10SHT_3 + ADC10ON;
	ADC10CTL1 = INCH_0 + ADC10DIV_3;

	__delay_cycles(5000);       	// settle time 30ms

	/*ADC conversion start*/
	__delay_cycles(1000);           // Wait for reference to settle
	ADC10CTL0 |= ENC + ADC10SC; 	// Sampling and conversion start
	while(ADC10CTL1 & BUSY);    	// Wait..i am converting...
	light = ADC10MEM;                    // Read ADC memory
	light = light / 3;

	while (!(IFG2&UCA0TXIFG));      // Wait until TX buffer is ready
	UCA0TXBUF = light;
	
	if(light < 120){		//reading values from RealTerm
		return 1;		//120 is a little brigther than ambient light
	}				//the smaller the value, the brighter is light
	else{
		return 0;

void waterlevel(){
	
}

void feedlevel(){
	
}

void motor(){
	
}

void UART(){

}

/***************************************************************************************************
 *         History of changes                                                                       *
 ***************************************************************************************************/

//[Week 8] Added empty function slots for future code.
