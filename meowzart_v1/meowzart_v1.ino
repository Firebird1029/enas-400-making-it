#include <Audio.h>
#include <Bounce2.h>
#include <SD.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <Wire.h>

/*
 * FUTURE
 *
 * new timbres, button to change timbre
 * volume knob
 * sounds from SD card (e.g. bird call)
 */

#define MASTER_VOLUME 0.5  // <= 0.8 to prevent distortion

#define BASE_FREQ 349.23  // F4, default tonic

// MUSIC
#define WAVEFORM_VOLUME 0.3
#define RANDOM_OFFSET 2  // 0 for no randomness
#define ENV_ATTACK 10.5
#define ENV_HOLD 30    // 2.5
#define ENV_DECAY 400  // 35
#define ENV_SUSTAIN 0  // 0.8
#define ENV_RELEASE 300

enum SCALE_TYPE { MAJOR, MINOR, PENTATONIC };

// PINOUT
#define BTN_1_PIN 28
#define BTN_2_PIN 29
#define BTN_3_PIN 30
#define BTN_4_PIN 31
#define BTN_5_PIN 32

// BUTTONS
Button btn1 = Button();
Button btn2 = Button();
Button btn3 = Button();
Button btn4 = Button();
Button btn5 = Button();

#define NUM_BUTTONS 5
Button allButtons[] = {btn1, btn2, btn3, btn4, btn5};
const int BTN_PINS[] = {BTN_1_PIN, BTN_2_PIN, BTN_3_PIN, BTN_4_PIN, BTN_5_PIN};

// GLOBAL VARIABLES
// elapsedMillis timer;  // master timer (auto-incrementing)
int lastButtonPressed = -1;  // for random scales
int lastNote = 0;            // for random scales

// PJRC CODE

// GUItool: begin automatically generated code
AudioSynthWaveform waveform1;   // xy=107,85
AudioEffectEnvelope envelope1;  // xy=282,85
AudioOutputI2S i2s1;            // xy=457,85
AudioConnection patchCord1(waveform1, envelope1);
AudioConnection patchCord2(envelope1, 0, i2s1, 0);
AudioConnection patchCord3(envelope1, 0, i2s1, 1);
AudioControlSGTL5000 sgtl5000_1;  // xy=64.5,20
// GUItool: end automatically generated code

void setup(void) {
  // SERIAL SETUP
  Serial.begin(115200);
  Serial.println("Serial started");

  // INIT AUDIO SETUP
  AudioMemory(200);
  AudioNoInterrupts();

  // SYNTH SETUP
  waveform1.begin(WAVEFORM_VOLUME, BASE_FREQ, WAVEFORM_SINE);
  envelope1.attack(ENV_ATTACK);
  envelope1.hold(ENV_HOLD);
  envelope1.decay(ENV_DECAY);
  envelope1.sustain(ENV_SUSTAIN);
  envelope1.release(ENV_RELEASE);

  // FINISH AUDIO SETUP
  sgtl5000_1.enable();
  sgtl5000_1.volume(MASTER_VOLUME);
  AudioInterrupts();

  // BUTTON SETUP
  btn1.attach(BTN_1_PIN, INPUT_PULLUP);
  btn1.interval(5);
  btn1.setPressedState(LOW);
  for (int i = 0; i < NUM_BUTTONS; i++) {
    allButtons[i].attach(BTN_PINS[i], INPUT_PULLUP);
    allButtons[i].interval(5);
    allButtons[i].setPressedState(LOW);
  }

  // SETUP DONE
  Serial.println("Finished setup.");
  delay(100);
}

void loop() {
  // update buttons
  for (int i = 0; i < NUM_BUTTONS; i++) {
    allButtons[i].update();
  }

  // if a button is pressed, play a note
  for (int i = 0; i < NUM_BUTTONS; i++) {
    if (allButtons[i].fell()) {
      Serial.print("Button ");
      Serial.print(i);
      Serial.println(" pressed");

      // static scale
      // int semitones = mapScale(PENTATONIC, i);

      // random melody
      lastNote = mapRandomScale(PENTATONIC, lastNote, i - lastButtonPressed);

      // TODO: this depends on BASE_FREQ!
      // clip lastNote between -10 and 30
      lastNote = max(-10, min(30, lastNote));
      int semitones = lastNote;

      // play note
      Serial.print("Playing note ");
      Serial.println(semitones);

      waveform1.frequency(BASE_FREQ * pow(2, semitones / 12.0));
      envelope1.noteOn();

      lastButtonPressed = i;
    }
  }

  delay(10);
}

// HELPER FUNCTIONS
const int majorScale[8] = {0, 2, 4, 5, 7, 9, 11, 12};
const int minorScale[8] = {0, 2, 3, 5, 7, 8, 10, 12};
const int pentatonicScale[12] = {0, 2, 4, 7, 9, 12, 14, 16, 19, 21, 24};
int mapScale(SCALE_TYPE scaleType, int step) {
  if (step == -1) {
    return -1;
  }
  if (scaleType == MAJOR) {
    // convert step (from 0-6 inclusive) to a major scale
    return majorScale[step];
  } else if (scaleType == MINOR) {
    // convert step (from 0-6 inclusive) to a minor scale
    return minorScale[step];
  } else if (scaleType == PENTATONIC) {
    // convert step (from 0-5 inclusive) to a pentatonic scale
    return pentatonicScale[step];
  }

  return step;
}

int mapRandomScale(SCALE_TYPE scaleType, int step, int stepChange) {
  if (stepChange == 0) {
    // if you hit same note, play same note
    return step;
  }
  if (scaleType == PENTATONIC) {
    // if RANDOM_OFFSET is 1 and:
    // if stepChange is 1 (adjacent piano key was pressed), then simulate
    // pressing a piano key 1 to 1+RANDOM_OFFSET=2 (inclusive) steps away
    // if stepChange is 2 (piano key 2 steps away was pressed), then simulate
    // pressing a piano key 1 to 2+RANDOM_OFFSET=3 (inclusive) steps away
    int delta = random(1, abs(stepChange) + RANDOM_OFFSET + 1);

    // (current semitones away from base freq) + (semitones given steps away
    // from current note) * (direction)
    return step + pentatonicScale[delta] * (stepChange > 0 ? 1 : -1);
  }

  return step;
}
