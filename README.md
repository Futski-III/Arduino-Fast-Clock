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

## Library Dependencies
* LiquidMenu by Vasil Kalchev [Get it here on GitHub](https://github.com/VaSe7u/LiquidMenu)
* next link
