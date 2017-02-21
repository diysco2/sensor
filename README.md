# Sensor
This repository describes the steps needed to build a DIYSCO2 Sensor. Both, the hardware and software components are covered here. Please note that the hardware setup is subject to change and therefore the documentation will update accordingly. 


<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [Hardware: Build A DIYSCO2](#hardware-build-a-diysco2)
  - [Parts & Components](#parts--components)
  - [Putting together your Custom Hardware Board](#putting-together-your-custom-hardware-board)
  - [Assembling the Arduino Components](#assembling-the-arduino-components)
    - [Adafruit GPS](#adafruit-gps)
    - [Standard A-B USB cable to power the Arduino Mega](#standard-a-b-usb-cable-to-power-the-arduino-mega)
    - [Temperature Sensors](#temperature-sensors)
  - [Power Supply:](#power-supply)
  - [Inlet Tube System & Filter](#inlet-tube-system--filter)
- [Software: Arduino & Li-820](#software-arduino--li-820)
  - [The DIYSCO2 Software & Arduino](#the-diysco2-software--arduino)
    - [Install the libraries](#install-the-libraries)
    - [Upload the DIYSCO2 logger program](#upload-the-diysco2-logger-program)
  - [Upload Li-820 Configurations](#upload-li-820-configurations)
    - [Check your system](#check-your-system)
    - [Sync your system](#sync-your-system)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->


# Hardware: Build A DIYSCO2

## Parts & Components
Before you start, you are going to need the following items:

* IRGA:
    - 1 x Licor Li-820 infrared gas analyzer
* Microcontroller System:
    - 1 x Arduino Mega microcontroller
    - 1 x Adafruit Ultimate GPS Logger Shield with GPS Module
    - 2 x Maxum Integrated One Wire Gitial Temperature Sensor DS18B20
    - 1 x Custom hardware board (contact us!)
    - 3 x LED lights
    - 1 x 8 or 16gb microSD card
    - 2 x standard A-B USB cable, for USB 1.1 or 2.0 (1 for connecting to your computer and one to modify to deliver power to arduino)
* Custom Hardware Board:
    - 1 x DC/DC power supply (PXE30-12S12)
    - 1 x 4-pin RS232 connector (header)
    - A handful of single row Socket pins
    - A handful of 8-pin headers
    - 1 x Relay driver socket 
    - 1 x resistor network
    - 1x diode (DS1)
    - 4 x resistors (2 x 10k, 2 x 5k) 
    - 3 x fuses
    - 1 x LED light
    - 2 x Relays
    - 2 x capacitors
    - 1 x 7805
    - 1 x fuse holder
    - 2 x 8 position black connectors
    - 1 x UNL2803A
* RS232- Cable (PC to Arduino)
    - 1 x Jtron Mini TTL to RS232 MAX3232
    - 1 x Connector to Arduino (Model EC100F-4-D; Newark:36B6799 )
    - Heat Shring 2.5'' ( 1/2'' FIT221 or similar)
    - 1 x Ribbon cable: 9 conductor, #28 Str., 0.05 pitch
    3M 3302/9SF
    - 1 x D9F, Amphenol (Newark #99K7473 or 46F2695)
* Inlet tube system & filter:
    - 1 x 3m long inlet tube (6.35mm diameter, Synflex, polythelene-aluminum composite)
    - 1 x KNF NMP015 microdiaphragm pump
    - 1 x Balston Disposable filter unit (DFU)
    - 1 x Swagelok needle valve
    - a handful of tube fittings (https://www.swagelok.com/en/catalog/Fittings)
* Box container:
    - 1 x weatherproof Nanuk 910 case or equivalent
* Power supply:
    - 1 x 9-18C DC/DC battery 
    - 1 x 12v Socket Plug for Cigarette lighter (https://www.aliexpress.com/item/2016-New-Hot-12V-24V-1-5m-Male-Auto-Car-Motorcycle-Truck-Cigarette-Lighter-Socket/32674033316.html?spm=2114.40010508.4.86.IYuJtW)
* Additional components:
    - plenty of wire
    - soldering iron / tin
    - 1 x 12VDC micro brushless fan (http://www.ebay.com/p/RadioShack-3in-Brushless-12-VDC-Cooling-Fan-273-243b/1300133744)
    - Soft Tubing for between the balston tube & licor (see licor specs)
    - handful of 2-ping molex connectors (male and female)
* Other considerations:
    - Drill
    - heat gun
    - nuts and bolts
    - crimper for molex fittings and pins

***
## Putting together your Custom Hardware Board
<img src="http://placehold.it/700x300">


***
## Assembling the Arduino Components
### Adafruit GPS
Your adafruit GPS will come with break away headers. You will need to solder the pins in order to connect it to the custom hardware board. 

The diagram below shows which pins should be soldered for our use case.

<img src="http://placehold.it/700x300">

### Standard A-B USB cable to power the Arduino Mega
The Arduino Mega is powered by 5V which is fed in through the USB port. In order to feed power into the Arduino Mega, follow the steps below:

<img src="http://placehold.it/700x300">

1. Cut the USB cable to the X cm, keeping the side which plugs into the Arduino and discarding the other end.
2. Strip the shrink wrapping around the cut end to expose the wires below - there should be a red and black wire. 
3. Strip the shrink wrapping around the red and the black wire


### Temperature Sensors
There are 2 temperature sensors that are part of the system. One thermometer goes within the box to monitor the internal box temperature and the other thermometer goes outside next to the inlet. You will need to add wire to each temperature sensor to elongate them. The internal thermometer will be extended to ~25-30cm and the external thermometer to ~3m in length.

<img src="http://placehold.it/700x300">




***
## Power Supply:

We rely on an input 12V power supply for the DIYSCO2 system. You may choose to power your systems however you'd like, but we suggest the following setup:

<img src="http://placehold.it/700x300">

You will take your wires for your sensor and attach 2-pin molex fittings (http://cablesandconnectors.com/PIX/moljr2pin.jpg) - the male side will go on the wires attached to your DIYSCO2 and the female fittings will go to where the power is coming from.

***You will need a crimper to crimp the pins to the ends of the wires.




***
## Inlet Tube System & Filter

The inlet tube system 
<img src="http://placehold.it/700x300">



***
# Software: Arduino & Li-820

## The DIYSCO2 Software & Arduino

### Install the libraries

Install the sensor library dependencies in the `Arduino/libraries` directory to your machine. Follow [the instructions listed here](https://www.arduino.cc/en/Guide/Libraries) to make the libraries available on your machine.

### Upload the DIYSCO2 logger program

To upload the `li_820` logger program to the arduino board:

1. open the `Arduino/LI820_Logger/LI820_Logger.ino` in the [Arduino IDE](https://www.arduino.cc/en/Main/Software)
2. Plug your arduino mega into the computer using the USB cable
3. click `upload`

***
## Upload Li-820 Configurations
You will need to make sure that the Li-820 has the right configurations uploaded. To do this you will need `hyperterminal` (only on windows)

### Check your system
To check what the current config of your system - send this via hyperterminal:

1. First: connect to the instrument (look for the little telephone icon)

2. Second: copy 1 of the following commands from a text editor:


		This should send you back the current configuration of your system.
		
		<LI820> <CFG> ? </CFG> </LI820>
		    
		or
		    
		<LI820>?</LI820>

3. Third: edit --> paste to terminal (or something like this)


4. Forth: get the current config of your system

### Sync your system
Using hyperterminal or any other RS-232 communication program - send this config to the LI-820:

Once you know what the config is, make sure that this config is the same for all systems - particularly, be sure that the filter rate is 1 second. 

1. First: connect to the instrument (look for the little telephone icon)

2. Second: copy 1 of the following commands from a text editor:

		
		<LI820>
		 <CFG>
		    <OUTRATE>1</OUTRATE>
		    <PCOMP>TRUE</PCOMP>
		    <HEATER>TRUE</HEATER>
		    <FILTER>1</FILTER>
		    <BENCH>14</BENCH>
		    <ALARMS>
		        <ENABLED>FALSE</ENABLED>
		        <HIGH>900</HIGH>
		        <HDEAD>-1</HDEAD>
		        <LOW>300</LOW>
		        <LDEAD>-1</LDEAD>
		    </ALARMS>
		    <DACS>
		        <RANGE>5.0</RANGE>
		        <D1>CO2</D1>
		    </DACS>
		 </CFG>
		 <RS232>
		     <CO2>TRUE</CO2>
		     <CO2ABS>TRUE</CO2ABS>
		     <CELLTEMP>TRUE</CELLTEMP>
		     <CELLPRES>TRUE</CELLPRES>
		     <IVOLT>TRUE</IVOLT>
		     <STRIP>FALSE</STRIP>
		     <ECHO>TRUE</ECHO>
		     <RAW>FALSE</RAW>
		 </RS232>
		</LI820> 
		    


3. Third: edit --> paste to terminal (or something like this)

4. Fourth: Make sure the <AKS> tags = "TRUE" --> this confirms that the configuration was received and valid.






