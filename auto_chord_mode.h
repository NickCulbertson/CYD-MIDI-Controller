#ifndef AUTO_CHORD_MODE_H
#define AUTO_CHORD_MODE_H

#include "common_definitions.h"
#include "ui_elements.h"
#include "midi_utils.h"

// Auto Chord mode variables - traditional piano chords
struct ChordType {
  String name;
  int intervals[4]; // Root, 3rd, 5th, optional 7th
  int numNotes;
};

// Traditional chord types - classic piano voicings
ChordType diatonicChords[] = {
  {"I", {0, 4, 7, -1}, 3},      // Major triad
  {"ii", {0, 3, 7, -1}, 3},     // Minor triad  
  {"iii", {0, 3, 7, -1}, 3},    // Minor triad
  {"IV", {0, 4, 7, -1}, 3},     // Major triad
  {"V", {0, 4, 7, -1}, 3},      // Major triad
  {"vi", {0, 3, 7, -1}, 3},     // Minor triad
  {"vii°", {0, 3, 6, -1}, 3},   // Diminished triad
  {"I+", {0, 4, 7, -1}, 3}      // Major triad (octave)
};

int chordOctave = 4;
int chordScale = 0;
int activeChordNotes[8][4]; // [chord][note index]
bool chordPressed[8] = {false}; // 8 diatonic chords

// Function declarations
void initializeAutoChordMode();
void drawAutoChordMode();
void handleAutoChordMode();
void drawChordKeys();
void playChord(int scaleDegree, bool on);
void stopAllChords();

// Implementations
void initializeAutoChordMode() {
  chordOctave = 4;
  chordScale = 0;
  stopAllChords();
  for (int i = 0; i < 8; i++) {
    chordPressed[i] = false;
    for (int j = 0; j < 4; j++) {
      activeChordNotes[i][j] = -1;
    }
  }
}

void drawAutoChordMode() {
  tft.fillScreen(THEME_BG);
  drawHeader("CHORD MODE", scales[chordScale].name + " Diatonic");
  
  drawChordKeys();
  
  // Controls
  drawRoundButton(10, 180, 40, 25, "OCT-", THEME_SECONDARY);
  drawRoundButton(60, 180, 40, 25, "OCT+", THEME_SECONDARY);
  drawRoundButton(110, 180, 60, 25, "SCALE", THEME_ACCENT);
  drawRoundButton(180, 180, 60, 25, "CLEAR", THEME_ERROR);
  
  // Status
  tft.setTextColor(THEME_TEXT_DIM, THEME_BG);
  tft.drawString("Oct " + String(chordOctave), 10, 210, 1);
  tft.drawString("Classic piano chords", 110, 210, 1);
}

void drawChordKeys() {
  int keyWidth = 320 / 8;
  int keyHeight = 80;
  int keyY = 80;
  
  uint16_t degreeColors[] = {
    THEME_PRIMARY, THEME_SECONDARY, THEME_ACCENT, THEME_SUCCESS,
    THEME_WARNING, THEME_ERROR, 0xF81F, 0x07E0
  };
  
  for (int i = 0; i < 8; i++) {
    int x = i * keyWidth;
    
    uint16_t bgColor = chordPressed[i] ? degreeColors[i] : THEME_SURFACE;
    uint16_t textColor = chordPressed[i] ? THEME_BG : degreeColors[i];
    
    tft.fillRect(x + 2, keyY + 2, keyWidth - 4, keyHeight - 4, bgColor);
    tft.drawRect(x, keyY, keyWidth, keyHeight, degreeColors[i]);
    tft.drawRect(x + 1, keyY + 1, keyWidth - 2, keyHeight - 2, degreeColors[i]);
    
    // Roman numeral
    tft.setTextColor(textColor, bgColor);
    tft.drawCentreString(diatonicChords[i].name, x + keyWidth/2, keyY + 20, 4);
    
    // Root note name
    int rootNote;
    if (i == 7) { // I+ octave
      rootNote = getNoteInScale(chordScale, 0, chordOctave + 1);
    } else {
      rootNote = getNoteInScale(chordScale, i, chordOctave);
    }
    String rootName = getNoteNameFromMIDI(rootNote);
    tft.drawCentreString(rootName, x + keyWidth/2, keyY + 50, 2);
  }
}

