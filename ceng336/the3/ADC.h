/* 
 * File:   ADC.h
 *
 */

#ifndef ADC_H
#define	ADC_H

#ifdef	__cplusplus
extern "C" {
#endif

    #define _XTAL_FREQ 10000000L

    void readADCChannel(unsigned char channel){
        // Select channel
        // 0b 0101 -> 5th chanel
        ADCON0bits.CHS0 =  channel & 0x1;
        ADCON0bits.CHS1 = (channel >> 1) & 0x1;
        ADCON0bits.CHS2 = (channel >> 2) & 0x1;
        ADCON0bits.CHS3 = (channel >> 3) & 0x1;
        
        ADCON0bits.GODONE = 1; //Start convertion
    }

    
    void ADC_init(){
        ADCON1bits.VCFG0 = 0; // Vref += 5.0
        ADCON1bits.VCFG1 = 0; // Vref = 0

        TRISAbits.RA0 = 1;
        TRISAbits.RA1 = 1;
        TRISAbits.RA2 = 0;

        ADCON2bits.ADCS2 = 0; // Tad (32xTOSC) -> 0.8us
        ADCON2bits.ADCS1 = 1;
        ADCON2bits.ADCS0 = 0;

        ADCON2bits.ACQT2 = 0; // Acquisition time (4xTad) = 3.2 us
        ADCON2bits.ACQT1 = 1;
        ADCON2bits.ACQT0 = 0;

        ADCON2bits.ADFM = 1; // Right justified

        ADCON0bits.ADON = 1; // ADC module is enabled
    }

#ifdef	__cplusplus
}
#endif

#endif	/* ADC_H */