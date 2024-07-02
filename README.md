MSP430 Projekt zur Messung eines Pulses und ausgabe der Frequenz über LED und Piezo Lautsprecher

Projektbeschreibung
Dieses Projekt verwendet den MSP430FR2355 Mikrocontroller, um ADC-Werte von einem Sensor zu lesen und basierend auf diesen Werten LEDs und einen Piezo-Lautsprecher zu steuern. Wenn der ADC-Wert einen bestimmten Schwellenwert überschreitet, wird eine rote LED eingeschaltet und ein Piezo-Lautsprecher piept bei 2 kHz. Andernfalls bleibt die blaue LED eingeschaltet.

Hardwareanforderungen
MSP430FR2355 Mikrocontroller
Sensor (z.B. Pulssensor) angeschlossen an P1.2
Rote LED angeschlossen an P3.0 mit Vorwiderstand (100 Ohm) und VCC 3.3V
Blaue LED angeschlossen an P3.2 mit Vorwiderstand (140 Ohm) und VCC 5V
Piezo-Lautsprecher angeschlossen an P3.4
Schaltplan

Code kopieren
                 MSP430FR2355
              -----------------
          /|\|              XIN|-
           | |                 |
           --|RST          XOUT|-
             |                 |
             |             P1.2|<-- Sensor-Eingang (ADC)
             |                 |
             |             P3.0|--> Rote LED
             |                 |
             |             P3.2|--> Blaue LED
             |                 |
             |             P3.4|--> Piezo-Lautsprecher
             |                 |
             |                 |
             |                 |
             |                 |
             |                 |
             |                 |
             |                 |
             -------------------
Code
Der Hauptcode befindet sich in msp430fr2355_pulseconverter.c und umfasst die Konfiguration von GPIOs, ADC und die Steuerlogik für LEDs und Piezo-Lautsprecher.

Wichtige Funktionen
configureGPIO(): Konfiguriert die GPIO-Pins für LEDs und Piezo-Lautsprecher.
configureADC(): Konfiguriert den ADC zur Messung des Sensorwerts.
delayMicroseconds(): Erzeugt eine Verzögerung in Mikrosekunden.
togglePiezo(): Schaltet den Piezo-Lautsprecher um.

Kompilierung und Upload
Stellen Sie sicher, dass Sie die MSP430 Toolchain installiert haben.
Kompilieren Sie das Projekt mit Ihrem bevorzugten Compiler.
Laden Sie den erzeugten Code auf den MSP430FR2355 Mikrocontroller.