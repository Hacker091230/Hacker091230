/*
 * mic1_week3.ino
 * Послідовний порт (UART): керування світлодіодами та вивід статусу
 * програми в Serial Monitor.
 *
 * Name:           <Ваше ім'я>
 * Student number: <Ваш номер студента>
 * Date:           <дата>
 *
 * Що робить скетч:
 *   - кожну секунду друкує "Timeout";
 *   - при натисканні SW0 друкує "SW0 pressed" і гасить LED на D8;
 *   - друкує отриманий рядок у шістнадцятковому вигляді;
 *   - якщо отримано номер студента -> "Hello <ім'я>!" і вмикає LED на D8,
 *     інакше -> "Unknown";
 *   - LED на D9 блимає з частотою 5 Гц (100 мс увімк. / 100 мс вимк.).
 *
 * У Serial Monitor: швидкість 9600 baud, закінчення рядка "New Line".
 */

// --- Дані студента (ЗАМІНІТЬ НА СВОЇ) ---
const char STUDENT_NUMBER[] = "123456\n";   // номер + символ нового рядка '\n'
const char FIRST_NAME[]     = "Name";

// --- Піни ---
const int LED_STUDENT = 8;    // D8: вмикається при правильному номері
const int LED_BLINK   = 9;    // D9: блимає з 5 Гц
const int SW0_PIN     = 21;   // SW0 на платі Xplained mini (PB7 = D21)

// --- Таймінг ---
const unsigned long TIMEOUT_INTERVAL_MS = 1000;  // "Timeout" раз на секунду
const unsigned long BLINK_INTERVAL_MS   = 100;   // 100 мс увімк. / 100 мс вимк.

// --- Змінні стану ---
unsigned long previousTimeoutMillis = 0;
unsigned long previousBlinkMillis   = 0;
int blinkState = LOW;

int lastSw0State = HIGH;      // SW0 не натиснута = HIGH (pullup R209)

String inputString = "";      // сюди накопичуються отримані символи
bool stringComplete = false;  // true, коли прийшов '\n'

void setup() {
  Serial.begin(9600);
  inputString.reserve(200);   // заздалегідь виділяємо пам'ять під рядок

  pinMode(LED_STUDENT, OUTPUT);
  pinMode(LED_BLINK, OUTPUT);
  digitalWrite(LED_STUDENT, LOW);
  digitalWrite(LED_BLINK, LOW);

  // SW0 вже має зовнішній pullup-резистор (R209) і конденсатор (C204)
  pinMode(SW0_PIN, INPUT);
}

void loop() {
  unsigned long currentMillis = millis();

  // ---------- 1. "Timeout" кожну секунду ----------
  if (currentMillis - previousTimeoutMillis >= TIMEOUT_INTERVAL_MS) {
    previousTimeoutMillis = currentMillis;
    Serial.println("Timeout");
  }

  // ---------- 2. LED D9 блимає з 5 Гц ----------
  if (currentMillis - previousBlinkMillis >= BLINK_INTERVAL_MS) {
    previousBlinkMillis = currentMillis;
    blinkState = (blinkState == LOW) ? HIGH : LOW;
    digitalWrite(LED_BLINK, blinkState);
  }

  // ---------- 3. SW0: реагуємо на момент натискання ----------
  int sw0State = digitalRead(SW0_PIN);
  if (sw0State != lastSw0State) {
    if (sw0State == LOW) {                 // HIGH -> LOW = натиснули
      Serial.println("SW0 pressed");
      digitalWrite(LED_STUDENT, LOW);      // SW0 гасить LED на D8
    }
    lastSw0State = sw0State;
  }

  // ---------- 4. Обробка отриманого рядка ----------
  if (stringComplete) {
    // Друкуємо кожен символ у шістнадцятковому вигляді
    Serial.print("Characters received in hexadecimal: ");
    for (unsigned int i = 0; i < inputString.length(); i++) {
      Serial.print("0x");
      if ((byte)inputString[i] < 0x10) {
        Serial.print('0');                 // щоб 0x0A, а не 0xA
      }
      Serial.print((byte)inputString[i], HEX);
      Serial.print(' ');
    }
    Serial.println();

    // Порівнюємо з номером студента
    if (inputString == STUDENT_NUMBER) {
      Serial.print("Hello ");
      Serial.print(FIRST_NAME);
      Serial.println("!");
      digitalWrite(LED_STUDENT, HIGH);
    } else {
      Serial.println("Unknown");
    }

    // Готуємося до наступного рядка
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
