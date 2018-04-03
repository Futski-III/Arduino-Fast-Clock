# Arduino Fast Clock V1.0
This project is an Arduino based Fast Clock implementation.  Typically used in model railroading, this fast clock runs faster than an ordinary clock.  You can specify exactly how fast you want it to run.  For example, if set to a 2:1 ratio, it will advance 2 minutes for every 1 real minute.  When set to 10:1, it will advance 10 minutes for every 1 real minute.  Or stated anoter way, it will advance one minute every 6 real seconds.

Note: Because of the way the fast clock runs, it is best to select a speed ratio that is an even multiple of 60, otherwise it may not appear to update smoothly.
i.e. choose one of the following:  1:1, 2:1, 3:1, 4:1, 5:1, 6:1, 10:1, 12:1, 15:1.  

The code has not been posted yet, but should be here by the beginning of April 2018.  Please come back at a later date.

Dr. Ivan Von Futski III


## Parts List
* 1 - Arduino Uno or similar
* 1 - 4x20 LCD display with !2C backpack
* 1 - TM1637 LED display w/SPI connections
* 1 - Rotary Encoder
* 1 - DS3231 RTC (Real Time Clock) module
* 2 - 0.1uF capacitor (100 nf)
* 2 - 10K Resistor

## Library Dependencies
The following libraries musty be installed for this project.
* __LiquidCrystal_I2C__ by Frank de Brabander [Get it here on GitHub](https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library)

  **Note** This library may conflict with the NewLiquidCrystal library.  If you have NewLiquidCrystal library installed, you may need to unindtall it and use this library instead.  
* __TM1637Display__ byAvishay [Get it here on GitHub](https://github.com/avishorp/TM1637)
* __DS3232RTC__ by Jack Christensen [Get it here on GitHub](https://github.com/JChristensen/DS3232RTC)
* __Bounce2__ by Thomas O Fredericks [Get it here on GitHub](https://github.com/thomasfredericks/Bounce2)
* __TimeLib__ by Paul Stoffregen [Get it here on GitHub](https://github.com/PaulStoffregen/Time)
* __LiquidMenu__ by Vasil Kalchev [Get it here on GitHub](https://github.com/VaSe7u/LiquidMenu) 

  **Note:**  By default, the LiquidMenu library will not work with an I2C 4x20 LCD.  To fix that you will need to edit the
"LiquidMenu_config.h" file.  Change the I2C option from "false" to "true".  You will find the LiquidMenu_config.h file in the library folder.  You can use notepad or any other text editor to make the change.  

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

## Fast Clock Source Code
The source code for the Fast Clock project can be found in the **"Fast_Clock.ino"** file.  

The easiest way to copy the code to your Arduino IDE is to simply click on the file name.  It will open the code on a new page.  Find and click the **"Raw"** button.  It's near the top of the code block about 2/3 the way over on the right side of the screen.  That'll open the code in a window with nothing else.  Hit Ctrl-A then Ctrl-C.  

Now open a new blank Arduino project and paste (ctrl-V) the code into the blank Arduino Window.  

Save the project with a good name like **"Fast_Clock"**.  
