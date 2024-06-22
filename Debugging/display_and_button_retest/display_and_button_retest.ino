#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define startButton 18 // GPIO 18 (Pin 24)
#define stopButton 19 // GPIO 19 (Pin 25)
#define LED_PIN 25 // Onboard LED (GPIO 25, Pin 13)

#define DEBOUNCE_MS 50 // Debounce time in milliseconds

unsigned long lastBlinkTime = 0;
const unsigned long blinkInterval = 500; // Blink interval in milliseconds

unsigned long lastStartButtonPress = 0;
unsigned long lastStopButtonPress = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

volatile bool startButtonPressed = false;
volatile bool stopButtonPressed = false;

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
  pinMode(LED_PIN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(startButton), startButtonISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(stopButton), stopButtonISR, FALLING);

  display.clearDisplay();
}

void loop() {
  blinkLED();
  checkButtons();
}

void startButtonISR() {
  unsigned long currentTime = millis();
  if (currentTime - lastStartButtonPress >= DEBOUNCE_MS) {
    startButtonPressed = true;
    lastStartButtonPress = currentTime;
  }
}

void stopButtonISR() {
  unsigned long currentTime = millis();
  if (currentTime - lastStopButtonPress >= DEBOUNCE_MS) {
    stopButtonPressed = true;
    lastStopButtonPress = currentTime;
  }
}

void blinkLED() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastBlinkTime >= blinkInterval) {
    lastBlinkTime = currentMillis;
    digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Toggle the LED state
  }
}

void checkButtons() {
  if (startButtonPressed) {
    startButtonPressed = false;
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Start Button Pressed");
    display.display();
  }
  if (stopButtonPressed) {
    stopButtonPressed = false;
    display.clearDisplay();
    display.setCursor(0, 20);
    display.println("Stop Button Pressed");
    display.display();
  }
  else {
    display.clearDisplay();
    display.display();
  }
}