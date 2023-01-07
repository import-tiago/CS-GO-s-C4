#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <toneAC.h>

uint8_t buzzer_volume = 10;
#define LED_RED_PIN 8
#define LCD_BACKLIGHT_PWM_PIN 11

#define C4_SHELL1_PIN A0
#define C4_SHELL2_PIN A1
#define C4_SHELL3_PIN A2

#define C4_SHELL_DISCONNECTED_AD_VALUE_MAX 20
#define SPEED_UP_START_TIME_SECS 30

int8_t hours = 0;
int8_t minutes = 0;
int8_t seconds = 0;

bool speedrun = false;
bool countdown_running = false;
bool countdown_finished = false;
uint8_t c4_shell_disconnection_detected = 0;

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


void reset_system() {
	asm volatile ("jmp 0");
}

void alert_beeps() {
	for (uint8_t i = 0; i < 3; i++) {
		toneAC(5000, 10, 70, false);
		delay(70);
	}
}

void tick_beep() {
	toneAC(7000, buzzer_volume, 50, true);
	static bool state = true;
	state = !state;
	digitalWrite(LED_RED_PIN, state);
}

void terrorists_win() {
	LCD.noBlink();
	LCD.clear();
	LCD.home();
	LCD.setCursor(0, 0);
	LCD.print("TERRORISTS WIN!!");

	countdown_running = false;
	countdown_finished = true;

	while (1) {
		toneAC(7000, 10, 50, false);
		static bool state = true;
		state = !state;
		digitalWrite(LED_RED_PIN, state);
		delay(50);
	}
}

void monitor_c4_shell_disconnection() {

	static bool shell1_disconnected = false;
	static bool shell2_disconnected = false;
	static bool shell3_disconnected = false;
	uint32_t secs_remain = (hours * 3600) + (minutes * 60) + seconds;

	if (!shell1_disconnected && (analogRead(C4_SHELL1_PIN) < C4_SHELL_DISCONNECTED_AD_VALUE_MAX)) {
		shell1_disconnected = true;
		alert_beeps();
		c4_shell_disconnection_detected++;
		if (c4_shell_disconnection_detected == 1) {
			hours = 0;
			minutes = 0;
			if (secs_remain > SPEED_UP_START_TIME_SECS)
				seconds = SPEED_UP_START_TIME_SECS;
		}
		else if (c4_shell_disconnection_detected == 2)
			terrorists_win();
	}

	if (!shell2_disconnected && (analogRead(C4_SHELL2_PIN) < C4_SHELL_DISCONNECTED_AD_VALUE_MAX)) {
		shell2_disconnected = true;
		alert_beeps();
		c4_shell_disconnection_detected++;
		if (c4_shell_disconnection_detected == 1) {
			hours = 0;
			minutes = 0;
			if (secs_remain > SPEED_UP_START_TIME_SECS)
				seconds = SPEED_UP_START_TIME_SECS;
		}
		else if (c4_shell_disconnection_detected == 2)
			terrorists_win();
	}

	if (!shell3_disconnected && (analogRead(C4_SHELL3_PIN) < C4_SHELL_DISCONNECTED_AD_VALUE_MAX)) {
		shell3_disconnected = true;
		alert_beeps();
		c4_shell_disconnection_detected++;
		if (c4_shell_disconnection_detected == 1) {
			hours = 0;
			minutes = 0;
			if (secs_remain > SPEED_UP_START_TIME_SECS)
				seconds = SPEED_UP_START_TIME_SECS;
		}
		else if (c4_shell_disconnection_detected == 2)
			terrorists_win();
	}
}

void lcd_low_power(int16_t timeout_mins) {
	int16_t seconds_remaining = (hours * 3600) + (minutes * 60) + seconds;
	static int16_t last_remaining = seconds_remaining;

	if ((last_remaining - seconds_remaining) > (timeout_mins * 60)) {
		analogWrite(LCD_BACKLIGHT_PWM_PIN, 10);
		last_remaining = seconds_remaining;
		Serial.println(1);
	}

	char key = KEYPAD.getKey();

	if (key != NO_KEY) {
		Serial.println(key);
		key = NO_KEY;
		analogWrite(LCD_BACKLIGHT_PWM_PIN, 255);
		last_remaining = seconds_remaining;
	}
}

void display_print_current_countdown() {
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
}

void countdown_speedrun_monitor() {
	static const uint32_t NORMAL_INTERVAL_MS = 1000;
	static const uint32_t INITIAL_SPEED_INTERVAL_MS = NORMAL_INTERVAL_MS;
	static const uint32_t FINAL_SPEED_INTERVAL_MS = 50;
	static uint32_t interval_ms = INITIAL_SPEED_INTERVAL_MS;

	if (!hours && !minutes && (seconds < SPEED_UP_START_TIME_SECS)) {

		speedrun = true;
		static uint32_t t0 = millis();

		if ((millis() - t0) >= interval_ms) {
			if (interval_ms > (FINAL_SPEED_INTERVAL_MS * 2))
				interval_ms -= (INITIAL_SPEED_INTERVAL_MS / (SPEED_UP_START_TIME_SECS / 2));

			toneAC(7000, buzzer_volume, 50, false);
			t0 = millis();
			seconds--;
			tick_beep();

			if (!seconds)
				terrorists_win();
		}
	}
}

