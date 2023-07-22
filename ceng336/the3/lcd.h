/* 
 * File:   LCD.h
 *
 */

#ifndef LCD_H
#define	LCD_H

#ifdef	__cplusplus
extern "C" {
#endif
#include <xc.h>

typedef unsigned char byte;     // define byte here for readability and sanity.

#define _XTAL_FREQ 10000000L

#define LCD_RS PORTEbits.RE2
#define LCD_EN PORTEbits.RE1
#define LCD_PORT PORTD
#define LCD_PULSE_TIME 40

#define LCD_FS_1LINE 0x30
#define LCD_FS_2LINE 0x38
#define ROW1 0x80
#define ROW2 0xC0
#define ROW3 0x90
#define ROW4 0xD0
#define LCD_OFF 0x08
#define LCD_ON  0x0C
#define LCD_CURSOR_ON 0x0A
#define LCD_CURSOR_BLINK 0x09
#define LCD_ENTRY_FORWARD 0x06
#define LCD_ENTRY_REVERSE 0x04
#define LCD_ENTRY_SLIDE 0x05
#define LCD_CLEAR 0x01
#define LCD_CURSOR_MOVE_RIGHT 0x14
#define LCD_CURSOR_MOVE_LEFT 0x10
#define LCD_CONTENT_MOVE_RIGHT 0x1C
#define LCD_CONTENT_MOVE_LEFT 0x18
  
byte lcd_x = 1, lcd_y = 1;  // indices start from 1
  
void LCD_init(void);
void LCD_cmd(unsigned char cmd);
void LCD_dat(unsigned char dat);
void LCD_str(const char* str);
void LCD_add_special_character(byte character_index, byte * data);
void LCD_goto(byte p_2, byte p_1);

void LCD_cmd(unsigned char cmd) {
  PORTEbits.RE1 = 0;
  PORTEbits.RE2 = 0;
  PORTD = cmd;
  PORTEbits.RE1 = 1;
  __delay_us(LCD_PULSE_TIME);
  PORTEbits.RE1 = 0;
  __delay_us(LCD_PULSE_TIME);
}

void LCD_dat(unsigned char dat) {
  PORTEbits.RE1 = 0;
  PORTEbits.RE2 = 1;
  PORTD = dat;
  PORTEbits.RE1 = 1;
  __delay_us(LCD_PULSE_TIME);
  PORTEbits.RE1 = 0;
  __delay_us(LCD_PULSE_TIME);
  PORTEbits.RE2 = 0;
}

//Sets the current display cursor of the LCD
// p_1 : the row at which the text will be displayed, a value from [1, 4]
// p_2 : the column  at which the text will be displayed, a value from [1, 16]
void LCD_goto(byte p_2, byte p_1) {
  if(p_1==1) {
    LCD_cmd(ROW1+((p_2-1)%16));
  } 
  else if (p_1==2){
    LCD_cmd(ROW2+((p_2-1)%16));
  }
  else if (p_1==3){
    LCD_cmd(ROW3+((p_2-1)%16));
  }
  else {
    LCD_cmd(ROW4+((p_2-1)%16));
  }
  lcd_x = p_2;
  lcd_y = p_1;
}

// Prints the given null terminated string `str` at the current cursor position.
// It auto-wraps the given string if it doesn't fit the display.
void LCD_str(const char* str) {
  for (unsigned char i = 0; str[i] != 0; i++) {
    LCD_dat(str[i]);
    lcd_x ++;
    if(lcd_x == 17) {
      lcd_x = 1;
      lcd_y++;
      if (lcd_y == 5) {
         lcd_y = 1;
      }
      //LCDGoto(lcd_x, lcd_y);
    }
  }
}

// Initializes the LCD
// First sets the TRIS as required then configures LCD
void LCD_init(void) {
  TRISEbits.RE1 = 0;
  TRISEbits.RE2 = 0;
  TRISD = 0;
    
  PORTEbits.RE1 = 0;
  PORTEbits.RE2 = 0;
  LCD_cmd(LCD_FS_2LINE);
  LCD_cmd(LCD_OFF);
  LCD_cmd(LCD_ON);
  LCD_cmd(LCD_ENTRY_FORWARD);
  LCD_cmd(LCD_CLEAR);
  LCD_cmd(ROW1);
}



// Stores the custom character provided in the CGRAM of LCD character_index:
// specifies the index of the character in CGRAM, [0, 7] max 8 characters can be
// designed (for 16x4 display) data: is a array of 8 bytes where each entry
// specifies one of the character (5x8 dot)
void LCD_add_special_character(byte character_index, byte * data) {
  //Each custom character occupies 8 byte location
  // First custom character address 0x40-0x47
  // Second custom character address 0x48-0x4f
  // And so on...
    LCD_cmd(0b01000000+(character_index*8));
    for(byte i=0; i < 8; i++) {
        LCD_dat(data[i]);
    }
    //LCDGoto(lcd_x, lcd_y);-
}

#ifdef	__cplusplus
}
#endif

#endif	/* LCD_H */