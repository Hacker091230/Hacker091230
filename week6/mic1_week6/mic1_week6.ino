/*
 * mic1_week6.ino
 * Настільний годинник з будильником. Час будильника зберігається в EEPROM.
 *
 * Name:           <Ваше ім'я>
 * Student number: <Ваш номер студента>
 * Date:           <дата>
 *
 * LCD 16x2 I2C: GND-GND, 5V-VCC, A4/SDA-SDA, A5/SCL-SCL
 *
 *   |hh:mm:ss    25°C|   <- поточний час + температура чипа
 *   |hh:mm:ss        |   <- час будильника (з EEPROM)
 *
 * - Після скидання час 00:00:00, щосекунди друкується в Serial (9600 baud).
 * - Коли час == будильник, підсвітка блимає 10 с з частотою 2 Гц.
 * - SW0: будильник = поточний час, новий будильник записується в EEPROM.
 */

#include <EEPROM.h>

// ===================================================================
// LCD. Уся робота з бібліотекою LCD 16x2 I2C зібрана в трьох функціях.
// lcd_set_cursor() і lcd_put() взяті з PDF завдання. Назву .h-файлу,
// функцію ініціалізації та функцію підсвітки звірте з прикладом
// File > Examples > LCD 16x2 I2C > hello_world і за потреби виправте.
// ===================================================================
#include <Wire.h>
#include <lcd_16x2_i2c.h>

void lcdInit() {
  lcd_init();
}

void lcdPrintAt(uint8_t col, uint8_t row, char *text) {
  lcd_set_cursor(col, row);
  lcd_put(text);
}

void lcdBacklight(bool on) {
  lcd_backlight(on);
}

// --- Піни ---
const int SW0_PIN = 21;       // SW0 на платі Xplained mini (PB7 = D21)

// --- Будильник ---
struct ClockTime {
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
};

const int EEPROM_MAGIC_ADDR = 0;        // байт-позначка "дані записані"
const uint8_t EEPROM_MAGIC  = 0xA5;
const int EEPROM_ALARM_ADDR = 1;        // тут лежить ClockTime (3 байти)
const ClockTime DEFAULT_ALARM = {0, 1, 23};   // 00:01:23

const unsigned long ALARM_DURATION_MS = 10000;  // блимати 10 с
const unsigned long BLINK_INTERVAL_MS = 250;    // 250 мс увімк. / 250 мс вимк.

// --- Змінні стану ---
ClockTime now   = {0, 0, 0};          // після скидання 00:00:00
ClockTime alarm;
unsigned long previousSecondMillis = 0;

bool alarmActive = false;
unsigned long alarmStartMillis = 0;
unsigned long previousBlinkMillis = 0;
bool backlightOn = true;

int lastSw0State = HIGH;               // SW0 не натиснута = HIGH (pullup R209)

void setup() {
  Serial.begin(9600);
  pinMode(SW0_PIN, INPUT);             // pullup і конденсатор уже є на платі

  lcdInit();
  lcdBacklight(true);

  loadAlarm();

  previousSecondMillis = millis();
  showTime();
  showTemperature();
  showAlarm();
  printTime();
}

void loop() {
  unsigned long currentMillis = millis();

  // ---------- 1. Годинник: +1 секунда кожні 1000 мс ----------
  if (currentMillis - previousSecondMillis >= 1000) {
    previousSecondMillis += 1000;      // += 1000, а не = millis(): без накопичення похибки
    tick();

    showTime();
    showTemperature();
    printTime();

    if (sameTime(now, alarm)) {        // настав час будильника
      alarmActive = true;
      alarmStartMillis = currentMillis;
      previousBlinkMillis = currentMillis;
    }
  }

  // ---------- 2. Будильник: блимання підсвітки 2 Гц протягом 10 с ----------
  if (alarmActive) {
    if (currentMillis - alarmStartMillis >= ALARM_DURATION_MS) {
      alarmActive = false;
      backlightOn = true;              // після будильника підсвітка світить
      lcdBacklight(true);
    } else if (currentMillis - previousBlinkMillis >= BLINK_INTERVAL_MS) {
      previousBlinkMillis = currentMillis;
      backlightOn = !backlightOn;
      lcdBacklight(backlightOn);
    }
  }

  // ---------- 3. SW0: новий час будильника ----------
  int sw0State = digitalRead(SW0_PIN);
  if (sw0State != lastSw0State) {
    if (sw0State == LOW) {             // HIGH -> LOW = натиснули
      alarm = now;
      EEPROM.put(EEPROM_ALARM_ADDR, alarm);
      showAlarm();
      Serial.println("Alarm saved");
    }
    lastSw0State = sw0State;
  }
}

