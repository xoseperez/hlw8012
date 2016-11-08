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

#include <Arduino.h>
#include "HLW8012.h"

void HLW8012::begin(unsigned char cf_pin, unsigned char cf1_pin, unsigned char sel_pin, unsigned char currentWhen, bool use_interrupts) {

    _cf_pin = cf_pin;
    _cf1_pin = cf1_pin;
    _sel_pin = sel_pin;
    _current_mode = currentWhen;
    _use_interrupts = use_interrupts;

    pinMode(_cf_pin, INPUT_PULLUP);
    pinMode(_cf1_pin, INPUT_PULLUP);
    pinMode(_sel_pin, OUTPUT);

    _calculateDefaultMultipliers();

    _mode = _current_mode;
    digitalWrite(_sel_pin, _mode);


}

void HLW8012::handle(unsigned long interval) {

    double T;
    static unsigned long last_reading = millis();

    // Safety check
    if (_use_interrupts) return;

    if ((millis() - last_reading) > interval) {

        last_reading = millis();

        T = 2 * pulseIn(_cf1_pin, HIGH);
        if (_mode == _current_mode) {
            _current_pulse_width = T;
        } else {
            _voltage_pulse_width = T;
        }
        _mode = 1 - _mode;
        digitalWrite(_sel_pin, _mode);

    }

}

double HLW8012::getCurrent() {
    if (_power == 0) {
        _current = 0;
    } else {
        if (_use_interrupts) _checkCF1Signal();
        _current = (_current_pulse_width > 0) ? _current_multiplier / _current_pulse_width : 0;
    }
    return _current;
}

unsigned int HLW8012::getVoltage() {
    _voltage = (_voltage_pulse_width > 0) ? _voltage_multiplier / _voltage_pulse_width : 0;
    return _voltage;
}

unsigned int HLW8012::getActivePower() {
    if (_use_interrupts) {
        _checkCFSignal();
    } else {
        _power_pulse_width = 2 * pulseIn(_cf_pin, HIGH);
    }
    _power = (_power_pulse_width > 0) ? _power_multiplier / _power_pulse_width : 0;
    return _power;
}

unsigned int HLW8012::getApparentPower() {
    double current = getCurrent();
    unsigned int voltage = getVoltage();
    return voltage * current;
}

double HLW8012::getPowerFactor() {
    unsigned int active = getActivePower();
    unsigned int apparent = getApparentPower();
    if (active > apparent) return 1;
    if (apparent == 0) return 0;
    return (double) active / apparent;
}

void HLW8012::expectedCurrent(double value) {
    if (_current == 0) getCurrent();
    if (_current > 0) _current_multiplier *= (value / _current);
}

void HLW8012::expectedVoltage(unsigned int value) {
    if (_voltage == 0) getVoltage();
    if (_voltage > 0) _voltage_multiplier *= ((double) value / _voltage);
}

void HLW8012::expectedActivePower(unsigned int value) {
    if (_power == 0) getActivePower();
    if (_power > 0) _power_multiplier *= ((double) value / _power);
}

void HLW8012::setResistors(double current, double voltage_upstream, double voltage_downstream) {
    if (voltage_downstream > 0) {
        _current_resistor = current;
        _voltage_resistor = (voltage_upstream + voltage_downstream) / voltage_downstream;
        _calculateDefaultMultipliers();
    }
}

void HLW8012::cf_interrupt() {

    unsigned long now = micros();

    _power_pulse_width = 2 * (now - _last_cf_interrupt);
    _last_cf_interrupt = now;

}

void HLW8012::cf1_interrupt() {

    unsigned long now = micros();
    unsigned long pulse_width = now - _last_cf1_interrupt;

    _last_cf1_interrupt = now;
    _cf1_interrupt_count++;
    if ((_cf1_interrupt_count % CF1_SWITCH_COUNT) == 0) {

        if (_mode == _current_mode) {
            _current_pulse_width = 2 * pulse_width;
        } else {
            _voltage_pulse_width = 2 * pulse_width;
        }

        _mode = 1 - _mode;
        digitalWrite(_sel_pin, _mode);

    }

}

void HLW8012::_checkCFSignal() {
    if ((micros() - _last_cf_interrupt) > PULSE_TIMEOUT) _power_pulse_width = 0;
}

void HLW8012::_checkCF1Signal() {
    if ((micros() - _last_cf1_interrupt) > PULSE_TIMEOUT) _current_pulse_width = 0;
}

// These are the multipliers for current, voltage and power as per datasheet
// These values divided by output period (in useconds) give the actual value
// For power a frequency of 1Hz means around 12W
// For current a frequency of 1Hz means around 15mA
// For voltage a frequency of 1Hz means around 0.5V
void HLW8012::_calculateDefaultMultipliers() {
    _current_multiplier = ( 1000000.0 * 512 * V_REF / _current_resistor / 24.0 / F_OSC );
    _voltage_multiplier = ( 1000000.0 * 512 * V_REF * _voltage_resistor / 2.0 / F_OSC );
    _power_multiplier = ( 1000000.0 * 128 * V_REF * V_REF * _voltage_resistor / _current_resistor / 48.0 / F_OSC );
}
