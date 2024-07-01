#include <msp430.h>
 
#define THRESHOLD_VOLTAGE   3000  // ~2V (3000/4095 * 3.3V reference = 2V)
 
void configureGPIO(void) {
    // Configure Red LED (P3.0) as output
    P3DIR |= BIT0;     // Set P3.0 output direction
    P3OUT &= ~BIT0;    // Initialize P3.0 to off
 
    // Configure Blue LED (P3.2) as output
    P3DIR |= BIT2;     // Set P3.2 output direction
    P3OUT &= ~BIT2;    // Initialize P3.2 to off
 
    // Configure ADC input (P1.2)
    P1SEL0 |= BIT2;    // Set P1.2 for ADC function
    P1SEL1 |= BIT2;
}
 
void configureADC(void) {
    // Configure ADC
    ADCCTL0 = ADCSHT_2 | ADCON;             // Sample-and-hold time, ADC on
    ADCCTL1 = ADCSHP;                       // ADC sample-and-hold pulse mode
    ADCCTL2 = ADCRES_2;                     // 12-bit conversion results
    ADCMCTL0 = ADCINCH_2 | ADCSREF_0;       // A2 input channel, Vcc/Vss reference
}
 
void delay(unsigned int ms) {
    // Assuming 1 MHz clock, 1 ms delay needs 1000 cycles
    // 1 MHz / 1000 = 1 ms
    while (ms--) {
        __delay_cycles(1000); // 1 ms delay
    }
}
 
int main(void) {
    WDTCTL = WDTPW | WDTHOLD; // Stop WDT
 
    configureGPIO();
    configureADC();
 
    // Disable the GPIO power-on default high-impedance mode
    PM5CTL0 &= ~LOCKLPM5;
 
    while (1) {
        // Start ADC conversion
        ADCCTL0 |= ADCENC | ADCSC;
 
        // Wait for ADC conversion to complete
        while (ADCCTL1 & ADCBUSY);
 
        // Check if ADC value is above the threshold
        if (ADCMEM0 >= THRESHOLD_VOLTAGE) {
            // Blink the LEDs
            P3OUT ^= BIT0;
            P3OUT &= ~BIT2;
            delay(200);
 
 
        } else {
            // Ensure LEDs are off if below threshold
            P3OUT ^= BIT2;
            P3OUT &= ~BIT0;
        }
    }
}