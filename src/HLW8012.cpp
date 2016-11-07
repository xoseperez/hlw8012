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

double HLW8012::getRMSCurrent() {
    if (_power == 0) {
        _current = 0;
    } else {
        if (_use_interrupts) _checkCF1Signal();
        _current = (_current_pulse_width > 0) ? _current_factor / _current_pulse_width : 0;
    }
    return _current;
}

unsigned int HLW8012::getRMSVoltage() {
    if (_use_interrupts) _checkCF1Signal();
    _voltage = (_voltage_pulse_width > 0) ? _voltage_factor / _voltage_pulse_width : 0;
    return _voltage;
}

unsigned int HLW8012::getRMSPower() {
    if (_use_interrupts) {
        _checkCFSignal();
    } else {
        _power_pulse_width = 2 * pulseIn(_cf_pin, HIGH);
    }
    _power = (_power_pulse_width > 0) ? _power_factor / _power_pulse_width : 0;
    _cf_interrupt_count = 0;
    return _power;
}

void HLW8012::expectedRMSCurrent(double value) {
    if (_current == 0) getRMSCurrent();
    if (_current > 0) _current_factor *= (value / _current);
}

void HLW8012::expectedRMSVoltage(unsigned int value) {
    if (_voltage == 0) getRMSVoltage();
    if (_voltage > 0) _voltage_factor *= ((double) value / _voltage);
}

void HLW8012::expectedRMSPower(unsigned int value) {
    if (_power == 0) getRMSPower();
    if (_power > 0) _power_factor *= ((double) value / _power);
}

void HLW8012::cf_interrupt() {

    unsigned long now = micros();
    _power_pulse_width = now - _last_cf_interrupt;
    _last_cf_interrupt = now;
    _cf_interrupt_count++;

}

void HLW8012::cf1_interrupt() {

    unsigned long now = micros();
    unsigned long pulse_width = now - _last_cf1_interrupt;

    _last_cf1_interrupt = now;
    _cf1_interrupt_count++;
    if ((_cf1_interrupt_count % CF1_SWITCH_COUNT) == 0) {

        pulse_width *= 2;

        if (_mode == _current_mode) {
            _current_pulse_width = pulse_width;
        } else {
            _voltage_pulse_width = pulse_width;
        }

        _mode = 1 - _mode;
        digitalWrite(_sel_pin, _mode);

    }

}

void HLW8012::_checkCFSignal() {
    static unsigned long last = millis();
    if (_cf_interrupt_count == 0) {
        if ((millis() - last) > PULSE_TIMEOUT) _power_pulse_width = 0;
    }
    last = millis();
}

void HLW8012::_checkCF1Signal() {
    static unsigned long last = millis();
    static unsigned long last_count = 0;
    if (_cf1_interrupt_count == last_count) {
        if ((millis() - last) > PULSE_TIMEOUT) _current_pulse_width = _voltage_pulse_width = 0;
    }
    last = millis();
}
