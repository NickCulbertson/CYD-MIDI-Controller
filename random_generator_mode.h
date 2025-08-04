#ifndef RANDOM_GENERATOR_MODE_H
#define RANDOM_GENERATOR_MODE_H

#include "common_definitions.h"
#include "ui_elements.h"
#include "midi_utils.h"

// Random Generator mode variables
struct RandomGen {
  int rootNote = 60; // C4
  int scaleType = 0; // Major
  int minOctave = 3;
  int maxOctave = 6;
  int probability = 50; // 0-100%
  int bpm = 120; // BPM instead of interval
  int subdivision = 4; // 4=quarter, 8=eighth, 16=sixteenth
  bool isPlaying = false;
  unsigned long lastNoteTime = 0;
  unsigned long nextNoteTime = 0;
  int currentNote = -1;
  unsigned long noteInterval = 500; // Calculated from BPM
};

RandomGen randomGen;

// Function declarations
void initializeRandomGeneratorMode();
void drawRandomGeneratorMode();
void handleRandomGeneratorMode();
void drawRandomGenControls();
void updateRandomGenerator();
void playRandomNote();
void calculateNoteInterval();

// Implementations
void initializeRandomGeneratorMode() {
  randomGen.rootNote = 60;
  randomGen.scaleType = 0;
  randomGen.minOctave = 3;
  randomGen.maxOctave = 6;
  randomGen.probability = 50;
  randomGen.bpm = 120;
  randomGen.subdivision = 4;
  randomGen.isPlaying = false;
  randomGen.currentNote = -1;
  calculateNoteInterval();
  randomGen.nextNoteTime = millis() + randomGen.noteInterval;
}

void drawRandomGeneratorMode() {
  tft.fillScreen(THEME_BG);
  drawHeader("RNG JAMS", "Random Music");
  
  drawRandomGenControls();
}

void drawRandomGenControls() {
  int y = 55;
  int spacing = 22;
  
  // Play/Stop and Root note on same line
  drawRoundButton(10, y, 60, 25, randomGen.isPlaying ? "STOP" : "PLAY", 
                 randomGen.isPlaying ? THEME_ERROR : THEME_SUCCESS);
  
  tft.setTextColor(THEME_TEXT, THEME_BG);
  tft.drawString("Key:", 80, y + 6, 1);
  String rootName = getNoteNameFromMIDI(randomGen.rootNote);
  drawRoundButton(110, y, 35, 25, rootName, THEME_PRIMARY);
  drawRoundButton(150, y, 25, 25, "+", THEME_SECONDARY);
  drawRoundButton(180, y, 25, 25, "-", THEME_SECONDARY);
  
  // Scale selector
  drawRoundButton(220, y, 80, 25, scales[randomGen.scaleType].name, THEME_ACCENT);
  
  y += spacing + 5;
  
  // Octave range
  tft.drawString("Oct:", 10, y + 6, 1);
  tft.drawString(String(randomGen.minOctave) + "-" + String(randomGen.maxOctave), 35, y + 6, 1);
  drawRoundButton(70, y, 35, 25, "MIN-", THEME_SECONDARY);
  drawRoundButton(110, y, 35, 25, "MIN+", THEME_SECONDARY);
  drawRoundButton(150, y, 35, 25, "MAX-", THEME_SECONDARY);
  drawRoundButton(190, y, 35, 25, "MAX+", THEME_SECONDARY);
  
  y += spacing + 5;
  
  // Probability with visual bar
  tft.drawString("Chance:", 10, y + 6, 1);
  tft.drawString(String(randomGen.probability) + "%", 55, y + 6, 1);
  drawRoundButton(85, y, 25, 25, "-", THEME_SECONDARY);
  drawRoundButton(115, y, 25, 25, "+", THEME_SECONDARY);
  
  // Compact probability bar - clear and redraw
  int barW = 80;
  int barX = 145;
  tft.fillRect(barX, y + 8, barW, 10, THEME_BG); // Clear old bar
  tft.drawRect(barX, y + 8, barW, 10, THEME_TEXT_DIM);
  int fillW = (barW * randomGen.probability) / 100;
  if (fillW > 0) {
    tft.fillRect(barX + 1, y + 9, fillW, 8, THEME_PRIMARY);
  }
  
  y += spacing + 5;
  
  // BPM and subdivision controls
  tft.drawString("BPM:", 10, y + 6, 1);
  tft.drawString(String(randomGen.bpm), 40, y + 6, 1);
  drawRoundButton(65, y, 25, 25, "-", THEME_SECONDARY);
  drawRoundButton(95, y, 25, 25, "+", THEME_SECONDARY);
  
  tft.drawString("Beat:", 125, y + 6, 1);
  String subdivText;
  if (randomGen.subdivision == 4) subdivText = "1/4";
  else if (randomGen.subdivision == 8) subdivText = "1/8";
  else if (randomGen.subdivision == 16) subdivText = "1/16";
  tft.drawString(subdivText, 160, y + 6, 1);
  drawRoundButton(190, y, 25, 25, "<", THEME_SECONDARY);
  drawRoundButton(220, y, 25, 25, ">", THEME_SECONDARY);
  
  y += spacing + 5;
  
  // Current note indicator (compact)
  if (randomGen.currentNote != -1) {
    tft.setTextColor(THEME_PRIMARY, THEME_BG);
    tft.drawString("Now: ", 10, y, 1);
    String currentNoteName = getNoteNameFromMIDI(randomGen.currentNote);
    tft.setTextColor(THEME_ACCENT, THEME_BG);
    tft.drawString(currentNoteName, 45, y, 2);
  }
}

