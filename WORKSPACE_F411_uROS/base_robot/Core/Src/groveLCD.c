/*
 * groveLCD.c
 *
 *  Created on: Jan 8, 2020
 *      Author: kerhoas
 */

#include "groveLCD.h"
#include <math.h>
#include "util.h"

  uint8_t _displayfunction;
  uint8_t _displaycontrol;
  uint8_t _displaymode;
  uint8_t _initialized;
  uint8_t _numlines,_currline;

//=================================================================
void groveLCD_test()
{
	int8_t tab[1];
	tab[1] = 100;
  i2c1_WriteRegBuffer(RGB_ADDRESS, REG_RED,  tab, 1);

}
//=================================================================
void i2c_send_byte(unsigned char dta)
{
	i2c1_WriteBuffer(LCD_ADDRESS, &dta, 1);
}
//=================================================================
void i2c_send_byteS(unsigned char *dta, unsigned char len)
{
	i2c1_WriteBuffer(LCD_ADDRESS, dta, len);
}
//=================================================================
void groveLCD_begin(uint8_t cols, uint8_t lines, uint8_t dotsize)
{
    if (lines > 1) {
        _displayfunction |= LCD_2LINE;
    }
    _numlines = lines;
    _currline = 0;

    // for some 1 line displays you can select a 10 pixel high font
    if ((dotsize != 0) && (lines == 1)) {
        _displayfunction |= LCD_5x10DOTS;
    }

    // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
    // according to datasheet, we need at least 40ms after power rises above 2.7V
    // before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
    HAL_Delay(50);


    // this is according to the hitachi HD44780 datasheet
    // page 45 figure 23

    // Send function set command sequence
    groveLCD_command(LCD_FUNCTIONSET | _displayfunction);
    HAL_Delay(5);  // wait more than 4.1ms

    // second try
    groveLCD_command(LCD_FUNCTIONSET | _displayfunction);
    HAL_Delay(5);

    // third go
    groveLCD_command(LCD_FUNCTIONSET | _displayfunction);


    // finally, set # lines, font size, etc.
    groveLCD_command(LCD_FUNCTIONSET | _displayfunction);

    // turn the display on with no cursor or blinking default
    _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    groveLCD_display();

    // clear it off
    groveLCD_clear();

    // Initialize to default text direction (for romance languages)
    _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    // set the entry mode
    groveLCD_command(LCD_ENTRYMODESET | _displaymode);


    // backlight init
    groveLCD_setReg(REG_MODE1, 0);
    // set LEDs controllable by both PWM and GRPPWM registers
    groveLCD_setReg(REG_OUTPUT, 0xFF);
    // set MODE2 values
    // 0010 0000 -> 0x20  (DMBLNK to 1, ie blinky mode)
    groveLCD_setReg(REG_MODE2, 0x20);

    groveLCD_setColorWhite();

}
//=================================================================
void groveLCD_setColorAll(){groveLCD_setRGB(0, 0, 0);}
void groveLCD_setColorWhite(){groveLCD_setRGB(255, 255, 255);}
//=================================================================