void handleAutoChordMode() {
  // Back button
  if (touch.justPressed && isButtonPressed(10, 10, 50, 25)) {
    exitToMenu();
    return;
  }
  
  if (touch.justPressed) {
    // Octave controls
    if (isButtonPressed(10, 180, 40, 25)) {
      chordOctave = max(2, chordOctave - 1);
      drawAutoChordMode();
      return;
    }
    if (isButtonPressed(60, 180, 40, 25)) {
      chordOctave = min(6, chordOctave + 1);
      drawAutoChordMode();
      return;
    }
    
    // Scale selector
    if (isButtonPressed(110, 180, 60, 25)) {
      chordScale = (chordScale + 1) % NUM_SCALES;
      drawAutoChordMode();
      return;
    }
    
    // Clear all
    if (isButtonPressed(180, 180, 60, 25)) {
      stopAllChords();
      drawChordKeys();
      return;
    }
    
    // Chord keys - only handle on initial press
    int keyWidth = 320 / 8;
    int keyHeight = 80;
    int keyY = 80;
    
    for (int i = 0; i < 8; i++) {
      int x = i * keyWidth;
      if (isButtonPressed(x, keyY, keyWidth, keyHeight)) {
        if (!chordPressed[i]) {
          // Turn on chord
          playChord(i, true);
          chordPressed[i] = true;
          drawChordKeys();
        }
        return;
      }
    }
  }
  
  // Handle single key press/hold functionality
  if (touch.isPressed) {
    int keyWidth = 320 / 8;
    int keyHeight = 80;
    int keyY = 80;
    
    int currentKey = -1;
    
    // Find which key is being pressed
    for (int i = 0; i < 8; i++) {
      int x = i * keyWidth;
      if (touch.x >= x && touch.x < x + keyWidth && 
          touch.y >= keyY && touch.y < keyY + keyHeight) {
        currentKey = i;
        break;
      }
    }
    
    // Only allow one chord at a time
    if (currentKey != -1) {
      // Turn off all other chords first
      for (int i = 0; i < 8; i++) {
        if (i != currentKey && chordPressed[i]) {
          playChord(i, false);
          chordPressed[i] = false;
        }
      }
      
      // Turn on the current chord if not already on
      if (!chordPressed[currentKey]) {
        playChord(currentKey, true);
        chordPressed[currentKey] = true;
        drawChordKeys();
      }
    }
  } else {
    // Touch released - turn off all chords
    bool anyChanged = false;
    for (int i = 0; i < 8; i++) {
      if (chordPressed[i]) {
        playChord(i, false);
        chordPressed[i] = false;
        anyChanged = true;
      }
    }
    if (anyChanged) {
      drawChordKeys();
    }
  }
}

void playChord(int scaleDegree, bool on) {
  if (!deviceConnected) return;
  
  // Get root note for this scale degree
  int rootNote;
  if (scaleDegree == 7) { // I+ octave
    rootNote = getNoteInScale(chordScale, 0, chordOctave + 1);
  } else {
    rootNote = getNoteInScale(chordScale, scaleDegree, chordOctave);
  }
  
  if (on) {
    // Play traditional diatonic chord (root, 3rd, 5th)
    ChordType chord = diatonicChords[scaleDegree];
    
    for (int i = 0; i < chord.numNotes; i++) {
      if (chord.intervals[i] >= 0) {
        int chordNote = rootNote + chord.intervals[i];
        if (chordNote >= 24 && chordNote <= 108) {
          sendMIDI(0x90, chordNote, 100);
          activeChordNotes[scaleDegree][i] = chordNote;
        }
      }
    }
  } else {
    // Stop chord for this specific scale degree
    for (int i = 0; i < 4; i++) {
      if (activeChordNotes[scaleDegree][i] != -1) {
        sendMIDI(0x80, activeChordNotes[scaleDegree][i], 0);
        activeChordNotes[scaleDegree][i] = -1;
      }
    }
  }
}

void stopAllChords() {
  for (int i = 0; i < 8; i++) {
    if (chordPressed[i]) {
      playChord(i, false);
      chordPressed[i] = false;
    }
  }
}

#endif