void handleRandomGeneratorMode() {
  // Back button
  if (touch.justPressed && isButtonPressed(10, 10, 50, 25)) {
    exitToMenu();
    return;
  }
  
  if (touch.justPressed) {
    int y = 55;
    int spacing = 22;
    
    // Play/Stop and Root note controls
    if (isButtonPressed(10, y, 60, 25)) {
      randomGen.isPlaying = !randomGen.isPlaying;
      if (randomGen.isPlaying) {
        randomGen.nextNoteTime = millis() + randomGen.noteInterval;
      } else if (randomGen.currentNote != -1) {
        sendMIDI(0x80, randomGen.currentNote, 0);
        randomGen.currentNote = -1;
      }
      drawRandomGenControls();
      return;
    }
    
    if (isButtonPressed(150, y, 25, 25)) {
      randomGen.rootNote = min(127, randomGen.rootNote + 1);
      drawRandomGenControls();
      return;
    }
    if (isButtonPressed(180, y, 25, 25)) {
      randomGen.rootNote = max(0, randomGen.rootNote - 1);
      drawRandomGenControls();
      return;
    }
    
    // Scale selector
    if (isButtonPressed(220, y, 80, 25)) {
      randomGen.scaleType = (randomGen.scaleType + 1) % NUM_SCALES;
      drawRandomGenControls();
      return;
    }
    
    y += spacing + 5;
    
    // Octave controls
    if (isButtonPressed(70, y, 35, 25)) {
      randomGen.minOctave = max(1, randomGen.minOctave - 1);
      if (randomGen.minOctave >= randomGen.maxOctave) {
        randomGen.maxOctave = randomGen.minOctave + 1;
      }
      drawRandomGenControls();
      return;
    }
    if (isButtonPressed(110, y, 35, 25)) {
      randomGen.minOctave = min(8, randomGen.minOctave + 1);
      if (randomGen.minOctave >= randomGen.maxOctave) {
        randomGen.maxOctave = randomGen.minOctave + 1;
      }
      drawRandomGenControls();
      return;
    }
    if (isButtonPressed(150, y, 35, 25)) {
      randomGen.maxOctave = max(randomGen.minOctave + 1, randomGen.maxOctave - 1);
      drawRandomGenControls();
      return;
    }
    if (isButtonPressed(190, y, 35, 25)) {
      randomGen.maxOctave = min(9, randomGen.maxOctave + 1);
      drawRandomGenControls();
      return;
    }
    
    y += spacing + 5;
    
    // Probability controls
    if (isButtonPressed(85, y, 25, 25)) {
      randomGen.probability = max(0, randomGen.probability - 5);
      drawRandomGenControls();
      return;
    }
    if (isButtonPressed(115, y, 25, 25)) {
      randomGen.probability = min(100, randomGen.probability + 5);
      drawRandomGenControls();
      return;
    }
    
    y += spacing + 5;
    
    // BPM controls
    if (isButtonPressed(65, y, 25, 25)) {
      randomGen.bpm = max(60, randomGen.bpm - 5);
      calculateNoteInterval();
      drawRandomGenControls();
      return;
    }
    if (isButtonPressed(95, y, 25, 25)) {
      randomGen.bpm = min(200, randomGen.bpm + 5);
      calculateNoteInterval();
      drawRandomGenControls();
      return;
    }
    
    // Subdivision controls
    if (isButtonPressed(190, y, 25, 25)) {
      if (randomGen.subdivision == 16) randomGen.subdivision = 8;
      else if (randomGen.subdivision == 8) randomGen.subdivision = 4;
      calculateNoteInterval();
      drawRandomGenControls();
      return;
    }
    if (isButtonPressed(220, y, 25, 25)) {
      if (randomGen.subdivision == 4) randomGen.subdivision = 8;
      else if (randomGen.subdivision == 8) randomGen.subdivision = 16;
      calculateNoteInterval();
      drawRandomGenControls();
      return;
    }
  }
  
  // Update random generator
  updateRandomGenerator();
}

void updateRandomGenerator() {
  if (!randomGen.isPlaying || !deviceConnected) return;
  
  unsigned long now = millis();
  
  if (now >= randomGen.nextNoteTime) {
    playRandomNote();
    randomGen.nextNoteTime = now + randomGen.noteInterval;
  }
}

void playRandomNote() {
  // Stop current note if playing
  if (randomGen.currentNote != -1) {
    sendMIDI(0x80, randomGen.currentNote, 0);
    randomGen.currentNote = -1;
  }
  
  // Check probability
  if (random(100) < randomGen.probability) {
    // Generate random note in scale and octave range
    Scale& scale = scales[randomGen.scaleType];
    int degree = random(scale.numNotes);
    int octave = random(randomGen.minOctave, randomGen.maxOctave + 1);
    int note = randomGen.rootNote % 12 + scale.intervals[degree] + (octave * 12);
    
    if (note >= 0 && note <= 127) {
      sendMIDI(0x90, note, 100);
      randomGen.currentNote = note;
      
      Serial.printf("Random note: %s (prob: %d%%)\n", 
                   getNoteNameFromMIDI(note).c_str(), randomGen.probability);
      
      // Update display
      drawRandomGenControls();
    }
  }
}

void calculateNoteInterval() {
  // Calculate note interval from BPM and subdivision
  float beatsPerSecond = randomGen.bpm / 60.0;
  float notesPerSecond = beatsPerSecond * (randomGen.subdivision / 4.0);
  randomGen.noteInterval = 1000.0 / notesPerSecond;
}

#endif