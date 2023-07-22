#include "lcd.h"
#include "the3.h"
#include "ADC.h"
#include "display.h"
#include <xc.h>
#include <pic18f4620.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#pragma config PBADEN = OFF
#define _XTAL_FREQ 10000000L

#define TMR0_400ms_preload 49911
#define TMR0_800ms_preload 34286
#define TMR0_1200ms_preload 18661
#define TMR0_1600ms_preload 3036

// <editor-fold defaultstate="collapsed" desc="Function Declarations">
bool RB0_valid();

void PORTS_init();
void INTERRUPTS_init();

void TMR0_load();
void TMR0_init();
void TMR0_isr();
void TMR0_interrupt_occurred();

void TMR1_init();
void TMR1_isr();
void TMR1_interrupt_occurred();

void TMR2_init();
void TMR2_isr();
void TMR2_interrupt_occurred();

void TMR3_init();
void TMR3_isr();

void update_display();
void initialize_character_coordinates();

void move_up();
void move_down();
void move_left();
void move_right();
// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="Initialize Global Variables">
char                    previous_PORTB = 0;

bool                  int0_int_flag = true;
bool                  int1_int_flag = true;
bool               gamepad_int_flag = true;
bool              tmr3_initialized = false;
bool             gamepad_interrupt = false;
bool          rb0_or_rb1_interrupt = false;

unsigned char              store_PORTD = 0;
unsigned char            current_PORTB = 0;

volatile char                    blink = 0;
volatile char                  CONVERT = 0;
volatile char                game_mode = 0;
volatile char                rb_button = 4;
volatile char              show_target = 0;
volatile char              rb0_pressed = 0;
volatile char         interrupt_source = 0;
volatile char        current_PORTB_RB4 = 0;
volatile char        current_PORTB_RB5 = 0;
volatile char        current_PORTB_RB6 = 0;
volatile char        current_PORTB_RB7 = 0;
volatile char       previous_PORTB_RB4 = 0;
volatile char       previous_PORTB_RB5 = 0;
volatile char       previous_PORTB_RB6 = 0;
volatile char       previous_PORTB_RB7 = 0;

