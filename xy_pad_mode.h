#ifndef XY_PAD_MODE_H
#define XY_PAD_MODE_H

#include "common_definitions.h"
#include "ui_elements.h"
#include "midi_utils.h"

// XY Pad mode variables
int xCC = 1;  // CC number for X axis (Modulation Wheel by default)
int yCC = 7;  // CC number for Y axis (Volume by default)
int xValue = 64;  // Current X value (0-127)
int yValue = 64;  // Current Y value (0-127)
bool padPressed = false;
int padX = 0, padY = 0;  // Touch position on pad
bool xyPadNeedsReset = false;  // Flag to reset static variables

// Pad area dimensions
#define PAD_X 20
#define PAD_Y 60
#define PAD_WIDTH 200
#define PAD_HEIGHT 140
#define PAD_CENTER_X (PAD_X + PAD_WIDTH/2)
#define PAD_CENTER_Y (PAD_Y + PAD_HEIGHT/2)

// Function declarations
void initializeXYPadMode();
void drawXYPadMode();
void handleXYPadMode();
void drawXYPad();
void drawCCControls();
void updateXYValues(int touchX, int touchY);
void sendXYValues();

// Implementations
void initializeXYPadMode() {
  xCC = 74;  // Cutoff/Filter Frequency
  yCC = 71;  // Resonance/Filter Q
  xValue = 64;
  yValue = 64;
  padPressed = false;
}

void drawXYPadMode() {
  tft.fillScreen(THEME_BG);
  drawHeader("XY PAD", "Touch Control");
  
  // Signal that static variables should be reset
  xyPadNeedsReset = true;
  
  drawXYPad();
  drawCCControls();
}

void drawXYPad() {
  static int lastIndicatorX = -1, lastIndicatorY = -1;
  static bool lastPadPressed = false;
  static int lastXValue = -1, lastYValue = -1;
  static bool backgroundDrawn = false;
  
  // Reset static variables if requested
  if (xyPadNeedsReset) {
    lastIndicatorX = -1;
    lastIndicatorY = -1;
    lastPadPressed = false;
    lastXValue = -1;
    lastYValue = -1;
    backgroundDrawn = false;
    xyPadNeedsReset = false;
  }
  
  // Always ensure background is drawn properly
  if (!backgroundDrawn || lastIndicatorX == -1) {
    // Draw pad background
    tft.fillRoundRect(PAD_X, PAD_Y, PAD_WIDTH, PAD_HEIGHT, 8, THEME_SURFACE);
    tft.drawRoundRect(PAD_X, PAD_Y, PAD_WIDTH, PAD_HEIGHT, 8, THEME_PRIMARY);
    
    // Draw crosshairs
    tft.drawFastHLine(PAD_X, PAD_CENTER_Y, PAD_WIDTH, THEME_TEXT_DIM);
    tft.drawFastVLine(PAD_CENTER_X, PAD_Y, PAD_HEIGHT, THEME_TEXT_DIM);
    
    backgroundDrawn = true;
  }
  
  // Calculate position indicator location
  int indicatorX = map(xValue, 0, 127, PAD_X + 5, PAD_X + PAD_WIDTH - 5);
  int indicatorY = map(yValue, 0, 127, PAD_Y + PAD_HEIGHT - 5, PAD_Y + 5);
  
  // Erase previous indicator if position changed
  if (lastIndicatorX != indicatorX || lastIndicatorY != indicatorY || lastPadPressed != padPressed) {
    if (lastIndicatorX != -1) {
      // Erase old indicator
      tft.fillCircle(lastIndicatorX, lastIndicatorY, 9, THEME_SURFACE);
      // Always redraw the full crosshairs after erasing
      tft.drawFastHLine(PAD_X, PAD_CENTER_Y, PAD_WIDTH, THEME_TEXT_DIM);
      tft.drawFastVLine(PAD_CENTER_X, PAD_Y, PAD_HEIGHT, THEME_TEXT_DIM);
      // Always redraw the border to prevent edge disappearing
      tft.drawRoundRect(PAD_X, PAD_Y, PAD_WIDTH, PAD_HEIGHT, 8, THEME_PRIMARY);
    }
    
    // Draw new indicator
    tft.fillCircle(indicatorX, indicatorY, 8, THEME_PRIMARY);
    tft.fillCircle(indicatorX, indicatorY, 5, padPressed ? THEME_ACCENT : THEME_TEXT);
    
    lastIndicatorX = indicatorX;
    lastIndicatorY = indicatorY;
    lastPadPressed = padPressed;
  }
  
  // Update value display only if values changed
  if (lastXValue != xValue || lastYValue != yValue) {
    // Clear previous text
    tft.fillRect(PAD_X, PAD_Y + PAD_HEIGHT + 10, 160, 16, THEME_BG);
    
    // Draw new values
    tft.setTextColor(THEME_TEXT, THEME_BG);
    tft.drawString("X: " + String(xValue), PAD_X, PAD_Y + PAD_HEIGHT + 10, 2);
    tft.drawString("Y: " + String(yValue), PAD_X + 80, PAD_Y + PAD_HEIGHT + 10, 2);
    
    lastXValue = xValue;
    lastYValue = yValue;
  }
}

