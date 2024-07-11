#include <Arduino.h>
#include "tone.h"

void Tone::OK() {
  for (int thisNote = 0; thisNote < 5; thisNote++) {
    int noteDuration = 500 / noteDurations[thisNote];
    tone(pinTone, melodyOK[thisNote], noteDuration);

    int pauseBetweenNotes = noteDuration * 1;
    delay(pauseBetweenNotes);
    
    noTone(5);
  }
}

void Tone::NG() {
  for (int thisNote = 0; thisNote < 5; thisNote++) {
    int noteDuration = 200 / noteDurations[thisNote];
    tone(pinTone, melodyNG[thisNote], noteDuration);

    int pauseBetweenNotes = noteDuration * 0.5;
    delay(pauseBetweenNotes);
    
    noTone(5);
  }
}

void Tone::WARNING() {
  for (int thisNote = 0; thisNote < 5; thisNote++) {
    int noteDuration = 200 / noteDurations[thisNote];
    tone(pinTone, melodyNG[thisNote], noteDuration);

    int pauseBetweenNotes = noteDuration * 5;
    delay(pauseBetweenNotes);
    
    noTone(5);
  }
}