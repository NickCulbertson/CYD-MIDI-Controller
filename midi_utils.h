#ifndef MIDI_UTILS_H
#define MIDI_UTILS_H

#include "common_definitions.h"

// Scale definitions
Scale scales[] = {
  {"Major", {0, 2, 4, 5, 7, 9, 11, 0}, 7},
  {"Minor", {0, 2, 3, 5, 7, 8, 10, 0}, 7},
  {"Dorian", {0, 2, 3, 5, 7, 9, 10, 0}, 7},
  {"Penta", {0, 2, 4, 7, 9, 0, 0, 0}, 5},
  {"Blues", {0, 3, 5, 6, 7, 10, 0, 0}, 6},
  {"Chrome", {0, 1, 2, 3, 4, 5, 6, 7}, 8}
};
const int NUM_SCALES = 6;

// MIDI utility functions
// A MIDI message, as a BLE MIDI notification from its own buffer: the characteristic's stored value stays
// empty (what a read must return), and nothing another task does can change a message on its way out
void sendMIDI(byte cmd, byte note, byte vel) {
  if (!deviceConnected || midiGattsIf == ESP_GATT_IF_NONE) return;
  if (midiCccd != nullptr && !midiCccd->getNotifications()) return;   // not listening yet
  uint8_t packet[5] = {midiPacket[0], midiPacket[1], cmd, note, vel};  // header, timestamp, message
  esp_ble_gatts_send_indicate(midiGattsIf, midiConnId, pCharacteristic->getHandle(), sizeof packet, packet, false);
}

int getNoteInScale(int scaleIndex, int degree, int octave) {
  if (scaleIndex >= NUM_SCALES) return 60;
  
  // If degree exceeds scale notes, wrap to next octave
  int actualDegree = degree % scales[scaleIndex].numNotes;
  int octaveOffset = degree / scales[scaleIndex].numNotes;
  
  int rootNote = 60; // C4
  return rootNote + scales[scaleIndex].intervals[actualDegree] + ((octave - 4 + octaveOffset) * 12);
}

String getNoteNameFromMIDI(int midiNote) {
  String noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
  int noteIndex = midiNote % 12;
  int octave = (midiNote / 12) - 1;
  return noteNames[noteIndex] + String(octave);
}

// MIDI panic (leaving a mode): All Notes Off, then a Note Off for every note, paced so Bluetooth's send
// queue never overflows (128 at once could drop some, the very ones left sounding)
void stopAllModes() {
  sendMIDI(0xB0, 123, 0);   // All Notes Off
  for (int i = 0; i < 128; i++) {
    sendMIDI(0x80, i, 0);
    delay(2);
  }
}

#endif