unsigned short                   speed = 1;
unsigned short                  scoreA = 0;
unsigned short                  scoreB = 0;
unsigned short              convertion = 0;
unsigned short            num_of_steps = 0;
unsigned short           display_stage = 1;
unsigned short          tmr1_interrupt = 1;
unsigned short        frisbee_movement = 0;
unsigned short      selected_player_no = 0;
// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="Utility Functions">
bool RB0_valid(){
    for(int i = 0; i <= 3; i++){
        if(frisbee_coordinates[0] == players[i][0] && 
                frisbee_coordinates[1] == players[i][1]) return true;
    }
    return false;
}
void update_display(){
//    store_PORTD = PORTD;
//    INTCONbits.TMR0IE  = 0;
//    PIE1bits.TMR1IE    = 0;
//    PIE1bits.TMR2IE    = 0;
    LCD_cmd(LCD_CLEAR);
    bool flag = true;
    for(int i = 0; i <= 3; i++){
        unsigned char p_x = players[i][0];
        unsigned char p_y = players[i][1];
        unsigned char f_x = frisbee_coordinates[0];
        unsigned char f_y = frisbee_coordinates[1];
        LCD_goto(p_x, p_y);
        if(i<2){
            if(players[i][0] == f_x && players[i][1] == f_y){
                LCD_dat(4);
                flag = false;
                continue;
            }
            else{
                if(i == selected_player_no){
                    LCD_dat(2);
                }
                else{
                    LCD_dat(0);
                }
            }
        }
        else{
            if(players[i][0] == f_x && players[i][1] == f_y){
                LCD_dat(5);
                flag = false;
                continue;
            }
            else{ 
                if(i == selected_player_no){
                    LCD_dat(3);
                }
                else{
                    LCD_dat(1);
                }
            }
        }
    }
    if(flag){
        LCD_goto(frisbee_coordinates[0], frisbee_coordinates[1]);
        LCD_dat(6);
    }
//    PORTD = store_PORTD;
//    INTCONbits.TMR0IE  = 0;
//    PIE1bits.TMR1IE    = 0;
//    PIE1bits.TMR2IE    = 1;
}
void initialize_character_coordinates(){
    players[0][0] = 3; players[0][1] = 2;
    players[1][0] = 3; players[1][1] = 3;
    players[2][0] = 14; players[2][1] = 2;
    players[3][0] = 14; players[3][1] = 3;
    frisbee_coordinates[0] = 9;
    frisbee_coordinates[1] = 2;
}
void move_up(){
    bool flag = true;
    unsigned short x = players[selected_player_no][0];
    unsigned short y = players[selected_player_no][1];
    for(int i=0; i<=3; i++){
        if(x == players[i][0] && (y-1 == players[i][1])){
            flag = false;
            break;
        }
    }
    if(y != 1 && flag){
        players[selected_player_no][1] = y - 1;
    }
}
void move_down(){
    bool flag = true;
    unsigned short x = players[selected_player_no][0];
    unsigned short y = players[selected_player_no][1];
    for(int i=0; i<=3; i++){
        if(x == players[i][0] && (y+1 == players[i][1])){
            flag = false;
            break;
        }
    }
    if(y != 4 && flag){
        players[selected_player_no][1] = y + 1;
    }
}
void move_left(){
    bool flag = true;
    unsigned short x = players[selected_player_no][0];
    unsigned short y = players[selected_player_no][1];
    for(int i=0; i<=3; i++){
        if((x-1 == players[i][0]) && y == players[i][1]){
            flag = false;
            break;
        }
    }
    if(x != 1 && flag){
        players[selected_player_no][0] = x - 1;
    }
}
void move_right(){
    bool flag = true;
    unsigned short x = players[selected_player_no][0];
    unsigned short y = players[selected_player_no][1];
    for(int i=0; i<=3; i++){
        if((x+1 == players[i][0]) && y == players[i][1]){
            flag = false;
            break;
        }
    }
    if(x != 16 && flag){
        players[selected_player_no][0] = x + 1;
    }
}
// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="Initialize PORTS">
void PORTS_init(){
    TRISA = 0;          // PORTA is output
    TRISD = 0;          // PORTD is output
    
    // configure necessary PORTB pins as input
    TRISB = 0b11110011;
    
    // clear ports
    PORTB = 0;
    PORTA = 0;
    PORTD = 0;
    
    // configure analog and digital pins
    // AN2, AN1 and AN0 are analog
    // AN0 and AN1 is used for ADC
    // The rest is digital
    // AN10 and AN12 is used for RB0 and RB1
    ADCON1bits.PCFG3 = 1;
    ADCON1bits.PCFG2 = 1;
    ADCON1bits.PCFG1 = 0;
    ADCON1bits.PCFG0 = 0;
}
// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="Initialize Interrupts">
void INTERRUPTS_init(){
    INTCON = 0;              // clear INTCON
    
    INTCONbits.INT0IF = 0;   // clear INT0 interrupt flag
    INTCON3bits.INT1IF = 0;  // clear INT1 interrupt flag
    INTCONbits.TMR0IF = 0;   // clear TMR0 interrupt flag
    PIR1bits.TMR1IF = 0;     // clear TMR1 interrupt flag
    PIR1bits.TMR2IF = 0;     // clear TMR2 interrupt flag
    PIR2bits.TMR3IF = 0;     // clear TMR3 interrupt flag
    //PIR1bits.ADIF = 0;       // clear AD interrupt flag
    
    INTCON2bits.INTEDG0 = 1; // rising edge triggered
    INTCON2bits.INTEDG1 = 1; // rising edge triggered
    
    // INT0 is high priority only so we don't need to configure it
    INTCON3bits.INT1IP = 1;  // configure INT1 as high priority
    IPR2bits.TMR3IP = 1;     // configure TMR3 as high priority
    
    RBIE = 0;
    
    INTCONbits.GIE = 1;      // enable global interrupts
    
    PORTB = PORTB;
    INTCONbits.RBIF = 0;     // clear RB interrupt flag
    
    INTCONbits.PEIE = 1;     // enable peripheral interrupts
    INTCONbits.RBIE = 1;     // enable RB interrupt
    INTCONbits.INT0IE = 1;   // enable INT0 interrupt
    INTCON3bits.INT1IE = 1;  // enable INT1 interrupt
    INTCONbits.TMR0IE = 1;   // enable TMR0 interrupt
    PIE1bits.TMR1IE = 1;     // enable TMR1 interrupt
    PIE1bits.TMR2IE = 1;     // enable TMR2 interrupt
    PIE2bits.TMR3IE = 1;     // enable TMR3 interrupt
    //PIE1bits.ADIE = 1;       // enable AD interrupt
}
// </editor-fold>

