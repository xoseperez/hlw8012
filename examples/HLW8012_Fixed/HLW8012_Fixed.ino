#include <Arduino.h>
#include "HLW8012.h"

#define SERIAL_BAUDRATE                 115200

// GPIOs
#define RELAY_PIN                       12
#define SEL_PIN                         5
#define CF1_PIN                         13
#define CF_PIN                          14

// Check values every 2 seconds
#define UPDATE_TIME                     2000

// We have the SEL pin tied so there is only one avilable mode
#define FIXED_MODE                      MODE_CURRENT

// These are the nominal values for the resistors in the circuit
#define CURRENT_RESISTOR                0.001
#define VOLTAGE_RESISTOR_UPSTREAM       ( 5 * 470000 ) // Real: 2280k
#define VOLTAGE_RESISTOR_DOWNSTREAM     ( 1000 ) // Real 1.009k

HLW8012 hlw8012;

void setup() {

    // Init serial port and clean garbage
    Serial.begin(SERIAL_BAUDRATE);
    Serial.println();
    Serial.println();

    // Close the relay to switch on the load
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);

    // Initialize HLW8012
    // void begin(unsigned char cf_pin, unsigned char cf1_pin, unsigned char sel_pin, unsigned char currentWhen = HIGH, bool use_interrupts = false, unsigned long pulse_timeout = PULSE_TIMEOUT);
    // * cf_pin, cf1_pin and sel_pin are GPIOs to the HLW8012 IC
    // * currentWhen is the value in sel_pin to select current sampling
    // * set use_interrupts to false, we will have to call handle() in the main loop to do the sampling
    // * set pulse_timeout to 500ms for a fast response but losing precision (that's ~24W precision :( )
    hlw8012.begin(CF_PIN, CF1_PIN, MODE_CURRENT, false, 500000);

    // These values are used to calculate current, voltage and power factors as per datasheet formula
    // These are the nominal values for the Sonoff POW resistors:
    // * The CURRENT_RESISTOR is the 1milliOhm copper-manganese resistor in series with the main line
    // * The VOLTAGE_RESISTOR_UPSTREAM are the 5 470kOhm resistors in the voltage divider that feeds the V2P pin in the HLW8012
    // * The VOLTAGE_RESISTOR_DOWNSTREAM is the 1kOhm resistor in the voltage divider that feeds the V2P pin in the HLW8012
    hlw8012.setResistors(CURRENT_RESISTOR, VOLTAGE_RESISTOR_UPSTREAM, VOLTAGE_RESISTOR_DOWNSTREAM);

}

void loop() {

    static unsigned long last = millis();

    // This UPDATE_TIME should be at least twice the minimum time for the current or voltage
    // signals to stabilize. Experimentally that's about 1 second.
    if ((millis() - last) > UPDATE_TIME) {

        last = millis();
        Serial.print("[HLW] Active Power (W)    : "); Serial.println(hlw8012.getActivePower());
        Serial.print("[HLW] Current (A)         : "); Serial.println(hlw8012.getCurrent());
        Serial.println();

    }

}
