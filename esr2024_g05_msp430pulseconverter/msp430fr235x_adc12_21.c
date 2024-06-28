#include <msp430.h>

#define THRESHOLD_VOLTAGE 3000  // ~2V (2486/4095 * 3.3V reference = 2V)

void configureGPIO(void) {
    // Configure LED (P3.0) as output
    P3DIR |= BIT0;     // Set P3.0 output direction
    P3OUT &= ~BIT0;    // Initialize P3.0 to off

    // Configure LED (P3.3) as output
    P3DIR |= BIT3;     // Set P3.3 output direction
    P3OUT |= BIT3;     // Turn on LED on P3.3

    // Configure ADC input (P1.2)
    P1SEL0 |= BIT2;    // Set P1.2 for ADC function
    P1SEL1 |= BIT2;
}

void configureADC(void) {
    // Configure ADC
    ADCCTL0 = ADCSHT_2 | ADCON;                     // Sample-and-hold time, ADC on
    ADCCTL1 = ADCSHP;                               // ADC sample-and-hold pulse mode
    ADCCTL2 = ADCRES_2;                             // 12-bit conversion results
    ADCMCTL0 = ADCINCH_2 | ADCSREF_0;               // A2 input channel, Vcc/Vss reference
}

void configureTimerB(void) {
    // Configure Timer B
    TB0CCTL0 = CCIE;                               // TBCCR0 interrupt enabled
    TB0CCR0 = 16384 - 1;                           // 16384/32768 = 0.5s, assuming ACLK = 32.768 kHz
    TB0CTL = TBSSEL__ACLK | MC__UP | TBCLR;        // ACLK, up mode, clear TAR
}

#pragma vector = TIMER0_B0_VECTOR
__interrupt void TIMER0_B0_ISR(void) {
    // No action needed in ISR for LED toggling
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;                       // Stop WDT

    configureGPIO();
    configureADC();
    configureTimerB();

    // Disable the GPIO power-on default high-impedance mode
    PM5CTL0 &= ~LOCKLPM5;

    // Enable global interrupts
    __bis_SR_register(GIE);

    int thresholdCrossed = 0;

    while (1) {
        ADCCTL0 |= ADCENC | ADCSC;                  // Enable and start conversion
        while (ADCCTL1 & ADCBUSY);                  // Wait for conversion to complete

        if (ADCMEM0 >= THRESHOLD_VOLTAGE) {         // Check if input signal is above 2V
            if (!thresholdCrossed) {
                P3OUT |= BIT0;                      // Turn on LED on P3.0

                thresholdCrossed = 1;               // Set the flag to indicate threshold was crossed
            }
        } else {
            thresholdCrossed = 0;                   // Reset the flag when the signal falls below the threshold

            P3OUT &= ~BIT0;                         // Turn off LED on P3.0
        }
    }

    return 0;
}
