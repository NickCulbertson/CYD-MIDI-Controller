#ifndef LFO_MODE_H
#define LFO_MODE_H

#include "common_definitions.h"
#include "ui_elements.h"
#include "midi_utils.h"

// LFO mode variables
struct LFOParams {
  float rate = 1.0;      // Hz (0.1 - 10.0)
  int amount = 64;       // 0-127
  int ccTarget = 1;      // CC number (0-127) or -1 for pitchwheel
  bool isRunning = false;
  float phase = 0.0;     // Current phase (0-2π)
  int waveform = 0;      // 0=Sine, 1=Triangle, 2=Square, 3=Sawtooth
  unsigned long lastUpdate = 0;
  int lastValue = 64;    // Last sent value
  bool pitchWheelMode = false; // Special mode for pitchwheel
};

LFOParams lfo;
String waveNames[] = {"SINE", "TRI", "SQR", "SAW"};

// Function declarations
void initializeLFOMode();
void drawLFOMode();
void handleLFOMode();
void drawLFOControls();
void updateLFO();
float calculateLFOValue();
void sendLFOValue(int value);
void drawWaveform();

// Implementations
void initializeLFOMode() {
  lfo.rate = 1.0;
  lfo.amount = 64;
  lfo.ccTarget = 1; // Modulation wheel by default
  lfo.isRunning = false;
  lfo.phase = 0.0;
  lfo.waveform = 0;
  lfo.lastUpdate = 0;
  lfo.lastValue = 64;
  lfo.pitchWheelMode = false;
}

void drawLFOMode() {
  tft.fillScreen(THEME_BG);
  drawHeader("LFO MOD", lfo.pitchWheelMode ? "Pitchwheel" : ("CC " + String(lfo.ccTarget)));
  
  drawLFOControls();
  drawWaveform();
}

void drawLFOControls() {
  int y = 55;
  int spacing = 30;
  
  // Play/Stop and Rate
  drawRoundButton(10, y, 60, 25, lfo.isRunning ? "STOP" : "START", 
                 lfo.isRunning ? THEME_ERROR : THEME_SUCCESS);
  
  tft.setTextColor(THEME_TEXT, THEME_BG);
  tft.drawString("Rate:", 80, y + 6, 1);
  tft.drawString(String(lfo.rate, 1) + "Hz", 115, y + 6, 1);
  drawRoundButton(160, y, 25, 25, "-", THEME_SECONDARY);
  drawRoundButton(190, y, 25, 25, "+", THEME_SECONDARY);
  
  // Waveform selector
  drawRoundButton(230, y, 60, 25, waveNames[lfo.waveform], THEME_ACCENT);
  
  y += spacing;
  
  // Amount
  tft.drawString("Amount:", 10, y + 6, 1);
  tft.drawString(String(lfo.amount), 60, y + 6, 1);
  drawRoundButton(85, y, 25, 25, "-", THEME_SECONDARY);
  drawRoundButton(115, y, 25, 25, "+", THEME_SECONDARY);
  
  // Amount bar
  int barW = 100;
  int barX = 150;
  tft.drawRect(barX, y + 8, barW, 10, THEME_TEXT_DIM);
  int fillW = (barW * lfo.amount) / 127;
  tft.fillRect(barX + 1, y + 9, fillW, 8, THEME_PRIMARY);
  
  y += spacing;
  
  // Target selection
  tft.drawString("Target:", 10, y + 6, 1);
  if (lfo.pitchWheelMode) {
    tft.drawString("PITCH", 60, y + 6, 1);
  } else {
    tft.drawString("CC" + String(lfo.ccTarget), 60, y + 6, 1);
  }
  
  drawRoundButton(110, y, 25, 25, "-", THEME_SECONDARY);
  drawRoundButton(140, y, 25, 25, "+", THEME_SECONDARY);
  drawRoundButton(180, y, 70, 25, "PITCH", lfo.pitchWheelMode ? THEME_PRIMARY : THEME_WARNING);
  
  y += spacing;
  
  // Current value display
  tft.setTextColor(THEME_PRIMARY, THEME_BG);
  tft.drawString("Value: ", 10, y, 1);
  tft.setTextColor(THEME_ACCENT, THEME_BG);
  tft.drawString(String(lfo.lastValue), 60, y, 2);
  
  // Status indicator
  if (lfo.isRunning) {
    tft.fillCircle(250, y + 8, 8, THEME_SUCCESS);
    tft.drawCircle(250, y + 8, 8, THEME_TEXT);
  } else {
    tft.drawCircle(250, y + 8, 8, THEME_TEXT_DIM);
  }
}

void drawWaveform() {
  // Draw a mini waveform visualization
  int waveX = 10;
  int waveY = 180;
  int waveW = 200;
  int waveH = 30;
  
  tft.drawRect(waveX, waveY, waveW, waveH, THEME_TEXT_DIM);
  
  // Draw waveform based on type
  for (int x = 0; x < waveW - 2; x++) {
    float phase = (x / (float)(waveW - 2)) * 2 * PI;
    float value = 0;
    
    switch (lfo.waveform) {
      case 0: // Sine
        value = sin(phase);
        break;
      case 1: // Triangle
        value = (phase <= PI) ? (2 * phase / PI - 1) : (3 - 2 * phase / PI);
        break;
      case 2: // Square
        value = (phase <= PI) ? 1 : -1;
        break;
      case 3: // Sawtooth
        value = 2 * phase / (2 * PI) - 1;
        break;
    }
    
    int y = waveY + waveH/2 - (value * waveH/4);
    tft.drawPixel(waveX + 1 + x, y, THEME_PRIMARY);
  }
  
  // Phase indicator removed per user request
}

