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
GLOBAL current_rb4
GLOBAL current_rb5
GLOBAL current_rb6
GLOBAL current_rb7
GLOBAL last_rb4
GLOBAL last_rb5
GLOBAL last_rb6
GLOBAL last_rb7
GLOBAL speed
GLOBAL number
GLOBAL beat
GLOBAL bar_length
GLOBAL store_beat
GLOBAL metronome_running
GLOBAL display_stage
GLOBAL store_display_stage
GLOBAL beat_was_reset
GLOBAL current_PORTB
GLOBAL last_PORTB
GLOBAL store_speed
GLOBAL store_metronome_running

; Define space for the variables in RAM
PSECT udata_acs
current_rb4:
    DS 1
current_rb5:
    DS 1
current_rb6:
    DS 1
current_rb7:
    DS 1
last_rb4:
    DS 1
last_rb5:
    DS 1
last_rb6:
    DS 1
last_rb7:
    DS 1
speed:
    DS 1
number:
    DS 1
beat:
    DS 1
bar_length:
    DS 1
store_beat:
    DS 1
metronome_running:
    DS 1
display_stage:
    DS 1
store_display_stage:
    DS 1
beat_was_reset:
    DS 1
current_PORTB:
    DS 1
last_PORTB:
    DS 1
store_speed:
    DS 1
store_metronome_running:
    DS 1

PSECT CODE
org 0x0000
    goto main

;################################ INTERRUPT CONFIGURATIONS ###########################

org 0x0008
interrupt_service_routine:
    btfsc   INTCON, 0	      ; Check RB interrupt flag, skip if clear
    goto    rb_interrupt
    btfsc   PIR1, 1           ; Check Timer2 interrupt flag, skip if clear
    goto    timer2_interrupt
    btfsc   PIR1, 0           ; Check Timer1 interrupt flag, skip if clear
    goto    timer1_interrupt
    btfsc   INTCON, 2         ; Check Timer0 interrupt flag, skip if clear
    goto    timer0_interrupt
    retfie  1                 ; No match

timer2_interrupt:
    ; used for wait time delay between DIS1, DIS2, etc.
    bcf     PIR1, 1		    ; Clear Timer2 interrupt flag
    bcf     T2CON, 2                ; Disable Timer2
    call timer2_interrupt_occurred  ; display the next stage
    call start_timer2		    ; start Timer2 again
    retfie 1

timer1_interrupt:
    ; used to clear RC0 and RC1 pins
    bcf     PIR1, 0         ; Clear Timer1 interrupt flag
    bcf     T1CON, 0        ; Disable Timer1
    call timer1_interrupt_occurred
    ; Timer1 is started again after the beat is advanced at
    ; "timer0_interrupt_occurred"
    retfie 1

timer0_interrupt:
    ; used to advance current beat by 1
    ; configured according to the speed level
    bcf     INTCON, 2                    ; Clear Timer0 interrupt flag
    call    timer0_interrupt_occurred
    call    decide_load_value_for_timer	 ; load Timer0
    retfie 1

rb_interrupt:
    ; used to interrupt if an RB button is pressed
    movff    PORTB, current_PORTB ; read PORTB
    nop			          ; wait 1 instruction cycle
    bcf	     INTCON, 0	          ; Clear RB interrupt flag
    call     rb_interrupt_occurred
    retfie 1
    
;################################# MAIN FUNCTION ###############################