void __interrupt(high_priority) highPriorityISR(void) {
    if(INTCONbits.RBIF){
        current_PORTB = PORTB;
        NOP();
        INTCONbits.RBIF = 0;
        if(gamepad_int_flag){
            current_PORTB = (current_PORTB & 0xf0);
            if(current_PORTB == 0xE0){
                rb_button = 4;
            }
            else if(current_PORTB == 0xD0){
                rb_button = 5;
            }
            else if(current_PORTB == 0xB0){
                rb_button = 6;
            }
            else if(current_PORTB == 0x70){
                rb_button = 7;
            }
            TMR3_init();
            interrupt_source = 1;
        }
    }
    if(PIR1bits.ADIF){
        PIR1bits.ADIF = 0; // Clear interrupt flag
        convertion = (unsigned short)((ADRESH << 8)+ADRESL);
        CONVERT = 1;
    }
    if(INTCONbits.INT0IF){
        INTCONbits.INT0IF = 0;
        if(int0_int_flag){
            TMR3_init();
            rb0_pressed = 1;
            int0_int_flag = false;
            interrupt_source = 0;
        }
    }
    if(INTCON3bits.INT1IF){
        INTCON3bits.INT1IF = 0;
        if(int1_int_flag){
            TMR3_init();
            rb0_pressed = 0;
            int1_int_flag = false;
            interrupt_source = 0;
        }
    }
    if(INTCONbits.TMR0IF){
        TMR0_isr(); // interrupt depends on speed, default 400 ms
    }
    if(PIR1bits.TMR1IF){
        TMR1_isr(); // interrupts in 100 ms
    }
    if(PIR1bits.TMR2IF){
        TMR2_isr(); // interrupts in 1.2 ms
    }
    if(PIR2bits.TMR3IF){
        PIR2bits.TMR3IF = 0;
        T3CONbits.TMR3ON = 0;
        if(interrupt_source == 0){
            rb0_or_rb1_interrupt = true;
        }
        else{
            gamepad_interrupt = true;
        }
    }
}

