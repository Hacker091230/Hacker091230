/*
 * mic1_week2.ino
 * LED-біжучий вогник на D8..D13, який змінює напрямок при натисканні
 * кнопки на D2.
 *
 * Name:           <Ваше ім'я>
 * Student number: <Ваш номер студента>
 * Date:           <дата>
 *
 * Поєднує ідеї з прикладів:
 *   - Blink Without Delay      -> таймінг через millis() замість delay()
 *   - State Change Detection   -> реагуємо на момент натискання, а не на стан
 */

// --- Світлодіоди ---
const int FIRST_LED = 8;    // D8
const int LAST_LED  = 13;   // D13

// --- Кнопка ---
// Кнопка K1..K4 підключена між D2 і GND. Зовнішнього pullup-резистора немає,
// тому вмикаємо внутрішній (INPUT_PULLUP):
//   не натиснута -> HIGH, натиснута -> LOW
const int BUTTON_PIN = 2;

// --- Таймінг ---
const unsigned long INTERVAL_MS = 100;  // час між двома світлодіодами
const unsigned long DEBOUNCE_MS = 50;   // захист від брязкоту контактів

// --- Змінні стану ---
int currentLed = FIRST_LED;             // світлодіод, що зараз світиться
int direction  = 1;                     // +1: D8 -> D13, -1: D13 -> D8
unsigned long previousMillis = 0;       // коли востаннє переключали світлодіод

int lastButtonState = HIGH;             // попередній стан кнопки
unsigned long lastButtonChange = 0;     // коли востаннє зафіксували зміну кнопки

void setup() {
  for (int pin = FIRST_LED; pin <= LAST_LED; pin++) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(currentLed, HIGH);       // запалюємо перший світлодіод
  previousMillis = millis();
}

void loop() {
  unsigned long currentMillis = millis();

  // ---------- 1. Кнопка: State Change Detection ----------
  int buttonState = digitalRead(BUTTON_PIN);

  if (buttonState != lastButtonState &&
      currentMillis - lastButtonChange >= DEBOUNCE_MS) {
    lastButtonChange = currentMillis;

    if (buttonState == LOW) {
      // Перехід HIGH -> LOW = кнопку щойно натиснули -> змінюємо напрямок
      direction = -direction;
    }
    lastButtonState = buttonState;
  }

  // ---------- 2. Біжучий вогник: Blink Without Delay ----------
  if (currentMillis - previousMillis >= INTERVAL_MS) {
    previousMillis = currentMillis;

    digitalWrite(currentLed, LOW);      // гасимо поточний

    currentLed += direction;            // крок у поточному напрямку
    if (currentLed > LAST_LED) {        // вийшли за D13 -> починаємо з D8
      currentLed = FIRST_LED;
    } else if (currentLed < FIRST_LED) {// вийшли за D8 -> починаємо з D13
      currentLed = LAST_LED;
    }

    digitalWrite(currentLed, HIGH);     // запалюємо наступний
  }
}
