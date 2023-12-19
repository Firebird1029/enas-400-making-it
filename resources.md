# Resources

## References

- [https://learn.adafruit.com/16-channel-pwm-servo-driver?view=all]
- [https://github.com/pfeerick/elapsedMillis/wiki]

## SD Card

To put a set track on SD card:

1. Download Youtube video as a mp3 / wav file.
2. Convert the audio file to a wav file with Teensy standards.
   1. Online converter: [https://onlineaudioconverter.com/]
   2. Teensy standards: [https://www.pjrc.com/teensy/gui/?info=AudioPlaySdWav]
      1. Summary: "Only 16 bit PCM, 44100 Hz WAV files are supported. When mono files are played, both output ports transmit a copy of the single sound. Of course, stereo WAV files play with the left channel on port 0 and the right channel on port 1."
3. Put the wav file on the SD card.
4. Put the SD card in the Teensy, NOT the audio shield!

## I2C Bus Conflict

- [https://forums.adafruit.com/viewtopic.php?t=153374]
- [https://forum.pjrc.com/index.php?threads/teensy-4-0-use-of-2-different-i2c-bus.59547/]
- [https://forums.adafruit.com/viewtopic.php?t=141337]

## Misc

- [https://forum.fritzing.org/t/teensy-4-1-part/11035/8]
- [https://learn.adafruit.com/adafruit-neopixel-uberguide/best-practices]
