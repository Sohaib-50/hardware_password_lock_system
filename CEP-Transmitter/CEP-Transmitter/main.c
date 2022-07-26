/*
 * CEP-Transmitter.c
 *
 * Created: 7/11/2022 1:57:30 PM
 * Author : Sohaib
 */ 

#include <avr/io.h>
#define F_CPU 1000000UL
#include<util/delay.h>

#define PASSWORD_LENGTH 6

// global variables
enum states {entering_password, checking_password, incorrect_password, correct_password};
enum states CURRENT_STATE = entering_password;
const char CORRECT_PASSWORD[] = "12ABCD";

// function prototypes
void USART1_init(void);
void keypad_init(void);
char get_seven_segment_code(char data);
int get_keypad_position(void);
void USART1_transmit(char data);
char USART1_receive(void);



int main(void)
{
	// initializations
	USART1_init();  // initialize USART1
	// set_USART1_mode(transmitter);  // set USART1 to transmitter mode
	keypad_init();  // initialize keypad

	DDRA = 0xFF;  // set port A as output (for seven segment display)

	char keypad_chars[20] = {'1', '2', '3', 'A', '4', '5', '6', 'B', '7', '8', '9', 'C', '*', '0', '#', 'D', '\0'};
	char key_position, char_entered, password_correct_response;
	int chars_entered = 0;  // number of characters entered in the password so far
	char password[PASSWORD_LENGTH + 1] = ""; // password entered by the user

	while (-10 < 10)
	{
		switch (CURRENT_STATE)
		{

			case entering_password:
				PORTA = get_seven_segment_code('E');  // show "E" on the seven segment display
				
				// if password is full, go to checking_password state
				if (chars_entered == PASSWORD_LENGTH)
				{
					CURRENT_STATE = checking_password;
					break;
				}
				
				key_position = get_keypad_position();  // get position of key pressed
				char_entered = keypad_chars[key_position];
				if (char_entered == '\0')
				{
					break;
				}
				else if (char_entered == '*')  // if * key pressed, go to checking_password state
				{
					CURRENT_STATE = checking_password;  
				}
				else
				{
					password[chars_entered] = char_entered;  // add character entered to password
					chars_entered++;
				}
				break;

			case checking_password:
				PORTA = get_seven_segment_code('C');  // show "C" on the seven segment display

				// send password to other microcontroller via USART1 charater by character
				for (int i = 0; i < PASSWORD_LENGTH; i++)
				{
					USART1_transmit(password[i]);
				}
				USART1_transmit('*');  // send * to  indicate end of password

				//  reset password and number of characters entered
				chars_entered = 0;
				memset(password, 0, PASSWORD_LENGTH + 1);  // clear password array

				// recieve response from other microcontroller
				password_correct_response = USART1_receive();
				if (password_correct_response == '1')
				{
					CURRENT_STATE = correct_password;
				}
				else
				{
					CURRENT_STATE = incorrect_password;
				}
				break;

			case incorrect_password:
				PORTA = get_seven_segment_code('L');  // show "I" on the seven segment display
				
				// when * key is pressed 
				key_position = GetKeyPressed();
				key_pressed = digit[key_position];
				if (key_pressed == '*')
				{
					state = entering_password;
				}
				break;

			case correct_password:
				PORTA = get_seven_segment_code('U');  // show "O" on the seven segment display
				
				// wait till other microcontroller sends L to indicate that lock is closed again
				while (USART1_receive() != 'L');

				state = entering_password;
		}
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
	UCSR1B |= (1 << RXEN1) | (1 << TXEN1);  // enable recieving and transmition 
}


char get_seven_segment_code(char data)
{
	char seven_segment_code = 0;
	switch (data)
	{
		case 'C':  // Checking password state
			seven_segment_code = 0b11000110;  // turn on a, f, e, d of seven segment display
			break;
		case 'E':  //  Entering password state
			seven_segment_code = 0b10000110;  // turn on a, f, e, d, g of seven segment display
			break;
		case 'U':  // Unlocked state
			seven_segment_code = 0b11000001;  // turn on b, c, d, e, f of seven segment display
			break;
		case 'L':  // Locked state
			seven_segment_code = 0b11000111;  // d, e, f of seven segment display
			break;
	}
	return seven_segment_code;
}


int get_keypad_position(void)
{
	char data_available;
	int data;
	PORTB &= ~(1 << PB1);  // set port B bit 1 as 0 to enable OE' of of keypad decoder IC
	data_available = PINC;
	if (data_available == 0x01)
	{
		data = (PIND & 0x0F);  // read data from port D bits 0 to 3
		return data;
	}
	return 16;  // default value
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