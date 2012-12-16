Lumiboard
=========

Lumiboard is an open-source, Bluetooth-enabled, sensor-driven, Arduino-compatible LED controller platform intended to make it easy to control nearly every type of LED I have seen. Use Lumiboard to control discrete LEDs, RGB LEDs (up to 5W), LED strips and even networked lights like the BlinkM!

More information
----------------
* Author website: http://jason-webb.info
* Wiki: http://jason-webb.info/wiki/index.php?title=Lumiboard

Board features
--------------
* Six PWM pins that you can use to sink six LEDs, or two high-power common-anode RGB LEDs
* An I2C interface (SDA and SCL pins) for programmable LEDs like the BlinkM
* Pins used by the I2C interface can also be used as digital OR analog IO pins (A4 and A5) for general use or controlling LED strips
* Three analog input pins intended to be connected to three potentiometers to allow for direct control over RGB values or animations. These three pins are directly connected to * Arduino analog pins 0, 1 and 2, so you can also connect nearly any other kind of sensor
* Connection to an optional external sensor board, which contains a temperature sensor, ambient light sensor and sound sensor (see Sensor Board section)
* On-board Bluetooth transceiver, accessible via software serial, so you can easily control your LEDs remotely using your smartphone or tablet
* FTDI interface (3.3V) so you can re-program the board at any time manually