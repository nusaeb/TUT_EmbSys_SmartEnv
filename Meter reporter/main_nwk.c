/*
 * main.h
 *
 *  Created  on: 06.02.2015
 *  Author: Mairo Leier, Maksim Gorev
 *
 *  Version: 0.3		15.10.2015
 */
/***************************************************************************************************
 *	        Include section					                       		   					       *
 ***************************************************************************************************/

#include "system.h"
#include "network.h"
#include "general.h"

// Drivers
#include "uart.h"
#include "spi.h"
#include "radio.h"

// Network
#include "nwk_security.h"
#include "nwk_radio.h"

/***************************************************************************************************
 *	        Define section					                       		   					       *
 ***************************************************************************************************/


/***************************************************************************************************
 *	        Prototype section					                       							   *
 ***************************************************************************************************/
void Print_Error(uint8 error_code);

/***************************************************************************************************
 *	        Global Variable section  				                            				   *
 ***************************************************************************************************/
uint8 exit_code = 0;		// Exit code number that is set after function exits
uint8 payload_length;
uint8 cntr;
unsigned char addr_remote = 0x01;

/***************************************************************************************************
 *         Main section                                                                            *
 ***************************************************************************************************/
void main(void) {

	// Initialize system
	Print_Error(System_Init());

	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	/* Start : CPU clock configuration */

	if (CALBC1_1MHZ==0xFF)		    // If calibration constant erased
	{
		while(1);                   // do not load, trap CPU
	}
	DCOCTL = 0;                     // Select lowest DCOx and MODx settings
	BCSCTL1 = CALBC1_1MHZ;          // Set DCO to 1MHz
	DCOCTL = CALDCO_1MHZ;

	/* End : CPU clock configuration */

	/* Start : data pin configuration */

	P1IE |= DATA_PIN;					// DATA_PIN interrupt enabled
//    P1IES |= DATA_PIN;					// DATA_PIN HI to LOW edge
	P1IFG &= ~DATA_PIN;					// DATA_PIN IFG cleared

	/* End : data pin configuration */

	/* Start : Timer interrupt configuration */

	TACCTL0 = CCIE;						// TACCR0 interrupt enabled
	TACCR0 = 32768;						// Interrupt every one second (
	TACTL = TASSEL_1 + ID_0 + MC_1; 	// Set the timer A to ACLK, divide clock cycle by 1, Up mode

	/* End : Timer interrupt configuration */

	__enable_interrupt();				// Enabling interrupt
	__bis_SR_register(LPM0 + GIE);		// LPM0 with interrupts enabled

	while (1) {

		payload_length = 0;

		// Clear Tx packet buffer
		cntr = 0;
		for (cntr=RF_BUFFER_SIZE; cntr > 0; cntr--)
			TxPacket[cntr] = 0;

		// Construct the packet
		// This is the place where you can put your own data to send
		TxPacket[payload_length++] = PKT_CTRL | PKT_CTRL_REQUEST;
		TxPacket[payload_length++] = PKT_TYPE_VOLTAGE;
		TxPacket[payload_length++] = 255;
		TxPacket[payload_length++] = 90;

		// Print out all data that we are goind to send

		// Payload length includes Control packet and does not take into account
		// packet encryption nor source, destination adress bytes. Only actual payload that
		// you have defined above
		UART_Send_Data("\r\nPayload length:\t");
		UART0_Send_ByteToChar(&payload_length);

		UART_Send_Data("\r\nDestination address: ");

		UART0_Send_ByteToChar(&addr_remote);

		UART_Send_Data("\r\nMessage: ");
		for (cntr = 1; cntr < payload_length; cntr++) {
			UART0_Send_ByteToChar(&TxPacket[cntr]);
		}

		/* NWK level data sending */
		Print_Error(Radio_Send_Data(TxPacket, payload_length, addr_remote, PAYLOAD_ENC_ON, PCKT_ACK_ON));

		//UART_Send_Data("\r\nERR: ");
		//UART0_Send_ByteToChar((uint8 *)exit_code);
		exit_code = 0;

		UART_Send_Data("\r\n");		// Insert new line to separate packets


		// Add some delay (around 2sec)
		__delay_cycles(5000000*SYSTEM_SPEED_MHZ);

	}
}		/* END: main */

