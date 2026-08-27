#include <Adafruit_INA219.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define hTank 26
#define sTank 27

#define hTankLS 10 // GPIO 10 (Pin 14)
#define hTankHS 11 // GPIO 11 (Pin 15)
#define sTankLS 14 // GPIO 14 (Pin 19)
#define sTankHS 15 // GPIO 15 (Pin 20)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_INA219 tankDepth1;
Adafruit_INA219 tankDepth2(0x41);

void setup() {
  Wire.begin();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  pinMode(hTankLS, INPUT_PULLUP);
  pinMode(hTankHS, INPUT_PULLUP);
  pinMode(sTankLS, INPUT_PULLUP);
  pinMode(sTankHS, INPUT_PULLUP);

  pinMode(hTank, OUTPUT);
  pinMode(sTank, OUTPUT);

  digitalWrite(hTank, LOW);
  digitalWrite(sTank, LOW);
}

void loop() {
  bool houseTankLow = digitalRead(hTankLS);
  bool houseTankHigh = digitalRead(hTankHS);
  bool sandTankHigh = digitalRead(sTankLS);
  bool sandTankHigh = digitalRead(sTankHS);
  
  // Ensure only one pump runs at a time
  if (sandTankHigh) {
    // Tank 2 is full, stop pumping into it and check if it needs to be dumped
    digitalWrite(hTank, LOW);
    if (houseTankHigh) {
      // If Tank 1 is full, dump Tank 2
      digitalWrite(sTank, HIGH);
    } else {
      digitalWrite(sTank, LOW);
    }
  } else if (sandTankHigh) {
    // Tank 2 is not full, allow pumping from Tank 1
    if (houseTankLow) {
      digitalWrite(hTank, HIGH);
      digitalWrite(sTank, LOW);
    } else {
      digitalWrite(hTank, LOW);
      digitalWrite(sTank, LOW);
    }
  } else {
    // Default case: both tanks are not full or empty
    digitalWrite(hTank, LOW);
    digitalWrite(sTank, LOW);
  }
  }
}