/********** high level commands, for the user! */
void groveLCD_clear()
{
	groveLCD_command(LCD_CLEARDISPLAY);        // clear display, set cursor position to zero
    HAL_Delay(2000);          // this command takes a long time!
}
//=================================================================
void groveLCD_home()
{
	groveLCD_command(LCD_RETURNHOME);        // set cursor position to zero
    HAL_Delay(2000);        // this command takes a long time!
}
//=================================================================
void groveLCD_setCursor(uint8_t col, uint8_t row)
{
    col = (row == 0 ? col|0x80 : col|0xc0);
    unsigned char dta[2] = {0x80, col};
    i2c_send_byteS(dta, 2);
}
//=================================================================
// Turn the display on/off (quickly)
void groveLCD_noDisplay()
{
    _displaycontrol &= ~LCD_DISPLAYON;
    groveLCD_command(LCD_DISPLAYCONTROL | _displaycontrol);
}
//=================================================================
void groveLCD_display() {
    _displaycontrol |= LCD_DISPLAYON;
    groveLCD_command(LCD_DISPLAYCONTROL | _displaycontrol);
}
//=================================================================
// Turns the underline cursor on/off
void groveLCD_noCursor()
{
    _displaycontrol &= ~LCD_CURSORON;
    groveLCD_command(LCD_DISPLAYCONTROL | _displaycontrol);
}
//=================================================================
void groveLCD_cursor() {
    _displaycontrol |= LCD_CURSORON;
    groveLCD_command(LCD_DISPLAYCONTROL | _displaycontrol);
}
//=================================================================
// Turn on and off the blinking cursor
void groveLCD_noBlink()
{
    _displaycontrol &= ~LCD_BLINKON;
    groveLCD_command(LCD_DISPLAYCONTROL | _displaycontrol);
}
//=================================================================
void groveLCD_blink()
{
    _displaycontrol |= LCD_BLINKON;
    groveLCD_command(LCD_DISPLAYCONTROL | _displaycontrol);
}
//=================================================================
// These commands scroll the display without changing the RAM
void groveLCD_scrollDisplayLeft(void)
{
	groveLCD_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
//=================================================================
void groveLCD_scrollDisplayRight(void)
{
	groveLCD_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}
//=================================================================
// This is for text that flows Left to Right
void groveLCD_leftToRight(void)
{
    _displaymode |= LCD_ENTRYLEFT;
    groveLCD_command(LCD_ENTRYMODESET | _displaymode);
}
//=================================================================
// This is for text that flows Right to Left
void groveLCD_rightToLeft(void)
{
    _displaymode &= ~LCD_ENTRYLEFT;
    groveLCD_command(LCD_ENTRYMODESET | _displaymode);
}
//=================================================================
// This will 'right justify' text from the cursor
void groveLCD_autoscroll(void)
{
    _displaymode |= LCD_ENTRYSHIFTINCREMENT;
    groveLCD_command(LCD_ENTRYMODESET | _displaymode);
}
//=================================================================
// This will 'left justify' text from the cursor
void groveLCD_noAutoscroll(void)
{
    _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
    groveLCD_command(LCD_ENTRYMODESET | _displaymode);
}
//=================================================================
// Allows us to fill the first 8 CGRAM locations
// with custom characters
void groveLCD_createChar(uint8_t location, uint8_t charmap[])
{
    location &= 0x7; // we only have 8 locations 0-7
    groveLCD_command(LCD_SETCGRAMADDR | (location << 3));

    unsigned char dta[9];
    dta[0] = 0x40;
    for(int i=0; i<8; i++)
    {
        dta[i+1] = charmap[i];
    }
    i2c_send_byteS(dta, 9);
}
//=================================================================
// Control the backlight LED blinking
void groveLCD_blinkLED(void)
{
    // blink period in seconds = (<reg 7> + 1) / 24
    // on/off ratio = <reg 6> / 256
	groveLCD_setReg(0x07, 0x17);  // blink every second
	groveLCD_setReg(0x06, 0x7f);  // half on, half off
}
//=================================================================
void groveLCD_noBlinkLED(void)
{
	groveLCD_setReg(0x07, 0x00);
	groveLCD_setReg(0x06, 0xff);
}
//=================================================================
/*********** mid level commands, for sending data/cmds */

// send command
void groveLCD_command(uint8_t value)
{
    unsigned char dta[2] = {0x80, value};
    i2c_send_byteS(dta, 2);
}
//=================================================================
// send data
int groveLCD_write(uint8_t value)
{
    unsigned char dta[2] = {0x40, value};
    i2c_send_byteS(dta, 2);
    return 1; // assume sucess
}
//=================================================================
void groveLCD_putString(char* s)
{
	while(*s != '\0')
	{
		groveLCD_write(*s);
		s++;
	}
}
//=================================================================
void groveLCD_setReg(unsigned char addr, unsigned char dta)
{
    i2c1_WriteRegBuffer(RGB_ADDRESS, addr,  &dta, 1);
}
//=================================================================
void groveLCD_setRGB(unsigned char r, unsigned char g, unsigned char b)
{
	groveLCD_setReg(REG_RED, r);
	groveLCD_setReg(REG_GREEN, g);
	groveLCD_setReg(REG_BLUE, b);
}
//=================================================================
const unsigned char color_define[4][3] =
{
    {255, 255, 255},            // white
    {255, 0, 0},                // red
    {0, 255, 0},                // green
    {0, 0, 255},                // blue
};
//=================================================================
void groveLCD_setColor(unsigned char color)
{
    if(color > 3)return ;
    groveLCD_setRGB(color_define[color][0], color_define[color][1], color_define[color][2]);
}
//============================================================
void groveLCD_term_printf(const char* fmt, ...)
{
	__gnuc_va_list         ap;
	char          *p;
	char           ch;
	unsigned long  ul;
	unsigned long long ull;
	unsigned long  size;
	unsigned int   sp;
	char           s[60];
	int first=0;

	va_start(ap, fmt);

	while (*fmt != '\0') {
		if (*fmt =='%') {
			size=0; sp=1;
			if (*++fmt=='0') {fmt++; sp=0;}	// parse %04d --> sp=0
			ch=*fmt;
			if ((ch>'0') && (ch<='9')) {	// parse %4d --> size=4
				char tmp[10];
				int i=0;
				while ((ch>='0') && (ch<='9')) {
					tmp[i++]=ch;
					ch=*++fmt;
				}
				tmp[i]='\0';
				size=str2num(tmp,10);
			}
			switch (ch) {
				case '%':
					groveLCD_write('%');
					break;
				case 'c':
					ch = va_arg(ap, int);
					groveLCD_write(ch);
					break;
				case 's':
					p = va_arg(ap, char *);
					groveLCD_putString(p);
					break;
				case 'd':
					ul = va_arg(ap, long);
					if ((long)ul < 0) {
						groveLCD_write('-');
						ul = -(long)ul;
						//size--;
					}
					num2str(s, ul, 10, size, sp);
					groveLCD_putString(s);
					break;
				case 'u':
					ul = va_arg(ap, unsigned int);
					num2str(s, ul, 10, size, sp);
					groveLCD_putString(s);
					break;
				case 'o':
					ul = va_arg(ap, unsigned int);
					num2str(s, ul, 8, size, sp);
					groveLCD_putString(s);
					break;
				case 'p':
					groveLCD_write('0');
					groveLCD_write('x');
					ul = va_arg(ap, unsigned int);
					num2str(s, ul, 16, size, sp);
					groveLCD_putString(s);
					break;
				case 'x':
					ul = va_arg(ap, unsigned int);
					num2str(s, ul, 16, size, sp);
					groveLCD_putString(s);
					break;
				case 'f':
					if(first==0){ ull = va_arg(ap, long long unsigned int); first = 1;}
					ull = va_arg(ap, long long unsigned int);
					int sign = ( ull & 0x80000000 ) >> 31;
					int m = (ull & 0x000FFFFF) ; // should be 0x007FFFFF
					float mf = (float)m ;
					mf = mf / pow(2.0,20.0);
					mf = mf + 1.0;
					int e = ( ull & 0x78000000 ) >> 23 ; // should be int e = ( ul & 0x7F800000 ) >> 23;
					e = e | (( ull & 0x000F00000 ) >> 20);
					e = e - 127;
					float f = mf*myPow(2.0,e);
					if(sign==1){ groveLCD_write('-'); }
					float2str((char*)s, f, 5);
					groveLCD_putString((char*)s);
					break;

				default:
					groveLCD_write(*fmt);
			}
		} else groveLCD_write(*fmt);
		fmt++;
	}
	va_end(ap);
}
//=================================================================