main:
    ; Initialize variables
    clrf TRISA ; PORTA is output
    clrf TRISD ; PORTD is output
    clrf TRISC ; PORTC is output
    setf TRISB ; PORTB is input

    ; variables to hold last and current values of portb pins
    ; used for checking if a click occurred
    clrf current_rb4
    clrf current_rb5
    clrf current_rb6
    clrf current_rb7
    clrf last_rb4
    clrf last_rb5
    clrf last_rb6
    clrf last_rb7

    ; variables to hold current and last values of PORTB
    ; Current and last values of PORTB are 0 initially.
    clrf current_PORTB
    clrf last_PORTB

    clrf speed                  ; holds speed
    clrf number                 ; holds an assigned number to display
    clrf beat                   ; holds current beat
    clrf bar_length             ; holds bar length
    clrf store_beat             ; stores current beat
    clrf store_display_stage    ; stores display_stage
    clrf store_speed		; stores speed
    clrf metronome_running	; metronome isn't running at first
    clrf store_metronome_running; stores metronome state
    clrf beat_was_reset		; holds whether beat was reset or not

    ; takes values 1, 2, 3, 4 to store which DIS stage we are currently at
    movlw 1
    movwf display_stage

    movlw 6
    movwf speed        ; speed starts as 6

    movlw 4
    movwf bar_length   ; default bar length is 4

    movlw 0
    movwf beat         ; beat starts at 0

    ; Enable global and peripheral interrupts
    ; by setting GIE and PEIE bits to 1
    bsf INTCON, 7
    bsf INTCON, 6

    ; Enable RB, Timer2, Timer1 and Timer0 interrupts
    bsf INTCON, 3	  ; Enable RB	  interrupt
    bsf   PIE1, 1   ; Enable Timer2 interrupt
    bsf   PIE1, 0   ; Enable Timer1 interrupt
    bsf INTCON, 5   ; Enable Timer0 interrupt

    ; Clear interrupt flags of RB, Timer2, Timer1, Timer0
    bcf INTCON, 0 ; clear RB interrupt flag
    bcf   PIR1, 1 ; clear Timer2 interrupt flag
    bcf   PIR1, 0 ; clear Timer1 interrupt flag
    bcf INTCON, 2 ; clear Timer0 interrupt flag

    call start_timer2 ; metronome starts displaying
  
;################################# MAIN LOOP #######################################  

main_loop:
    bra main_loop
    
;################################# RUNNING STATE ####################################

running_state:
    movlw 0
    movwf beat ; reset beat
    call timer0_interrupt_occurred	; display first beat right as the metronome starts
    call start_timer0			; start Timer0
    call decide_load_value_for_timer	; load Timer0
    ; we don't call "start_timer2" for display function here beacuse it is 
    ; already called at initialization.
    ; also we don't call "start_timer0" again and again since it is handled
    ; recursively at "timer0_interrupt".
    return

;################################# PAUSED STATE #####################################

