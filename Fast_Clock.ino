/*
Fast_Clock.ino - Arduino Fast Clock for Model Railroading
Copyright (c) Dave Heili 2018
This software is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public
License as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.
This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.
You should have received a copy of the GNU General Public
License along with this software; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <Arduino.h>			// Define normal Arduino Library
#include <Wire.h>				// Needed for I2C usage 
#include <LiquidCrystal_I2C.h>  // I2C LCD display				//github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library
#include <TM1637Display.h>		// 4 Digit LED display			//github.com/avishorp/TM1637
#include <TimeLib.h>			// Time management				//github.com/PaulStoffregen/Time
//#include <DS1307RTC.h>			// Basic DS1307 / DS3231 RTC	//github.com/PaulStoffregen/DS1307RTC
#include <DS3232RTC.h>			// Basic DS3232 RTC				//github.com/JChristensen/DS3232RTC
#include <Bounce2.h>			// Debouce library				//github.com/thomasfredericks/Bounce2
#include <LiquidMenu.h>			// Menu Library					//github.com/VaSe7u/LiquidMenu

//===============================================================================================
//  Initial Settings - You may want to adjust these settings
//===============================================================================================
#define ROAD_NAME "L & BS"		
#define INITIAL_PAUSE_STATE true  // Tell if the fast clock starts paused or not (true / false)
#define DEFAULTHH 7				// Set starting FastClock Hour in Military Time
#define DEFAULTMM 0				// Set starting FastClock minute
#define MILITARY_TIME false		// Display 24 hour clock or not.  Use true or false
								// else to not print messages, change DEBUG_MODE to DEBUG_MODE_OFF
#define FAST_CLOCK_RATIO 20		// Fastclock Speed   ie. 4 is 4:1 factor or 4 fast minutes per 1 real minute,
								// Or 1 fast minute every 15 real seconds
                                // For best operation, pick a ratio that is evenly divisible by 60.  
								// ie 1,2,3,4,5,6,10,12,15,20,30,60  Although any number will work
#define WELCOME_DELAY 3000      // Time, in millis, that the welcome screen will show
#define LCD_ADDRESS 0x27        // Address of LCD Display.  Common addresses 0X27, 0x3F

#define DEBUG_MODE_off			// When DEBUG_MODE is defined, Serial Prints will show status

//===============================================================================================

// SPI Pins for TM1637 LED 4 digit display
#define LED_CLK 11				// Pin # for TM1637 LED CLK pin
#define LED_DIO 12				// Pin # for TM1637 LED DIO pin

// Pins for Rotary Encoder
#define ROTARY_ENCODE_A 2
#define ROTARY_ENCODE_B 3
#define ROTARY_ENCODE_SW 4
#define DEBOUNCEINTERVAL 5		// Specify how long to wait for a stable debounce state (in milli-sec)


// Clock update Frequency variables
unsigned int clock_check_wait_time = 200;  // in Millis - Default=200 or 5 times per second
unsigned long prev_millis = 0;

// Rotary Encoder variables
volatile boolean rotaryFired;			//Volatile because this are updated in an interupt
volatile boolean rotaryDirection;			//Volatile because this are updated in an interupt

// Setup deBounce object for rotary encoder switch
Bounce pushButton1 = Bounce();

//  Clock Variables
time_t fastClock=0, realClock=0, prevRealClock=0, tempClock = 0; // Create clock variables
tmElements_t timeParts;			// time_t Breakout
unsigned short rcmm = 0, prev_RcMM = 0;
unsigned short fcmm = 0, prev_FcMM = 0;

char rcText[10];				// Real Clock Display string
char fcText[10];				// Fast Clock Display string
char tcText[10];				// Temp Clock Display string
char* prcText;					// Pointer to Real Clock Display string
char* pfcText;					// Pointer to Fast Clock Display string
char* ptcText;					// Pointer to Temp Clock Display string
char* timePartText = "Set HH";

bool fcPaused = true;
char* pausedText = "Paused  ";
bool MilitaryTime = MILITARY_TIME;
char* milText = "12 HR ";
short ratio = FAST_CLOCK_RATIO;
byte storedHH, storedMM, storedByte;


// Setup LED
TM1637Display LED_Display(LED_CLK, LED_DIO);

// Define the LCD menu system
LiquidCrystal_I2C lcd(0x27, 16, 2);

//Save dynamic memory by defining strings as Progmem
const char road_Name[] PROGMEM = ROAD_NAME;
const char welcome_text1[] PROGMEM = "Arduino Fast Clock";
const char welcome_text2[] PROGMEM = "V 1.0 - 4/5/2018";
const char welcome_text3[] PROGMEM = "By Futski";

// Define the LCD screen definitions
LiquidLine welcome_line1(1, 0, welcome_text1);
LiquidLine welcome_line2(2, 1, welcome_text2);
LiquidLine welcome_line3(5, 3, welcome_text3);
LiquidScreen welcome_screen(welcome_line1, welcome_line2, welcome_line3);

LiquidLine main1(4, 0, road_Name);
LiquidLine main2(0, 1, "Fast ", "Time ", pfcText);
LiquidLine main3(0, 2, "Real ", "Time ", prcText);
LiquidLine main_opt_pause(3, 3, pausedText);
LiquidLine main_opt_menu(12, 3, "Menu");
LiquidScreen main_screen(main1, main2, main3, main_opt_pause);

LiquidLine menu_opt_ratio(1, 0, "Clock ", "Ratio ", ratio, ":1");
LiquidLine menu_opt_set_clock_menu(1, 1, "Set ", "Clock ", ">");
LiquidLine menu_opt_restart(1, 2, "Restart ", "Clock ");
LiquidLine menu_opt_back(1, 3, "BACK");
LiquidScreen menu1(menu_opt_ratio, menu_opt_set_clock_menu, menu_opt_restart, menu_opt_back);

LiquidLine menu_opt_set_fc(1, 0, "Set ","Fast ", "Clock ", ">");
LiquidLine menu_opt_set_rc(1, 1, "Set ", "Real ", "Clock ", ">");
LiquidLine menu_opt_set_format(1, 2, "Time ", "Format ", milText);
LiquidScreen menu2(menu_opt_set_fc, menu_opt_set_rc, menu_opt_set_format, menu_opt_back);

LiquidLine menu_opt_set_clock_header(1, 0, "Set ", "Fast ", "Clock ");
LiquidLine menu_opt_set_clock(1, 1, ptcText, "  ", timePartText);
LiquidLine menu_opt_set_clock_save(1, 3, "Save");
LiquidLine menu_opt_set_clock_canc(8, 3, "Canc");
LiquidScreen menuSetClock(menu_opt_set_clock_header, menu_opt_set_clock, menu_opt_set_clock_save, menu_opt_set_clock_canc);

LiquidMenu menu(lcd);

uint8_t myCursor[8] = {
	0b00000,
	0b00100,
	0b00010,
	0b11111,
	0b00010,
	0b00100,
	0b00000,
	0b00000
};

bool updateLcdScreen = false;

uint8_t whichClock = 0;
enum WhichClock {
	eFastClock = 1,
	eRealClock = 2,
};

uint8_t menuState= 0;
enum MenuState {
	New = 0,
	Done = 0,
	InMenu = 1,
	SetHH = 1,
	SetMM =2,
};

enum UserAction {
	ButtonPushed = 1,
	RotateUp = 2,
	RotateDown = 3,
};

enum RotaryResponse {
	Up = true,
	Down = false,
};

enum SaveDataAddress {
	SaveMilMode = 0x07,
	SaveMM = 0x08,
	SaveHH = 0x09,
	SaveRatio = 0x0A,
};
// Interrupt Service Routine for a change to encoder pin A
void isr()
{
	if (digitalRead(ROTARY_ENCODE_A)) 
		// When A goes High - Used when isr is triggered by CHANGE or RISING 
		rotaryDirection = digitalRead(ROTARY_ENCODE_B);
	else 
		//  When A goes low - Used when isr is triggered by CHANGE or FALLING
		rotaryDirection = !digitalRead(ROTARY_ENCODE_B);
	
	rotaryFired = true;
}

void setup()
   {

	Serial.begin(115200);

	// Setup Rotary Encoder
	{
		pinMode(ROTARY_ENCODE_A, INPUT_PULLUP);     // enable pull-ups
		pinMode(ROTARY_ENCODE_B, INPUT_PULLUP);
		pinMode(ROTARY_ENCODE_SW, INPUT_PULLUP);
		attachInterrupt(digitalPinToInterrupt(ROTARY_ENCODE_A), isr, FALLING);   // interrupt 0 is pin 2

		pushButton1.attach(ROTARY_ENCODE_SW);
		pushButton1.interval(DEBOUNCEINTERVAL); // interval in ms
	}

	// Setup LCD screen 
	{
		lcd.init();
		lcd.backlight();
		// Get LCD started
		lcd.begin(20, 4);   // initialize the lcd for 20 chars 4 lines, turn on backlight
							//  lcd.blink();      // When blink is on, cursor will show regardless of noCursor setting
//		lcd.noCursor();     // Make sure display Cursor is Off
		lcd.noBlink();      // Make sure Cursor isn't blinking
	}

	// Finish setting up menu system
	{
		welcome_line1.set_asProgmem(1);
		welcome_line2.set_asProgmem(1);
		welcome_line3.set_asProgmem(1);
		main1.set_asProgmem(1);

		// Need to add 5th line to some screens
		main_screen.add_line(main_opt_menu);

		// Set custom cursor positions for some menu options
		main_opt_pause.set_focusPosition(Position::CUSTOM, 2, 4);
		menu_opt_set_clock_canc.set_focusPosition(Position::CUSTOM, 7, 3);

		// Add function calls to selectable menu items.

		//	Set actions for main screen
		main_opt_pause.attach_function(ButtonPushed, TogglePause);
		main_opt_pause.attach_function(RotateUp, OptNext);
		main_opt_pause.attach_function(RotateDown, noop);

		main_opt_menu.attach_function(ButtonPushed, opt_menu1);
		main_opt_menu.attach_function(RotateUp, noop);
		main_opt_menu.attach_function(RotateDown, OptPrev);

		//	Set Menu 1 Actions
		menu_opt_ratio.attach_function(ButtonPushed, ChangeRatioClick);
		menu_opt_ratio.attach_function(RotateUp, ChangeRatioRotateUp);
		menu_opt_ratio.attach_function(RotateDown, ChangeRatioRotateDown);

		menu_opt_restart.attach_function(ButtonPushed, InitialSetClocks);
		menu_opt_restart.attach_function(RotateUp, OptNext);
		menu_opt_restart.attach_function(RotateDown, OptPrev);

		menu_opt_set_clock_menu.attach_function(ButtonPushed, opt_menu2);
		menu_opt_set_clock_menu.attach_function(RotateUp, OptNext);
		menu_opt_set_clock_menu.attach_function(RotateDown, OptPrev);

		menu_opt_back.attach_function(ButtonPushed, opt_back);
		menu_opt_back.attach_function(RotateUp, noop);
		menu_opt_back.attach_function(RotateDown, OptPrev);

		//	Set Menu 2 Actions
		menu_opt_set_fc.attach_function(ButtonPushed, SetFastClockClick);
		menu_opt_set_fc.attach_function(RotateUp, OptNext);
		menu_opt_set_fc.attach_function(RotateDown, noop);

		menu_opt_set_rc.attach_function(ButtonPushed, SetRealClockClick);
		menu_opt_set_rc.attach_function(RotateUp, OptNext);
		menu_opt_set_rc.attach_function(RotateDown, OptPrev);

		menu_opt_set_format.attach_function(ButtonPushed, ToggleMilTime);
		menu_opt_set_format.attach_function(RotateUp, OptNext);
		menu_opt_set_format.attach_function(RotateDown, OptPrev);

		//  Set Clock screen actions
		menu_opt_set_clock.attach_function(ButtonPushed, ChangeTimeClick);
		menu_opt_set_clock.attach_function(RotateUp, ChangeTimeRotateUp);
		menu_opt_set_clock.attach_function(RotateDown, ChangeTimeRotateDown);

		menu_opt_set_clock_save.attach_function(ButtonPushed, SaveClick);
		menu_opt_set_clock_save.attach_function(RotateUp, OptNext);
		menu_opt_set_clock_save.attach_function(RotateDown, OptPrev);

		menu_opt_set_clock_canc.attach_function(ButtonPushed, CancClick);
		menu_opt_set_clock_canc.attach_function(RotateUp, noop);
		menu_opt_set_clock_canc.attach_function(RotateDown, OptPrev);

		// Add screens to menu
		menu.add_screen(welcome_screen);
		menu.add_screen(main_screen);
		menu.add_screen(menu1);
		menu.add_screen(menu2);
		menu.add_screen(menuSetClock);
		menu.set_focusPosition(Position::LEFT);
		menu.set_focusSymbol(Position::LEFT, myCursor);
		menu.update();
	}
	delay(WELCOME_DELAY);		// Show welcome screen for 5 seconds

	// Get Real Time clock Time 
	setSyncProvider(RTC.get);   // the function to get the time from the RTC
    #ifdef DEBUG_MODE
	if (timeStatus() != timeSet)
		Serial.println(F("Unable to sync with the RTC"));

	else {
		Serial.println(F("RTC has set the system time"));
	}
    #endif // DEBUG_MODE

	// Get any saved ratio - Override default if ration is not zero
	storedByte = RTC.readRTC(SaveRatio);
	if (storedByte >= 0 && storedByte <= 60) {
		ratio = storedByte;
	}

	storedByte = RTC.readRTC(SaveMilMode);
	if (storedByte == 0 || storedByte == 1) {
		MilitaryTime = storedByte;
	}

	// This block sets the real and fast clocks
	{
		InitialSetClocks();
		
/*		//  Get time from the Real Time Clock
		if (timeStatus() != timeSet) {	// Check if RTC set the real clock
			realClock = 1;				// When RTC fails, Set real clock time to midnight 1/1/1970
			setTime(realClock);
		}
		realClock = now();
		prevRealClock = realClock;

		//  Set default start time for fastClock
		timeParts.Hour = DEFAULTHH;
		timeParts.Minute = DEFAULTMM;
		timeParts.Second = 0;
		timeParts.Wday = weekday();			// Set Day of week, sunday is day 1 - Default to Real Clock
		timeParts.Day = day();				// Set Day of month - Default to Real Clock
		timeParts.Month = month();			// Set Month - Default to Real Clock
		timeParts.Year = year() - 1970;		// Set year offset from 1970 - Default to Real Clock
		fastClock = makeTime(timeParts);	//Create FastClock storage
											//fastClock = 68400UL;				//Set starting fasttime
		*/
		#ifdef DEBUG_MODE
		Serial.print(F("Real="));
		Serial.print(hour());
		Serial.print(F(":"));
		Serial.print(minute());
		Serial.print(F(":"));
		Serial.print(second());
		Serial.print(F(":"));
		Serial.print(month());
		Serial.print(F("/"));
		Serial.print(day());
		Serial.print(F("/"));
		Serial.println(year());

		Serial.print(F("Fast="));
		Serial.print(hour(fastClock));
		Serial.print(F(":"));
		Serial.print(minute(fastClock));
		Serial.print(F(":"));
		Serial.print(second(fastClock));
		Serial.print(F(":"));
		Serial.print(month(fastClock));
		Serial.print(F("/"));
		Serial.print(day(fastClock));
		Serial.print(F("/"));
		Serial.println(year(fastClock));
		#endif // DEBUG_MODE

		FormatTimeText(rcText, realClock);
		FormatTimeText(fcText, fastClock);
		prcText = (char*)rcText;
		pfcText = (char*)fcText;

		#ifdef DEBUG_MODE
		Serial.print("rcText: ");
		Serial.println(rcText);
		Serial.print("fcText: ");
		Serial.println(fcText);
		#endif // DEBUG_mode
	}

	// Initial LED screen
	{
		LED_Display.setBrightness(0x04, true);
		//	int x = 1234;
		//	LED_Display.showNumberDecEx(x, 4, true);
		int iTime = 0000;
		LED_Display.showNumberDecEx(iTime, iTime, true);
		uint8_t segto;
		segto = 0x80 | LED_Display.encodeDigit((iTime / 100) % 10);
		LED_Display.setSegments(&segto, 1, 1);
	}
	
	// Go to the Main Screen
	SetPause(INITIAL_PAUSE_STATE);
	menu.next_screen();
	menu.switch_focus();
	//	menu.update();

}

