#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Ticker.h>

#define LED_RED_PIN 8
#define BUZZER_PIN 9

#define C4_SHELL_1 A0
#define C4_SHELL_2 A1
#define C4_SHELL_3 A2

#define C4_SHELL_DISCONNECTED_AD_VALUE_MAX 20

#define SPEED_UP_START_TIME_SECS 30

int8_t hours = 0;
int8_t minutes = 0;
int8_t seconds = 0;

bool countdown_running = false;
bool countdown_finished = false;
bool c4_shell_disconnection_detected = false;

void IST_Timer() {

	static uint8_t ms_interval = 0;
	static bool state = true;

	ms_interval++;

	if (countdown_running) {

		static uint8_t _seconds = seconds;

		if (seconds != _seconds) {
			state = !state;
			digitalWrite(LED_RED_PIN, state);
			_seconds = seconds;
		}
	}

	else if (countdown_finished) {

		const static uint8_t INTERVAL_TARGET = 100;

		if (ms_interval >= INTERVAL_TARGET) {
			state = !state;
			digitalWrite(LED_RED_PIN, state);
			ms_interval = 0;
		}
	}

	if (
		(
			(analogRead(C4_SHELL_1) < C4_SHELL_DISCONNECTED_AD_VALUE_MAX) ||
			(analogRead(C4_SHELL_2) < C4_SHELL_DISCONNECTED_AD_VALUE_MAX) ||
			(analogRead(C4_SHELL_3) < C4_SHELL_DISCONNECTED_AD_VALUE_MAX)
			) && !c4_shell_disconnection_detected
		) {
		hours = 0;
		minutes = 0;
		seconds = SPEED_UP_START_TIME_SECS;
		c4_shell_disconnection_detected = true;
	}

}

#define KEYPAD_ROWS 4
#define  KEYPAD_COLUMNS 3

char keys[KEYPAD_ROWS][KEYPAD_COLUMNS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte rowPins[KEYPAD_ROWS] = { 0, 7, 2, 3 };
byte colPins[KEYPAD_COLUMNS] = { 4, 5, 6 };

Keypad KEYPAD = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLUMNS);
LiquidCrystal_I2C LCD(0x27, 16, 2);
Ticker ticker(IST_Timer, 1, 0, MILLIS);

void reset_system() {
	asm volatile ("jmp 0");
}

