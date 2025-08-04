#ifndef SEQUENCER_MODE_H
#define SEQUENCER_MODE_H

#include "common_definitions.h"
#include "ui_elements.h"
#include "midi_utils.h"

// Sequencer mode variables
#define SEQ_STEPS 16
#define SEQ_TRACKS 4
bool sequencePattern[SEQ_TRACKS][SEQ_STEPS];
int currentStep = 0;
unsigned long lastStepTime = 0;
unsigned long noteOffTime[SEQ_TRACKS] = {0};
int bpm = 120;
int stepInterval;
bool sequencerPlaying = false;

// Function declarations
void initializeSequencerMode();
void drawSequencerMode();
void handleSequencerMode();
void drawSequencerGrid();
void toggleSequencerStep(int track, int step);
void updateSequencer();
void playSequencerStep();

// Implementations
void initializeSequencerMode() {
  bpm = 120;
  stepInterval = 60000 / bpm / 4; // 16th notes
  sequencerPlaying = false;
  currentStep = 0;
  
  // Clear all patterns
  for (int t = 0; t < SEQ_TRACKS; t++) {
    for (int s = 0; s < SEQ_STEPS; s++) {
      sequencePattern[t][s] = false;
    }
  }
}

void drawSequencerMode() {
  tft.fillScreen(THEME_BG);
  drawHeader("BEATS", String(bpm) + " BPM");
  
  drawSequencerGrid();
  
  // Transport controls - positioned to avoid overlap
  drawRoundButton(10, 200, 50, 25, sequencerPlaying ? "STOP" : "PLAY", 
                 sequencerPlaying ? THEME_ERROR : THEME_SUCCESS);
  drawRoundButton(70, 200, 50, 25, "CLEAR", THEME_WARNING);
  drawRoundButton(130, 200, 40, 25, "BPM-", THEME_SECONDARY);
  drawRoundButton(180, 200, 40, 25, "BPM+", THEME_SECONDARY);
  
  // BPM display
  tft.setTextColor(THEME_TEXT_DIM, THEME_BG);
  tft.drawString(String(bpm), 240, 207, 2);
}

void drawSequencerGrid() {
  int gridX = 10;
  int gridY = 50;
  int cellW = 15;
  int cellH = 28;
  int spacing = 1;
  
  // 808-style track labels and colors
  String trackLabels[] = {"KICK", "SNRE", "HHAT", "OPEN"};
  uint16_t trackColors[] = {THEME_ERROR, THEME_WARNING, THEME_PRIMARY, THEME_ACCENT};
  
  for (int track = 0; track < SEQ_TRACKS; track++) {
    int y = gridY + track * (cellH + spacing + 3);
    
    // Track name with color coding
    tft.setTextColor(trackColors[track], THEME_BG);
    tft.drawString(trackLabels[track], gridX, y + 12, 1);
    
    // Steps - 16 steps in 808 style
    for (int step = 0; step < SEQ_STEPS; step++) {
      int x = gridX + 35 + step * (cellW + spacing);
      
      bool active = sequencePattern[track][step];
      bool current = (sequencerPlaying && step == currentStep);
      
      uint16_t color;
      if (current && active) color = THEME_TEXT;
      else if (current) color = trackColors[track];
      else if (active) color = trackColors[track];
      else color = THEME_SURFACE;
      
      // Highlight every 4th step (like 808)
      if (step % 4 == 0) {
        tft.drawRect(x-1, y-1, cellW+2, cellH+2, THEME_TEXT_DIM);
      }
      
      tft.fillRect(x, y, cellW, cellH, color);
      tft.drawRect(x, y, cellW, cellH, THEME_TEXT_DIM);
    }
  }
}

void handleSequencerMode() {
  // Back button
  if (touch.justPressed && isButtonPressed(10, 10, 50, 25)) {
    sequencerPlaying = false;
    exitToMenu();
    return;
  }
  
  // Handle touch input
  if (touch.justPressed) {
    // Transport controls
    if (isButtonPressed(10, 200, 50, 25)) {
      sequencerPlaying = !sequencerPlaying;
      if (sequencerPlaying) {
        currentStep = 0;
        lastStepTime = millis();
      }
      drawSequencerMode();
      return;
    }
    
    if (isButtonPressed(70, 200, 50, 25)) {
      // Clear all patterns
      for (int t = 0; t < SEQ_TRACKS; t++) {
        for (int s = 0; s < SEQ_STEPS; s++) {
          sequencePattern[t][s] = false;
        }
      }
      drawSequencerGrid();
      return;
    }
    
    if (isButtonPressed(130, 200, 40, 25)) {
      bpm = max(60, bpm - 1);
      stepInterval = 60000 / bpm / 4;
      drawSequencerMode();
      return;
    }
    
    if (isButtonPressed(180, 200, 40, 25)) {
      bpm = min(200, bpm + 1);
      stepInterval = 60000 / bpm / 4;
      drawSequencerMode();
      return;
    }
    
    // Grid interaction
    int gridX = 45;
    int gridY = 50;
    int cellW = 15;
    int cellH = 28;
    int spacing = 1;
    
    for (int track = 0; track < SEQ_TRACKS; track++) {
      for (int step = 0; step < SEQ_STEPS; step++) {
        int x = gridX + step * (cellW + spacing);
        int y = gridY + track * (cellH + spacing + 3);
        
        if (isButtonPressed(x, y, cellW, cellH)) {
          toggleSequencerStep(track, step);
          drawSequencerGrid();
          return;
        }
      }
    }
  }
  
  // Update sequencer timing
  updateSequencer();
}

void toggleSequencerStep(int track, int step) {
  sequencePattern[track][step] = !sequencePattern[track][step];
}

void updateSequencer() {
  if (!sequencerPlaying) return;
  
  unsigned long now = millis();
  
  // Check for notes to turn off
  int drumNotes[] = {36, 38, 42, 46};
  for (int track = 0; track < SEQ_TRACKS; track++) {
    if (noteOffTime[track] > 0 && now >= noteOffTime[track]) {
      sendMIDI(0x80, drumNotes[track], 0);
      noteOffTime[track] = 0;
    }
  }
  
  if (now - lastStepTime >= stepInterval) {
    playSequencerStep();
    currentStep = (currentStep + 1) % SEQ_STEPS;
    lastStepTime = now;
    drawSequencerGrid();
  }
}

void playSequencerStep() {
  if (!deviceConnected) return;
  
  int drumNotes[] = {36, 38, 42, 46}; // Kick, Snare, Hi-hat, Open Hi-hat
  int noteLengths[] = {200, 150, 50, 300}; // Note lengths in ms
  
  unsigned long now = millis();
  
  for (int track = 0; track < SEQ_TRACKS; track++) {
    if (sequencePattern[track][currentStep]) {
      // Turn on note
      sendMIDI(0x90, drumNotes[track], 100);
      // Schedule note off
      noteOffTime[track] = now + noteLengths[track];
    }
  }
}

#endif