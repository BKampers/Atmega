#ifndef _MCP23X17_H_
#define _MCP23X17_H_

#include <stdint.h>

#define OUTPUT 0
#define INPUT 1

#define LOW 0
#define HIGH 1

void mcpInit(uint8_t address);
void mcpPinModeA(uint8_t mask, uint8_t mode);
void mcpPinModeB(uint8_t mask, uint8_t mode);
void mcpDigitalWriteA(uint8_t mask, uint8_t level);
void mcpDigitalWriteB(uint8_t mask, uint8_t level);
void mcpPullUpA(uint8_t mask, uint8_t level);
void mcpPullUpB(uint8_t mask, uint8_t level);
uint8_t mcpReadGpioA();
uint8_t mcpReadGpioB();
uint16_t mcpReadGpioAB();
void mcpWriteGpioAB(uint16_t ba);

#endif /* _MCP23X17_H_ */