void countdown_calcs() {
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

void mute_buzzer() {

	static uint32_t t0;
	KeyState state = KEYPAD.getState();

	if (state == PRESSED)
		t0 = millis();

	else if (state == HOLD) {

		if ((millis() - t0) > 2000) {

			alert_beeps();

			if (buzzer_volume == 10)
				buzzer_volume = 0;

			else if (buzzer_volume == 0)
				buzzer_volume = 10;
		}
	}
}

void setup() {

	pinMode(C4_SHELL1_PIN, INPUT);
	pinMode(C4_SHELL2_PIN, INPUT);
	pinMode(C4_SHELL3_PIN, INPUT);

	pinMode(LED_RED_PIN, OUTPUT);
	digitalWrite(LED_RED_PIN, LOW);

	pinMode(LCD_BACKLIGHT_PWM_PIN, OUTPUT);
	analogWrite(LCD_BACKLIGHT_PWM_PIN, 255);

	Serial.begin(115200);

	LCD.init();
	LCD.backlight();

	alert_beeps();

	LCD.clear();
	LCD.setCursor(0, 0);

	LCD.setCursor(0, 0);

	LCD.print(" SET COUNTDOWN: ");
	LCD.setCursor(0, 1);
	LCD.print("    hh:mm:ss    ");

	uint8_t step = 4;

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
			case 4: {
					char input = KEYPAD.getKey();
					LCD.setCursor(step, 1);
					LCD.blink();
					if (input >= '0' && input <= '9') {
						hour_ten = input - '0';
						toneAC(5000, buzzer_volume, 100, false);
						LCD.print(input);
						step++;
					}
					break;
				}

			case 5: {
					char input = KEYPAD.getKey();
					LCD.setCursor(step, 1);
					LCD.blink();
					if (input >= '0' && input <= '9') {
						hour_one = input - '0';
						toneAC(5000, buzzer_volume, 100, false);
						LCD.print(input);
						step++;
					}
					else if (input == '*') {
						toneAC(5000, buzzer_volume, 100, false);
						LCD.setCursor(step - 1, 1);
						LCD.print(" ");
						step--;
					}
					break;
				}

			case 6: {
					char input = KEYPAD.getKey();
					LCD.setCursor(step + 1, 1);
					LCD.blink();
					if (input >= '0' && input <= '9') {
						min_ten = input - '0';
						toneAC(5000, buzzer_volume, 100, false);
						LCD.print(input);
						step++;
					}
					else if (input == '*') {
						toneAC(5000, buzzer_volume, 100, false);
						LCD.setCursor(step - 1, 1);
						LCD.print(" ");
						step--;
					}
					break;
				}
			case 7: {
					char input = KEYPAD.getKey();
					LCD.setCursor(step + 1, 1);
					LCD.blink();
					if (input >= '0' && input <= '9') {
						min_one = input - '0';
						toneAC(5000, buzzer_volume, 100, false);
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
						toneAC(5000, buzzer_volume, 100, false);
						LCD.setCursor(step, 1);
						LCD.print(" ");
						step--;
					}
					break;
				}

			case 8: {
					char input = KEYPAD.getKey();
					LCD.setCursor(step + 2, 1);
					LCD.blink();
					if (input >= '0' && input <= '9') {
						sec_ten = input - '0';
						toneAC(5000, buzzer_volume, 100, false);
						LCD.print(input);
						step++;
					}
					else if (input == '*') {
						toneAC(5000, buzzer_volume, 100, false);
						LCD.setCursor(step, 1);
						LCD.print(" ");
						step--;
					}
					break;
				}

			case 9: {
					char input = KEYPAD.getKey();
					LCD.setCursor(step + 2, 1);
					LCD.blink();
					if (input >= '0' && input <= '9') {
						sec_one = input - '0';
						toneAC(5000, buzzer_volume, 100, false);
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
						step++;
					}
					else if (input == '*') {
						toneAC(5000, buzzer_volume, 100, false);
						LCD.setCursor(step + 1, 1);
						LCD.print(" ");
						step--;
					}
					break;
				}

			case 10: {
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
					step++;
					digitalWrite(LED_RED_PIN, HIGH);
					break;
				}
			case 11: {
					char armkey = KEYPAD.getKey();

					if (armkey == '#') {
						alert_beeps();
						LCD.clear();
						step = 0;
						c4_shell_disconnection_detected = false;
						countdown_running = true;
					}
					break;
				}
			default:
				break;
		}
	} while (step != 0);
}

void loop() {

	lcd_low_power(5); // LCD low power mode after 5 mins

	monitor_c4_shell_disconnection();

	if (!speedrun) {
		static uint32_t	t0 = millis();
		if ((millis() - t0) >= 1000) {
			t0 = millis();
			seconds--;
			tick_beep();
		}
	}

	mute_buzzer();

	display_print_current_countdown();

	countdown_speedrun_monitor();

	countdown_calcs();
}