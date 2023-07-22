
// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef DISPLAY_H
#define	DISPLAY_H

#include <xc.h>
#include "lcd.h"
#ifdef	__cplusplus
extern "C" {
    #endif /* __cplusplus */

    #define _XTAL_FREQ 10000000L
    void display_number(unsigned short number){
        if(number == 9){
            PORTD = 0b01101111; // display 9
        }
        else if(number == 8){
            PORTD = 0b01111111; // display 8
        }
        else if(number == 7){
            PORTD = 0b00000111; // display 7
        }
        else if(number == 6){
            PORTD = 0b01111101; // display 6
        }
        else if(number == 5){
            PORTD = 0b01101101; // display 5
        }
        else if(number == 4){
            PORTD = 0b01100110; // display 4
        }
        else if(number == 3){
            PORTD = 0b01001111; // display 3
        }
        else if(number == 2){
            PORTD = 0b01011011; // display 2
        }
        else if(number == 1){
            PORTD = 0b00000110; // display 1
        }
        else{
            PORTD = 0b00111111; // display 0
        }
        
    }
    void display_nothing(){
        PORTD = 0;
    }
    void display_dash(){
        PORTD = 0x40;
    }
    
    void LCD_add_characters(){
        LCD_add_special_character(0, teamA_player);
        LCD_add_special_character(1, teamB_player);
        LCD_add_special_character(2, selected_teamA_player);
        LCD_add_special_character(3, selected_teamB_player);
        LCD_add_special_character(4, selected_teamA_player_with_frisbee);
        LCD_add_special_character(5, selected_teamB_player_with_frisbee);
        LCD_add_special_character(6, frisbee);
        LCD_add_special_character(7, frisbee_target);
    }
    
    void LCD_place_characters(){
        LCD_goto(players[0][0], players[0][1]);
        LCD_dat(2);
        LCD_goto(players[1][0], players[1][1]);
        LCD_dat(0);
        LCD_goto(players[2][0], players[2][1]);
        LCD_dat(1);
        LCD_goto(players[3][0], players[3][1]);
        LCD_dat(1);
        LCD_goto(frisbee_coordinates[0], frisbee_coordinates[1]);
        LCD_dat(6);
    }
    #ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* DISPLAY_H */


