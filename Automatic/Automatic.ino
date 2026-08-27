// --- Configurable Fault Timeouts (milliseconds) ---
unsigned long TANK_FAULT_TIMEOUT = 15UL * 60UL * 1000UL;  // 15 minutes
unsigned long SAND_FAULT_TIMEOUT = 45UL * 60UL * 1000UL;  // 45 minutes
// Maybe self explanatory but used to ensure that if the pump run command 
// is being sent and no water level reduction is being detected to pause the system until an operator resets it
// Use shorter times for debugging (e.g. 5000 and 10000 ms)

// --- Pin Assignments ---
// the first xxxPin variable is the corresponding level sensor
// the second xxxLED is a status LED the shows the state of the corresponding level sensor
// the third xxxSSR is the pin for operating the SSR
// the fourth xxxSSRLED is a status LED for the state of the SSR firing, I'm not sure if this is implemented in hardware
const int tankPin = 3, tankLED = 2, tankSSR = 14, tankSSRLED = 15;
const int sandPin = 5, sandLED = 4, sandSSR = 10, sandSSRLED = 16;

// --- State Variables ---
bool tankFault = false, sandFault = false;
unsigned long tankStart = 0, sandStart = 0;

void setup() {
  pinMode(tankPin, INPUT);     pinMode(tankLED, OUTPUT);
  pinMode(tankSSR, OUTPUT);    pinMode(tankSSRLED, OUTPUT);
  pinMode(sandPin, INPUT);     pinMode(sandLED, OUTPUT);
  pinMode(sandSSR, OUTPUT);    pinMode(sandSSRLED, OUTPUT);
}

void loop() {
  // find state of both tanks
  bool tankVal = digitalRead(tankPin);
  bool sandVal = digitalRead(sandPin);

  unsigned long now = millis();

  // Default: SSRs off
  bool tankSSRActive = false, sandSSRActive = false;

  // Fault state: disable everything and blink faulting SSR LED
  if (tankFault || sandFault) {
    handleFaultLEDs(now, tankVal, sandVal);
    delay(100);
    return;
  }

  // LED feedback (steady state)
  digitalWrite(tankLED, tankVal);
  digitalWrite(sandLED, sandVal);

  // Decide which SSR should be active
  if (sandVal) sandSSRActive = true;
  else if (tankVal) tankSSRActive = true;

  // Apply SSR outputs
  setSSRState(tankSSR, tankSSRLED, tankSSRActive);
  setSSRState(sandSSR, sandSSRLED, sandSSRActive);

  // Fault detection only while SSR is active and input is HIGH
  tankFault = checkFault(tankVal, tankSSRActive, tankStart, now, TANK_FAULT_TIMEOUT);
  sandFault = checkFault(sandVal, sandSSRActive, sandStart, now, SAND_FAULT_TIMEOUT);

  delay(100);
}

// --- Helper: Set SSR and its LED ---
void setSSRState(int ssrPin, int ledPin, bool state) {
  digitalWrite(ssrPin, state);
  digitalWrite(ledPin, state);
}

// --- Helper: Check fault condition ---
bool checkFault(bool sensorHigh, bool ssrActive, unsigned long &startTime, unsigned long now, unsigned long timeout) {
  if (sensorHigh && ssrActive) {
    if (startTime == 0) startTime = now;
    if (now - startTime >= timeout) return true;
  } else {
    startTime = 0;
  }
  return false;
}

// --- Helper: Blink SSR LED for faults ---
void handleFaultLEDs(unsigned long now, bool tankVal, bool sandVal) {
  digitalWrite(tankSSR, LOW);
  digitalWrite(sandSSR, LOW);
  digitalWrite(tankLED, tankVal);
  digitalWrite(sandLED, sandVal);

  digitalWrite(tankSSRLED, tankFault ? (now / 500) % 2 : LOW);
  digitalWrite(sandSSRLED, sandFault ? (now / 500) % 2 : LOW);
}