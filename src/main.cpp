#include <Arduino.h>
#include <M5StickC.h>

const int MAIN_BUTTON_PIN = 37;
const int LED_PIN = 10;

bool mainButtonPressed = false;

bool measuring = true;

int width = 0;
int height = 0;

void onButtonPressed()
{
  static unsigned long lastCall = 0;

  // Debounce
  unsigned long now = millis();

  if (now - lastCall < 200)
  {
    return;
  }
  
  lastCall = now;
  mainButtonPressed = true;
}

uint16_t constructValue(uint8_t l, uint8_t h)
{
  uint16_t result;
  ((uint8_t*)&result)[0] = l;
  ((uint8_t*)&result)[1] = h;

  return result;
}

void discardSerialData()
{
  int receivedByteCount = Serial2.available();

  for (int i = 0; i < receivedByteCount; i++)
  {
    Serial2.read();
  }
}

void deconstructValue(uint16_t value, uint8_t& l, uint8_t& h)
{
  l = ((uint8_t*)&value)[0];
  h = ((uint8_t*)&value)[1];
}

bool readDistance(int& distance, int& strength, int& temp)
{
  if (Serial2.read() != 0x59) return false;
  if (Serial2.read() != 0x59) return false;

  uint8_t distL = Serial2.read();
  uint8_t distH = Serial2.read();

  uint8_t strengthL = Serial2.read();
  uint8_t strengthH = Serial2.read();

  uint8_t tempL = Serial2.read();
  uint8_t tempH = Serial2.read();

  uint8_t checksum = Serial2.read();

  /*
  if (0x59 + 0x59 + distL + distH + strengthL + strengthH + tempL + tempH != checksum & 0xFF)
  {
    return false;
  }
  */

  distance = constructValue(distL, distH);
  strength = constructValue(strengthL, strengthH);
  temp = constructValue(tempL, tempH);
  temp = temp / 8 - 256;

  return true;
}

void printDistance(int distance)
{
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(40, 20);

  if (distance < 100)
  {
    // Show in cm.
    M5.Lcd.print(distance);
    M5.Lcd.print(" cm");
  }
  else if (distance < 1000)
  {
    // Show m with 2 decimal places.
    M5.Lcd.print(distance / 100);
    M5.Lcd.print('.');
    M5.Lcd.print(distance % 100);
    M5.Lcd.print(" m");
  }
  else
  {
    // Show in m.
    M5.Lcd.print(distance / 100);
    M5.Lcd.print(" m");
  }
  
  // If a previous print had more chars than this one
  // this will reset those overhanging chars.
  M5.Lcd.print("    ");
}

void printTemp(int temp)
{
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(width - 35, height - 10);
  M5.Lcd.print(temp);
  M5.Lcd.print(" C");
}

void printStrength(int strength)
{
  static const int barWidth = width - 65;

  float percentage = strength / 65535.0f;
  uint16_t color = TFT_WHITE;

  if (percentage < 0.1)
  {
    color = TFT_RED;
  }
  
  // White bar.
  M5.Lcd.fillRect(20, height - 10, barWidth * percentage, 10, color);
  // Black remaining inner bar.
  M5.Lcd.fillRect(20 + barWidth * percentage, height - 9, barWidth * (1.0 - percentage), 8, TFT_BLACK);
  // Frame.
  M5.Lcd.drawRect(20, height - 10, barWidth, 10, color);
}

void drawSignalStrengthIcon()
{
  M5.Lcd.fillRect(7, height - 4, 2, 3, TFT_WHITE);
  M5.Lcd.fillRect(10, height - 6, 2, 5, TFT_WHITE);
  M5.Lcd.fillRect(13, height - 8, 2, 7, TFT_WHITE);
}

int getBatteryPercentage() {
  double voltage = M5.Axp.GetBatVoltage();
  return map(voltage * 1000, 3000, 4200, 0, 100);
}

void printBattery()
{
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(width - 35, 0);
  M5.Lcd.print(getBatteryPercentage());
  M5.Lcd.print(" %");
}

void drawBatteryIcon()
{
  float level = getBatteryPercentage() / 100.0f;
  
  uint16_t color = TFT_WHITE;
  if (level < 0.3)
  {
    color = TFT_RED;
  }

  // Outline
  M5.Lcd.drawRect(width - 55, 0, 15, 6, color);
  // Inner rect
  M5.Lcd.fillRect(width - 55, 0, 15 * level, 6, color);
  // Thingy at the front
  M5.Lcd.drawLine(width - 40, 2, width - 40, 3, color);
}

void setup() {
  Serial2.begin(115200, (uint32_t)SERIAL_8N1, G26, G0);

  pinMode(LED_PIN, OUTPUT);
  // LED is wired as active low.
  digitalWrite(LED_PIN, HIGH);

  pinMode(MAIN_BUTTON_PIN, INPUT_PULLDOWN);
  attachInterrupt(MAIN_BUTTON_PIN, onButtonPressed, FALLING);

  M5.begin();
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(TFT_BLACK);
  width = M5.Lcd.width();
  height = M5.Lcd.height();

  drawSignalStrengthIcon();

  drawBatteryIcon();
  printBattery();
}

void loop() {
  if (measuring)
  {
    int distance;
    int strength;
    int temp;
    if (readDistance(distance, strength, temp))
    {
      digitalWrite(LED_PIN, HIGH);

      printDistance(distance);
      printTemp(temp);
      printStrength(strength);
    }
    else
    {
      // Turn on LED on read error.
      digitalWrite(LED_PIN, LOW);
    }
  }

  if (M5.Axp.GetBtnPress() == 0x02)
  {
    // Power button short press.
    M5.Axp.PowerOff(); 
  }

  if (mainButtonPressed)
  {
    mainButtonPressed = false;

    if (measuring)
    {
      // Stop measuring
      // Turn off red LED.
      digitalWrite(LED_PIN, HIGH);

      // Draw line under current distance value.
      M5.Lcd.drawLine(20, 45, 120, 45, TFT_WHITE);
    }
    else
    {
      // Remove line under distance value.
      M5.Lcd.drawLine(20, 45, 120, 45, TFT_BLACK);
    }

    discardSerialData();

    measuring = !measuring;
  }
}