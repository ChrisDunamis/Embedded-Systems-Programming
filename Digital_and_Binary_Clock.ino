/* I used this method to set bits for LEDS, this will alllow me to manipulate number 8, to indiviidual bits. 
 * Each number has its binary representation:
 * 0B0001111111111000 << 3 
 * 0B11111111 << 3
 * 0B11111XX0
 * 0B00010101 << 3
 * 0B10101XX0
 *		21 1
 * 0B11111000 | 0B00000110 = 0B11111110
 * LSB1      8
 ************************************************************************************************************/
union
{
	struct
	{
		unsigned int bit0: 1;
		unsigned int bit1: 1; // LED 2 
		unsigned int bit2: 1; // LED 1 
		unsigned int bit3: 1; // H 
		unsigned int bit4: 1;  //H
		unsigned int bit5: 1; //H
		unsigned int bit6: 1; //H
		unsigned int bit7: 1; //H
	};

	unsigned int theTime;
} timeSS, timeMM, timeHH;


// Functions
void clockTimeDisplay(int);  // This will print two digits for HH:MM:SS 
void displayClock();		 // This will display the unixtime to LCD
void clockTime();
void setClockTime();
void stopClock();			 // This function how many times you presed the button and resets time clock
void startAlarm();			 // This will make the Alarm sound from the Piezo

const long interval = 1000;  // Interval at which to blink (milliseconds)
const int pinSRInput = 4;	 // INPUT (pin number 14 - Shift Register)
const int pinSRClock = 2;	 // SHIFT REGISTER CLOCK (pin number 11 - Shift Register)
const int pinSROutputRegisterClock = 3;  // OUTPUT REGISTER CLOCK (pin number 12 - Shift Register)
const int pushbuttonHH = 5;  // Pushbutton: Set Hours - HH
const int pushbuttonMM = 6;  // Pushbutton: Set Minutes - MM
const int pushbuttonSR = 7;  // Pushbutton: Start/Reset RT Clock

unsigned long int unixTime = 1588701600 + 55;  /* This is based on computers time. It always increments.
* Reference: https://www.epochconverter.com/
*
* This is based on computers time. It always increments. But because the Digital Clock starts from 00:00:00, 
* and can be reset, so we are getting from this variable, the Hours, Minutes and Seconds, and is incremented 
* every second and extracting time from it.
*
* Included the library code:
*************************************************************************************************************/


#include <LiquidCrystal.h>  // From the Arduino library

// Initialise the library by associating any needed LCD interface pin with the arduino pin number that it is
const int rs = 13, en = 12, d4 = 11, d5 = 10, d6 = 9, d7 = 8;  // connected to.

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


void setup()
{
	Serial.begin(9600);		// For debugging set up the LCD's number of columns and rows:

	lcd.begin(16, 2);		// Starts the LCD (16 columns and 2 rows)

	pinMode(pinSRInput, OUTPUT);
	pinMode(pinSRClock, OUTPUT);
	pinMode(pinSROutputRegisterClock, OUTPUT);
	pinMode(A0, OUTPUT);  // Analogue pin for the Piezo, which will act as an Alarm
	pinMode(pushbuttonHH, INPUT);
	pinMode(pushbuttonMM, INPUT);
	pinMode(pushbuttonSR, INPUT);

	lcd.setCursor(1, 0);  // This will adjust the text, at the top of the LCD.
	lcd.print("Digital Clock:");
}


unsigned long previousMillis = 0;  // will store last time LED was updated
bool clockStarted = false;


void loop()
{
	delay(1);

	if (clockStarted == true)
	{
		stopClock();
		startAlarm();
		clockTime();
	}
	else
	{
		setClockTime();
		blinkLED();
	}
}


unsigned long int previousMillis2;
bool blinkLEDS = false;


void blinkLED()
{
	unsigned long currentMillis = millis();

	if (currentMillis - previousMillis2 >= 1000)
	{  // DisplayedMilliSeconds = displayedMilliSeconds + 1000;
	   // Save the last time you blinked the LED.

		previousMillis2 = currentMillis;

		if (blinkLEDS == false)
			blinkLEDS = true;
		else
			blinkLEDS = false;

	// Serial.println(blinkLEDS);
	timeHH.bit2 = blinkLEDS;
    timeHH.bit1 = blinkLEDS;
	}
}


