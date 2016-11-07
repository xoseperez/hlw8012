# HLW8012

HLW8012 library for Arduino and ESP8266 using the [Arduino Core for ESP8266][1].

![HLW8012 Pinoout](/docs/HLW8012_pinout.png)


This is the IC present in some chinese products like [Itead's Sonoff POW][2].
The HLW8012 is a current, voltage and power monitor IC that outputs a pulse of a frequency inversely proportional to the value to be read.
This IC provides two PWM outputs, the first one for power and the second one for current or voltage, depending on the SEL pin. The output values are always RMS. Power measurements are very consistent but current or voltage measurements require a minimum time lapse after changing the SEL pin value to become stable. This fact reduces sampling frequency.
Higher values (of power, current or voltage) mean shorter pulses.

Typical values are:

* A 1Hz pulse on CF pin means around 10W RMS
* A 100Hz pulse on CF pin means around 1kW RMS
* A 1Hz pulse on CF1 pin means 15mA or 0.41V RMS depending on the value in SEL pin
* A 100Hz pulse on CF1 pin means 1.5A or 41V RMS depending on the value in SEL pin

These ratios are per datasheet typical application, but the actual circuitry might be different.
Even if the circuit matches that on the datasheet the IC tolerances are quite loosy (+-15% for clock frequency, for instance).
So a manual calibration is necessary.

## Features

The main features of the HLW8012 library are:

* Two available modes: interrupt-driven or non-interrupt-driven.
* Default calibration based on product datasheet (3.1 Typical Applications).
* Optional manual calibration based on expected values.

## Issues

When using interrupts, values are monitored in the background. When calling the getRMS*** methods the last sampled value is returned, this value might be up to some seconds old if they are very low values. This is specially obvious when switching off the load. The new value of 0W or 0mA is ideally represented by infinite-length pulses. That means that the interrupt is not triggered, the value does not get updated and it will only timeout after 10 seconds (PULSE_TIMEOUT constant in HLW8012.h). During that 10 seconds the library will still return the last non-zero value.

On the other hand, when not using interrupts, you have to let some time for the pulses in CF1 to stabilize before reading the value. This is the "interval" parameter in the handle() method that defaults to 3 seconds. The longer this value the more stable the readings (in particular for low values), but also decreases the frequency sampling.

## Notes

I've put together this library after doing a lot of tests with a Sonoff POW[2]. The HLW8012 datasheet (in the "docs" folder) gives some information but I couldn't find any about this issue in the CF1 line that requires some time for the pulse length to stabilize (apparently). Any help about that will be very welcome.

## Usage

Check the examples for indications on how to use the library.


[1]:https://github.com/esp8266/Arduino
[2]:https://www.itead.cc/sonoff-pow.html?acc=70efdf2ec9b086079795c442636b55fb
