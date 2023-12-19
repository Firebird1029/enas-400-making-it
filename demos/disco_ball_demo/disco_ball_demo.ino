// Pin 13 has an LED connected on most Arduino boards.
// Pin 11 has the LED on Teensy 2.0
// Pin 6  has the LED on Teensy++ 2.0
// Pin 13 has the LED on Teensy 3.0
// give it a name:
int led = 13;

int numLasers = 3;
int laserPins[] = {2, 3, 4};
int laserPinStatus[] = {LOW, LOW, LOW};
int primes[] = {7, 11, 17};

void setup() {
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);

  for (size_t i = 0; i < numLasers; i++) {
    pinMode(laserPins[i], OUTPUT);
  }
}

int globalCount = 0;

void loop() {
  for (size_t i = 0; i < numLasers; i++) {
    if (globalCount % primes[i]) {
      digitalWrite(laserPins[i], laserPinStatus[i]);
      laserPinStatus[i] = !laserPinStatus[i];
    }
  }

  globalCount = (globalCount + 1) % 10000;
  delay(100);
}
