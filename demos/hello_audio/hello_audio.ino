// MODIFIED FROM SIMPLE DRUM EXAMPLE
#include <Audio.h>
#include <SD.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <Wire.h>

// GUItool: begin automatically generated code
AudioSynthSimpleDrum drum;  // xy=399,244
AudioOutputI2S i2s1;        // xy=979,214
AudioConnection patchCord1(drum, 0, i2s1, 0);
AudioConnection patchCord2(drum, 0, i2s1, 1);
AudioControlSGTL5000 sgtl5000_1;  // xy=930,518
// GUItool: end automatically generated code

static uint32_t next;

void setup() {
  Serial.begin(115200);

  // audio library init
  AudioMemory(15);

  next = millis() + 1000;

  AudioNoInterrupts();

  drum.frequency(60);
  drum.length(300);
  drum.secondMix(0.0);
  drum.pitchMod(1.0);

  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);

  AudioInterrupts();

  pinMode(28, INPUT_PULLUP);
}

void loop() {
  if (millis() < next) {
    return;
  }
  next = millis() + 1000;

  if (digitalRead(28) == LOW) {
    drum.frequency(60);
  } else {
    drum.frequency(120);
  }
  drum.noteOn();

  Serial.print("Diagnostics: ");
  Serial.print(AudioProcessorUsageMax());
  Serial.print(" ");
  Serial.println(AudioMemoryUsageMax());
  AudioProcessorUsageMaxReset();
}
