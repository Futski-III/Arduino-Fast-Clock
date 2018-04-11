# Arduino Fast Clock V1.0
This project is an Arduino based Fast Clock implementation.  Typically used in model railroading, this fast clock runs faster than an ordinary clock.  You can specify exactly how fast you want it to run.  For example, if set to a 2:1 ratio, it will advance 2 minutes for every 1 real minute.  When set to 10:1, it will advance 10 minutes for every 1 real minute.  Or stated anoter way, it will advance one minute every 6 real seconds.

Note: Because of the way the fast clock runs, it is best to select a speed ratio that is an even multiple of 60, otherwise it may not appear to update smoothly.
i.e. choose one of the following:  1:1, 2:1, 3:1, 4:1, 5:1, 6:1, 10:1, 12:1, 15:1.  


## Parts List
* 1 - Arduino Uno or similar
* 1 - 4x20 LCD display with I2C backpack
* 1 - TM1637 LED display w/SPI connections
* 1 - Rotary Encoder
* 1 - DS3231 RTC (Real Time Clock) module
* 2 - 100 nF capacitor (0.1uF)  (# 104 may be printed on the cap.)
* 2 - 10K Resistor (Brown, Black, Orange)

## Library Dependencies
The following libraries musty be installed for this project.

Click here for instructions [How to Install Libraries from Github](https://github.com/Futski-III/Arduino-Fast-Clock/blob/master/How%20to%20Install%20Libraries.md)

* __LiquidCrystal_I2C__ by Frank de Brabander [Get it here on GitHub](https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library)

  **Note** This library may conflict with the NewLiquidCrystal library.  If you have NewLiquidCrystal library installed, you may need to uninstall it and use this library instead.  
* __TM1637Display__ by Avishay [Get it here on GitHub](https://github.com/avishorp/TM1637)
* __DS3232RTC__ by Jack Christensen [Get it here on GitHub](https://github.com/JChristensen/DS3232RTC)
* __Bounce2__ by Thomas O Fredericks [Get it here on GitHub](https://github.com/thomasfredericks/Bounce2)
* __TimeLib__ by Paul Stoffregen [Get it here on GitHub](https://github.com/PaulStoffregen/Time)
* __LiquidMenu__ by Vasil Kalchev [Get it here on GitHub](https://github.com/VaSe7u/LiquidMenu) 

  **Note:**  By default, the LiquidMenu library will not work with an I2C 4x20 LCD.  To fix that you will need to edit the
"LiquidMenu_config.h" file.  Change the I2C option from "false" to "true".  You will find the LiquidMenu_config.h file in the library folder.  You can use Notepad or any other text editor to make the change.  

## Pin Connections
| Device | Device pin | Arduino Pin | Protocol |
| ------------- |:----------:|:----------:| -------- |
|Rotary Encoder|GND|GND|none|
|Rotary Encoder|VCC+|VCC+|none|
|Rotary Encoder|SW|4|none|
|Rotary Encoder|DT|2|none|
|Rotary Encoder|CLK|3|none|
|LCD|GND|GND|I2C|
|LCD|VCC+|VCC+|I2C|
|LCD|SDA|A4|I2C|
|LCD|SCL|A5|I2C|
|RTC|GND|GND|I2C|
|RTC|VCC+|VCC+|I2C|
|RTC|SDA|A4|I2C|
|RTC|SCL|A5|I2C|
|TM1637 LED|GND|GND|SPI|
|TM1637 LED|VCC+|VCC+|SPI|
|TM1637 LED|DIO|12|SPI|
|TM1637 LED|CLK|11|SPI|

![alt text](https://github.com/Futski-III/Arduino-Fast-Clock/blob/master/fast_clock_bb.jpg "Fritzing Fast Clock")

## Fast Clock Source Code
The source code for the Fast Clock project can be found in the **"Fast_Clock.ino"** file.  

The easiest way to copy the code to your Arduino IDE is to
1. Open the code by doing **one** of the following:
   * Click on the file name ( [Fast_Clock.ino](https://github.com/Futski-III/Arduino-Fast-Clock/blob/master/Fast_Clock.ino) ).  It will open the code on a new page.  Find and click the **"Raw"** button.  It's near the top of the code block about 2/3 the way over on the right side of the screen.  That'll open the code in a window with nothing else.  
   * [Click this link](https://raw.githubusercontent.com/Futski-III/Arduino-Fast-Clock/master/Fast_Clock.ino) to go directly to the Raw code.
1. Hit Ctrl-A then Ctrl-C.  
1. Open the Arduino IDE and open a new blank project.  
   1. Delete any code that Auduino inserts
   1. paste (ctrl-V) the code into the blank Arduino Window.  
1. Save the project with a meaningful name like **"Fast_Clock"**.  

## Interesting Links
* [How does a Rotary Encoder Work](https://howtomechatronics.com/tutorials/arduino/rotary-encoder-works-use-arduino/
)
* [Debouncing a Rotary Encoder with hardware](http://embeddedsystemengineering.blogspot.com/2016/07/arm-cortex-m3-stm32f103-tutorial.html)

## Having Issues
Are you having problems or noticed a bug in the code?  At the top of this page is a tab called "Issues".  Click there, then open a "New Issue" and tell me about the problem, you're having.  If it's a code problem, I can work on fixing it.
