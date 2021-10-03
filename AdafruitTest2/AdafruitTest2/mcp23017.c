#include "mcp23x17.h"

#include "i2cmaster.h"
//#include "uart.h"


// registers
#define MCP23017_IODIRA 0x00
#define MCP23017_IPOLA 0x02
#define MCP23017_GPINTENA 0x04
#define MCP23017_DEFVALA 0x06
#define MCP23017_INTCONA 0x08
#define MCP23017_IOCONA 0x0A
#define MCP23017_GPPUA 0x0C
#define MCP23017_INTFA 0x0E
#define MCP23017_INTCAPA 0x10
#define MCP23017_GPIOA 0x12
#define MCP23017_OLATA 0x14
#define MCP23017_IODIRB 0x01
#define MCP23017_IPOLB 0x03
#define MCP23017_GPINTENB 0x05
#define MCP23017_DEFVALB 0x07
#define MCP23017_INTCONB 0x09
#define MCP23017_IOCONB 0x0B
#define MCP23017_GPPUB 0x0D
#define MCP23017_INTFB 0x0F
#define MCP23017_INTCAPB 0x11
#define MCP23017_GPIOB 0x13
#define MCP23017_OLATB 0x15


/*
** Private
*/
uint8_t i2cAddress;


uint8_t mcpReadRegister(uint8_t reg)
{
    i2c_start_wait(i2cAddress + I2C_WRITE);
    i2c_write(reg);
    i2c_rep_start(i2cAddress + I2C_READ);
    uint8_t data = i2c_readNak();
    i2c_stop();
    return data;
}

/*
uint16_t mcpReadTwoBytes(uint8_t reg)
{
    uint8_t a, b;
    uint16_t data;
    i2c_start_wait(i2caddress + I2C_WRITE);
    //print("  started\r\n");
    i2c_write(reg);
    //printValue("  written reg", reg);
    i2c_rep_start( i2caddress + I2C_READ);
    //print("  rep_started\r\n");
    a = i2c_readNak();
    b = i2c_readNak();
    data = a;
    data <<= 8;
    data |= b;
    //printValue("  readAck", data & 0xFF);
    //printValue("  readNak", data >> 8);
    i2c_stop();
    //print("ready mcp23017ReadTwoByte\r\n");
    return data; // ~data?
}
*/

void mcpWriteRegister(uint8_t reg, uint8_t value)
{
   i2c_start(i2cAddress + I2C_WRITE);
   i2c_write(reg);
   i2c_write(value);
   i2c_stop();
}


void mcpClearRegisterMask(uint8_t reg, uint8_t mask)
{
    uint8_t data = mcpReadRegister(reg);
    data &= ~mask;
    mcpWriteRegister(reg, data);
}


void mcpSetRegisterMask(uint8_t reg, uint8_t mask)
{
    uint8_t data = mcpReadRegister(reg);
    data |= mask;
    mcpWriteRegister(reg, data);
}


/*
** Interface
*/
void mcpInit(uint8_t address)
{
    i2cAddress = address;
    mcpWriteRegister(MCP23017_IODIRA, 0xFF); // All inputs
    mcpWriteRegister(MCP23017_IODIRB, 0xFF); // All inputs
}


void mcpPinModeA(uint8_t mask, uint8_t mode) 
{
    if (mode == OUTPUT)
    {
        mcpClearRegisterMask(MCP23017_IODIRA, mask);
    }
    else
    {
        mcpSetRegisterMask(MCP23017_IODIRA, mask);
    }
}


void mcpPinModeB(uint8_t mask, uint8_t mode)
{
    if (mode == OUTPUT)
    {
        mcpClearRegisterMask(MCP23017_IODIRB, mask);
    }
    else
    {
        mcpSetRegisterMask(MCP23017_IODIRB, mask);
    }
}


void mcpDigitalWriteA(uint8_t mask, uint8_t level) {
    uint8_t gpio = mcpReadRegister(MCP23017_OLATA);
    if (level == LOW)
    {
        gpio &= ~mask;
    }
    else
    {
        gpio |= mask;
    }
    mcpWriteRegister(MCP23017_GPIOA, gpio);
}


void mcpDigitalWriteB(uint8_t mask, uint8_t level) {
    uint8_t gpio = mcpReadRegister(MCP23017_OLATB);
    if (level == LOW)
    {
        gpio &= ~mask;
    }
    else
    {
        gpio |= mask;
    }
    mcpWriteRegister(MCP23017_GPIOB, gpio);
}


void mcpPullUpA(uint8_t mask, uint8_t level)
{
    if (level == LOW)
    {
        mcpClearRegisterMask(MCP23017_GPPUA, mask);
    }
    else
    {
        mcpSetRegisterMask(MCP23017_GPPUA, mask);
    }

}


void mcpPullUpB(uint8_t mask, uint8_t level)
{
    if (level == LOW)
    {
        mcpClearRegisterMask(MCP23017_GPPUB, mask);
    }
    else
    {
        mcpSetRegisterMask(MCP23017_GPPUB, mask);
    }

}


uint8_t mcpReadGpioA()
{
    return mcpReadRegister(MCP23017_GPIOA);
}


uint8_t mcpReadGpioB()
{
    return mcpReadRegister(MCP23017_GPIOB);
}


uint16_t mcpReadGpioAB()
{
    uint16_t ba;
    ba = mcpReadRegister(MCP23017_GPIOB) << 8 | mcpReadRegister(MCP23017_GPIOA);
    //printValue("ba", ba);
    //printValue("twoBytes",  mcp23017ReadTwoBytes(MCP23017_GPIOA));
    return ba;
}


void mcpWriteGpioAB(uint16_t ba)
{
    i2c_start(i2cAddress + I2C_WRITE);
    i2c_write(MCP23017_GPIOA);
    i2c_write(ba & 0xFF);
    i2c_write(ba >> 8);
    i2c_stop();
}
