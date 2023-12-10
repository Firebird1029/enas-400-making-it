#include <Adafruit_PWMServoDriver.h>
#include <Audio.h>
#include <Bounce2.h>
#include <SD.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <Wire.h>
#include <elapsedMillis.h>

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
#define WAVEFORM_VOLUME 0.7
#define RANDOM_OFFSET 1  // 0 for no randomness
#define ENV_ATTACK 10.5
#define ENV_HOLD 30    // 2.5
#define ENV_DECAY 400  // 35
#define ENV_SUSTAIN 0  // 0.8
#define ENV_RELEASE 300

enum SCALE_TYPE { MAJOR, MINOR, PENTATONIC };

// PINOUT
#define BTN_1_PIN 32
#define BTN_2_PIN 31
#define BTN_3_PIN 30
#define BTN_4_PIN 29
#define BTN_5_PIN 28
#define BTN_6_PIN 33

// BUTTONS
#define NUM_KEYS 6
Button allButtons[NUM_KEYS] = {Button(), Button(), Button(),
                               Button(), Button(), Button()};
const int BTN_PINS[NUM_KEYS] = {BTN_1_PIN, BTN_2_PIN, BTN_3_PIN,
                                BTN_4_PIN, BTN_5_PIN, BTN_6_PIN};

// SERVOS
// This is the 'minimum' pulse length count (out of 4096)
#define SERVO_MIN 150  // 150
// This is the 'maximum' pulse length count (out of 4096)
#define SERVO_MAX 250  // 600
// This is the rounded 'minimum' microsecond length based on the minimum pulse
// of 150
#define SERVO_USMIN 600
// This is the rounded 'maximum' microsecond length based on the maximum pulse
// of 600
#define SERVO_USMAX 2400
// Analog servos run at ~50 Hz updates
#define SERVO_FREQ 50

#define FEATHER_HIDDEN SERVO_MAX
#define FEATHER_SHOWN SERVO_MIN

Adafruit_PWMServoDriver pwm =
    Adafruit_PWMServoDriver(0x40, Wire2);  // use the 3RD I2C bus on Teensy 4.1
// first I2C bus (Wire) conflicts with audio shield, second (Wire1) doesn't have
// PWM, so use third (Wire2)

// 0 -> feather already hidden, if >0 -> hide feather at this timestamp
unsigned long int lastFeatherAction =
    0;  // timestamp of last time feather popped up
unsigned long int hideFeatherAt[NUM_KEYS] = {0, 0, 0, 0, 0, 0};

// GLOBAL VARIABLES
elapsedMillis timer;         // master timer (auto-incrementing)
int lastButtonPressed = -1;  // for random scales
int lastNote = 0;            // for random scales
int playMode = 10;           // TODO change back to 1
/*
 * PLAY MODES:
 * 1 = major scale, random feathers
 * 10 = set track from SD card, random feathers
 */
unsigned long int stopAudioTrackAt = 0;

// PJRC CODE

// GUItool: begin automatically generated code
AudioPlaySdWav playSdWav1;      // xy=90,252
AudioSynthWaveform waveform1;   // xy=107.5,85
AudioEffectFade fade1;          // xy=250,246
AudioEffectEnvelope envelope1;  // xy=282.5,85
AudioMixer4 mixer1;             // xy=600,165
AudioOutputI2S i2s1;            // xy=746.5,167
AudioConnection patchCord1(playSdWav1, 0, fade1, 0);
AudioConnection patchCord2(waveform1, envelope1);
AudioConnection patchCord3(fade1, 0, mixer1, 1);
AudioConnection patchCord4(envelope1, 0, mixer1, 0);
AudioConnection patchCord5(mixer1, 0, i2s1, 0);
AudioConnection patchCord6(mixer1, 0, i2s1, 1);
AudioControlSGTL5000 sgtl5000_1;  // xy=64.5,20
// GUItool: end automatically generated code

