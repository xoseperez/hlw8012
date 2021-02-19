#include <Arduino.h>
#include "HLW8012.h"

#define SERIAL_BAUDRATE                 115200

// GPIOs
#define RELAY_PIN                       4
#define SEL_PIN                         12
#define CF1_PIN                         14
#define CF_PIN                          5

// Check values every 10 seconds
#define UPDATE_TIME                     5000

// Set SEL_PIN to LOW to sample current on the HJL-01
#define CURRENT_MODE                    LOW

// These are experimental ratios for the BLITZWOLF BWSHP2 using the HJL-01
#define HJL01_CURRENT_RATIO             25740
#define HJL01_VOLTAGE_RATIO             313400
#define HJL01_POWER_RATIO               3414290

HLW8012 hjl01;

// When using interrupts we have to call the library entry point
// whenever an interrupt is triggered
void ICACHE_RAM_ATTR hjl01_cf1_interrupt() {
    hjl01.cf1_interrupt();
}
void ICACHE_RAM_ATTR hjl01_cf_interrupt() {
    hjl01.cf_interrupt();
}

void setup() {

    // Init serial port and clean garbage
    Serial.begin(SERIAL_BAUDRATE);
    Serial.println();
    Serial.println();

    // Close the relay to switch on the load
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);

    // Set the experimental ratios
    hjl01.setCurrentMultiplier(HJL01_CURRENT_RATIO);
    hjl01.setVoltageMultiplier(HJL01_VOLTAGE_RATIO);
    hjl01.setPowerMultiplier(HJL01_POWER_RATIO);

    // Initialize HLW8012
    // void begin(unsigned char cf_pin, unsigned char cf1_pin, unsigned char sel_pin, unsigned char currentWhen = HIGH, bool use_interrupts = false, unsigned long pulse_timeout = PULSE_TIMEOUT);
    // * cf_pin, cf1_pin and sel_pin are GPIOs to the HLW8012 IC
    // * currentWhen is the value in sel_pin to select current sampling
    // * set use_interrupts to true to use interrupts to monitor pulse widths
    // * leave pulse_timeout to the default value, recommended when using interrupts
    hjl01.begin(CF_PIN, CF1_PIN, SEL_PIN, CURRENT_MODE, true);

    // Set the interrupts on FALLING
    attachInterrupt(digitalPinToInterrupt(CF1_PIN), hjl01_cf1_interrupt, FALLING);
    attachInterrupt(digitalPinToInterrupt(CF_PIN), hjl01_cf_interrupt, FALLING);

}

char buffer[50];

void loop() {

    static unsigned long last = millis();

    // This UPDATE_TIME should be at least twice the interrupt timeout (2 second by default)
    if ((millis() - last) > UPDATE_TIME) {

        last = millis();
        Serial.print("[HLW] Active Power (W)    : "); Serial.println(hjl01.getActivePower());
        Serial.print("[HLW] Voltage (V)         : "); Serial.println(hjl01.getVoltage());
        Serial.print("[HLW] Current (A)         : "); Serial.println(hjl01.getCurrent());
        Serial.print("[HLW] Apparent Power (VA) : "); Serial.println(hjl01.getApparentPower());
        Serial.print("[HLW] Power Factor (%)    : "); Serial.println((int) (100 * hjl01.getPowerFactor()));
        Serial.print("[HLW] Agg. energy (Ws)    : "); Serial.println(hjl01.getEnergy());
        Serial.println();

    }

}