// <editor-fold defaultstate="collapsed" desc="TMR0 Configurations">
void TMR0_isr(){
    INTCONbits.TMR0IF = 0; // clear TMR0 interrupt flag
    T0CONbits.TMR0ON = 0; // disable TMR0
    TMR0_interrupt_occurred();
    if(num_of_steps != 0){
        TMR0_init();
    }
    else{
        game_mode = 0;
        frisbee_movement = 0;
        unsigned char x = players[selected_player_no][0];
        unsigned char y = players[selected_player_no][1];
        unsigned char f_x = frisbee_target_loc[0];
        unsigned char f_y = frisbee_target_loc[1];
        if(x == f_x && y == f_y){
            LCD_goto(f_x, f_y);
            LCD_dat(4);
            if(selected_player_no < 2){
                scoreA++;
            }
            else{
                scoreB++;
            }
        }
    }
    INTCONbits.TMR0IE  = 0;
    PIE1bits.TMR1IE    = 0;
    PIE1bits.TMR2IE    = 0;
    update_display();
    PIE1bits.TMR2IE    = 1;
}
void TMR0_load(){ // timer for movement
    if(speed == 1){
        TMR0H = (TMR0_400ms_preload >> 8) & 0xff;
        TMR0L = TMR0_400ms_preload & 0xff;
    }
    else if(speed == 2){
        TMR0H = (TMR0_800ms_preload >> 8) & 0xff;
        TMR0L = TMR0_800ms_preload & 0xff;
    }
    else if(speed == 3){
        TMR0H = (TMR0_1200ms_preload >> 8) & 0xff;
        TMR0L = TMR0_1200ms_preload & 0xff;
    }
    else{
        TMR0H = (TMR0_1600ms_preload >> 8) & 0xff;
        TMR0L = TMR0_1600ms_preload & 0xff;
    }
}
void TMR0_init(){
    TMR0_load();
    // Bit 7   TMR0ON  = 1         enable Timer0 now
    // Bit 6   T08BIT  = 0         16 bit mode
    // Bit 5   T0CS    = 0         use internal instruction cycle clock
    // Bit 4   T0SE    = 0         increment on low-to-high transition
    // Bit 3   PSA     = 0         enable prescaling
    // Bit 2-0 T0PS2-0 = 101       1:64 prescaling
    T0CON = 0b10000101;
}
void TMR0_interrupt_occurred(){ // timer for movement
    if(frisbee_steps[frisbee_movement][0] != 0) frisbee_coordinates[0] = frisbee_steps[frisbee_movement][0]; 
    if(frisbee_steps[frisbee_movement][1] != 0) frisbee_coordinates[1] = frisbee_steps[frisbee_movement][1];         
    frisbee_movement++;
    for(int i = 0; i<=3; i++){
        if(i != selected_player_no){
            random_player_movement(i);
        }
    }
    num_of_steps--;
}
// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="TMR1 Configurations">
void TMR1_isr(){
    PIR1bits.TMR1IF = 0;  // clear TMR1 interrupt flag
    if(tmr1_interrupt==2){
        T1CONbits.TMR1ON = 0; // disable TMR1
        TMR1_interrupt_occurred();
        if(num_of_steps != 0){
            TMR1_init();
        }
        else{
            unsigned char x = players[selected_player_no][0];
            unsigned char y = players[selected_player_no][1];
            unsigned char f_x = frisbee_target_loc[0];
            unsigned char f_y = frisbee_target_loc[1];
            if(x == f_x && y == f_y){
                if(selected_player_no < 2){
                    LCD_goto(frisbee_target_loc[0], frisbee_target_loc[1]);
                    LCD_dat(4);
                }
                else{
                    LCD_goto(frisbee_target_loc[0], frisbee_target_loc[1]);
                    LCD_dat(5);
                }
            }
            else{
                LCD_goto(frisbee_target_loc[0], frisbee_target_loc[1]);
                LCD_dat(6);
            }
        }
        tmr1_interrupt = 1;
    }
    else{
        tmr1_interrupt++;
    }
}
void TMR1_init(){
	TMR1H = 0xf0;
    TMR1L = 0;
    // Bit 7   RD16        = 0      read/write in two 8-bit mode
    // Bit 6   T1RUN       = 1      device clock is derived from Timer1 oscillator
    // Bit 5-4 T1CKPS1-0   = 11     1:8 prescaling
    // Bit 3   T1OSCEN     = 1      oscillator is enabled
    // Bit 2   T1SYNC0     = 0      this bit is ignored
    // Bit 1   TMR1CS      = 0      use internal clock
    // Bit 0   TMR1ON      = 1      enable Timer1 now
    T1CON = 0b01001001;
}
void TMR1_interrupt_occurred(){ // timer for frisbee target blink
	if(!blink){
        LCD_goto(frisbee_target_loc[0], frisbee_target_loc[1]);
        LCD_dat(7);
        blink = 1;
    }
    else{
        LCD_goto(frisbee_target_loc[0], frisbee_target_loc[1]);
        LCD_str(" ");
        blink = 0;
    }
}
// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="TMR2 Configurations">
void TMR2_isr(){
    PIR1bits.TMR2IF = 0;    // clear TMR2 interrupt flag
    T2CONbits.TMR2ON = 0;   // disable TMR2
    TMR2_interrupt_occurred();
    TMR2_init();
}
void TMR2_init(){
    // timer for random number generator
    // Also used for 7-segment display
    TMR2 = 0; // clear TMR2
    // Bits 6-3     T2OUTPS3-0 = 0000    no postscaling
    // Bit  2       TMR2ON     = 1       enable timer2 now
    // Bits 1-0     T2CKPS1-0  = 10      1:4 prescaling
    T2CON = 0b00010101;
}
void TMR2_interrupt_occurred(){
    if(display_stage == 1){
        LATD = 0;
        PORTA = 0b00000100;
        display_nothing();
    }
    else if(display_stage == 2){
        LATD = 0;
        PORTA = 0b00001000;
        display_number(scoreA);
    }
    else if(display_stage == 3){
        LATD = 0;
        PORTA = 0b00010000;
        display_dash();
    }
    else{
        LATD = 0;
        PORTA = 0b00100000;
        display_number(scoreB);
        display_stage = 1;
        return;
    }
    display_stage+=1;
}
// </editor-fold>