void setup(void) {
  // SERIAL SETUP
  Serial.begin(9600);  // 115200
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

  // MIXER SETUP
  mixer1.gain(0, 1.0);  // piano
  mixer1.gain(1, 1.0);  // set track from SD
  mixer1.gain(2, 0.0);
  mixer1.gain(3, 0.0);

  // FINISH AUDIO SETUP
  sgtl5000_1.enable();
  sgtl5000_1.volume(MASTER_VOLUME);
  AudioInterrupts();

  // BUTTON SETUP
  for (int i = 0; i < NUM_KEYS; i++) {
    allButtons[i].attach(BTN_PINS[i], INPUT_PULLUP);
    allButtons[i].interval(5);
    allButtons[i].setPressedState(LOW);
  }

  // SERVO SETUP
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);  // Analog servos run at ~50 Hz updates

  // SD SETUP
  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("Unable to access SD Card");
    while (true) {
      delay(10);
    }
  }

  // SETUP DONE
  Serial.println("Finished setup.");
  delay(100);
}

void loop() {
  buttonCode();
  servoCode();

  // stop SD audio track if applicable
  if (playMode == 10 && stopAudioTrackAt != 0 && timer >= stopAudioTrackAt) {
    Serial.println("Stopping audio track");
    fade1.fadeOut(5000);
    stopAudioTrackAt = 0;
  }

  delay(10);
}

// BUTTON CODE
void buttonCode() {
  // update buttons
  for (int i = 0; i < NUM_KEYS; i++) {
    allButtons[i].update();
  }

  // if a button is pressed, play a note
  for (int i = 0; i < NUM_KEYS; i++) {
    if (allButtons[i].fell()) {
      Serial.print("Button ");
      Serial.print(i);
      Serial.println(" pressed");

      int semitones;
      switch (playMode) {
        case 1:
          // major scale

          // static scale
          //   int semitones = mapScale(PENTATONIC, i);
          semitones = mapScale(MAJOR, i);

          // random melody
          //   lastNote = mapRandomScale(PENTATONIC, lastNote, i -
          //   lastButtonPressed);

          //   // TODO: this depends on BASE_FREQ!
          //   // clip lastNote between -10 and 30
          //   lastNote = max(-10, min(30, lastNote));
          //   int semitones = lastNote;

          // play note
          Serial.print("Playing note ");
          Serial.println(semitones);

          waveform1.frequency(BASE_FREQ * pow(2, semitones / 12.0));
          envelope1.noteOn();
          break;

        case 10:
          // set track from SD card
          if (stopAudioTrackAt == 0) {
            // audio not currently playing, so play audio
            Serial.println("Playing mariah.wav");
            fade1.fadeIn(100);

            // start track if not playing, else resume from where
            // it left off
            if (!playSdWav1.isPlaying()) {
              playSdWav1.play("mariah.wav");
            }
          }

          stopAudioTrackAt = timer + 10000;  // stop track 10 sec later

          break;

        default:
          break;
      }

      lastButtonPressed = i;
    }
  }
}

// SERVO CODE
void servoCode() {
  // hide feather if its hide time has arrived
  for (int i = 0; i < NUM_KEYS; i++) {
    if (hideFeatherAt[i] != 0 && timer >= hideFeatherAt[i]) {
      Serial.print("Hiding feather ");
      Serial.println(i);
      hideFeatherAt[i] = 0;
      pwm.setPWM(i, 0, FEATHER_HIDDEN);
    }
  }

  // (potentially) show random feather every 2-3 seconds
  if (timer >= lastFeatherAction + random(2000, 3000)) {
    lastFeatherAction = timer;
    int randomFeather = random(0, NUM_KEYS);
    // if feather is already shown, ignore
    if (hideFeatherAt[randomFeather] != 0) {
      return;
    }
    Serial.print("Showing feather ");
    Serial.println(randomFeather);
    pwm.setPWM(randomFeather, 0, FEATHER_SHOWN);
    hideFeatherAt[randomFeather] =
        timer + random(2000, 4000);  // hide feather after 2-4 seconds
  }
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
