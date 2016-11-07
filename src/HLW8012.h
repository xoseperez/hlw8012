/*

HLW8012

Copyright (C) 2016 by Xose Pérez <xose dot perez at gmail dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef HLW8012_h
#define HLW8012_h

#include <Arduino.h>

// Internal voltage reference value
#define V_REF               2.43

// The factor of a 1mOhm resistor
// as per recomended circuit in datasheet
// A 1mOhm resistor allows a ~30A max measurement
#define R_CURRENT           1000

// This is the factor of a voltage divider of 5x 470K upstream and 1k downstream
// as per recomended circuit in datasheet
#define R_VOLTAGE           2351

// Frequency of the HLW8012 internal clock
#define F_OSC               3579000

// These are the multipliers for current, voltage and power as per datasheet
// These values divided by output period (in useconds) give the actual value
// For power a frequency of 1Hz means around 10W
// For current a frequency of 1Hz means around 15mA
// For voltage a frequency of 1Hz means around 0.41V
#define X_CURRENT           ( 1000000.0 * 512 * V_REF * R_CURRENT / 24.0 / F_OSC )
#define X_VOLTAGE           ( 1000000.0 * 512 * V_REF * R_VOLTAGE / 2.0 / F_OSC )
#define X_POWER             ( 1000000.0 * 128 * V_REF * V_REF * R_VOLTAGE * R_CURRENT / 48.0 / F_OSC )

// Minimum delay between selecting a mode and reading a sample
#define READING_INTERVAL    3000

// Maximum pulse with, if longer than this a 0 value is returned
// This effectively limits the precission of the measurements
#define PULSE_TIMEOUT       10000

// Switch from voltage to current and back every this many interrupts on CF1
// This value is purely experimental, this is quite limitant since when low
// currents it stays for several seconds sampling the current...
#define CF1_SWITCH_COUNT    100

class HLW8012 {

    public:

        void cf_interrupt();
        void cf1_interrupt();

        void begin(unsigned char cf_pin, unsigned char cf1_pin, unsigned char sel_pin, unsigned char currentWhen = HIGH, bool use_interrupts = false);
        void handle(unsigned long interval = READING_INTERVAL);

        double getRMSCurrent();
        unsigned int getRMSVoltage();
        unsigned int getRMSPower();

        void expectedRMSCurrent(double current);
        void expectedRMSVoltage(unsigned int current);
        void expectedRMSPower(unsigned int power);

        double getCurrentFactor() { return _current_factor; };
        double getVoltageFactor() { return _voltage_factor; };
        double getPowerFactor() { return _power_factor; };

        void setCurrentFactor(double current_factor) { _current_factor = current_factor; };
        void setVoltageFactor(double voltage_factor) { _voltage_factor = voltage_factor; };
        void setPowerFactor(double power_factor) { _power_factor = power_factor; };

    private:

        unsigned char _cf_pin;
        unsigned char _cf1_pin;
        unsigned char _sel_pin;

        double _current_factor = X_CURRENT;
        double _voltage_factor = X_VOLTAGE;
        double _power_factor = X_POWER;

        unsigned long _voltage_pulse_width = 0;
        unsigned long _current_pulse_width = 0;
        unsigned long _power_pulse_width = 0;

        double _current = 0;
        unsigned int _voltage = 0;
        unsigned int _power = 0;

        unsigned char _current_mode = HIGH;
        unsigned char _mode;

        bool _use_interrupts;
        volatile unsigned long _last_cf_interrupt;
        volatile unsigned long _last_cf1_interrupt;
        volatile unsigned long _cf_interrupt_count = 0;
        volatile unsigned long _cf1_interrupt_count = 0;

        void _checkCFSignal();
        void _checkCF1Signal();

};

#endif
