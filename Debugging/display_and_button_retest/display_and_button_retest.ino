#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define startButton 18 // GPIO 18 (Pin 24)
#define stopButton 19 // GPIO 19 (Pin 25)

#define DEBOUNCE_MS 50 // Debounce time in milliseconds

#define PUMP1 26
#define PUMP2 27

unsigned long lastStartButtonPress = 0;
unsigned long lastStopButtonPress = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

bool startButtonPressed = false;
bool stopButtonPressed = false;

void setup() {
  Serial.begin(115200);

  // Initialize I2C (default pins)
  Wire.begin();

  // Initialize display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Test display
  display.setCursor(0, 0);
  display.println(F("Display Initialized"));
  display.display();
  delay(2000); // Wait for 2 seconds to see the message

  pinMode(startButton, INPUT_PULLUP);
  pinMode(stopButton, INPUT_PULLUP);
  pinMode(PUMP1, OUTPUT);
  pinMode(PUMP2, OUTPUT);

  display.clearDisplay();
}

void loop() {
  checkButtons();
  updateDisplay();
}

void checkButtons() {
  unsigned long currentMillis = millis();
  
  if (digitalRead(startButton) == LOW) {
    if (currentMillis - lastStartButtonPress >= DEBOUNCE_MS) {
      startButtonPressed = true;
      lastStartButtonPress = currentMillis;
    }
  }
  
  if (digitalRead(stopButton) == LOW) {
    if (currentMillis - lastStopButtonPress >= DEBOUNCE_MS) {
      stopButtonPressed = true;
      lastStopButtonPress = currentMillis;
    }
  }
}

void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);

  if (startButtonPressed) {
    //digitalWrite(PUMP2, LOW);
    display.setCursor(0, 20);
    display.println("Start Button Pressed");
    //digitalWrite(PUMP1, HIGH);
  } 
  if (stopButtonPressed) {
    //digitalWrite(PUMP1, LOW);
    display.setCursor(0, 20);
    display.println("Stop Button Pressed");
    //digitalWrite(PUMP2, HIGH);
  } 
  if (!stopButtonPressed && !startButtonPressed) {
    //digitalWrite(PUMP1, LOW);
    //digitalWrite(PUMP2, LOW);
    display.setCursor(0, 20);
    display.println("Waiting for button press...");
  }

  display.display();
  
  // Reset button states
  startButtonPressed = false;
  stopButtonPressed = false;

  // Small delay to ensure display update
  delay(1000);
}