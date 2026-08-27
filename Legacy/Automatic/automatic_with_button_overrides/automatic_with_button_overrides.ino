#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define motor1 26 // GPIO 26 (Pin 31)
#define motor2 27 // GPIO 27 (Pin 32)

#define startButton 18 // GPIO 18 (Pin 24)
#define stopButton 19 // GPIO 19 (Pin 25)

#define tank1LowSwitch 10 // GPIO 10 (Pin 14)
#define tank1HighSwitch 11 // GPIO 11 (Pin 15)
#define tank2LowSwitch 14 // GPIO 14 (Pin 19)
#define tank2HighSwitch 15 // GPIO 15 (Pin 20)

#define LED_PIN 25 // Onboard LED (GPIO 25, Pin 13)

#define DEBOUNCE_MS 50 // Debounce time in milliseconds

#define PUMP1_MAX_RUNTIME 1800000 // 30 minutes in milliseconds
#define PUMP2_MAX_RUNTIME 7200000 // 2 hours in milliseconds
#define PUMP_REST_TIME 600000 // 10 minutes in milliseconds

enum PumpState { IDLE, WAIT_CONFIRMATION, RUNNING, FAULT };

volatile PumpState pumpState = IDLE;
volatile bool startButtonPressed = false;
volatile bool stopButtonPressed = false;

unsigned long pump1RunTime = 0;
unsigned long pump2RunTime = 0;
unsigned long pump1StopTime = 0;
unsigned long pump2StopTime = 0;

bool pump1Fault = false;
bool pump2Fault = false;

unsigned long lastBlinkTime = 0;
const unsigned long blinkInterval = 500; // Blink interval in milliseconds

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void startButtonISR() {
  startButtonPressed = true;
}

void stopButtonISR() {
  stopButtonPressed = true;
}

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

  pinMode(startButton, INPUT_PULLUP);
  pinMode(stopButton, INPUT_PULLUP);
  pinMode(motor1, OUTPUT);
  pinMode(motor2, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  pinMode(tank1LowSwitch, INPUT_PULLUP);
  pinMode(tank1HighSwitch, INPUT_PULLUP);
  pinMode(tank2LowSwitch, INPUT_PULLUP);
  pinMode(tank2HighSwitch, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(startButton), startButtonISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(stopButton), stopButtonISR, FALLING);
}

void loop() {
  displayTankLevels();
  blinkLED();

  unsigned long currentMillis = millis();

  if (pumpState == IDLE) {
    if (tankNeedsPumping() && !pump1Fault && !pump2Fault) {
      pumpState = WAIT_CONFIRMATION;
    }
  } else if (pumpState == WAIT_CONFIRMATION) {
    displayConfirmationMessage();
    if (startButtonPressed) {
      startButtonPressed = false;
      if (!pump1Fault) {
        pumpState = RUNNING;
        pump1RunTime = currentMillis;
        startMotor(motor1);
      } else if (!pump2Fault) {
        pumpState = RUNNING;
        pump2RunTime = currentMillis;
        startMotor(motor2);
      }
    }
  } else if (pumpState == RUNNING) {
    if (stopButtonPressed || tank1LowOpened() || tank2HighClosed()) {
      stopButtonPressed = false;
      stopMotors();
      pumpState = IDLE;
      if (currentMillis - pump1RunTime >= PUMP1_MAX_RUNTIME) {
        if (currentMillis - pump1StopTime < PUMP_REST_TIME) {
          pump1Fault = true;
          pumpState = FAULT;
        }
        pump1StopTime = currentMillis;
      }
      if (currentMillis - pump2RunTime >= PUMP2_MAX_RUNTIME) {
        if (currentMillis - pump2StopTime < PUMP_REST_TIME) {
          pump2Fault = true;
          pumpState = FAULT;
        }
        pump2StopTime = currentMillis;
      }
    }

    if (!pump1Fault && currentMillis - pump1RunTime >= PUMP1_MAX_RUNTIME) {
      stopMotors();
      pumpState = IDLE;
      pump1StopTime = currentMillis;
      delay(PUMP_REST_TIME);
      if (currentMillis - pump1RunTime >= PUMP1_MAX_RUNTIME) {
        pump1Fault = true;
        pumpState = FAULT;
      }
    }

    if (!pump2Fault && currentMillis - pump2RunTime >= PUMP2_MAX_RUNTIME) {
      stopMotors();
      pumpState = IDLE;
      pump2StopTime = currentMillis;
      delay(PUMP_REST_TIME);
      if (currentMillis - pump2RunTime >= PUMP2_MAX_RUNTIME) {
        pump2Fault = true;
        pumpState = FAULT;
      }
    }
  } else if (pumpState == FAULT) {
    displayFaultMessage();
  }

  delay(100); // Reduce loop speed
}

void displayTankLevels() {
  bool tank1Low = digitalRead(tank1LowSwitch) == LOW;
  bool tank1High = digitalRead(tank1HighSwitch) == LOW;
  bool tank2Low = digitalRead(tank2LowSwitch) == LOW;
  bool tank2High = digitalRead(tank2HighSwitch) == LOW;

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Tank 1: ");
  if (tank1High) {
    display.println("Full");
  } else if (tank1Low) {
    display.println("Low");
  } else {
    display.println("Intermediate");
  }

  display.setCursor(0, 20);
  display.print("Tank 2: ");
  if (tank2High) {
    display.println("Full");
  } else if (tank2Low) {
    display.println("Low");
  } else {
    display.println("Intermediate");
  }
  display.display();
}

void displayConfirmationMessage() {
  display.clearDisplay();
  display.setCursor(0, 30);
  display.println("Press Start to pump");
  display.display();
}

void displayFaultMessage() {
  display.clearDisplay();
  display.setCursor(0, 30);
  display.println("FAULT STATE: Restart Required");
  display.display();
}

bool tankNeedsPumping() {
  bool tank1Low = digitalRead(tank1LowSwitch) == LOW;
  bool tank1High = digitalRead(tank1HighSwitch) == LOW;
  bool tank2Low = digitalRead(tank2LowSwitch) == LOW;
  bool tank2High = digitalRead(tank2HighSwitch) == LOW;

  // Example condition: Pump if tank 1 is high or tank 2 is high
  return tank1High || tank2High;
}

bool tank1LowOpened() {
  return digitalRead(tank1LowSwitch) == HIGH; // Tank 1 low switch opened
}

bool tank2HighClosed() {
  return digitalRead(tank2HighSwitch) == LOW; // Tank 2 high switch closed
}

void startMotor(int motor) {
  digitalWrite(motor1, motor == motor1 ? HIGH : LOW);
  digitalWrite(motor2, motor == motor2 ? HIGH : LOW);
}

void stopMotors() {
  digitalWrite(motor1, LOW);
  digitalWrite(motor2, LOW);
}

void blinkLED() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastBlinkTime >= blinkInterval) {
    lastBlinkTime = currentMillis;
    digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Toggle the LED state
  }
}