// PORT1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	isFlowing = 1;
	P1IFG &= ~DATA_PIN;					// DATA_PIN IFG cleared
	P1OUT |= (LED_RED + LED_GREEN);

	P1OUT &= ~(LED_RED + LED_GREEN);
	pulse_counter++;
	//while (!(IFG2&UCA0TXIFG));
	//UCA0TXBUF = pulse_counter;
}

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
	timer_counter++;
	//while (!(IFG2&UCA0TXIFG));
	//UCA0TXBUF = timer_counter;
	if(isFlowing == 1){
		isFlowing = 0;
		flowing_time++;
		//while (!(IFG2&UCA0TXIFG));
		//UCA0TXBUF = flowing_time;
	}
	// If water flow has stopped
	else{
		// Pulse characteristic: 7Q L/MIN (Q is flow rate in L/MIN)
		// Flow-pulse: 1L/MIN = 8Hz;
		// Dividing it by 60 to get the flow pulse per second
		// Multiplying it by flowing time to get the amount of water flowed
		/*water_used += ((pulse_counter / 0.125) * (flowing_time));
		water_used += pulse_counter;
		if(pulse_counter != 0){
			int data = (pulse_counter * flowing_time)/0.12;
			while (!(IFG2&UCA0TXIFG));
			UCA0TXBUF = data;
		}

		//UCA0TXBUF = water_used;
		//pulse_counter = 0;
		//flowing_time = 0;
		*/

		//flow_rate = ((float)pulse_counter) / calibration_factor;
		flow_rate = (float)pulse_counter / flowing_time;
		//flow_rate = (pulse_counter / calibration_factor) * (1000 / 60);		// flow rate in ml/s
		//water_used = flow_rate * flowing_time;
		// Divide the flow rate in litres/minute by 60 to determine how many litres have
		// passed through the sensor in this 1 second interval, then multiply by 1000 to
		// convert to millilitres. After that multiplying by flowing_time will give the total
		// amount of water flowed in this time duration.
		//flow_milli_litres = ((flow_rate / 60)) * flowing_time;
		/*
		 * 7Hz -> 60 second -> 1 Litre
		 * 1Hz -> 01 second -> 1 / (7 * 60)
		 * flow_rate Hz -> flowing_time second -> (flow_rate * flowing_time) / (7 * 60)
		 */
		//water_used = ((flow_rate * flowing_time) / (calibration_factor * 60)) * 1000;
		flow_milli_litres = ((flow_rate * flowing_time) / (calibration_factor * 60)) * 1000;
		// Calculate total amount of water flowed for a certain period (i.e. for 10 minute)
		total_flow_milli_litres += flow_milli_litres;

		total_flow_litres = total_flow_milli_litres / 1000;
		int_part = (unsigned int)total_flow_milli_litres;
		decimal_part = (total_flow_milli_litres - int_part) * 10;

		if(pulse_counter != 0){
			//int data = (pulse_counter * flowing_time)/0.12;
			while (!(IFG2&UCA0TXIFG));
			//UCA0TXBUF = int_part;
			//total_flow_milli_litres = 0;
		}

		pulse_counter = 0;
		flowing_time = 0;
		flow_milli_litres = 0;
		water_used = 0;
		flow_rate = 0;

	}

	// Send data to the server in every 1 min
	if(timer_counter == 10){

		timer_counter = 0;							// reset timer counter
		unsigned char low_byte = get_low_byte(10004);
		unsigned char high_byte = get_high_byte(10004);
		//pulse_counter = 0;
		//flowing_time = 0;
		while (!(IFG2&UCA0TXIFG));     						// Wait until TX buffer is ready
		UCA0TXBUF = high_byte;              				// Send amount of water used to the server(to UART)
		//water_used = 0;										// reset the amount of water used to 0

		//total_flow_milli_litres = 0;
	}

}

// *************************************************************************************************
// @fn          Print_Error
// @brief       Print code that is set when function exits. If error code is 0 then nothin is prnted
//				out and packet is received or sent correctly
// @param       uint8 error_code		Error code number that is set by the function
// @return      none
// *************************************************************************************************
void Print_Error(uint8 error_code) {

	// Print out error code only if it is not 0
	if (error_code) {
		UART_Send_Data("\r\nError code: ");
		UART0_Send_ByteToChar(&error_code);
		error_code = 0;
	}

	// If packet size has wrong length then reset radio module
	if (error_code == ERR_RX_WRONG_LENGTH) {
		Radio_Init(RF_DATA_RATE, TX_POWER, RF_CHANNEL);
	}

	exit_code = 0;
}