// Додає одну секунду з переносом у хвилини й години (після 23:59:59 -> 00:00:00)
void tick() {
  now.seconds++;
  if (now.seconds >= 60) {
    now.seconds = 0;
    now.minutes++;
    if (now.minutes >= 60) {
      now.minutes = 0;
      now.hours++;
      if (now.hours >= 24) {
        now.hours = 0;
      }
    }
  }
}

bool sameTime(const ClockTime &a, const ClockTime &b) {
  return a.hours == b.hours && a.minutes == b.minutes && a.seconds == b.seconds;
}

/*
 * Читає будильник з EEPROM. Якщо там ще нічого не записано (немає позначки
 * або значення некоректні), записує будильник за замовчуванням 00:01:23.
 * Так будильник, встановлений кнопкою, не перезаписується після вимкнення
 * живлення.
 */
void loadAlarm() {
  EEPROM.get(EEPROM_ALARM_ADDR, alarm);

  bool valid = EEPROM.read(EEPROM_MAGIC_ADDR) == EEPROM_MAGIC &&
               alarm.hours < 24 && alarm.minutes < 60 && alarm.seconds < 60;
  if (!valid) {
    alarm = DEFAULT_ALARM;
    EEPROM.put(EEPROM_ALARM_ADDR, alarm);
    EEPROM.write(EEPROM_MAGIC_ADDR, EEPROM_MAGIC);
  }
}

// "hh:mm:ss" у буфер (мінімум 9 символів)
void formatTime(char *buffer, const ClockTime &t) {
  snprintf(buffer, 9, "%02d:%02d:%02d", t.hours, t.minutes, t.seconds);
}

void showTime() {
  char buffer[9];
  formatTime(buffer, now);
  lcdPrintAt(0, 0, buffer);
}

void showAlarm() {
  char buffer[9];
  formatTime(buffer, alarm);
  lcdPrintAt(0, 1, buffer);
}

// Температура праворуч у першому рядку: стовпці 11..15, наприклад " 25°C"
void showTemperature() {
  char buffer[6];
  snprintf(buffer, sizeof(buffer), "%3d%cC", readChipTemperature(), (char)0xDF);
  lcdPrintAt(11, 0, buffer);
}

void printTime() {
  char buffer[9];
  formatTime(buffer, now);
  Serial.println(buffer);
}

/*
 * Внутрішній датчик температури ATmega328P (канал АЦП 8, опорна напруга 1.1 В).
 * Якщо хочете використати бібліотеку Chip temperature, замініть тіло цієї
 * функції на виклик з прикладу print_temperature.
 * Формула з типової калібровки: T = (ADC - 324.31) / 1.22. Точність
 * приблизно +/-10 °C без індивідуальної калібровки.
 */
int readChipTemperature() {
  ADMUX = (1 << REFS1) | (1 << REFS0) | (1 << MUX3);   // 1.1 В, канал 8
  delay(2);                                            // опорна напруга встановлюється

  ADCSRA |= (1 << ADSC);                               // перше перетворення відкидаємо
  while (ADCSRA & (1 << ADSC)) {}

  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC)) {}

  return (int)((ADCW - 324.31) / 1.22);
}