void setup() {

	pinMode(C4_SHELL_1, INPUT);
	pinMode(C4_SHELL_2, INPUT);
	pinMode(C4_SHELL_3, INPUT);

	pinMode(LED_RED_PIN, OUTPUT);
	digitalWrite(LED_RED_PIN, LOW);

	LCD.init();
	LCD.backlight();

	uint16_t beep = 70;
	uint16_t freq = 5000;
	tone(BUZZER_PIN, freq, beep);
	delay(beep);
	tone(BUZZER_PIN, 0, beep);
	delay(beep);
	tone(BUZZER_PIN, freq, beep);
	delay(beep);
	tone(BUZZER_PIN, 0, beep);

	LCD.clear();
	LCD.setCursor(0, 0);

	LCD.setCursor(0, 0);
	LCD.print("TIMER: HH:MM:SS");
	LCD.setCursor(0, 1);
	LCD.print("SET:   :  :");

	uint8_t step = 5;

	uint8_t hour_ten;
	uint8_t hour_one;

	uint8_t min_ten;
	uint8_t min_one;

	uint8_t sec_ten;
	uint8_t sec_one;

	char number_arr[3];
	uint8_t number = 0;

	do {
		switch (step) {
			case 5: {
					char input = KEYPAD.getKey();
					LCD.setCursor(5, 1);
					LCD.blink();
					if (input >= '0' && input <= '9') {
						hour_ten = input - '0';
						tone(BUZZER_PIN, 5000, 100);
						LCD.print(input);
						step++;
					}
					break;
				}

			case 6: {
					char input = KEYPAD.getKey();
					LCD.setCursor(6, 1);
					LCD.blink();
					if (input >= '0' && input <= '9') {
						hour_one = input - '0';
						tone(BUZZER_PIN, 5000, 100);
						LCD.print(input);
						step++;
					}
					else if (input == '*') {
						tone(BUZZER_PIN, 5000, 100);
						LCD.setCursor(5, 1);
						LCD.print(" ");
						step--;
					}
					break;
				}

			case 7: {
					char input = KEYPAD.getKey();
					LCD.setCursor(8, 1);
					LCD.blink();
					if (input >= '0' && input <= '9') {
						min_ten = input - '0';
						tone(BUZZER_PIN, 5000, 100);
						LCD.print(input);
						step++;
					}
					else if (input == '*') {
						tone(BUZZER_PIN, 5000, 100);
						LCD.setCursor(6, 1);
						LCD.print(" ");
						step--;
					}
					break;
				}
			case 8: {
					char input = KEYPAD.getKey();
					LCD.setCursor(9, 1);
					LCD.blink();
					if (input >= '0' && input <= '9') {
						min_one = input - '0';
						tone(BUZZER_PIN, 5000, 100);
						LCD.print(input);

						sprintf(number_arr, "%u%u", min_ten, min_one);
						number = atoi(number_arr);
						if (number >= 60) {
							LCD.blink();
							LCD.clear();
							LCD.setCursor(0, 0);
							LCD.print(" INVALID NUMBER ");
							LCD.setCursor(0, 1);
							LCD.print(" RESETTING ... ");
							delay(3000);
							reset_system();
						}
						step++;
					}
					else if (input == '*') {
						tone(BUZZER_PIN, 5000, 100);
						LCD.setCursor(8, 1);
						LCD.print(" ");
						step--;
					}
					break;
				}

			case 9: {
					char input = KEYPAD.getKey();
					LCD.setCursor(11, 1);
					LCD.blink();
					if (input >= '0' && input <= '9') {
						sec_ten = input - '0';
						tone(BUZZER_PIN, 5000, 100);
						LCD.print(input);
						step = 10;
					}
					else if (input == '*') {
						tone(BUZZER_PIN, 5000, 100);
						LCD.setCursor(9, 1);
						LCD.print(" ");
						step--;
					}
					break;
				}

			case 10: {
					char input = KEYPAD.getKey();
					LCD.setCursor(12, 1);
					LCD.blink();
					if (input >= '0' && input <= '9') {
						sec_one = input - '0';
						tone(BUZZER_PIN, 5000, 100);
						LCD.print(input);
						sprintf(number_arr, "%u%u", sec_ten, sec_one);
						number = atoi(number_arr);
						if (number >= 60) {
							LCD.blink();
							LCD.clear();
							LCD.setCursor(0, 0);
							LCD.print(" INVALID NUMBER ");
							LCD.setCursor(0, 1);
							LCD.print(" RESETTING ... ");
							delay(3000);
							reset_system();
						}
						step = 11;
					}
					else if (input == '*') {
						tone(BUZZER_PIN, 5000, 100);
						LCD.setCursor(11, 1);
						LCD.print(" ");
						step--;
					}
					break;
				}

			case 11: {
					hours = (hour_ten * 10) + hour_one;
					minutes = (min_ten * 10) + min_one;
					seconds = (sec_ten * 10) + sec_one;
					delay(200);
					LCD.noBlink();
					LCD.clear();

					LCD.setCursor(0, 0);
					LCD.print("   PRESS # TO   ");
					LCD.setCursor(0, 1);
					LCD.print("   ACTIVATE!!");
					delay(50);
					step = 12;
					digitalWrite(LED_RED_PIN, HIGH);
					ticker.start();
					countdown_running = true;
					break;
				}
			case 12: {
					char armkey = KEYPAD.getKey();

					if (armkey == '#') {
						tone(BUZZER_PIN, 5000, 100);
						delay(50);
						tone(BUZZER_PIN, 0, 100);
						delay(50);
						tone(BUZZER_PIN, 5000, 100);
						delay(50);
						tone(BUZZER_PIN, 0, 100);
						delay(50);
						tone(BUZZER_PIN, 5000, 100);
						delay(50);
						tone(BUZZER_PIN, 0, 100);
						LCD.clear();
						step = 0;
						c4_shell_disconnection_detected = false;
					}
					break;
				}
			default:
				break;
		}
	} while (step != 0);
}

