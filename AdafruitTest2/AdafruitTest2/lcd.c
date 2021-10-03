#include "lcd.h"

#include <avr/io.h>
#include <util/delay.h>
#include "mcp23x17.h"
//#include "uart.h" 

// commands
#define LCD_CLEARDISPLAY 0x01 //!< Clear display, set cursor position to zero
#define LCD_RETURNHOME 0x02   //!< Set cursor position to zero
#define LCD_ENTRYMODESET 0x04 //!< Sets the entry mode
#define LCD_DISPLAYCONTROL 0x08 //!< Controls the display; allows you to do stuff like turn it on and off
#define LCD_CURSORSHIFT 0x10 //!< Lets you move the cursor
#define LCD_FUNCTIONSET 0x20 //!< Used to send the function set to the display
#define LCD_SETCGRAMADDR 0x40 //!< Used to set the CGRAM (character generator RAM)
#define LCD_SETDDRAMADDR 0x80 //!< Used to set the DDRAM (display data RAM)

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00 //!< Used to set text to flow from right to left
#define LCD_ENTRYLEFT 0x02 //!< Used to set text to flow from left to right
#define LCD_ENTRYSHIFTINCREMENT 0x01 //!< Used to 'right justify' text from the cursor
#define LCD_ENTRYSHIFTDECREMENT 0x00 //!< USed to 'left justify' text from the cursor

// flags for display on/off control
#define LCD_DISPLAYON 0x04  //!< Turns the display on
#define LCD_DISPLAYOFF 0x00 //!< Turns the display off
#define LCD_CURSORON 0x02   //!< Turns the cursor on
#define LCD_CURSOROFF 0x00  //!< Turns the cursor off
#define LCD_BLINKON 0x01    //!< Turns on the blinking cursor
#define LCD_BLINKOFF 0x00   //!< Turns off the blinking cursor

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08 //!< Flag for moving the display
#define LCD_CURSORMOVE 0x00  //!< Flag for moving the cursor
#define LCD_MOVERIGHT 0x04   //!< Flag for moving right
#define LCD_MOVELEFT 0x00    //!< Flag for moving left

// flags for function set
#define LCD_8BITMODE 0x10 //!< LCD 8 bit mode
#define LCD_4BITMODE 0x00 //!< LCD 4 bit mode
#define LCD_2LINE 0x08    //!< LCD 2 line mode
#define LCD_1LINE 0x00    //!< LCD 1 line mode
#define LCD_5x10DOTS 0x04 //!< 10 pixel high font mode
#define LCD_5x8DOTS 0x00  //!< 8 pixel high font mode

#define RS_PIN 15     // LOW: command.  HIGH: character.
#define RW_PIN 14     // LOW: write to LCD.  HIGH: read from LCD.
#define ENABLE_PIN 13 // activated by a HIGH pulse.

#define DATA_PIN_0 12
#define DATA_PIN_1 11
#define DATA_PIN_2 10
#define DATA_PIN_3 9
#define DATA_PIN_MASK (_BV(DATA_PIN_0) | _BV(DATA_PIN_1) | _BV(DATA_PIN_2) | _BV(DATA_PIN_3))

#define BACKLIGHT_PIN_0 6
#define BACKLIGHT_PIN_1 7
#define BACKLIGHT_PIN_2 8

#define BUTTON_PIN_MASK (_BV(SELECT_BUTTON_PIN) | _BV(RIGHT_BUTTON_PIN) | _BV(DOWN_BUTTON_PIN) | _BV(UP_BUTTON_PIN) | _BV(LEFT_BUTTON_PIN))

#define MAX_CHAR_LOCATION 7


#define BVA(i) _BV(i)
#define BVB(i) _BV(i-8)

uint8_t displayFunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
uint8_t displayControl;
uint8_t displayMode;
uint8_t lineCount;


void pulseEnable()
{
    mcpDigitalWriteB(BVB(ENABLE_PIN), LOW);
    _delay_us(1);
    mcpDigitalWriteB(BVB(ENABLE_PIN), HIGH);
    _delay_us(1); // enable pulse must be > 450 nanos
    mcpDigitalWriteB(BVB(ENABLE_PIN), LOW);
    _delay_us(100); // commands need > 37 micros to settle
}


