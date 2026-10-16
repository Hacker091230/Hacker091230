/*
 * mic1_week4.ino
 * Аналогові сигнали: фоторезистор на A0, ШІМ (PWM) на D9.
 *
 * Name:           <Ваше ім'я>
 * Student number: <Ваш номер студента>
 * Date:           <дата>
 *
 * Що робить скетч:
 *   - кожні 200 мс вимірює A0 і друкує значення, тільки якщо воно змінилося
 *     більш ніж на +/-10 відносно останнього надрукованого;
 *   - A0 < 300  -> LED D10 вимикається,
 *     A0 >= 500 -> LED D10 вмикається (між ними стан не змінюється);
 *   - цифра 0..9 з Serial Monitor задає яскравість LED D9 (0 = 0%, 9 = 100%);
 *   - кожне натискання SW0 перемикає LED D8 (увімк./вимк.).
 *
 * У Serial Monitor: швидкість 9600 baud, закінчення рядка "New Line".
 */

// --- Піни ---
const int LED_TOGGLE = 8;     // D8: перемикається кнопкою SW0
const int LED_FADE   = 9;     // D9: яскравість через ШІМ (D9 підтримує PWM)
const int LED_LEVEL  = 10;    // D10: вмикається/вимикається за рівнем A0
const int SW0_PIN    = 21;    // SW0 на платі Xplained mini (PB7 = D21)
const int SENSOR_PIN = A0;    // фоторезистор + резистор 10 кОм

// --- Налаштування ---
const unsigned long SAMPLE_INTERVAL_MS = 200;  // як часто вимірюємо A0
const int PRINT_THRESHOLD = 10;                // друкуємо при зміні > 10
const int LEVEL_OFF_BELOW = 300;               // < 300  -> D10 вимк.
const int LEVEL_ON_FROM   = 500;               // >= 500 -> D10 увімк.

// --- Змінні стану ---
unsigned long previousSampleMillis = 0;
int lastPrintedValue = -1000;  // заздалегідь "далеко", щоб перше значення надрукувалося

int lastSw0State = HIGH;       // SW0 не натиснута = HIGH (pullup R209)
int toggleLedState = LOW;

String inputString = "";
bool stringComplete = false;

void setup() {
  Serial.begin(9600);
  inputString.reserve(200);

  pinMode(LED_TOGGLE, OUTPUT);
  pinMode(LED_FADE, OUTPUT);
  pinMode(LED_LEVEL, OUTPUT);
  digitalWrite(LED_TOGGLE, LOW);
  analogWrite(LED_FADE, 0);
  digitalWrite(LED_LEVEL, LOW);

  pinMode(SW0_PIN, INPUT);     // pullup і конденсатор уже є на платі
}

void loop() {
  unsigned long currentMillis = millis();

  // ---------- 1. Вимірювання A0 кожні 200 мс ----------
  if (currentMillis - previousSampleMillis >= SAMPLE_INTERVAL_MS) {
    previousSampleMillis = currentMillis;

    int sensorValue = analogRead(SENSOR_PIN);   // 0..1023

    // 7a: друкуємо тільки при зміні більш ніж на +/-10
    if (abs(sensorValue - lastPrintedValue) > PRINT_THRESHOLD) {
      Serial.println(sensorValue);
      lastPrintedValue = sensorValue;
    }

    // 7b: D10 з гістерезисом
    if (sensorValue < LEVEL_OFF_BELOW) {
      digitalWrite(LED_LEVEL, LOW);
    } else if (sensorValue >= LEVEL_ON_FROM) {
      digitalWrite(LED_LEVEL, HIGH);
    }
    // 300..499 -> нічого не змінюємо, LED лишається як був
  }

  // ---------- 2. SW0 перемикає D8 ----------
  int sw0State = digitalRead(SW0_PIN);
  if (sw0State != lastSw0State) {
    if (sw0State == LOW) {                 // HIGH -> LOW = натиснули
      Serial.println("SW0 pressed");
      toggleLedState = (toggleLedState == LOW) ? HIGH : LOW;
      digitalWrite(LED_TOGGLE, toggleLedState);
    }
    lastSw0State = sw0State;
  }

  // ---------- 3. Цифра 0..9 -> яскравість D9 ----------
  if (stringComplete) {
    // Очікуємо рівно один символ-цифру і '\n', наприклад "5\n"
    if (inputString.length() == 2 &&
        inputString[0] >= '0' && inputString[0] <= '9') {
      int digit = inputString[0] - '0';          // '5' -> 5
      int pwmValue = map(digit, 0, 9, 0, 255);   // 0..9 -> 0..255
      analogWrite(LED_FADE, pwmValue);

      Serial.print(map(digit, 0, 9, 0, 100));    // 0..9 -> 0..100 %
      Serial.println("%");
    } else {
      Serial.println("Invalid input");
    }

    inputString = "";
    stringComplete = false;
  }
}

/*
 * serialEvent() викликається автоматично після кожного проходу loop(),
 * якщо в буфері UART є отримані байти.
 */
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    inputString += inChar;
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