void loop() {

	ticker.update();

	static bool c4_shell_disconnection_beep = false;

	if (c4_shell_disconnection_detected && !c4_shell_disconnection_beep) {

		c4_shell_disconnection_beep = true;

		tone(BUZZER_PIN, 5000, 100);
		delay(50);
		tone(BUZZER_PIN, 0, 100);
		delay(50);
		tone(BUZZER_PIN, 5000, 100);
		delay(50);
		tone(BUZZER_PIN, 0, 100);
		delay(50);
		tone(BUZZER_PIN, 5000, 100);
		delay(50);
		tone(BUZZER_PIN, 0, 100);
	}

	LCD.setCursor(0, 0);
	LCD.print("   COUNTDOWN:   ");

	if (hours >= 10) {
		LCD.setCursor(4, 1);
		LCD.print(hours);
	}
	if (hours < 10) {
		LCD.setCursor(4, 1);
		LCD.write('0');
		LCD.setCursor(5, 1);
		LCD.print(hours);
	}
	LCD.print(':');

	if (minutes >= 10) {
		LCD.setCursor(7, 1);
		LCD.print(minutes);
	}
	if (minutes < 10) {
		LCD.setCursor(7, 1);
		LCD.write('0');
		LCD.setCursor(8, 1);
		LCD.print(minutes);
	}
	LCD.print(':');

	if (seconds >= 10) {
		LCD.setCursor(10, 1);
		LCD.print(seconds);
	}

	if (seconds < 10) {
		LCD.setCursor(10, 1);
		LCD.write('0');
		LCD.setCursor(11, 1);
		LCD.print(seconds);
	}

	static const uint32_t NORMAL_INTERVAL_MS = 1000;
	static const uint32_t INITIAL_SPEED_INTERVAL_MS = NORMAL_INTERVAL_MS;
	static const uint32_t FINAL_SPEED_INTERVAL_MS = 50;

	static uint32_t interval_ms = INITIAL_SPEED_INTERVAL_MS;

	if (minutes || hours) {

		static uint32_t t0 = millis();

		if ((millis() - t0) >= 1000) {
			tone(BUZZER_PIN, 7000, 50);
			t0 = millis();
			seconds--;
		}
	}
	else if ((seconds < SPEED_UP_START_TIME_SECS) && seconds > 0) {

		static uint32_t t0 = millis();

		if ((millis() - t0) >= interval_ms) {

			if (interval_ms > (FINAL_SPEED_INTERVAL_MS * 2))
				interval_ms -= (INITIAL_SPEED_INTERVAL_MS / (SPEED_UP_START_TIME_SECS / 2));

			tone(BUZZER_PIN, 7000, 50);
			t0 = millis();
			seconds--;
		}
	}
	else if (seconds > 0) {

		static uint32_t t0 = millis();

		if ((millis() - t0) >= interval_ms) {

			tone(BUZZER_PIN, 7000, 50);
			t0 = millis();
			seconds--;
		}
	}
	else {

		LCD.noBlink();
		LCD.clear();
		LCD.home();
		LCD.print("  BOOOOOOOM!!  ");
		LCD.setCursor(0, 0);

		countdown_running = false;
		countdown_finished = true;

		while (1) {
			ticker.update();
			tone(BUZZER_PIN, 7000, 100);
			delay(100);

			tone(BUZZER_PIN, 7000, 100);
			delay(100);

			tone(BUZZER_PIN, 7000, 100);
			delay(100);

			tone(BUZZER_PIN, 7000, 100);
			delay(100);

			tone(BUZZER_PIN, 7000, 100);
			delay(100);

			tone(BUZZER_PIN, 7000, 100);
			delay(100);
		}
	}

	if (seconds < 1) {
		if (minutes > 0) {
			minutes--;
			seconds = 59;
		}
		else {
			if (hours > 0) {
				hours--;
				minutes = 59;
				seconds = 59;
			}
		}
	}
}