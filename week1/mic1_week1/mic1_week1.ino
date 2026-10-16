/*
 * mic1_week1.ino
 * LED-біжучий вогник (LED chaser) на шести світлодіодах D8..D13.
 *
 * Name:           <Ваше ім'я>
 * Student number: <Ваш номер студента>
 * Date:           <дата>
 *
 * Порядок роботи:
 *   D13 -> D12 -> D11 -> D10 -> D9 -> D8 -> знову D13 ...
 *   Кожен світлодіод світиться 200 мс, потім гасне і вмикається наступний.
 */

// Номери пінів, до яких підключені світлодіоди
const int LED_D8  = 8;
const int LED_D9  = 9;
const int LED_D10 = 10;
const int LED_D11 = 11;
const int LED_D12 = 12;
const int LED_D13 = 13;

// Затримка між світлодіодами, мс
const unsigned long DELAY_MS = 200;

void setup() {
  // Усі шість пінів налаштовуємо як виходи
  pinMode(LED_D8,  OUTPUT);
  pinMode(LED_D9,  OUTPUT);
  pinMode(LED_D10, OUTPUT);
  pinMode(LED_D11, OUTPUT);
  pinMode(LED_D12, OUTPUT);
  pinMode(LED_D13, OUTPUT);

  // Початковий стан: усі світлодіоди вимкнені
  digitalWrite(LED_D8,  LOW);
  digitalWrite(LED_D9,  LOW);
  digitalWrite(LED_D10, LOW);
  digitalWrite(LED_D11, LOW);
  digitalWrite(LED_D12, LOW);
  digitalWrite(LED_D13, LOW);
}

void loop() {
  // D13
  digitalWrite(LED_D13, HIGH);
  delay(DELAY_MS);
  digitalWrite(LED_D13, LOW);

  // D12
  digitalWrite(LED_D12, HIGH);
  delay(DELAY_MS);
  digitalWrite(LED_D12, LOW);

  // D11
  digitalWrite(LED_D11, HIGH);
  delay(DELAY_MS);
  digitalWrite(LED_D11, LOW);

  // D10
  digitalWrite(LED_D10, HIGH);
  delay(DELAY_MS);
  digitalWrite(LED_D10, LOW);

  // D9
  digitalWrite(LED_D9, HIGH);
  delay(DELAY_MS);
  digitalWrite(LED_D9, LOW);

  // D8 — після його вимкнення loop() закінчується
  // і Arduino автоматично викликає loop() знову, тобто починаємо з D13
  digitalWrite(LED_D8, HIGH);
  delay(DELAY_MS);
  digitalWrite(LED_D8, LOW);
}