void loop() {
	// Don't waste unnecessary time checking the clock.  
	// So check to see if it's time to check the clock 
	if (millis() - prev_millis > clock_check_wait_time) {
		prev_millis = millis();
		if (UpdateFastClock()) {
			// If the fast clock was updated, then check to see if the minute changed 
			// on either the real clock or the fast clock
			if (minute(fastClock) != prev_FcMM || minute(realClock) != prev_RcMM) {
				prev_FcMM = minute(fastClock);
				prev_RcMM = minute(realClock);

				FormatTimeText(rcText, realClock);  
				FormatTimeText(fcText, fastClock);
				prcText = (char*)rcText;
				pfcText = (char*)fcText;

				updateLcdScreen = true;
				DisplayLED(fastClock);
			}
		}
	}
	
	pushButton1.update();					// Update Rotary Encoder Switch status
	if (pushButton1.fell()) {
		menu.call_function(1);
	}

	if (rotaryFired) {
		if (rotaryDirection == Up)
			menu.call_function(2);
		else
			menu.call_function(3);
		rotaryFired = false;

	}  // end if fired

	if (updateLcdScreen) {
		menu.softUpdate();
		updateLcdScreen = false;
	}
}

void InitialSetClocks() {
	//  Get time from the Real Time Clock
	if (timeStatus() != timeSet) {	// Check if RTC set the real clock
		realClock = 1;				// When RTC fails, Set real clock time to midnight 1/1/1970
		setTime(realClock);
	}
	realClock = now();
	prevRealClock = realClock;

	//  Set default start time for fastClock
	storedHH = RTC.readRTC(SaveHH);
	storedMM = RTC.readRTC(SaveMM);
	#ifdef DEBUG_MODE
	Serial.print("Sto");
	Serial.print(storedHH);
	Serial.print(":");
	Serial.print(storedMM);
	#endif // DEBUG_mode
	if (storedHH >= 0 && storedHH <= 23 && storedMM >= 0 && storedMM <= 59) {
		timeParts.Hour = storedHH;
		timeParts.Minute = storedMM;
	}
	else {
		timeParts.Hour = DEFAULTHH;
		timeParts.Minute = DEFAULTMM;
	}
	timeParts.Second = 0;
	timeParts.Wday = weekday();			// Set Day of week, sunday is day 1 - Default to Real Clock
	timeParts.Day = day();				// Set Day of month - Default to Real Clock
	timeParts.Month = month();			// Set Month - Default to Real Clock
	timeParts.Year = year() - 1970;		// Set year offset from 1970 - Default to Real Clock
	fastClock = makeTime(timeParts);	//Create FastClock storage
										//fastClock = 68400UL;				//Set starting fasttime
	SetPause(true);

};

