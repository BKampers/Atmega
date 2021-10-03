/*
** UART
*/ 

#include "uart.h"

#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// UBBR_VALUE: lrint(( F_CPU / (16L * baud) ) - 1);
#define  UBBR_VALUE 12

char sendBuffer[SEND_BUFFER_SIZE];
char receiveBuffer[RECEIVE_BUFFER_SIZE];

size_t sendIndex = 0;
size_t receiveIndex = 0;

volatile bool messageReceived = false;


ISR(USART_TXC_vect)
{
    char ch = sendBuffer[sendIndex];
    if (ch != '\0')
    {
        sendIndex++;
        UDR = ch;
    }
    else
    {
        sendIndex = 0;
    }
}


ISR(USART_RXC_vect)
{
    char ch = UDR;
    if ((! messageReceived) && (receiveIndex < RECEIVE_BUFFER_SIZE))
    {   
        if (ch != '\n')
        {
            receiveBuffer[receiveIndex] = ch;
            receiveIndex++;
        }
        else
        {
            messageReceived = true;
        }
    }
}


void initUart()
{
    sendBuffer[SEND_BUFFER_SIZE - 1] = '\0';
    // High bits of the baud number (bits 8 to 11)
    UBRRH = (uint8_t) (UBBR_VALUE >> 8);
    // Low bits of the baud number (bits 0 to 7)
    UBRRL = (uint8_t) UBBR_VALUE;
    // Enable the receiver and transmitter
    UCSRB = _BV(RXEN) | _BV(TXEN) | _BV(TXCIE) | _BV(RXCIE);
    // No parity, 8 bits, 1 stop bit
    UCSRC = _BV(URSEL) | _BV(UCSZ0) | _BV(UCSZ1);
    // Clear the UART status register
    UCSRA= 0x00;  
}


inline bool uartIsSending()
{
    return sendIndex != 0;
}


void sendUart(const char* string) 
{
    if ((sendIndex == 0) && (*string != '\0'))
    {
        strncpy(sendBuffer, string, SEND_BUFFER_SIZE - 1);
        sendIndex = 1;
        UDR = *sendBuffer;
    }
}


bool receiveUart(char* string)
{
    if (messageReceived || (receiveIndex == RECEIVE_BUFFER_SIZE)) 
    {
        strncpy(string, receiveBuffer, receiveIndex);
        string[receiveIndex] = '\0';
        receiveIndex = 0;
        messageReceived = false;
        return true;
    }
    return false;
}


void print(const char* string)
{
    sei();
    sendUart(string);
    //while (uartIsSending());
    _delay_ms(100);
    cli();
}

void printValue(const char* name, unsigned int value)
{
    char text[SEND_BUFFER_SIZE];
    sprintf(text, "%s = %x\r\n", name, value);
    print(text);
}