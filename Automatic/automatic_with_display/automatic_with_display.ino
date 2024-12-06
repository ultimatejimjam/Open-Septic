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
const int currentSensor = 28;

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
bool pumping2;
bool pumping1;

// Initialiize Tanks as Empty
String tank1Fill = "Emp";
String tank2Fill = "Emp"; 

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
  // switches should be high when open
  bool tank1LowLevel  = digitalRead(tank1Low);
  bool tank1HighLevel = digitalRead(tank1High);
  bool tank2LowLevel  = digitalRead(tank2Low);
  bool tank2HighLevel = digitalRead(tank2High);
  int current = analogRead(currentSensor);

  // Determine tank fill levels
  // low bool means closed switch & vice-a
  if (tank1LowLevel && tank1HighLevel) {
    tank1Fill = "Ful";
  } if (tank1LowLevel && !tank1HighLevel){
     tank1Fill = "Med";
  } if (!tank1LowLevel && !tank1HighLevel) {
    tank1Fill = "Emp";
  } else {
    tank1Fill = "Err";
  }

  if (!tank2LowLevel && tank2HighLevel) {
    tank2Fill = "Ful";
  } if (!tank2LowLevel && !tank2HighLevel){
    tank2Fill = "Med";
  } if (tank2LowLevel && tank2HighLevel){
    tank2Fill = "Emp";
  }
  else {
    tank2Fill = "Err";
  }


  // Error handling: ADC average below threshold
  static int adcReadings[2] = {0, 0}; // Circular buffer for last two readings
  adcReadings[0] = adcReadings[1];
  adcReadings[1] = current;
  int adcAverage = (adcReadings[0] + adcReadings[1]) / 2;

  static unsigned long pump1StartBuffer = 0;
  static unsigned long pump2StartBuffer = 0;

  if (pumping1 || pumping2) {
    // Add a 2-second buffer after starting a pump before checking for ADC errors
    if ((pumping1 || pumping2) && millis() - pump1StartBuffer < 4000) {
      adcAverage = 1000; // Force ADC safe for 2 seconds after starting Pump 1
    }

    if (adcAverage < 275) {
      pump1Error = true;
      pump2Error = true;
      digitalWrite(pump1Relay, LOW);
      digitalWrite(pump2Relay, LOW);
      displayErrorMessage("ADC Error");
      return;
    }
  }

  // Pump 2 logic
  if (!pumping1 && !pump2Error) {
    if (tank2Fill == "Ful" && !pumping2) {
      pumping2 = true;
      pump2StartTime = millis();
      digitalWrite(pump2Relay, HIGH);
    } else if (pumping2) {
      if (tank2Fill == "Emp" || (millis() - pump2StartTime > 30 * 60 * 1000 && tank1Fill == "Ful")) {
        pumping2 = false;
        digitalWrite(pump2Relay, LOW);
      }
    }
  }

  // Pump 1 logic
  if (!pumping2 && !pump1Error) {
    if (tank1Fill == "Ful" && tank2Fill != "Ful" && !pumping1) {
      pumping1 = true;
      pump1StartTime = millis();
      digitalWrite(pump1Relay, HIGH);
    } else if (pumping1) {
      if (tank1Fill == "Emp") {
        pumping1 = false;
        digitalWrite(pump1Relay, LOW);
      }
    }
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
  display.print("ADC: ");
  display.print(current);
  display.setCursor(0, 31);

  if (pumping1){
    display.print("Pumping Tank 1");
  }
  else if (pumping2){
    display.print("Pumping Tank 2");
  }
  else {
    display.print("System Idle");
  }

  display.display();

  delay(500); // Update the display every 500 ms
}

// Helper function to display error messages
void displayErrorMessage(String message) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Error: ");
  display.print(message);
  display.display();
}