bool UpdateFastClock() {
	realClock = now();					// Get current Real Time
	if (realClock != prevRealClock) {	// If Real Time is not = Prev real time, then at least one second
										// has passed, so it's time to update the fast clock. 
		// Figure out  exactly how many seconds have passed since the last update.  Then multiply 
		// those seconds by the FastClock ratio and add those additional seconds to the fast clock
		if (!fcPaused) {
			fastClock = fastClock + ((realClock - prevRealClock) * ratio);
		}

		prevRealClock = realClock;		// Save the time as PrevRealTime  
		return true;					// Return true to indicate that the fast clock time has bee updated
	}
	return false;						// Fast time was not updated so return false
}

void FormatTimeText(char myTimeText[], time_t myTime) {
	if (MilitaryTime) 
		sprintf(myTimeText, "%02d:%02d", hour(myTime), minute(myTime));
	else
		sprintf(myTimeText, "%2d:%02d %s", hourFormat12(myTime), minute(myTime), (isAM(myTime) ? "AM" : "PM"));
}

void DisplayLED(time_t myTime) {

	int8_t hh = 0;
	if (MilitaryTime)
		hh = hour(myTime);
	else
		hh = hourFormat12(myTime);

	int16_t dispTime = (hh * 100) + minute(myTime);    // Put time in 4 digit format
	LED_Display.showNumberDecEx(dispTime, myTime, (MilitaryTime ? true : false));  //Display the digits on the LED
																				 //Compute and display the colon.
																				 //	uint8_t segto;
	uint8_t segto = 0x80 | LED_Display.encodeDigit((dispTime / 100) % 10);
	LED_Display.setSegments(&segto, 1, 1);

}

