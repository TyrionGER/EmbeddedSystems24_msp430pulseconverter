#include <msp430.h>

#define High_Threshold 3931 // ~2.4V
#define Low_Threshold 2789  // ~1.7V

unsigned int adcResult;              // Temporarily stores the ADC value
volatile unsigned int pwm_duty_cycle = 0; // PWM duty cycle
volatile int pwm_direction = 0;           // Direction of PWM change (1 = increase, -1 = decrease)

void configurePWM(void) {
    // Configure Timer0 for PWM
    TB0CCR0 = 1023;                      // PWM Period
    TB0CCTL1 = OUTMOD_7;                 // CCR1 reset/set
    TB0CCR1 = pwm_duty_cycle;            // CCR1 PWM duty cycle
    TB0CTL = TBSSEL_1 | MC_1 | TBCLR;    // ACLK, up mode, clear TBR
}

void configureADC(void) {
    // Configure ADC
    ADCCTL0 = ADCSHT_2 | ADCON;                         // sample-and-hold 16 ADCCLK cycles, ADCON
    ADCCTL1 = ADCSHP | ADCSHS_2 | ADCSSEL_1 | ADCCONSEQ_2; // TB1.1 trigger; ACLK for ADCCLK; Rpt single ch
    ADCCTL2 = ADCRES_2;                                 // 12-bit conversion results
    ADCMCTL0 = ADCINCH_2 | ADCSREF_1;                   // Vref 2.5v, A2
    ADCHI = High_Threshold;                             // Window Comparator Hi-threshold
    ADCLO = Low_Threshold;                              // Window Comparator Lo-threshold
    ADCIE |= ADCHIIE | ADCLOIE | ADCINIE;               // Enable ADC conv complete interrupt
}

void configureClocks(void) {
    // Configure XT1 oscillator
    P2SEL1 |= BIT6 | BIT7;                              // P2.6~P2.7: crystal pins
    CSCTL4 = SELA__XT1CLK;                              // Set ACLK = XT1; MCLK = SMCLK = DCO
    do {
        CSCTL7 &= ~(XT1OFFG | DCOFFG);                  // Clear XT1 and DCO fault flag
        SFRIFG1 &= ~OFIFG;
    } while (SFRIFG1 & OFIFG);                          // Test oscillator fault flag
}

void configureTimer1(void) {
    // Configure ADC timer trigger TB1.1
    TB1CCR0 = 8192 - 1;                                 // PWM Period, reduce the period for faster sampling
    TB1CCR1 = 4096 - 1;                                 // Duty cycle TB1.1
    TB1CCTL1 = OUTMOD_3;                                // TB1CCR1 set/reset mode
    TB1CTL = TBSSEL_1 | MC_1 | TBCLR;                   // ACLK, up mode
    TB1CCTL0 = CCIE;                                    // Enable Timer1 CCR0 interrupt
}

void configureReference(void) {
    // Configure Internal reference voltage
    PMMCTL0_H = PMMPW_H;                                // Unlock the PMM registers
    PMMCTL2 |= INTREFEN | REFVSEL_2;                    // Enable internal 2.5V reference
    __delay_cycles(400);                                // Delay for reference settling
}

void configureGPIO(void) {
    P3DIR |= BIT0;                                      // Set P3.0 output direction
    P3SEL0 |= BIT0;                                     // Select Timer output for P3.0
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;               // Stop WDT

    configureGPIO();
    configureClocks();
    configureReference();
    configurePWM();
    configureADC();
    configureTimer1();

    ADCCTL0 |= ADCENC;                                  // Enable conversion

    __bis_SR_register(LPM3_bits | GIE);                 // Enter LPM3 w/ interrupts
    __no_operation();                                   // Only for debugger
}

// ADC interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void)
#elif defined(__GNUC__)
void __attribute__((interrupt(ADC_VECTOR))) ADC_ISR(void)
#else
#error Compiler not supported!
#endif
{
    if (ADCIV == ADCIV_ADCOVIFG) {
        // ADC overflow
    }
    else if (ADCIV == ADCIV_ADCTOVIFG) {
        // ADC conversion time overflow
    }
    else if (ADCIV == ADCIV_ADCHIIFG) {                // ADCHI; A2 > 2.4V
        ADCIFG &= ~ADCHIIFG;                           // Clear interrupt flag
        pwm_direction = 1;                             // Start increasing PWM duty cycle
    }
    else if (ADCIV == ADCIV_ADCLOIFG) {                // ADCLO; A2 < 1.7V
        ADCIFG &= ~ADCLOIFG;                           // Clear interrupt flag
        pwm_direction = -1;                            // Start decreasing PWM duty cycle
    }
    else if (ADCIV == ADCIV_ADCINIFG) {                // ADCIN; 1.7V < A2 < 2.4V
        ADCIFG &= ~ADCINIFG;                           // Clear interrupt flag
        pwm_direction = 0;                             // Stop changing PWM duty cycle
    }
}

// Timer1 B0 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER1_B0_VECTOR
__interrupt void TIMER1_B0_ISR(void)
#elif defined(__GNUC__)
void __attribute__((interrupt(TIMER1_B0_VECTOR))) TIMER1_B0_ISR(void)
#else
#error Compiler not supported!
#endif
{
    if (pwm_direction != 0) {
        pwm_duty_cycle += pwm_direction * 10;          // Adjust the duty cycle
        if (pwm_duty_cycle > 1023) {                   // Clamp the duty cycle within 0-1023
            pwm_duty_cycle = 1023;
        }
        else if (pwm_duty_cycle < 0) {
            pwm_duty_cycle = 0;
        }
        TB0CCR1 = pwm_duty_cycle;                      // Update PWM duty cycle
    }
}
