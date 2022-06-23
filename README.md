# About the ARD-DCCSHIELD

The ARD-DCCSHIELD is an optoisolated interface shield for connecting an Arduino to a model railroad DCC control system.  Additionally, the board has provisions for connecting I2C accessories and general purpose I/O lines, as well as optionally (jumper selectably) powering the Arduino off of DCC power and providing up to 400mA of 5VDC to connected circuitry.

The board is designed to be compatible with various DCC decoder libraries and Arduino boards.  We typically use [Alex Shepherd's excellent NmraDcc library](https://github.com/mrrwa/NmraDcc/).  

The ARD-DCCSHIELD was designed by Nathan Holmes (maverick /at/ drgw.net) and Michael Petersen (railfan /at/ drgw.net).  The ARD-DCCSHIELD is open hardware, meaning that anyone is welcome to make their own or change/improve upon our design to meet their needs.  Please refer to the hardware and software license files in this repository for details.

For those not wanting to build their own, the boards are [available for purchase](http://www.iascaled.com/store/ARD-DCCSHIELD) from Iowa Scaled Engineering.

## Features

* Convenient way to interface standard Arduinos with DCC track signals, in order to build custom decoders
* Can route DCC signal to D2, D3, or D7 as an input, depending on the needs of the host processor
* Provides complete electrical isolation between the track and your Arduino/computer (if not powered off DCC)
* Can provide up to 500mA of 5V DC power off the DCC track signal
* Provides handy Qwiic-compatible I2C port for connecting more peripherals (v2.x only)
* Provides 8-position terminal block for connecting to Arduino I/O lines


## Getting Started

At the bare minimum, you'll need to:
 * Have a working knowledge of electronics and programming.  The ARD-DCCSHIELD is a building block that provides an electrical interface between an Arduino and the DCC track signal, not a complete decoder.
 * Connect DCC track voltage should be connected to the DCC connector, a blue two-pin terminal block at the bottom of the board (J7 on v1.x boards, J6 on v2.x)
 * Configure the board to route the DCC signal to the appropriate Arduino digital I/O pin (D2, D3, or D7).  With v1.x boards, this was done using JP4 and possibly JP2, pins 1-2).  With v2.x boards, this just involves flipping one of the DIP switches (SW1, switches 3-5) to "ON" matching the pin you want to route the signal into.
 * Provide power to the Arduino or configure the DCCSHIELD to power things off of DCC (JP5 on v1.x boards, the small slide switch SW2 on v2.x boards).

You'll also probably need to [install an Arduino library](https://www.arduino.cc/en/Guide/Libraries) to decode the DCC signal, unless you intend to write your own.

If all is good, you'll get both a green and an amber LED glowing on the shield.

When in doubt, please refer to the schematic.  The source files are in gEDA format, but there are PDFs of the production version schematics under the pg/ directory.  Just pick the version that corresponds to your PCB version and open the "ard-dccshield.pdf" file.

 * [v1.0 Schematic](https://github.com/IowaScaledEngineering/ard-dccshield/raw/master/pg/ard-dccshield-v1.0-ba9ec04/ard-dccshield.pdf)
 * [v1.1 Schematic](https://github.com/IowaScaledEngineering/ard-dccshield/raw/master/pg/ard-dccshield-v1.1-f6d6da8/ard-dccshield.pdf)
 * [v1.2 Schematic](https://github.com/IowaScaledEngineering/ard-dccshield/raw/master/pg/ard-dccshield-v1.2-2bd1af7/ard-dccshield.pdf)
 * [v1.3 Schematic](https://github.com/IowaScaledEngineering/ard-dccshield/raw/master/pg/ard-dccshield-v1.3-e23d249/ard-dccshield.pdf)
 * [v2.0 Schematic](https://github.com/IowaScaledEngineering/ard-dccshield/raw/master/pg/ard-dccshield-v2.0-4f6f1bd/ard-dccshield.pdf)

## Indicators

### D1 - Amber LED
The amber LED indicates that a signal is present on J6 (v2.x boards) or J7 (v1.x boards), the DCC input.  If the amber LED does not light, the board is not receiving a signal.

### D2 - Green LED
The green LED indicates that the ARD-DCCSHIELD is correctly powered.  If this light does not come on, check that either the Arduino is getting power from an external source or that the power switch SW2 (v2.x boards) or JP5 (v1.x boards)is set correctly.

## Connectors - Version 2.x Boards ONLY

*Version 2.x is the most recent revision of the ARD-DCCSHIELD.  All boards shipped by ISE after 6/15/2022 will be version 2.0 or later.*

### J1/J2 - Standard Rev 3 Arduino Headers

These are the big pin headers that connect the shield through to an Arduino.  

For some early Arduinos (pre-R3 pinout), you'll notice that the topmost pins (N/C, VIO, RST, 3.3V on J1, SCL, SDA, AREF, and GND on J2) do not connect.  See the JP6 entry in the Configuration section below for details on what you'll need to do.

### J3 - Holes for Arduino SPI

If you need to pass the 6-pin ICSP/SPI header through the ARD-DCCSHIELD to some other shield above it, we've left holes in the right spot.  You'll need to solder in an appropriate 2x3 pin header - see the bill of materials for a suggested Samtec part number.  These holes are not connected to anything on the ARD-DCCSHIELD.

### J4 - Digital I/O Terminal Block

Since most folks are going to decode a DCC signal to then trigger some other hardware, we've provided terminal blocks for eight of the Arduino I/O pins.  These connect to the digital I/O corresponding to the label except for D14, which connects to A0.

### J5 - Power and VIO

Most external hardware is going to need access to power and ground.  J5 provides access to the Arduino +5V line, ground, and VIO (the Arduino's native I/O voltage).

### J6 - DCC Input

This 2-position blue terminal block expects a DCC signal in the range of track voltage (9-22V).  Depending on track voltage, this will present a 9-20mA load to your booster.

### J7 - Qwiic-Compatible I2C Port

This connector is compatible with [SparkFun's Qwiic connector standard](https://www.sparkfun.com/qwiic) for I2C peripherals.  The signals here have been level-shifted to 3.3V for compatibility.  If you want to use an I2C peripheral operating at the Arduino's native VIO (5V with most AVR-based Arduinos, 3.3V with most ARM-based Arduinos), tap the signals out of J8 instead.

### J8 - Alternate 4-Pin I2C

This connector provides an alternate pinout for I2C connections.   These pins are ***not*** level shifted, and operate at the native VIO voltage of the Arduino (5V with most AVR-based Arduinos, 3.3V with most ARM-based Arduinos).


## Configuration - Version 2.x Boards ONLY

### SW1 - Configuration DIP Switch
* Switch 1 does nothing (not connected)
* Switch 2 connects the DCC acknowledgement current pulse generator (ACK) circuit to Arduino pin A1 if switched on.  This will allow the shield to emit a 60mA DCC programming acknowledgement pulse when pin A1 is set high.  The ACK circuit is not designed to operate continually, but only for the ~6 millisecond pulses needed for generating ACKs.  While it should protect itself, U4 and R8 will become quite hot if A1 remains high for significant lengths of time.  If left off, it will disable the ACK generator. 
* Switch 3 routes the incoming DCC signal to Arduino pin D2 if on
* Switch 4 routes the incoming DCC signal to Arduino pin D3 if on
* Switch 5 routes the incoming DCC signal to Arduino pin D7 if on

### SW2 - Power Supply Selection
If SW2 is set towards "ISOLATED", the Arduino is galvanically isolated from track power and can be safely connected to any other power source.  (In fact, it *must* be powered from some other source.)  This allows it to operate connected to an auxiliary power bus or other such power source that you don't want shorted to track power.

If SW2 is set towards "FROM DCC", the ARD-DCCSHIELD will use an onboard high efficiency DC-DC converter to convert track power into 5V to power the Arduino through the 5V pin, as well as the ARD-DCCSHIELD and any other attached hardware.  The power supply is good for 500mA.  Typically the Arduino and the ARD-DCCSHIELD will need 40-50mA of that, so you can safely power approximately 400mA of additional 5V load off of the regulator as well.

### JP6 - Power Supply Selection

Normally, the ARD-DCCSHIELD gets its power from the VIO pin, so as to be compatible with both 3.3V and 5V Arduino-format boards.  For early Arduino pinouts (now exceedingly rare) that lack a VIO (I/O voltage) pin, there's solder jumper JP6 which shorts the VIO line (which isn't present) to 5V.  You'll need to solder jumper JP6 closed to allow the shield to work correctly.  Just heat the pads and apply a small amount of solder to bridge the two halves.  

*Note:  Once JP6 is shorted, the board will fry any 3.3V Arduino it gets plugged into.  Be very careful.  If you want to use the board with a low voltage Arduino once again, carefully remove the solder bridge with solder braid or similar.*

---
# Documentation for v1.x ARD-DCCSHIELD Boards

*Version 1.x is the first generation of the ARD-DCCSHIELD.  The documentation is preserved here for reference, and will apply to all boards shipped by ISE before 6/15/2022.*

## Configuration - Version 1.x Boards ONLY

### JP1 - I2C Pull-Up Resistors

I2C is an open-collector bus, meaning each node can only pull the line low.  Therefore, both SDA and SCL must be pulled up with resistors to make the bus function.  If you intend to use the I2C connections on the ARD-DCCSHIELD and don't have pull-ups elsewhere, jumper pins 1-2 and 3-4 of JP1 to provide 2k pull-ups on both SDA and SCL.

### JP2 (Pins 1-2) - I2C /IORST

If you need power on the [Iowa Scaled Engineering standard I2C 6p6c (RJ11) connector](https://www.iascaled.com/store/6P6C-I2C), it's typically sent down the /IORST line.  Jumpering pins 1-2 will connect VIO on the Arduino/shield to the /IORST line on the connector (pin 4).  If you don't intend to use I2C or don't need power, leave this jumper open.

Alternately, if you're not using Arduino D7 for your DCC signal (see JP4 configuration below) and want the ability to reset I2C peripherals using this signal, you can tie /IORST to D7 by placing the number between pins 2-3.

### JP3 - I2C /INT and /OE 

The [Iowa Scaled Engineering standard I2C 6p6c (RJ11) connector](https://www.iascaled.com/store/6P6C-I2C) allows for two additional signals: /OE (active low Output Enable) going from the Arduino to the I2C bus; and /INT (active low interrupt), to allow I2C slaves to signal the Arduino they need attention.  Jumper JP3 pins 1-2 to connect /INT to Arduino pin D6 and/or JP3 pins 3-4 to connect /OE to Arduino pin D5.

### JP4 and JP2 (Pins 3-4) - DCC Signal to Arduino Pin

Probably the single most important configuration on the ARD-DCCSHIELD is to select which Arduino pin will receive the DCC signal.  Typically this is going to be a pin that can be conencted to a hardware change interrupt in the code, as edges need to be precisely detected to decode the signal correctly.  To accomodate a wide variety of Arduinos, there are three possible selections:
 * DCC signal to D2 - jumper JP4, pins 3-4
 * DCC signal to D3 - jumper JP4, pins 1-2
 * DCC signal to D7 - jumper JP2, pins 3-4

A complete list of which digital IO lines are available as hardware interrupts is available at the Arduino [attachInterrupt()](https://www.arduino.cc/en/Reference/AttachInterrupt) page.

### JP5 - Arduino Powered from DCC

If JP5 is left un-jumpered, the Arduino is galvanically isolated from track power and can be safely connected to any other power source.  (In fact, it *must* be powered from some other source.)  This allows it to operate connected to an auxilliary power bus or other such power source that you don't want shorted to track power.

If pins 1-2 and 3-4 of JP5 are jumpered, the ARD-DCCSHIELD will use an onboard high efficiency DC-DC converter to convert track power into 5V to power the Arduino, ARD-DCCSHIELD, and any other attached hardware.  The power supply is good for 500mA.  Typically the Arduino and the ARD-DCCSHIELD will need 40-50mA of that, so you can safely power approximately 400mA of additional 5V load off of the regulator as well.

### JP6 - Boards without VIO

Normally, the ARD-DCCSHIELD gets its power from the VIO pin, so as to be compatible with both 3.3V and 5V Arduino-format boards.  For early Arduino pinouts that lack a VIO (I/O voltage) pin, there's solder jumper JP6 which shorts the VIO line (which isn't present) to 5V.  You'll need to solder jumper JP6 closed to allow the shield to work correctly.  Just heat the pads and apply a small amount of solder to bridge the two halves.  

*Note:  Once JP6 is shorted, the board will fry any 3.3V Arduino it gets plugged into.  Be very careful.  If you want to use the board with a low voltage Arduino once again, carefully remove the solder bridge with solder braid or similar.*

### JP7 - Enable Programming ACK

Jumpering pins 1-2 & 3-4 of JP7 will allow the shield to emit a 60mA DCC programming acknowledgement pulse when pin .  The ACK circuit is connected to Arduino pin A1, and will generate an ACK current pulse when A1 is set high.  The ACK circuit is not designed to operate continually, but only for the ~6 millisecond pulses needed for generating ACKs.  While it should protect itself, U4 and R8 will become quite hot if A1 remains high for significant lengths of time.

## Connectors - Version 1.x Boards ONLY

*Version 1.x is the first generation of the ARD-DCCSHIELD.  The documentation is preserved here for reference, and will apply to all boards shipped by ISE before 6/15/2022.*

### J1/J2 - Standard Rev 3 Arduino Headers

These are the big pin headers that connect the shield through to an Arduino.  

For some early Arduinos, you'll notice that the topmost pins (N/C, VIO, RST, 3.3V on J1, SCL, SDA, AREF, and GND on J2) do not connect.  See the JP6 entry in the Configuration section below for details on what you'll need to do.

### J3 - Holes for Arduino SPI

If you need to pass the 6-pin ICSP/SPI header through the ARD-DCCSHIELD to some other shield above it, we've left holes in the right spot.  You'll need to solder in an appropriate 2x3 pin header - see the bill of materials for a suggested Samtec part number.  These holes are not connected to anything on the ARD-DCCSHIELD.

### J4 - Digital I/O Terminal Block

Since most folks are going to decode a DCC signal to then trigger some other hardware, we've provided terminal blocks for eight of the Arduino I/O pins.  These connect to the digital I/O corresponding to the label except for D14, which connects to A0.

### J5 - Power and VIO

Most external hardware is going to need access to power and ground.  J5 provides access to the Arduino +5V line, ground, and VIO (the Arduino's native I/O voltage).

### J6 - ISE Standard 6p6c I2C 

This is an [Iowa Scaled Engineering standard I2C 6p6c (RJ11) connector](https://www.iascaled.com/store/6P6C-I2C), which can be used to connect to a number of ISE I2C peripheral boards.  It provides a handy modular plug for the I2C bus.

### J7 - DCC Input

This expects a DCC signal in the range of track voltage (9-22V).  Depending on track voltage, this will present a 9-20mA load to your booster.

### J8 - Alternate 4-Pin I2C

This connector provides an alternate pinout for I2C connections. 
*Note:  On rev 1.1 of the PCB, the SDA and SCL silkscreen markings are reversed.*


