//***************************************************************************************
//  MSP430 ADC und LED Steuerungs-Demo - Schalte rote und blaue LEDs mit Piezo-Lautsprecher
//
//  Beschreibung: Dieses Programm verwendet den ADC, um einen Sensorwert an P1.2 zu lesen 
//  und schaltet eine rote LED an P3.0 und eine blaue LED an P3.2 basierend auf dem ADC-Wert. 
//  Ein Piezo-Lautsprecher, der an P3.4 angeschlossen ist, piept mit 2 kHz, wenn die rote LED leuchtet.
//
//                MSP430FR2355
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          --|RST          XOUT|-
//            |                 |
//            |             P1.2|<-- Sensor Input (ADC)
//            |                 |
//            |             P3.0|--> Red LED
//            |                 |
//            |             P3.2|--> Blue LED
//            |                 |
//            |             P3.4|--> Piezo Speaker
//            |                 |
//            |                 |
//            |                 |
//            |                 |
//            |                 |
//            |                 |
//            |                 |
//            -------------------
//***************************************************************************************
 
#include <msp430.h>
 
#define THRESHOLD_VOLTAGE   3000  // ~2V (3000/4095 * 3.3V reference = 2V)
#define PIEZO_PIN           BIT4  // Assuming Piezo is connected to P3.4
 
void configureGPIO(void) {
    // Configure Red LED (P3.0) as output
    P3DIR |= BIT0;     // Set P3.0 output direction
    P3OUT &= ~BIT0;    // Initialize P3.0 to off
}
 
void configureADC(void) {
    // Configure ADC
    ADCCTL0 = ADCSHT_2 | ADCON;             // Sample-and-hold time, ADC on
    ADCCTL1 = ADCSHP;                       // ADC sample-and-hold pulse mode
    ADCCTL2 = ADCRES_2;                     // 12-bit conversion results
    ADCMCTL0 = ADCINCH_2 | ADCSREF_0;       // A2 input channel, Vcc/Vss reference
}
 
void delayMicroseconds(unsigned int us) {
    while (us--) {
        __delay_cycles(1); // 1 cycle per microsecond for 1 MHz clock
    }
}
 
void togglePiezo(unsigned int frequency) {
    unsigned int delayTime = 500000 / frequency; // Calculate half period for given frequency in microseconds
    P3OUT ^= PIEZO_PIN;   // Toggle Piezo pin
    delayMicroseconds(delayTime); // Half period delay
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
            // Turn on Red LED and turn off Blue LED
            P3OUT |= BIT0;
            P3OUT &= ~BIT2;
 
            // Toggle Piezo as long as the red LED is on
            while (ADCMEM0 >= THRESHOLD_VOLTAGE) {
                togglePiezo(2000); // Toggle Piezo at 2 kHz
                ADCCTL0 |= ADCENC | ADCSC;
                while (ADCCTL1 & ADCBUSY);
            }
 
            // Ensure Piezo is off after the loop
            P3OUT &= ~PIEZO_PIN;
 
        } else {
            // Ensure Blue LED is on and Red LED is off
            P3OUT |= BIT2;
            P3OUT &= ~BIT0;
        }
    }
}