void clockTime()
{
	delay(1);  /* Tinkercad web hack, not needed in production, set the cursor to column 0, line 1.
	* Note: Line 1 is the second row, since counting begins with 0):
	*********************************************************************************************************/

	unsigned long currentMillis = millis();

	if (currentMillis - previousMillis >= interval)
	{
		// displayedMilliSeconds = displayedMilliSeconds + 1000;
		// save the last time you blinked the LED
		previousMillis = currentMillis;
		displayClock();
		shiftfn();
	}
}


int stopClockCounter = 0;
unsigned long previousMillis1 = 0;
int lastStateSC;


void stopClock()
{
	unsigned long currentMillis = millis();  // Reset stop button click count every 2 seconds

	if (currentMillis - previousMillis1 >= 2000)
	{
		previousMillis1 = currentMillis;
		stopClockCounter = 0;
	}

	int stateSC = digitalRead(pushbuttonSR);

	if (lastStateSC != stateSC)
	{
		lastStateSC = stateSC;

		if (stateSC != LOW)
		{
			stopClockCounter++;

			if (stopClockCounter > 1)
				clockStarted = false;
		}
	}  // Minute button is pressed or release
}


int lastStateH;
int lastStateM;
long int randomMinutes;
long int randomHours;


void setClockTime()
{
	delay(10);  // hack

	int stateH = digitalRead(pushbuttonHH);
	int stateM = digitalRead(pushbuttonMM);
	int stateSC = digitalRead(pushbuttonSR);

	if (lastStateH != stateH)
	{
		lastStateH = stateH;

		if(stateH != LOW)
		{
			randomHours++;

			if(randomHours > 23)
				randomHours = 0;
		}  // Hour button is pressed or released.
	}

	if (lastStateM != stateM)
	{
		lastStateM = stateM;

		if (stateM != LOW)
		{
			randomMinutes++;

			if (randomMinutes > 59)
				randomMinutes = 0;
		}  // Minute button is pressed or released.
	}

	if (lastStateSC != stateSC)
	{
		lastStateSC = stateSC;

		if (stateSC != LOW)
			// Serial.println("start");
			clockStarted = true;
	}  // Start clock button is pressed or released.

	unixTime = (randomHours * 60 * 60) + randomMinutes * 60;

	displayClock();
	shiftfn();
}


void displayClock()
{
	int currentSeconds = unixTime % 60;
	int currentMinutes = (unixTime % 3600) / 60;
	int currentHours = (unixTime % 86400) / 3600; 

	timeSS.theTime = currentSeconds;
	timeMM.theTime = currentMinutes;
	timeHH.theTime = currentHours << 3; 

	timeHH.bit2 = blinkLEDS;
	timeHH.bit1 = blinkLEDS;

	lcd.setCursor(4, 1);
	clockTimeDisplay(currentHours);
	lcd.print(":");
	clockTimeDisplay(currentMinutes);
	lcd.print(":");
	clockTimeDisplay(currentSeconds);

	unixTime++;
}


void clockTimeDisplay(int theTime)
{
	if (theTime < 10)		// This will allow display in two digits.
		lcd.print("0");		// This 'String' will display RTC, starting from '0'.

	lcd.print(theTime);
}


void shiftfn()
{
	digitalWrite(pinSROutputRegisterClock, LOW);  /* chip doenst have 8 and 7 pins connected so we shift 
	* the number to left by 2 bits, and we get 0b11111100 2 trailing zeros and then number.
	*********************************************************************************************************/

	shiftOut(pinSRInput, pinSRClock, LSBFIRST, timeSS.theTime << 2);  // 3rd chip: SS
	shiftOut(pinSRInput, pinSRClock, LSBFIRST, timeMM.theTime << 2);  // 2nd chip: MM
	shiftOut(pinSRInput, pinSRClock, LSBFIRST, timeHH.theTime);		  // 1st chip: HH

	digitalWrite(pinSROutputRegisterClock, HIGH);
}


int alarmCounts = 0;
bool soundAlarm = false;
unsigned long int previousMillis4 = 0;


void startAlarm()
{
	int currentSeconds = unixTime % 60;
	int currentMinutes = (unixTime % 3600) / 60;
	int currentHours = (unixTime % 86400) / 3600;

	if ((currentMinutes == 0) && (currentSeconds == 0) && (soundAlarm == false))
	{
		soundAlarm = true;
		alarmCounts = 0;
	}

	unsigned long currentMillis = millis();

	if ((currentMillis - previousMillis4) >= 500)
	{
		previousMillis4 = currentMillis;

		if (soundAlarm == true)
		{
			alarmCounts++;

			noTone(A0);
			tone(A0, 1000, 100);

			if (alarmCounts > 9)
				soundAlarm = false;
		}
	}
}
