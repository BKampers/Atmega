#ifndef _LCD_H_
#define _LCD_H_

#include <stdint.h>

#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

#define SELECT_BUTTON_PIN 0
#define RIGHT_BUTTON_PIN 1
#define DOWN_BUTTON_PIN 2
#define UP_BUTTON_PIN 3
#define LEFT_BUTTON_PIN 4

void lcdBegin(uint8_t address, uint8_t cols, uint8_t lines);
void lcdSetBacklight(uint8_t status);
void lcdClear();
void lcdHome();
void lcdSetCursor(uint8_t col, uint8_t row);
void lcdNoDisplay();
void lcdDisplay();
void lcdNoCursor();
void lcdCursor();
void lcdNoBlink();
void lcdBlink();
void lcdScrollDisplayLeft();
void lcdScrollDisplayRight();
void lcdLeftToRight();
void lcdRightToLeft();
void lcdAutoscroll();
void lcdNoAutoscroll();
void lcdCreateChar(char location, const uint8_t charmap[]);
void lcdPrint(const char* text);
uint8_t lcdButtonState();

#endif /* _LCD_H_ */