#ifndef _UART_H_
#define _UART_H_

#include <stdbool.h>

#define SEND_BUFFER_SIZE 32
#define RECEIVE_BUFFER_SIZE 16

void initUart();
bool uartIsSending();
void sendUart(const char* string);
bool receiveUart(char* string);
void print(const char* string);
void printValue(const char* name, unsigned int value);

#endif /* _UART_H_ */