void handleLFOMode() {
  // Back button
  if (touch.justPressed && isButtonPressed(10, 10, 50, 25)) {
    lfo.isRunning = false;
    exitToMenu();
    return;
  }
  
  if (touch.justPressed) {
    int y = 55;
    int spacing = 30;
    
    // Start/Stop
    if (isButtonPressed(10, y, 60, 25)) {
      lfo.isRunning = !lfo.isRunning;
      if (lfo.isRunning) {
        lfo.phase = 0.0;
        lfo.lastUpdate = millis();
      }
      drawLFOMode();
      return;
    }
    
    // Rate controls
    if (isButtonPressed(160, y, 25, 25)) {
      lfo.rate = max(0.1, lfo.rate - 0.1);
      drawLFOControls();
      return;
    }
    if (isButtonPressed(190, y, 25, 25)) {
      lfo.rate = min(10.0, lfo.rate + 0.1);
      drawLFOControls();
      return;
    }
    
    // Waveform selector
    if (isButtonPressed(230, y, 60, 25)) {
      lfo.waveform = (lfo.waveform + 1) % 4;
      drawLFOMode();
      return;
    }
    
    y += spacing;
    
    // Amount controls
    if (isButtonPressed(85, y, 25, 25)) {
      lfo.amount = max(0, lfo.amount - 5);
      drawLFOControls();
      return;
    }
    if (isButtonPressed(115, y, 25, 25)) {
      lfo.amount = min(127, lfo.amount + 5);
      drawLFOControls();
      return;
    }
    
    y += spacing;
    
    // Target controls
    if (isButtonPressed(110, y, 25, 25)) {
      if (lfo.pitchWheelMode) {
        lfo.pitchWheelMode = false;
        lfo.ccTarget = 1; // Back to modulation wheel
      } else {
        lfo.ccTarget = max(0, lfo.ccTarget - 1);
      }
      drawLFOMode();
      return;
    }
    if (isButtonPressed(140, y, 25, 25)) {
      if (!lfo.pitchWheelMode) {
        lfo.ccTarget = min(127, lfo.ccTarget + 1);
      }
      drawLFOMode();
      return;
    }
    
    // Pitchwheel mode toggle
    if (isButtonPressed(180, y, 70, 25)) {
      lfo.pitchWheelMode = !lfo.pitchWheelMode;
      drawLFOMode();
      return;
    }
  }
  
  // Update LFO
  updateLFO();
}

void updateLFO() {
  if (!lfo.isRunning) return;
  
  unsigned long now = millis();
  float deltaTime = (now - lfo.lastUpdate) / 1000.0; // Convert to seconds
  lfo.lastUpdate = now;
  
  // Update phase
  lfo.phase += 2 * PI * lfo.rate * deltaTime;
  while (lfo.phase >= 2 * PI) {
    lfo.phase -= 2 * PI;
  }
  
  // Calculate LFO value
  float lfoValue = calculateLFOValue();
  
  // Calculate output value based on target type
  int outputValue;
  if (lfo.pitchWheelMode) {
    // For pitch bend: 8192 center, scale by amount
    outputValue = 8192 + (lfoValue * lfo.amount * 64); // Scale for pitch bend range
    outputValue = constrain(outputValue, 0, 16383);
  } else {
    // For CC: 64 center, scale by amount  
    outputValue = 64 + (lfoValue * lfo.amount / 2);
    outputValue = constrain(outputValue, 0, 127);
  }
  
  // Send if value changed significantly (reduce MIDI spam)
  if (abs(outputValue - lfo.lastValue) >= 1) {
    sendLFOValue(outputValue);
    lfo.lastValue = outputValue;
    
    // Update display every few cycles
    static int displayUpdateCounter = 0;
    if (++displayUpdateCounter >= 10) {
      drawLFOControls();
      drawWaveform();
      displayUpdateCounter = 0;
    }
  }
}

float calculateLFOValue() {
  switch (lfo.waveform) {
    case 0: // Sine
      return sin(lfo.phase);
    case 1: // Triangle
      return (lfo.phase <= PI) ? (2 * lfo.phase / PI - 1) : (3 - 2 * lfo.phase / PI);
    case 2: // Square
      return (lfo.phase <= PI) ? 1 : -1;
    case 3: // Sawtooth
      return 2 * lfo.phase / (2 * PI) - 1;
    default:
      return 0;
  }
}

void sendLFOValue(int value) {
  if (!deviceConnected) return;
  
  if (lfo.pitchWheelMode) {
    // Send pitchwheel (14-bit value already calculated)
    byte lsb = value & 0x7F;
    byte msb = (value >> 7) & 0x7F;
    
    // Pitchwheel: 0xE0, LSB, MSB
    midiPacket[2] = 0xE0;
    midiPacket[3] = lsb;
    midiPacket[4] = msb;
    pCharacteristic->setValue(midiPacket, 5);
    pCharacteristic->notify();
  } else {
    // Send regular CC
    sendMIDI(0xB0, lfo.ccTarget, value);
  }
}

#endif