void write4bits(uint8_t data)
{  
    uint16_t out = mcpReadGpioAB();
    out &= ~(_BV(ENABLE_PIN) | DATA_PIN_MASK);
    for (uint8_t i = 0; i < 4; ++i)
    {
        out |= ((data >> i) & 0x01) << (DATA_PIN_0 - i);
    }
    mcpWriteGpioAB(out);
    _delay_us(1);
    out |= _BV(ENABLE_PIN);
    mcpWriteGpioAB(out);
    _delay_us(1);
    out &= ~_BV(ENABLE_PIN);
    mcpWriteGpioAB(out);
    _delay_us(100);
}


// Not supported
//void write8bits(uint8_t value)
//{
    //for (int i = 0; i < 8; i++)
    //{
    //    mcpPinMode(_data_pins[i], OUTPUT);
    //    mcpDigitalWrite(_data_pins[i], (value >> i) & 0x01);
    //}
    //pulseEnable();
//}


void send(uint8_t data, uint8_t mode)
{
    mcpDigitalWriteB(BVB(RS_PIN), mode);
    mcpDigitalWriteB(BVB(RW_PIN), LOW); // RW pin LOW to write
    if ((displayFunction & LCD_8BITMODE) == 0)
    {
        write4bits(data >> 4);
        write4bits(data);
    }
    //else
    //{
        // Not supported
        //write8bits(value);
    //}
}


inline void sendCommand(uint8_t command)
{
    send(command, LOW);
}


void lcdBegin(uint8_t address, uint8_t cols, uint8_t lines) 
{
     mcpInit(address);
     mcpPinModeA(BVA(BACKLIGHT_PIN_1) | BVA(BACKLIGHT_PIN_0), OUTPUT);
     mcpPinModeB(BVB(BACKLIGHT_PIN_2) | BVB(DATA_PIN_0) | BVB(DATA_PIN_1) | BVB(DATA_PIN_2) | BVB(DATA_PIN_3) | BVB(RW_PIN) | BVB(RS_PIN) | BVB(ENABLE_PIN), OUTPUT);
     mcpPinModeA(BVA(SELECT_BUTTON_PIN) | BVA(RIGHT_BUTTON_PIN) | BVA(DOWN_BUTTON_PIN) | BVA(UP_BUTTON_PIN) | BVA(LEFT_BUTTON_PIN), INPUT);
     mcpPullUpA(BVA(SELECT_BUTTON_PIN) | BVA(RIGHT_BUTTON_PIN) | BVA(DOWN_BUTTON_PIN) | BVA(UP_BUTTON_PIN) | BVA(LEFT_BUTTON_PIN), HIGH);
     lcdSetBacklight(WHITE);    

    // for some 1 line displays you can select a 10 pixel high font
    if (lines == 1)
    {
        displayFunction |= LCD_5x10DOTS;
    }
    else if (lines > 1)
    {
        displayFunction |= LCD_2LINE;
    }
    lineCount = lines;

    // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
    // according to datasheet, we need at least 40ms after power rises above 2.7V
    // before sending commands. Arduino can turn on way befer 4.5V so we'll wait
    // 50
    _delay_us(50000);
    // Now we pull both RS and R/W low to begin commands
    mcpDigitalWriteB(BVB(RS_PIN) | BVB(ENABLE_PIN) | BVB(RW_PIN), LOW);
   
    // put the LCD into 4 bit or 8 bit mode
    if ((displayFunction & LCD_8BITMODE) == 0)
    {
        // this is according to the hitachi HD44780 datasheet
        // figure 24, pg 46

        // we start in 8bit mode, try to set 4 bit mode
        write4bits(0x03);
        _delay_us(4500); // wait min 4.1ms

        // second try
        write4bits(0x03);
        _delay_us(4500); // wait min 4.1ms

        // third go!
        write4bits(0x03);
        _delay_us(150);

        // finally, set to 8-bit interface
        write4bits(0x02);
    }
    else
    {
        // this is according to the hitachi HD44780 datasheet
        // page 45 figure 23

        // Send function set command sequence
        sendCommand(LCD_FUNCTIONSET | displayFunction);
        _delay_us(4500); // wait more than 4.1ms

        // second try
        sendCommand(LCD_FUNCTIONSET | displayFunction);
        _delay_us(150);

        // third go
        sendCommand(LCD_FUNCTIONSET | displayFunction);
    }

    // finally, set # lines, font size, etc.
    sendCommand(LCD_FUNCTIONSET | displayFunction);

    // turn the display on with no cursor or blinking default
    displayControl = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    lcdDisplay();

    // clear it off
    lcdClear();

    // Initialize to default text direction (for romance languages)
    displayMode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    // set the entry mode
    sendCommand(LCD_ENTRYMODESET | displayMode);
}


