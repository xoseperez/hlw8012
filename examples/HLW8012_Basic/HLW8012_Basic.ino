#include <Arduino.h>
#include "HLW8012.h"

#define SERIAL_BAUDRATE     115200

// GPIOs
#define RELAY_PIN           12
#define SEL_PIN             5
#define CF1_PIN             13
#define CF_PIN              14

// Check values every 10 seconds
#define UPDATE_TIME         10000

// Set SEL_PIN to HIGH to sample current
// This is the case for Itead's Sonoff POW, where a
// the SEL_PIN drives a transistor that pulls down
// the SEL pin in the HLW8012 when closed
#define CURRENT_MODE        HIGH

// Use interrupt-driven approach
#define USE_INTERRUPTS      1

HLW8012 hlw8012;

#if USE_INTERRUPTS == 1

    // When using interrupts we have to call the library entry point
    // whenever an interrupt is triggered

    void hlw8012_cf1_interrupt() {
        hlw8012.cf1_interrupt();
    }

    void hlw8012_cf_interrupt() {
        hlw8012.cf_interrupt();
    }

    void setInterrupts() {
        attachInterrupt(CF1_PIN, hlw8012_cf1_interrupt, FALLING);
        attachInterrupt(CF_PIN, hlw8012_cf_interrupt, FALLING);
    }

#endif

void calibrate() {

    // Let some time to register values
    unsigned long timeout = millis();
    while ((millis() - timeout) < 10000) {
        #if USE_INTERRUPTS == 0
            hlw8012.handle();
        #endif
        delay(1);
    }

    // Calibrate using a 60W bulb (pure resistive) on a 230V line
    hlw8012.expectedActivePower(60.0);
    hlw8012.expectedVoltage(230.0);
    hlw8012.expectedCurrent(60.0 / 230.0);

    // Show corrected factors
    Serial.print("[HLW] New current multiplier : "); Serial.println(hlw8012.getCurrentMultiplier());
    Serial.print("[HLW] New voltage multiplier : "); Serial.println(hlw8012.getVoltageMultiplier());
    Serial.print("[HLW] New power multiplier   : "); Serial.println(hlw8012.getPowerMultiplier());
    Serial.println();

}

void setup() {

    // Init serial port and clean garbage
    Serial.begin(SERIAL_BAUDRATE);
    Serial.println();
    Serial.println();

    // Close the relay to switch on the load
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);

    // Initialize HLW8012
    // void begin(unsigned char cf_pin, unsigned char cf1_pin, unsigned char sel_pin, unsigned char currentWhen = HIGH, bool use_interrupts = false);
    // * cf_pin, cf1_pin and sel_pin are GPIOs to the HLW8012 IC
    // * currentWhen is the value in sel_pin to select current sampling
    // * set use_interrupts to true to use interrupts to monitor pulse widths, if false you should call handle() in the main loop to do the sampling
    hlw8012.begin(CF_PIN, CF1_PIN, SEL_PIN, CURRENT_MODE, USE_INTERRUPTS);

    #if USE_INTERRUPTS == 1
        setInterrupts();
    #endif

    // Show default (as per datasheet) multipliers
    Serial.print("[HLW] Default current multiplier : "); Serial.println(hlw8012.getCurrentMultiplier());
    Serial.print("[HLW] Default voltage multiplier : "); Serial.println(hlw8012.getVoltageMultiplier());
    Serial.print("[HLW] Default power multiplier   : "); Serial.println(hlw8012.getPowerMultiplier());
    Serial.println();

    //calibrate();

}

void loop() {

    static unsigned long last = millis();

    // When not using interrupts you have to call handle() every so often with an optional
    // interval value. This interval defaults to 3000ms but should not be less than 500ms. After this
    // time the code will switch from voltage to current monitor and viceversa. So you will have
    // new values for both after 2x this interval time.
    #if USE_INTERRUPTS == 0
        hlw8012.handle(500);
    #endif

    // This UPDATE_TIME should be at least twice the previous time in handle() when not using interrupts
    // or 1000ms when using interrupts
    if ((millis() - last) > UPDATE_TIME) {

        last = millis();
        Serial.print("[HLW] Voltage (V)         : "); Serial.println(hlw8012.getVoltage());
        Serial.print("[HLW] Current (A)         : "); Serial.println(hlw8012.getCurrent());
        Serial.print("[HLW] Active Power (W)    : "); Serial.println(hlw8012.getActivePower());
        Serial.print("[HLW] Apparent Power (VA) : "); Serial.println(hlw8012.getApparentPower());
        Serial.print("[HLW] Power Factor (%)    : "); Serial.println((int) (100 * hlw8012.getPowerFactor()));
        Serial.println();

    }

}
