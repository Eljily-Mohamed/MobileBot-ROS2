/*
 * groveLCD.h
 *
 *  Created on: Oct 16, 2019
 *      Author: kerhoas
 */
#ifndef INC_GROVELCD_H_
#define INC_GROVELCD_H_

#include "main.h"

// Device I2C Arress
#define LCD_ADDRESS     (0x7c)
#define RGB_ADDRESS     (0xc4)


// color define
#define WHITE           0
#define RED             1
#define GREEN           2
#define BLUE            3

#define REG_RED         0x04        // pwm2
#define REG_GREEN       0x03        // pwm1
#define REG_BLUE        0x02        // pwm0

#define REG_MODE1       0x00
#define REG_MODE2       0x01
#define REG_OUTPUT      0x08

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

void groveLCD_test();
void groveLCD_begin(uint8_t cols, uint8_t lines, uint8_t dotsize);
void groveLCD_setColorAll();
void groveLCD_setColorWhite();
void groveLCD_clear();
void groveLCD_home();
void groveLCD_setCursor(uint8_t col, uint8_t row);
void groveLCD_noDisplay();
void groveLCD_display() ;
void groveLCD_noCursor();
void groveLCD_cursor() ;
void groveLCD_noBlink();
void groveLCD_blink();
void groveLCD_scrollDisplayLeft(void);
void groveLCD_scrollDisplayRight(void);
void groveLCD_leftToRight(void);
void groveLCD_rightToLeft(void);
void groveLCD_autoscroll(void);
void groveLCD_noAutoscroll(void);
void groveLCD_createChar(uint8_t location, uint8_t charmap[]);
void groveLCD_blinkLED(void);
void groveLCD_noBlinkLED(void);
void groveLCD_command(uint8_t value);
int groveLCD_write(uint8_t value);
void groveLCD_setReg(unsigned char addr, unsigned char dta);
void groveLCD_setRGB(unsigned char r, unsigned char g, unsigned char b);
void groveLCD_setColor(unsigned char color);
void groveLCD_putString(char* s);
void groveLCD_term_printf(const char* fmt, ...);



#endif /* INC_GROVELCD_H_ */
