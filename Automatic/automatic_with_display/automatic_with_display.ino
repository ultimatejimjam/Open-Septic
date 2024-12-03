#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// this is current production code 8/5/24

// Define input pins for level switches
const int tank1Low = 10;
const int tank1High = 11;
const int tank2Low = 14;
const int tank2High = 15;

// Define output pins for relays
const int pump1Relay = 26;
const int pump2Relay = 27;

// Current Sensor Safety
const int currentSensor = 34;

// Define runtime limits (in milliseconds)
const unsigned long pump1MaxRunTime = 30 * 60 * 1000; // 30 minutes
const unsigned long pump2MaxRunTime = 3 * 60 * 60 * 1000; // 3 hours

// Variables to track pump runtimes and errors
unsigned long pump1StartTime = 0;
unsigned long pump2StartTime = 0;
bool pump1Error = false;
bool pump2Error = false;
bool emptyTank1;
bool emptyTank2;

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  // Initialize input pins
  pinMode(tank1Low, INPUT_PULLUP);
  pinMode(tank1High, INPUT_PULLUP);
  pinMode(tank2Low, INPUT_PULLUP);
  pinMode(tank2High, INPUT_PULLUP);
  pinMode(currentCensor, INPUT);

  // Initialize output pins
  pinMode(pump1Relay, OUTPUT);
  pinMode(pump2Relay, OUTPUT);
  
  // Start with pumps off
  digitalWrite(pump1Relay, LOW);
  digitalWrite(pump2Relay, LOW);

  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  // Read the level switches
  bool tank1LowLevel  = !digitalRead(tank1Low);
  bool tank1HighLevel = digitalRead(tank1High);
  bool tank2LowLevel  = !digitalRead(tank2Low);
  bool tank2HighLevel = digitalRead(tank2High);
  int current = analogRead(currentCensor);

  // Determine tank fill levels
  String tank1Fill = "Med";
  if (!tank1LowLevel && !tank1HighLevel) {
    tank1Fill = "Empty";
    emptyTank1 = 0;
  } else if (tank1LowLevel && tank1HighLevel) {
    tank1Fill = "Full";
  }


  String tank2Fill = "Med";
  if (!tank2LowLevel && !tank2HighLevel) {
    tank2Fill = "Empty";
    emptyTank2 = 0;
  } else if (tank2LowLevel && tank2HighLevel) {
    tank2Fill = "Full";
    emptyTank2 = 1;
  }

  // Display the current state on the OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("T1: ");
  display.print(tank1Fill);
  display.setCursor(63, 0);
  display.print("T2: ");
  display.print(tank2Fill);
  display.setCursor(0, 15);
  display.print("ADC: ")
  display.print(current);

  if (pump1Error) {
    display.setCursor(0, 16);
    display.print("Pump 1 Error!");
    digitalWrite(pump1Relay, LOW);
  } else if (pump2Error) {
    display.setCursor(0, 16);
    display.print("Pump 2 Error!");
    digitalWrite(pump2Relay, LOW);
  } else {
    if (tank2HighLevel || emptyTank2) {
      emptyTank2 = 1;
      // Tank 2 is full, stop pumping into it and check if it needs to be dumped
      digitalWrite(pump1Relay, LOW);
      if (tank2LowLevel || emptyTank2) {
        // If Tank 1 is full, dump Tank 2
        digitalWrite(pump2Relay, HIGH);
        if (pump2StartTime == 0) {
          pump2StartTime = millis();
        }
        if (millis() - pump2StartTime > pump2MaxRunTime) {
          pump2Error = true;
        }
      } else {
        digitalWrite(pump2Relay, LOW);
        pump2StartTime = 0;
      }
    } else if (!tank2HighLevel) {
      // Tank 2 is not full, allow pumping from Tank 1
      if (tank1HighLevel || emptyTank1) {
        emptyTank1 = 1;
        digitalWrite(pump1Relay, HIGH);
        if (pump1StartTime == 0) {
          pump1StartTime = millis();
        }
        if (millis() - pump1StartTime > pump1MaxRunTime) {
          pump1Error = true;
        }
        digitalWrite(pump2Relay, LOW);
      } else {
        digitalWrite(pump1Relay, LOW);
        pump1StartTime = 0;
        digitalWrite(pump2Relay, LOW);
      }
    } else {
      // Default case: both tanks are not full or empty
      digitalWrite(pump1Relay, LOW);
      pump1StartTime = 0;
      digitalWrite(pump2Relay, LOW);
      pump2StartTime = 0;
    }
  }

  // Display pump error status and operation status
  display.setCursor(0, 32);
  if (pump1Error || pump2Error) {
    if (pump1Error) {
      display.print("Pump 1 Error");
    } else {
      display.print("Pump 2 Error");
    }
  } else {
    display.print("P1: ");
    display.print(digitalRead(pump1Relay));
    display.print(" P2: ");
    display.print(digitalRead(pump2Relay));
  }

  display.display();
  delay(500); // Update the display every 500 ms
}