void drawCCControls() {
  // CC assignment controls
  int controlsX = PAD_X + PAD_WIDTH + 20;
  
  // X CC controls
  tft.setTextColor(THEME_PRIMARY, THEME_BG);
  tft.drawString("X CC", controlsX, PAD_Y, 2);
  
  drawRoundButton(controlsX, PAD_Y + 25, 30, 25, "-", THEME_SECONDARY);
  drawRoundButton(controlsX + 35, PAD_Y + 25, 30, 25, "+", THEME_SECONDARY);
  
  tft.setTextColor(THEME_TEXT, THEME_BG);
  tft.drawCentreString(String(xCC), controlsX + 32, PAD_Y + 55, 2);
  
  // Y CC controls
  tft.setTextColor(THEME_ACCENT, THEME_BG);
  tft.drawString("Y CC", controlsX, PAD_Y + 80, 2);
  
  drawRoundButton(controlsX, PAD_Y + 105, 30, 25, "-", THEME_SECONDARY);
  drawRoundButton(controlsX + 35, PAD_Y + 105, 30, 25, "+", THEME_SECONDARY);
  
  tft.setTextColor(THEME_TEXT, THEME_BG);
  tft.drawCentreString(String(yCC), controlsX + 32, PAD_Y + 135, 2);
  
  // Reset button removed per user request
}

void handleXYPadMode() {
  // Back button
  if (touch.justPressed && isButtonPressed(10, 10, 50, 25)) {
    exitToMenu();
    return;
  }
  
  if (touch.isPressed) {
    // Check if touching the pad
    if (touch.x >= PAD_X && touch.x <= PAD_X + PAD_WIDTH &&
        touch.y >= PAD_Y && touch.y <= PAD_Y + PAD_HEIGHT) {
      padPressed = true;
      updateXYValues(touch.x, touch.y);
      sendXYValues();
      drawXYPad();  // Update position indicator
      return;
    }
  } else {
    if (padPressed) {
      padPressed = false;
      drawXYPad();  // Update indicator appearance
    }
  }
  
  if (touch.justPressed) {
    int controlsX = PAD_X + PAD_WIDTH + 20;
    
    // X CC controls
    if (isButtonPressed(controlsX, PAD_Y + 25, 30, 25)) {
      xCC = max(0, xCC - 1);
      drawCCControls();
      return;
    }
    if (isButtonPressed(controlsX + 35, PAD_Y + 25, 30, 25)) {
      xCC = min(127, xCC + 1);
      drawCCControls();
      return;
    }
    
    // Y CC controls
    if (isButtonPressed(controlsX, PAD_Y + 105, 30, 25)) {
      yCC = max(0, yCC - 1);
      drawCCControls();
      return;
    }
    if (isButtonPressed(controlsX + 35, PAD_Y + 105, 30, 25)) {
      yCC = min(127, yCC + 1);
      drawCCControls();
      return;
    }
    
    // Reset button removed
  }
}

void updateXYValues(int touchX, int touchY) {
  // Constrain touch coordinates to pad area first
  touchX = constrain(touchX, PAD_X, PAD_X + PAD_WIDTH);
  touchY = constrain(touchY, PAD_Y, PAD_Y + PAD_HEIGHT);
  
  // Map touch coordinates to CC values
  xValue = map(touchX, PAD_X, PAD_X + PAD_WIDTH, 0, 127);
  yValue = map(touchY, PAD_Y + PAD_HEIGHT, PAD_Y, 0, 127);  // Invert Y axis
  
  // Constrain values
  xValue = constrain(xValue, 0, 127);
  yValue = constrain(yValue, 0, 127);
}

void sendXYValues() {
  if (deviceConnected) {
    // Send X CC
    sendMIDI(0xB0, xCC, xValue);
    // Send Y CC
    sendMIDI(0xB0, yCC, yValue);
  }
}

// Reset function removed - using global flag instead

#endif