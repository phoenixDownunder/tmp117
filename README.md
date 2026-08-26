# README.md

* Last update date August 26 2026.

--
# TMP117 Temperature Sensor

* The tmp117 is a simple temperature only sensor from Texas Instruments.

* The tmp117 has proven to be remarkably accurate, extremely stable with a rapid change and settle response. When testing other devices, it is now the preferred reference.

* Technically, the tmp117 sensor has been updated and replaced by the tmp119. (See Adafruit TMP119)

* The tmp117 version used here is the SparkFun Qwiic TMP117 i2c Breakout sensor. It is very easy to use although any Alert line would have have to be physically attached. i2c is used only.
See the image at [SparkFun TMP117](SparkFun-©-BME688-_Environmental_Sensor_Breakout.jpg)

--
# Contents

* TMP117 Temperature Sensor
* Contents
* Device Reference
* Raspberry Pi and i2c
* Licence
* Quick Build
* Quick Remove
* Keeping It Simple
* Raspberry Pi Preparation
* Author

--
# Device Reference

* The reference document for the TMP117 is...
* SNOSD82C – JUNE 2018 – REVISED APRIL 2021
* Description: TMP117 High-Accuracy, Low-Power, Digital Temperature Sensor With SMBusTM- and I2C-Compatible Interface
* [tmp117](https://www.ti.com/lit/gpn/TMP117) pdf document.
* The TI [TMP117 product description](https://www.ti.com/product/TMP117) is here.

--
# Raspberry Pi and i2c

* Raspberry Pi: A Raspberry Pi is used to manage the tmp117 along with a range of other i2c connected devices. Only tested with RPi4 (4Gb) but should work with other versions. The software footprint and runtime requirements are very small.

* Interface: The interface is 400kb i2c and the Sparkfun version which is being used for testing, has a default i2c address 0x48. See separate section below.

* Load: On a Raspberry Pi 4 and using Raspbian Lite (Trixie at this time), there is no load on the the RPi.

* Descriptors: All i2c devices attached are described in the header file i2cDevicesHeader.h. The active device is defined in the **Main** routine, in this case, \#define TMP117 . If any of the header files are altered, then the whole application should be re-compiled. See separate section below. The current i2cDevicesHeader.h file has been cut back and is basically static. New experimental device are attached occasionally.

--
# Licence

This software is released under the [BSD-3-Clause Licence](Licence.md) and a copy of the licence is provided at the top level. A copy of the said licence has been placed in each of the source files where possible or in the enclosing folder to satisfy the legal Dementors. 

--
# Quick build

* Prepare a Raspberry Pi(4+) with the i2c libraries (See Preparation below) in the usual way.
* Copy the tmp117.d folder to the **HOME** directory.
* Copy or move the headers folder to the **HOME** directory.
* To build the tmp117 executable, run
 
```
cd tmp117.d
make cleanall
make all
```
This is equivalent to

```
gcc -o tmp117 tmp117.c -I$HOME/headers/ -Wall -pedantic -std=c2x -D_GNU_SOURCE -li2c -lm
```

then run the executable with any options such as those presented below.
Set the link below for convenience if required.
The examples below terminate after operation.

```
./tmp117 ?
./tmp117 -v
./tmp117 -n 2
./tmp117 -n 2 debug

ln -s /home/(username)/tmp117.d/tmp117 ~/bin/tmp117
```
--
### Quick Remove
To remove the whole lot, remove the directories tmp117.d and headers. Nothing is saved elsewhere.

--
# Keeping It Simple

* With the exception of the main procedure, the main tmp117 software described is contained in the file tmp117Handlers.h.  
* All Command Line Argument handlers are contained in a separate file tmp117CLAHandlers.h
* Everything is written in C. The software has been written and designed to be readable and therefore sometimes a little verbose. No hidden tricks, obfuscating code or short unreadable naming tricks are used.
* No separate third party libraries are used.
* Makefiles are used to compile into a local directory. Or you can use gcc directly. The c2x standard is used.
* The most important variable is of course the device file pointer, **fp**. If the device is not found, everything stops.

--
# Raspberry Pi Preparation

* The first and most obvious stage is to prepare a Raspberry Pi.
* Any version can be used but a RPi4 or faster with at least 1Gb of RAM is suggested. I haven't tried RPi3 or RPiZero. An RPi5 won't function any better than the RPi4 because of the tmp117 design in this case.
* A reasonably fast SDRAM card 16Gb or better is suggested.
* Download a recent 64bit version of Raspbian Lite, currently Trixie, in the usual way and prepare the SDRAM. Strictly speaking, only Raspbian Lite is required but you can install a GUI version if you so desire. On a Mac Terminal screen, run

```
sudo dd bs=16m if=./2026-06-18-raspios-trixie-arm64-lite.img of=/dev/rdisk2
```

* or use balenaEtcher or similar.
 
* Install and boot the RPi.

```
sudo dpkg-reconfigure console-setup
```

* (For example, try UTF-8 -> Guess -> Terminus -> 12x24 framebuffer...
to set console font size from the default. Works for 27" and 32" monitors)

* Once started, log in and run the command line routine...

```
sudo raspi-config
```

* to set the host name (for example RPi44), wifi connection, enable interface->i2c and interface->ssh. All other options are you own. Reboot as required.
Run the following command (or ip -4 a (enter)) to find your local ip address for a local ssh connection and then complete all sensible updates...

```
ifconfig
```

```
sudo apt update
sudo apt upgrade -y
sync
sudo apt dist-upgrade
sudo apt autoremove
sync
```

* Create /etc/sudoers.d/010_pi-nopasswd and put in... (yes I know that the username pi shouldn't be used but its convenient!)

```
pi ALL=(ALL) NOPASSWD: ALL
```
```
sudo shutdown -r now
```

* Reconnect and...

```
sudo apt install i2c-tools
sudo apt-get install libi2c-dev 
```

* Now, to use a fast i2c interface, edit the file /boot/firmware/config.txt and add...

```
# i2c  
dtparam=i2c=on,i2c_baudrate=400000
```

* There appears to be whole pile of variations of this exercise to achieve the same result. This works at the moment (Trixie 2026) but check.

--
# Basic tmp117 options...

tmp117 [debug] [i2c adapterAddress] [slaveAddress] [-d] [-M 0xXXXX | YYY] [-m 0xXXXX | YYY] -o [YYY] -c [1 | c] -a [0,8,32,64] -v]

Argument options for the tmp117 are...

?, -?, -h, -H, h, H: Print Help and close.  
-d : set default values for all device registers  
-o : to set Temperature Offset Register [0..256] (may take a read cycle to clear the temperature buffer)  
-c : to set one shot conversion [1] or continuous conversion [c or C] (Set but unused here)  
-a : set the number for averaged rates of conversion [0,8(default),32,64]  
-v : Display all device registers  
-M : for Max or set High Limit Temperature Register 0xXXXX or [+-][0..256]  
-m : for Min or set Low  Limit Temperature Register 0xXXXX or [+-][0..256]  
-n : set loop count [0 for infinite or nn seconds]  
-f : Display the OUTPUT temperature in Fahrenheit  
-A : Set Temperature (1) or Alert (0) Mode  
-p : set Alert pin - DR Data Ready (1) or Alert pin (0)  
-P : set Alert Polarity Active High (1) or low (0)  

--
# Disclaimer

This is a development framework and experimental software. It mostly works as designed. Hopefully there aren't too many bugs. Use at your own risk. No liability. If it crashes your Raspberry Pi (highly unlikely) or blows up your cat (more likely), it's your problem.
Hopefully, there won't be too many bugs and not too many errors in this document!

--
# Author

* Author: Bruce Stephens.
* Country of origin: Australia.
* Most Recent Update: August 2026.
* Version Number: soon...
* Errors : bruce @ eigenspaces.com.au

--