bool TogglePause() {
	return SetPause(!fcPaused);
}

bool SetPause(bool myPause) {
	fcPaused = myPause;
	if (fcPaused)
		pausedText = "Paused";
	else
		pausedText = "Running";
	return fcPaused;
}

bool ToggleMilTime() {
	return SetMilTime(!MilitaryTime);
}

bool SetMilTime(bool myMiltime) {
	MilitaryTime = myMiltime;
	RTC.writeRTC(SaveMilMode, MilitaryTime);
	SetMilText();
	menu.softUpdate();
	return MilitaryTime;
}

void SetMilText() {
	if (MilitaryTime)
		milText = "24 HR";
	else
		milText = "12 HR";
}

void noop() {
	return;
}

void OptNext() {
	menu.switch_focus(true);
	return;
}

void OptPrev() {
	menu.switch_focus(false);
	return;
}

void opt_back() {
	menu.switch_focus(true);
	menu.previous_screen();
	FormatTimeText(rcText, realClock);
	FormatTimeText(fcText, fastClock);
	prcText = (char*)rcText;
	pfcText = (char*)fcText;
	updateLcdScreen = true;
	return;
}

void opt_menu1() {
	menu.change_screen(menu1);
	menu.switch_focus(true);
	return;
}

void opt_menu2() {
	SetMilText();
	menu.change_screen(menu2);
	menu.switch_focus(true);
	return;
}

