/*
 * CEP Receiver.c
 *
 * Created: 7/11/2022 1:57:51 PM
 * Author : Sohaib
 */ 
#include <avr/io.h>
#define F_CPU 1000000UL
#include<util/delay.h>

// global variables
enum states {entering_password, checking_password, incorrect_password, correct_password};
enum states CURRENT_STATE = checking_password;
// enum usart_modes {transmitter, receiver};

// function prototypes
void USART1_init(void);
void keypad_init(void);



int main(void)
{
	// initializations
	USART1_init();  // initialize USART1
	// set_USART1_mode(transmitter);  // set USART1 to transmitter mode
	keypad_init();  // initialize keypad

	DDRC = 0xFF; // set port C as output
	DDRA = 0x00;
	char receiveData;

	while (2 + 2 == 4)
	{
		
	}
}


void keypad_init(void)
{
	DDRB |= (1 << PB1);  // set port B bit 1 as output for OE' of keypad decoder IC
	DDRC &= ~(1 << PC0);  // set port C bit 0 as output for DA (data available) of keypad decoder IC
	DDRD |= 0xF0;  // set bits 0 to 3 of port D as input for keypad decoder IC ABCD
}


void USART1_init(void)
{
	DDRB |= (1<<PB3);  // set port B bit 3 as output for TXD1 of USART1
	DDRB &= ~(1<<PB2);  // set port B bit 2 as input for RXD1 of USART1
	PORTB |= (1<<PB2);  // set port B bit 2 as high for RXD1 of USART1 (pull-up)
	// int UBBRValue=25;
	UBRR1H=0x00;  // set baud rate
	UBRR1L=0x05;  // set baud rate
	UCSR1C |= (1 << USBS1) | (3 << UCSZ10);  // 2-bit Stop Bits, 8-bit character size
	UCSR1B |= (1 << RXEN1) | (1 << TXEN1);  // enable receiving and transmitting
}