paused_state:
    bcf T0CON, 7 ; disable Timer0(we aren't advancing beat in paused state)
    return

;############################## TIMER0 INTERRUPT ###################################

timer0_interrupt_occurred:
    ; interrupt to advance beat
    ; interrupt happens everytime timer0 overflows
    ; time is adjusted according to the speed
    ; Also, RC0 is set to 1 at the start of each beat
    ; and RC1 is set to 1 at the start of each bar.
    ; RC0 and RC1 are cleared with Timer1 interrupt.
    ; We enable Timer1 here so that after 50ms
    ; it clears RC0 and RC1
    movf bar_length, 0	    ; wreg = bar_length
    cpfslt beat		    ; if beat < bar_length, skip
    call reset_beat	    ; else, reset beat
    clrf WREG		    ; wreg = 0
    cpfsgt beat		    ; if beat > 0, skip
    call light_rc0_and_rc1  ; else, light up rc0 and rc1(start of bar)
    clrf WREG		    ; wreg = 0
    cpfseq beat		    ; if beat == 0, skip(it was reset)
    call light_rc0	    ; else, light up rc0
    call start_timer1	    ; clear RC0 and RC1
    incf beat		    ; advance beat
    return
reset_beat:
    clrf beat		    ; beat = 0
    return
light_rc0_and_rc1:
    movlw 00000011B
    movwf LATC		    ; light up RC0 and RC1
    return
light_rc0:
    movlw 00000001B
    movwf LATC		    ; light up RC0
    return

;############################## TIMER1 INTERRUPT ###############################

timer1_interrupt_occurred:
    ; interrupt to clear RC0 and RC1
    ; interrupt happens everytime Timer1 overflows
    ; Timer1 overflows in 50ms
    movlw 00000000B
    movwf LATC        ; clear both RC0 and RC1
    return

;############################## TIMER2 INTERRUPT ###############################

timer2_interrupt_occurred:
    ; interrupt to be used for 7-segment display
    ; 1ms ensures larger than 60 fps
    ; interrupt happens everytime timer2 overflows
    movff display_stage, store_display_stage ; store display stage for comparison
    dcfsnz store_display_stage               ; if display stage > 1, skip
    call stage_1			     ; else, display nothing
    dcfsnz store_display_stage               ; if display stage > 2, skip
    call stage_2			     ; else, display current beat
    dcfsnz store_display_stage               ; if display stage > 3, skip
    call stage_3			     ; else, display a dash
    dcfsnz store_display_stage               ; if display stage > 4, skip
    call stage_4			     ; else, display the bar length
    
    incf display_stage
    movlw 5
    cpfslt display_stage                     ; if display stage < 5, skip
    call reset_display_stage                 ; else, reset display stage
    return
stage_1:
    clrf LATD
    movlw 00000001B	       ; Select DIS0
    movwf PORTA
    btfss metronome_running, 0 ; if metronome is running, skip
    call display_p	       ; else, DIS0 = P
    btfsc metronome_running, 0 ; if metronome is paused, skip
    call display_nothing       ; else, DIS0 = nothing
    return
stage_2:
    clrf LATD
    movlw 00000010B	       ; Select DIS1
    movwf PORTA
    btfss metronome_running, 0 ; if metronome is running, skip
    call display_nothing       ; else, DIS1 = nothing
    btfsc metronome_running, 0 ; if metronome is paused, skip
    call display_beat	       ; else, DIS1 = beat
    return
stage_3:
    clrf LATD
    movlw 00000100B	       ; Select DIS2
    movwf PORTA
    btfss metronome_running, 0 ; if metronome is running, skip
    call display_nothing       ; else, DIS2 = nothing
    btfsc metronome_running, 0 ; if metronome is paused, skip
    call display_dash	       ; else, DIS2 = dash
    return
stage_4:
    clrf LATD
    movlw 00001000B	       ; Select DIS3
    movwf PORTA
    btfss metronome_running, 0 ; if metronome is running, skip
    call display_speed	       ; else, DIS3 = speed
    btfsc metronome_running, 0 ; if metronome is paused, skip
    call display_bar_length    ; else, DIS3 = bar length
    return
reset_display_stage:
    movlw 1
    movwf display_stage
    return

;############################# PORTB INTERRUPT #################################

rb_interrupt_occurred:
    ; When an interrupt occurs due to PORTB pins, this function is called.
    ; The last and current rb values are loaded
    ; Then, it checks which PORTB pin was pressed

    call load_current_and_last_rb_vals  ; load current and previous RB values

    movff last_rb4, WREG                ; WREG = previous RB4
    cpfseq current_rb4                  ; if current and previous RB4 is same, then skip
    call rb4_changed                    ; else, check if the change is a click
    movff last_rb5, WREG                ; WREG = previous RB5
    cpfseq current_rb5                  ; if current and previous RB5 is same, then skip
    call rb5_changed                    ; else, check if the change is a click
    movff last_rb6, WREG                ; WREG = previous RB6
    cpfseq current_rb6                  ; if current and previous RB6 is same, then skip
    call rb6_changed                    ; else, check if the change is a click
    movff last_rb7, WREG                ; WREG = previous RB7
    cpfseq current_rb7                  ; if current and previous RB7 is same, then skip
    call rb7_changed                    ; else, check if the change is a click

    movff current_PORTB, last_PORTB     ; last_PORTB = current_PORTB

    return

; "RBX_changed" functions below are called when a change in one of the RB
;  pins is detected. The functions check if the change was from 1 to 0.
rb4_changed:
    cpfsgt current_rb4 ; skip if current_rb4 > last_rb4 (button change isn't a click)
    call rb4_pressed   ; else, RB4 is clicked, take appropriate action
    return
rb5_changed:
    cpfsgt current_rb5 ; skip if current_rb5 > last_rb5 (button change isn't a click)
    call rb5_pressed   ; else, RB5 is clicked, take appropriate action
    return
rb6_changed:
    cpfsgt current_rb6 ; skip if current_rb6 > last_rb6 (button change isn't a click)
    call rb6_pressed   ; else, RB6 is clicked, take appropriate action
    return
rb7_changed:
    cpfsgt current_rb7 ; skip if current_rb7 > last_rb7 (button change isn't a click)
    call rb7_pressed   ; else, RB7 is clicked, take appropriate action
    return

rb4_pressed:
    ; if metronome is paused, start the metronome
    ; if metronome is running, pause the metronome
    movlw 1
    movwf display_stage ; reset display stage(so that it starts from DIS0)

    movff metronome_running, store_metronome_running ; store metronome state before changing it
    btfss store_metronome_running, 0 ; skip if metronome is running
    call start_metronome
    btfsc store_metronome_running, 0 ; skip if metronome is paused
    call pause_metronome
    return
start_metronome:
    bsf metronome_running, 0 ; metronome is running
    call running_state
    return
pause_metronome:
    bcf metronome_running, 0 ; metronome is paused
    call paused_state
    return

rb5_pressed:
    ; if metronome is paused, increase speed
    ; if metronome is running, increase bar length
    btfss metronome_running, 0
    call increase_speed
    btfsc metronome_running, 0
    call increase_bar_length
    return
increase_speed:
    movlw 9
    cpfslt speed ; if speed < 9, we can increment
    return       ; else return
    incf speed
    return
increase_bar_length:
    movlw 8
    cpfslt bar_length ; if bar length < 8, we can increment
    return            ; else return
    incf bar_length
    return

rb6_pressed:
    ; if metronome is paused, decrease speed
    ; if metronome is running, decrease bar length
    btfss metronome_running, 0
    call decrease_speed
    btfsc metronome_running, 0
    call decrease_bar_length
    return
decrease_speed:
    movlw 1
    cpfsgt speed ; if speed > 1, we can decrement
    return       ; else return
    decf speed
    return
decrease_bar_length:
    movlw 2
    cpfsgt bar_length ; if bar length > 2, we can decrement
    return            ; else return
    decf bar_length
    return

rb7_pressed:
    ; if metronome is paused, reset speed
    ; if metronome is running, reset bar length
    btfss metronome_running, 0
    call reset_speed
    btfsc metronome_running, 0
    call reset_bar_length
    return
reset_speed:
    movlw 6
    movwf speed
    return
reset_bar_length:
    movlw 4
    movwf bar_length
    return

load_current_and_last_rb_vals:
    ; load current rb values
    btfss current_PORTB, 4          ; skip if rb4 = 1
    clrf current_rb4                ; rb4 = 0
    btfsc current_PORTB, 4          ; skip if rb4 = 0
    setf current_rb4                ; rb4 = 1
    btfss current_PORTB, 5          ; skip if rb5 = 1
    clrf current_rb5                ; rb5 = 0
    btfsc current_PORTB, 5          ; skip if rb5 = 0
    setf current_rb5                ; rb5 = 1
    btfss current_PORTB, 6          ; skip if rb6 = 1
    clrf current_rb6                ; rb6 = 0
    btfsc current_PORTB, 6          ; skip if rb6 = 0
    setf current_rb6                ; rb6 = 1
    btfss current_PORTB, 7          ; skip if rb7 = 1
    clrf current_rb7                ; rb7 = 0
    btfsc current_PORTB, 7          ; skip if rb7 = 0
    setf current_rb7                ; rb7 = 1
    ; load previous rb values
    btfss last_PORTB, 4             ; skip if last rb4 = 1
    clrf last_rb4                   ; rb4 = 0
    btfsc last_PORTB, 4             ; skip if last rb4 = 0
    setf last_rb4                   ; rb4 = 1
    btfss last_PORTB, 5             ; skip if last rb5 = 1
    clrf last_rb5                   ; rb5 = 0
    btfsc last_PORTB, 5             ; skip if last rb5 = 0
    setf last_rb5                   ; rb5 = 1
    btfss last_PORTB, 6             ; skip if last rb6 = 1
    clrf last_rb6                   ; rb6 = 0
    btfsc last_PORTB, 6             ; skip if last rb6 = 0
    setf last_rb6                   ; rb6 = 1
    btfss last_PORTB, 7             ; skip if last rb7 = 1
    clrf last_rb7                   ; rb7 = 0
    btfsc last_PORTB, 7             ; skip if last rb7 = 0
    setf last_rb7                   ; rb7 = 1

    return

;############################# TIMER CONFIGURATIONS #################################

; Timer2 interrupt in 1ms
start_timer2:
    clrf TMR2
    ; Bits 6-3 T2OUTPS3:T2OUTPS0 = 0000, no postscaling.
    ; Bit 2 TMR2ON = 1, enable timer2 now.
    ; Bits 1-0 T2CKPS1:T2CKPS0 = 01, 1:4 prescaling.
    movlw   00000101B
    movwf   T2CON
    return
  
; This is the timer for clearing RC0 and RC1
; it interrupts every 50 milliseconds
start_timer1:
    movlw 0xE7
    movwf TMR1H
    movlw 0x96
    movwf TMR1L
    ; Bit 7 RD16 = 0, read/write in two 8-bit mode.
    ; Bit 6 T1RUN = 1, device clock is derived from Timer1 oscillator.
    ; Bit 5-4 T1CKPS1-T1CKPS0 = 11, 1:8 prescaling.
    ; Bit 3 T1OSCEN = 1, oscillator is enabled.
    ; Bit 2 T1SYNC0 = 0, this bit is ignored.
    ; Bit 1 TMR1CS = 0, use internal clock.
    ; Bit 0 TMR1ON = 1, enable Timer1 now.
    movlw 00110001B
    movwf T1CON
    return

; This is the timer for beat durations which uses Timer0
start_timer0:
    ; Bit 7 TMR0ON = 1, enable Timer0 now.
    ; Bit 6 T08BIT = 0, 16 bit mode.
    ; Bit 5 T0CS = 0, use internal instruction cycle clock.
    ; Bit 4 T0SE = 0, increment on low-to-high transition on T0CKI pin.
    ; Bit 3 PSA = 0, enable prescaling.
    ; Bit 2-0 T0PS2-T0PS0 = 011, 1:16 prescaling.
    movlw 10000011B
    movwf T0CON
    return

decide_load_value_for_timer:
    ; depending on speed, this function loads the timer0
    ; for waiting the right amount of milliseconds
    movff speed, store_speed   ; store speed
    dcfsnz store_speed         ; if speed > 1, skip
    call load_timer_for_1000ms ; else, load timer for 1000ms
    dcfsnz store_speed         ; if speed > 2, skip
    call load_timer_for_900ms  ; else, load timer for 900ms
    dcfsnz store_speed         ; if speed > 3, skip
    call load_timer_for_800ms  ; else, load timer for 800ms
    dcfsnz store_speed         ; if speed > 4, skip
    call load_timer_for_700ms  ; else, load timer for 700ms
    dcfsnz store_speed         ; if speed > 5, skip
    call load_timer_for_600ms  ; else, load timer for 600ms
    dcfsnz store_speed         ; if speed > 6, skip
    call load_timer_for_500ms  ; else, load timer for 500ms
    dcfsnz store_speed         ; if speed > 7, skip
    call load_timer_for_400ms  ; else, load timer for 400ms
    dcfsnz store_speed         ; if speed > 8, skip
    call load_timer_for_300ms  ; else, load timer for 300ms
    dcfsnz store_speed         ; if speed > 9, skip
    call load_timer_for_200ms  ; else, load timer for 200ms
    return
load_timer_for_1000ms:
    movlw 0x0B
    movwf TMR0H
    movlw 0xDC
    movwf TMR0L
    return
load_timer_for_900ms:
    movlw 0x24
    movwf TMR0H
    movlw 0x46
    movwf TMR0L
    return
load_timer_for_800ms:
    movlw 0x3C
    movwf TMR0H
    movlw 0xB0
    movwf TMR0L
    return
load_timer_for_700ms:
    movlw 0x55
    movwf TMR0H
    movlw 0x1A
    movwf TMR0L
    return
load_timer_for_600ms:
    movlw 0x6D
    movwf TMR0H
    movlw 0x84
    movwf TMR0L
    return
load_timer_for_500ms:
    movlw 0x85
    movwf TMR0H
    movlw 0xEE
    movwf TMR0L
    return
load_timer_for_400ms:
    movlw 0x9E
    movwf TMR0H
    movlw 0x58
    movwf TMR0L
    return
load_timer_for_300ms:
    movlw 0xB6
    movwf TMR0H
    movlw 0xC2
    movwf TMR0L
    return
load_timer_for_200ms:
    movlw 0xCF
    movwf TMR0H
    movlw 0x2C
    movwf TMR0L
    return

;############################# DISPLAY CONFIGURATIONS #################################

display_dash:
    movlw 01000000B
    movwf LATD
    return
display_beat:
    movff beat, number        ; number holds current beat
    call display_number
    return
display_bar_length:
    movff bar_length, number  ; number holds current bar length
    call display_number
    return
display_speed:
    movff speed, number       ; number holds speed
    call display_number
    return
display_p:
    movlw 01110011B           ; display P
    movwf LATD
    return
display_nothing:
    movlw 00000000B
    movwf LATD
    return
display_number:
    dcfsnz number         ; skip if number != 1
    movlw 00000110B       ; display 1
    dcfsnz number         ; skip if number != 2
    movlw 01011011B       ; display 2
    dcfsnz number         ; skip if number != 3
    movlw 01001111B       ; display 3
    dcfsnz number         ; skip if number != 4
    movlw 01100110B       ; display 4
    dcfsnz number         ; skip if number != 5
    movlw 01101101B       ; display 5
    dcfsnz number         ; skip if number != 6
    movlw 01111101B       ; display 6
    dcfsnz number         ; skip if number != 7
    movlw 00000111B       ; display 7
    dcfsnz number         ; skip if number != 8
    movlw 01111111B       ; display 8
    dcfsnz number         ; skip if number != 9
    movlw 01101111B       ; display 9

    movwf LATD
    return