void lcdSetBacklight(uint8_t status)
{
    mcpDigitalWriteB(BVB(BACKLIGHT_PIN_2), ~(status >> 2) & 0x1);
    mcpDigitalWriteA(BVA(BACKLIGHT_PIN_1), ~(status >> 1) & 0x1);
    mcpDigitalWriteA(BVA(BACKLIGHT_PIN_0), ~status & 0x1);
}


void lcdClear()
{
    sendCommand(LCD_CLEARDISPLAY); // clear display, set cursor position to zero
    _delay_us(2000);   // this command takes a long time!
}


void lcdHome() {
    sendCommand(LCD_RETURNHOME); // set cursor position to zero
    _delay_us(2000); // this command takes a long time!
}


void lcdSetCursor(uint8_t col, uint8_t row)
{
    uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    if (row > lineCount)
    {
        row = lineCount - 1; // we count rows starting w/0
    }
    sendCommand(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}


void lcdNoDisplay()
{
    displayControl &= ~LCD_DISPLAYON;
    sendCommand(LCD_DISPLAYCONTROL | displayControl);
}


void lcdDisplay()
{
    displayControl |= LCD_DISPLAYON;
    sendCommand(LCD_DISPLAYCONTROL | displayControl);
}


void lcdNoCursor()
{
    displayControl &= ~LCD_CURSORON;
    sendCommand(LCD_DISPLAYCONTROL | displayControl);
}


void lcdCursor()
{
    displayControl |= LCD_CURSORON;
    sendCommand(LCD_DISPLAYCONTROL | displayControl);
}


void lcdNoBlink()
{
    displayControl &= ~LCD_BLINKON;
    sendCommand(LCD_DISPLAYCONTROL | displayControl);
}


void lcdBlink()
{
    displayControl |= LCD_BLINKON;
    sendCommand(LCD_DISPLAYCONTROL | displayControl);
}


void lcdScrollDisplayLeft()
{
    sendCommand(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}


void lcdScrollDisplayRight()
{
    sendCommand(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}


void lcdLeftToRight()
{
    displayMode |= LCD_ENTRYLEFT;
    sendCommand(LCD_ENTRYMODESET | displayMode);
}


void lcdRightToLeft()
{
    displayMode &= ~LCD_ENTRYLEFT;
    sendCommand(LCD_ENTRYMODESET | displayMode);
}


void lcdAutoscroll()
{
    displayMode |= LCD_ENTRYSHIFTINCREMENT;
    sendCommand(LCD_ENTRYMODESET | displayMode);
}


void lcdNoAutoscroll()
{
    displayMode &= ~LCD_ENTRYSHIFTINCREMENT;
    sendCommand(LCD_ENTRYMODESET | displayMode);
}


void lcdCreateChar(char location, const uint8_t charmap[])
{
    if (location <= MAX_CHAR_LOCATION) 
    { 
        sendCommand(LCD_SETCGRAMADDR | (location << 3));
        for (uint8_t i = 0; i < 8; ++i)
        {
            send(charmap[i], HIGH);;
        }
        sendCommand(LCD_SETDDRAMADDR); // resets the location to 0,0
    }
}


void lcdPrint(const char* text)
{
    while (*text != '\0') 
    {
        send(*text, HIGH);
        text++;
    }
 }


 uint8_t lcdButtonState    ()
 {
     return mcpReadGpioA() & BUTTON_PIN_MASK;
 }
