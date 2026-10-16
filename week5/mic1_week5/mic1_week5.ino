/*
 * mic1_week5.ino
 * Вимірювання відстані ультразвуковим датчиком HC-SR04 і відображення
 * її у вигляді стовпчикової діаграми на LCD 16x2 (I2C).
 *
 * Name:           <Ваше ім'я>
 * Student number: <Ваш номер студента>
 * Date:           <дата>
 *
 * Підключення:
 *   LCD:     GND-GND, 5V-VCC, A4/SDA-SDA, A5/SCL-SCL
 *   HC-SR04: GND-GND, 5V-VCC, D6-TRIG, D7-ECHO
 *
 * Рядок 1: "MIC1 Week 5"
 * Рядок 2: '#' за кожен сантиметр + '|' на максимумі, якого досягала
 *          відстань (1 символ = 1 см, максимум 16 символів).
 *          SW0 скидає максимум до поточної відстані.
 */

// ===================================================================
// LCD: ці три місця (include, об'єкт, ініціалізація в setup()) мають
// збігатися з прикладом File > Examples > LCD 16x2 I2C > hello_world.
// Якщо там інші назви, просто скопіюйте їх звідти.
// ===================================================================
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);   // адреса 0x27, 16 символів, 2 рядки

// --- Піни ---
const int TRIG_PIN = 6;       // D6 -> TRIG
const int ECHO_PIN = 7;       // D7 <- ECHO
const int SW0_PIN  = 21;      // SW0 на платі Xplained mini (PB7 = D21)

// --- Налаштування ---
const int LCD_COLUMNS = 16;
const unsigned long MEASURE_INTERVAL_MS = 100;  // HC-SR04: не частіше ~60 мс
const unsigned long ECHO_TIMEOUT_US = 30000;    // ~5 м; довше чекати немає сенсу
const long OUT_OF_RANGE_CM = 400;               // немає луни -> "далеко"

// --- Змінні стану ---
long distance_cm  = 0;        // поточна виміряна відстань
long distance_max = 0;        // найбільша відстань з моменту скидання
unsigned long previousMeasureMillis = 0;
int lastSw0State = HIGH;      // SW0 не натиснута = HIGH (pullup R209)

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);
  pinMode(ECHO_PIN, INPUT);
  pinMode(SW0_PIN, INPUT);    // pullup і конденсатор уже є на платі

  lcd.init();                 // ініціалізація LCD (див. hello_world)
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("MIC1 Week 5");
}

void loop() {
  unsigned long currentMillis = millis();

  // ---------- 1. SW0: скидання максимуму ----------
  int sw0State = digitalRead(SW0_PIN);
  if (sw0State != lastSw0State) {
    if (sw0State == LOW) {                 // HIGH -> LOW = натиснули
      distance_max = distance_cm;          // вершина = поточна відстань
      showBar();
    }
    lastSw0State = sw0State;
  }

  // ---------- 2. Вимірювання і вивід ----------
  if (currentMillis - previousMeasureMillis >= MEASURE_INTERVAL_MS) {
    previousMeasureMillis = currentMillis;

    distance_cm = measureDistanceCm();
    if (distance_cm > distance_max) {
      distance_max = distance_cm;          // вершина тільки росте
    }
    showBar();
  }
}

/*
 * Вимірює відстань у см.
 * TRIG: імпульс 10 мкс -> датчик посилає ультразвук.
 * ECHO: тримає HIGH стільки, скільки звук летів туди й назад.
 * Швидкість звуку 343 м/с = 0.0343 см/мкс, шлях подвійний:
 *   відстань = час * 0.0343 / 2 = час / 58 (см)
 */
long measureDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);
  if (duration == 0) {
    return OUT_OF_RANGE_CM;                // луни немає -> об'єкт далеко
  }
  return duration / 58;
}

/*
 * Будує другий рядок LCD.
 *   level = кількість '#' (1 '#' на кожен см), не більше 16
 *   top   = номер стовпця для '|' (0..15); при 0 або 1 см -> стовпець 0,
 *           при 16 см і більше -> стовпець 15
 * Для кожного стовпця: '|' на вершині, '#' нижче поточної відстані,
 * пробіл в іншому разі. Пробіли також стирають старий вміст рядка.
 */
void showBar() {
  long level = constrain(distance_cm, 0, LCD_COLUMNS);
  long top   = constrain(distance_max, 1, LCD_COLUMNS) - 1;

  char line[LCD_COLUMNS + 1];
  for (int col = 0; col < LCD_COLUMNS; col++) {
    if (col == top) {
      line[col] = '|';
    } else if (col < level) {
      line[col] = '#';
    } else {
      line[col] = ' ';
    }
  }
  line[LCD_COLUMNS] = '\0';                // кінець рядка

  lcd.setCursor(0, 1);
  lcd.print(line);
}
