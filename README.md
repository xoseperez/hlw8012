# HLW8012

HLW8012 library for Arduino and ESP8266 using the [Arduino Core for ESP8266][1].

![HLW8012 Pinoout](/docs/HLW8012_pinout.png)


This is the IC present in some chinese products like [Itead's Sonoff POW][2].
The HLW8012 is a current, voltage and power monitor IC that outputs a pulse of a frequency inversely proportional to the value to be read.
This IC provides two PWM outputs, the first one for power and the second one for current or voltage, depending on the SEL pin. Power measurements are very consistent but current or voltage measurements require a minimum time lapse after changing the SEL pin value to become stable. This fact reduces sampling frequency.
Higher values (of power, current or voltage) mean shorter pulses.

Typical values are:

* A 1Hz pulse on CF pin means around 10W
* A 100Hz pulse on CF pin means around 100W
* A 1Hz pulse on CF1 pin means 15mA or 0.41V depending on the value in SEL pin
* A 100Hz pulse on CF1 pin means 1.5A or 41V depending on the value in SEL pin

These ratios are per datasheet typical application, but the actual circuitry might be different.
Even if the circuit matches that on the datasheet the IC tolerances are quite loosy (+-15% for clock frequency, for instance)
So a manual calibration is necessary.

## Features

The main features of the HLW8012 library are:

* Two available modes: interrupt-driven or non-interrupt-driven.
* Default calibration based on product datasheet (3.1 Typical Applications).
* Optional manual calibration based on expected values.

## Usage

Check the examples for indications on how to use the library.


[1]:https://github.com/esp8266/Arduino
[2]:https://www.itead.cc/sonoff-pow.html?acc=70efdf2ec9b086079795c442636b55fb
