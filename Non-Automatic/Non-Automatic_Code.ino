#include <Adafruit_INA219.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define motor1 26
#define motor2 27

#define leftButton 18
#define rightButton 19

#define DEBOUNCE_MS 50 // Debounce time in milliseconds

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_INA219 tankDepth1;
Adafruit_INA219 tankDepth2(0x41);

void setup() {
  Serial.begin(115200);
  
  // Initialize INA219 sensors
  if (!tankDepth1.begin()) {
    Serial.println("Failed to find tank 1 chip");
    while (1) { delay(10); }
  }
  tankDepth1.setCalibration_16V_400mA();

  if (!tankDepth2.begin()) {
    Serial.println("Failed to find tank 2 chip");
    while (1) { delay(10); }
  }
  tankDepth2.setCalibration_16V_400mA();
  
  // Initialize display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  pinMode(leftButton, INPUT_PULLUP);
  pinMode(rightButton, INPUT_PULLUP);
  pinMode(motor1, OUTPUT);
  pinMode(motor2, OUTPUT);
}

void loop() {
  displaySensorReadings();

  handleButtonPress(leftButton, motor1, "Pump 1 Manual");
  handleButtonPress(rightButton, motor2, "Pump 2 Manual");

  delay(100); // Reduce loop speed
}

void displaySensorReadings() {
  float tank1mA = tankDepth1.getCurrent_mA();
  float tank2mA = tankDepth2.getCurrent_mA();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Tank 1: "); display.print(tank1mA); display.println(" V");
  display.print("Tank 2: "); display.print(tank2mA); display.println(" V");
  display.display();
}

void startMotor(int motor) {
  digitalWrite(motor1, motor == motor1 ? HIGH : LOW);
  digitalWrite(motor2, motor == motor2 ? HIGH : LOW);
}

void stopMotors() {
  digitalWrite(motor1, LOW);
  digitalWrite(motor2, LOW);
}

void handleButtonPress(int buttonPin, int motor, const char* message) {
  if (digitalRead(buttonPin) == LOW) { // Button is pressed
    unsigned long pressTime = millis();
    display.clearDisplay();
    displaySensorReadings(); // Ensure sensor readings are shown
    display.setCursor(0, 30); // Adjust as needed to fit your layout
    display.println(message);
    display.display();

    startMotor(motor);

    while (digitalRead(buttonPin) == LOW) {
      if (millis() - pressTime > DEBOUNCE_MS) {
        pressTime = millis();
      }
    }

    stopMotors(); // Stop motors when button is released
    delay(DEBOUNCE_MS); // Debounce delay after release
  }
}
