/*
 * CEP Receiver.c
 *
 * Created: 7/11/2022 1:57:51 PM
 * Author : Sohaib
 */ 
#include <string.h>
#include <avr/io.h>
#define F_CPU 1000000UL
#include<util/delay.h>

#define PASSWORD_LENGTH 10

// global variables
enum states {locked, unlocked, checking_password};
enum states CURRENT_STATE = locked;
enum servo_states {latch, unlatch};
const char CORRECT_PASSWORD[] = "12AB#";

// function prototypes
void USART1_init(void);
void timer0_fastpwm_init(void);
void USART1_transmit(char data);
char USART1_receive(void);
void set_servo_position(enum servo_states servo_state);


int main(void)
{
	// initializations
	USART1_init();  // initialize USART1
	timer0_fastpwm_init();  // initialize timer / pwm
	set_servo_position(latch);  // set servo to latch position

	DDRD &= ~(1 << PD7);  // set port D pin 7 as input (for push button)
	PORTD &= ~(1 << PD7);
	
	char x = 0x00;
	char password[PASSWORD_LENGTH + 1] = ""; // password received from other microcontroller
	int chars_entered = 0;  // number of characters recieved
	while (2 + 2 == 4)
	{
	
		switch (CURRENT_STATE)
		{

			case locked:

				set_servo_position(latch);  // set servo to latch position
				
				// recieve password from other microcontroller
				while (chars_entered < PASSWORD_LENGTH)
				{
					char received_char = USART1_receive();
					if (received_char == '*')
					{
						break;  // break while loop
					}
					password[chars_entered] = received_char;
					chars_entered++;
				}

				CURRENT_STATE = checking_password;
				break;  // break case locked

			case unlocked:
				set_servo_position(unlatch);  // set servo to unlatch position
				
				do 
				{
					x = (PIND & (1 << PD7)) >> 7;  // read port D pin 7
				} while (x == 0x00);
				x = 
				PORTD &= ~(1 << PD7);
				
				USART1_transmit('L');

				CURRENT_STATE = locked;  // go to locked state
				break;  // break case unlocked

			case checking_password:
				if (strcmp(password, CORRECT_PASSWORD) == 0)
				{
					// send unlocked status to other microcontroller (correct password)
					USART1_transmit('1');
					CURRENT_STATE = unlocked;
				}
				else
				{
					// send locked status to other microcontroller (incorrect password)
					USART1_transmit('0');
					CURRENT_STATE = locked;
				}

				//  reset password and number of characters entered
				chars_entered = 0;
				memset(password, 0, PASSWORD_LENGTH);  // clear password array
				
				break;  // break case checking_password
		}
		_delay_ms(200);
	}
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
	UCSR1B |= (1 << RXEN1) | (1 << TXEN1);  // enable recieving and transmition 
}


void timer0_fastpwm_init(void)
{
	TCCR0 &= ~(1 << FOC0);  // set FOC0 as 0 
	/* Bit 7 – FOC0: Force Output Compare
	The FOC0 bit is only active when the WGM00 bit specifies a non-PWM mode. However, for
	ensuring compatibility with future devices, this bit must be set to zero when TCCR0 is written
	when operating in PWM mode. When writing a logical one to the FOC0 bit, an immediate Compare Match is forced on the Waveform Generation unit. The OC0 output is changed according to
	its COM01:0 bits setting. Note that the FOC0 bit is implemented as a strobe. Therefore it is the
	value present in the COM01:0 bits that determines the effect of the forced compare.
	A FOC0 strobe will not generate any interrupt, nor will it clear the timer in CTC mode using
	OCR0 as TOP.
	The FOC0 bit is always read as zero. */

	TCCR0 |= (1 << WGM00) | (1 << WGM01);  // Waveform Generation Mode => PWM
    TCCR0 |= (1 << COM01);  // Clear OC0 on Compare Match, set OC0 at TOP (non-inverting mode)
	TCCR0 &= ~(1 << COM00);  // Clear OC0 on Compare Match, set OC0 at TOP (non-inverting mode)
    TCCR0 |= (1 << CS01) | (1 << CS00);  // Clock Select => clk/64 (from prescaler)

	DDRB |= (1 << PB0);  // set PB0 as output for PWM (OC0)
}

void USART1_transmit(char data)
{
	while (!(UCSR1A & (1 << UDRE1)));  // wait until UDRE is empty
	UDR1 = data;  // send data
}


char USART1_receive(void)
{
	while (!(UCSR1A & (1 << RXC1)));  // wait until data is received
	return UDR1;  // return data
}


void set_servo_position(enum servo_states servo_state)
{
	// 22.5 value for neutral position, less than that for latch position, greater than that for unlatch position
	switch (servo_state)
	{
		case latch:
			OCR0 = 10;
			break;
		case unlatch:
			OCR0 = 40;
			break;
	}
}