void ChangeRatioClick() {
	switch (menuState) {
	case New:
		menuState = InMenu;
		break;
	case InMenu:
		menuState = Done;
		break;
	default:
		break;
	}
	return;
}

void ChangeRatioRotateUp() {
	switch (menuState) {
	case New:
		OptNext();
		return;
	case InMenu:
		ratio++;
		RTC.writeRTC(SaveRatio, ratio);
		menu.softUpdate();
		return;
	}
}

void ChangeRatioRotateDown() {
	switch (menuState) {
	case New:
		noop();  // Top entry on screen, so don't go anywhere
		return;
	case InMenu:
		ratio--;
		RTC.writeRTC(SaveRatio, ratio);
		menu.softUpdate();
		return;
	}
}

void SetFastClockClick() {
	whichClock = eFastClock;
	tempClock = fastClock;
	FormatTimeText(tcText, tempClock);
	ptcText = (char*)tcText;
	timePartText = "";
	menuState = New;
	menu.change_screen(menuSetClock);
	menu.switch_focus(true);
}

void SetRealClockClick() {
	whichClock = eRealClock;
	tempClock = realClock;
	FormatTimeText(tcText, tempClock);
	ptcText = (char*)tcText;
	timePartText = "";
	menuState = New;
	menu.change_screen(menuSetClock);
	menu.switch_focus(true);
}

