#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "lcd.h"
#include "uart.h"

#define LCD_ADDRESS 0x40
#define COLUMN_COUNT 16
#define LINE_COUNT 2


#define PRESSED_CHAR '\1'
#define RELEASED_CHAR '\2'
#define UNCHANGED_CHAR '\3'

const uint8_t PRESSED_MAP[] = {
    0b00000,
    0b00000,
    0b00000,
    0b01110,
    0b01110,
    0b01110,
    0b11111,
    0b11111
};


const uint8_t RELEASED_MAP[] = {
    0b00000,
    0b01110,
    0b01110,
    0b01110,
    0b01110,
    0b01110,
    0b11111,
    0b11111
};


const uint8_t UNCHANGED_MAP[] = {
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b11111,
    0b11111
};


char buffer[17];
uint8_t previousButtonState = 0x00;
uint8_t pressed;
uint8_t released;


void setup()
{
    // Debugging output
    initUart();
    _delay_ms(1);
    print("\r\nsetup\r\n");
    // set up the LCD's number of columns and rows:
    lcdBegin(LCD_ADDRESS, COLUMN_COUNT, LINE_COUNT);
    lcdNoCursor();
    lcdNoBlink();
    lcdSetCursor(10, 0);
    lcdPrint("Hello");
    lcdSetCursor(10, 1);
    lcdPrint("World!");
    lcdCreateChar(PRESSED_CHAR, PRESSED_MAP);
    lcdCreateChar(RELEASED_CHAR, RELEASED_MAP);
    lcdCreateChar(UNCHANGED_CHAR, UNCHANGED_MAP);
    buffer[16] = '\0';
}


void setBufferChar(uint8_t index)
{
    
}


int main(void)
{
    setup();
    print("\r\nAdafruit i2c 16x2 RGB\r\n");
    lcdSetCursor(0, 0);
    lcdPrint("SRDUL");
    for (;;) 
    {
        uint8_t buttonState = lcdButtonState();
        printValue("buttonState", buttonState);
        pressed = (buttonState ^ previousButtonState) & previousButtonState;
        released = (buttonState ^ previousButtonState) & ~previousButtonState;
        for (uint8_t i = SELECT_BUTTON_PIN; i <= LEFT_BUTTON_PIN; ++i)
        {
            buffer[i] = ((pressed & _BV(i)) != 0) ? PRESSED_CHAR : ((released & _BV(i)) != 0) ? RELEASED_CHAR : UNCHANGED_CHAR;
        }
        buffer[5] = '\0';
        lcdSetCursor(0, 1);
        lcdPrint(buffer);
        previousButtonState = buttonState;
        _delay_ms(250);
    }
}

