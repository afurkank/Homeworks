PROCESSOR    18F4620

#include <xc.inc>

; CONFIGURATION (DO NOT EDIT)
CONFIG OSC = HSPLL      ; Oscillator Selection bits (HS oscillator, PLL enabled (Clock Frequency = 4 x FOSC1))
CONFIG FCMEN = OFF      ; Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
CONFIG IESO = OFF       ; Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)
; CONFIG2L
CONFIG PWRT = ON        ; Power-up Timer Enable bit (PWRT enabled)
CONFIG BOREN = OFF      ; Brown-out Reset Enable bits (Brown-out Reset disabled in hardware and software)
CONFIG BORV = 3         ; Brown Out Reset Voltage bits (Minimum setting)
; CONFIG2H
CONFIG WDT = OFF        ; Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
; CONFIG3H
CONFIG PBADEN = OFF     ; PORTB A/D Enable bit (PORTB<4:0> pins are configured as digital I/O on Reset)
CONFIG LPT1OSC = OFF    ; Low-Power Timer1 Oscillator Enable bit (Timer1 configured for higher power operation)
CONFIG MCLRE = ON       ; MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled)
; CONFIG4L
CONFIG LVP = OFF        ; Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
CONFIG XINST = OFF      ; Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))


; GLOBAL SYMBOLS
; You need to add your variables here if you want to debug them.
GLOBAL counter1

; Define space for the variables in RAM
PSECT udata_acs
counter1:
    DS 1

  
PSECT CODE
; Reset vector
org 0x0000
  goto main

org 0x0008
interrupt_service_routine:
    ; WREG, STATUS and BSR registers are automatically saved to shadow registers
    ; They can be restored with retfie 1 command
    ; Check each interrupt flag. If it matches, go to its interrupt handler
    btfsc   PIR1, 1           ; Check Timer2 flag, skip if clear
    goto    timer2_interrupt
    retfie  1                 ; No match

timer2_interrupt:
    bcf     PIR1, 1         ; Clear Timer2 flag
    bcf     T2CON, 2        ; Disable timer 2
    comf LATA
    retfie 1


main:
    ; Initialize variables
    clrf counter1
    clrf TRISA
  
    ; Enable Timer2 interrupt
    bsf     PIE1, 1
    ; Enable global and peripheral interrupts by setting GIE and PEIE bits to 1
    movlw   11000000B 
    movwf   INTCON

    call start_timer_2ms
main_loop:
    bra main_loop

; Timer2 interrupt in 256 microseconds
start_timer_short:
    clrf    TMR2
    ; Bits 6-3 T2OUTPS3:T2OUTPS0 = 0000, no postscaling.
    ; Bit 2 TMR2ON = 1, enable timer2 now.
    ; Bits 1-0 T2CKPS1:T2CKPS0 = 00, no prescaling.
    movlw   00000100B
    movwf   T2CON
    return

; Timer2 interrupt in 4 milliseconds
start_timer_4ms:
    clrf    TMR2
    ; Bits 6-3 T2OUTPS3:T2OUTPS0 = 0000, no postscaling.
    ; Bit 2 TMR2ON = 1, enable timer2 now.
    ; Bits 1-0 T2CKPS1:T2CKPS0 = 11, 1:16 prescaling.
    movlw   00000111B
    movwf   T2CON
    return

; Timer2 interrupt in 2 milliseconds
start_timer_2ms:
    movlw   128
    movwf   TMR2
    ; Bits 6-3 T2OUTPS3:T2OUTPS0 = 0000, no postscaling.
    ; Bit 2 TMR2ON = 1, enable timer2 now.
    ; Bits 1-0 T2CKPS1:T2CKPS0 = 11, 1:16 prescaling.
    movlw   00000111B
    movwf   T2CON
    return