void TMR3_init(){
    TMR3H = 0;
    TMR3L = 0;
    // Bit 7         RD1        = 1     configure as 16 bit
    // Bit 6 & Bit 3 T3CCP      = 00    not important
    // Bit 5 & Bit 4 T3CKPS     = 01    1:2 prescaling
    // Bit 2         T3SYNC     = 0     ignored
    // Bit 1         TMR3CS     = 0     use internal clock
    // Bit 0         TMR3ON     = 1     enable TMR3
    T3CON = 0b10110001;
}

void main(void) {
    PORTS_init();
    INTERRUPTS_init();
    LCD_init();
    initialize_character_coordinates();
    ADC_init();
    LCD_add_characters();
    LCD_place_characters();
    TMR2_init();
    readADCChannel(0);
    while(1){
        if(gamepad_interrupt){
            if(rb_button == 4) move_up();
            if(rb_button == 5) move_right();
            if(rb_button == 6) move_down();
            if(rb_button == 7) move_left();
            INTCONbits.TMR0IE  = 0;
            PIE1bits.TMR1IE    = 0;
            PIE1bits.TMR2IE    = 0;
            update_display();
            PORTB = 0xff;
            gamepad_interrupt = false;
            gamepad_int_flag = true;
            PIE1bits.TMR2IE    = 1;
        }
        if(rb0_or_rb1_interrupt){ //RB0 or RB1 pressed
            if(rb0_pressed){ // Start the game
                if(!game_mode && RB0_valid()){
                    game_mode = 1;
                    num_of_steps = compute_frisbee_target_and_route(
                            frisbee_coordinates[0],
                            frisbee_coordinates[1]);
                    TMR0_init();
                    TMR1_init();
                }
                int0_int_flag = true;
            }
            else{ // Switch player
                if(frisbee_coordinates[0] == players[selected_player_no][0] &&
                    frisbee_coordinates[1] == players[selected_player_no][1]){
                    //do nothing
                }
                else selected_player_no = (selected_player_no + 1) % 4; // change selected player 
                update_display();
                int1_int_flag = true;
            }
            rb0_or_rb1_interrupt = false;
        }
		if(CONVERT){
    		if(convertion <= 255) speed = 1;
    		else if(convertion <= 511) speed = 2;
    		else if(convertion <= 767) speed = 3;
    		else if(convertion <= 1023) speed = 4;
    		CONVERT = 0;
    		ADCON0bits.GODONE = 1; //Start convertion again
		}
    }
}
