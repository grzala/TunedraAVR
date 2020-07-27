#include "serial.h"

void USART_Init (unsigned int ubrr)
{
	/* Set baud rate */
	UBRRH = (unsigned char)(ubrr>>8);
	UBRRL = (unsigned char)ubrr;
	/* Enable receiver and transmitter */
	UCSRB = (1<<RXEN)|(1<<TXEN);
	/* Set frame format: 8data, 2stop bit */
	UCSRC = (1<<URSEL)|(1<<USBS)|(3<<UCSZ0);
}

void USART_Transmit (unsigned char data) {
	/* Wait for empty transmit buffer */
	while ( !( UCSRA & (1<<UDRE)) );
	/* Put data into buffer, sends the data */
	UDR = data;
}

void USART_Transmit_ar (char* data) {
	int len = strlen(data);
	for (int i = 0; i < len; i++) {
		USART_Transmit(data[i]);
	}
}

void USART_Transmit_ar_ln (char* data) {
	USART_Transmit_ar(data);
	USART_Transmit('\r');
	USART_Transmit('\n');
}

void USART_Transmit_int (int data) {
	char fstr[15];
	sprintf(fstr, "%i", data);
	USART_Transmit_ar(fstr);
}

void USART_Println() {
	USART_Transmit('\r');
	USART_Transmit('\n');
}