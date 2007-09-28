/*
	Molole - Mobots Low Level library
	An open source toolkit for robot programming using DsPICs
	Copyright (C) 2007 Stephane Magnenat <stephane at magnenat dot net>
	
	Mobots group http://mobots.epfl.ch
	Robotics system laboratory http://lsro.epfl.ch
	EPFL Ecole polytechnique federale de Lausanne: http://www.epfl.ch
	
	See AUTHORS for more details about other contributors.
	
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

/** \file
	\brief Implementation of the wrapper around dsPIC33 ADC.
*/

//--------------------
// Usage documentation
//--------------------

/**
	This very simple wrapper ease the usage of the ADC converter.
*/

//------------
// Definitions
//------------

#include <p33fxxxx.h>

#include "adc.h"

//-----------------------
// Structures definitions
//-----------------------

/** ADC wrapper data */
static struct
{
	adc_simple_callback callback; /**< function to call upon conversion complete interrupt, 0 if none */
} ADC_Data = { 0 };


//-------------------
// Exported functions
//-------------------

/**
	\brief Initialize ADC1 for simple input conversion.
	
	The converter is put in 12 bits / single conversion mode and callback is called when conversion is completed.
	
	\param	callback
			Pointer to a function that will be called when conversion is completed.
	\param	inputs
			Bitfield that specify which physical input to use (AN0..AN31).
			1 put pins in analogic, 0 in digital.
*/
void adc1_init_simple(adc_simple_callback callback, unsigned long inputs)
{
	// Turn off ADC Module
	AD1CON1bits.ADON = 0;
	
	// Setup callback
	ADC_Data.callback = callback;
	
	// configure I/O pins in digital or analogic
	AD1PCFGH = ~((unsigned short)(inputs >> 16));
	AD1PCFGL = ~((unsigned short)(inputs));
	
	AD1CON1bits.ADSIDL = 1;		// Discontinue module operation when device enters Idle mode
	AD1CON1bits.AD12B = 1;		// 12-bit, 1-channel ADC operation
	AD1CON1bits.FORM = 0;		// Integer (DOUT = 0000 dddd dddd dddd)
	AD1CON1bits.SSRC = 7;		// Internal counter ends sampling and starts conversion (auto-convert)
	AD1CON1bits.ASAM = 0;		// Sampling begins when SAMP bit is set
	
	AD1CON2bits.VCFG = 0;		// Avdd, Avss Converter Voltage References
	AD1CON2bits.CSCNA = 0;		// Do not scan inputs
	AD1CON2bits.BUFM = 0;		// Always starts filling the buffer from the start address.
	AD1CON2bits.ALTS = 0;		// Always uses channel input selects for Sample A
	
	AD1CON3bits.ADRC = 1;		// ADC Internal RC Clock
	AD1CON3bits.SAMC = 1;		// Auto Sample Time bits 1 TAD
	
	// No input to scan yet
	AD1CSSH = 0x0000;
	AD1CSSL = 0x0000;
	
	AD1CHS0bits.CH0NA = 0;		// Channel 0 negative input is VREFL
	AD1CHS0bits.CH0SA = 0;		// Channel 0 positive input is AN0
	
	// Reset ADC 1 interrupt flag
	_AD1IF = 0;
	// Enable ADC 1 Interrupt
	_AD1IE = 1;
	
	// Turn on ADC Module
	AD1CON1bits.ADON = 1;
}

/**
	\brief Request a conversion on a specific channel.
	
	The user must call adc1_init_simple prior to this function.
	
	\param	channel
			physical input to convert (AN0..AN31)
*/
void adc1_start_simple_conversion(int channel)
{
	// Select channel
	AD1CHS0bits.CH0SA = channel;
	// Start sampling
	AD1CON1bits.ASAM = 1;
}


//--------------------------
// Interrupt service routine
//--------------------------

/**
	\brief	ADC 1 Interrupt Service Routine.
 
	Call the user-defined function.
*/
void _ISR  _ADC1Interrupt(void)
{
	// Call user-defined function with result of conversion
	ADC_Data.callback(ADC1BUF0);
	// Clear ADC 1 interrupt flag
	_AD1IF = 0;
} 

