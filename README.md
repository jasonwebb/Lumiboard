## Lumiboard

Lumiboard is an open-source platform for controlling many types of LEDs, including high-power RGB LEDs, LED strips and I2C LEDs.

* Discrete LEDs
* High-power common-cathode RGB (or single-color) LEDs, up to 5W
* Basic LED strips
* Digitally-addressable LED arrays (such as NeoPixel-based packages)
* I2C LED pixels (like the BlinkM)

### Board features
* **ATMega328** with the Arduino Pro Mini (8 mHz @ 3.3V) bootloader flashed
* **Screw terminals** allow all board functions to be easily tapped into.
* **Six PWM pins** broken out and connected through a ULN2003, so you can sink two high-power RGB LEDs.
* **I2C interface** (SDA and SCL pins) for programmable LEDs like the BlinkM. 
* **Three potentiometer inputs** so you can control RGB colors directly, or connect other sensors as analog inputs.
* **External sensor board** containing a temperature sensor, ambient light sensor and sound sensor (see Sensor Board section)
* **Bluetooth transceiver** accessible via SoftwareSerial, so you can easily control your LEDs remotely using your smartphone or tablet.
* **FTDI interface (3.3V)** so you can re-program the board at any time manually.

### More information
* Author website: http://jason-webb.info
* All my blog posts about this project: http://jason-webb.info/tag/lumiboard/
* Wiki: http://jason-webb.info/wiki/index.php?title=Lumiboard