void ChangeTimeClick(){

	//  Need to Click on Date to start setting, otherwise you can scroll past it
	switch (menuState) {
	case New:
		menuState = SetHH;
		timePartText = "Set HH";
		return;
	case SetHH:
		menuState = SetMM;
		timePartText = "Set MM";
		return;
	case SetMM:
		timePartText = "";
		menu.switch_focus(true);   //When Clicking on on Minute, Go to Save Option
		menuState = Done;
		return;
	}

}

void ChangeTimeRotateUp() {
	switch (menuState) {
	case New:
		menu.switch_focus(true);   //When Clicking on on Minute, Go to Save Option
		return;
	case SetHH:	// Increase 1 Hour
		tempClock += 3600;
		FormatTimeText(tcText, tempClock);
		ptcText = (char*)tcText;
		menu.softUpdate();
		return;
	case SetMM:	// Increase 1 minute
		tempClock += 60;
		FormatTimeText(tcText, tempClock);
		ptcText = (char*)tcText;
//		menu.softUpdate();
		return;
	}
}

void ChangeTimeRotateDown() {
	switch (menuState) {
	case New:
		noop();   //Cant scroll up
		return;
	case SetHH:
		tempClock -= 3600;	// Decease  1 hour
		FormatTimeText(tcText, tempClock);
		ptcText = (char*)tcText;
		menu.softUpdate();
		return;
	case SetMM:
		tempClock -= 60;	// Decease  1 minute
		FormatTimeText(tcText, tempClock);
		ptcText = (char*)tcText;
		menu.softUpdate();
		return;
	}

}

void SaveClick() {

	if (whichClock == eFastClock) {
		fastClock = tempClock; 
		SetPause(true);
		storedHH = hour(fastClock);
		storedMM = minute(fastClock);
		RTC.writeRTC(SaveHH, storedHH);
		RTC.writeRTC(SaveMM, storedMM);
	}
	else {
		realClock = tempClock;
		prevRealClock = realClock;
		RTC.set(realClock);
		setTime(realClock);
	}

	FormatTimeText(rcText, realClock);
	FormatTimeText(fcText, fastClock);
	prcText = (char*)rcText;
	pfcText = (char*)fcText;
	DisplayLED(fastClock);

	menuState = Done;
	menu.switch_focus(false);
	menu.switch_focus(false);
	menu.previous_screen();
}

void CancClick() {
	menu.switch_focus(true);
	menu.previous_screen();
	menuState = Done;
}

