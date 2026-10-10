#include <Arduino.h>

#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32_NimBLE.h>
#include <MIDI.h>
#include "esp32-hal-tinyusb.h"
#include "USB.h"

BLEMIDI_CREATE_INSTANCE("TSC MIDI Pedal", MIDI)

namespace
{
  constexpr uint8_t FOOTSWITCH_PIN4 = 4;
  constexpr uint8_t FOOTSWITCH_PIN5 = 5;
  constexpr uint8_t FOOTSWITCH_PIN6 = 6;
  constexpr uint8_t FOOTSWITCH_PIN7 = 7;
  constexpr uint8_t MIDI_MODE_SWITCH_PIN = 10; // LOW selects USB MIDI.
  constexpr uint8_t MIDI_CHANNEL = 1;
  constexpr uint8_t PROGRAM_NUMBER0 = 0; // MIDI is zero-based; 0 is Program 01.
  constexpr uint8_t PROGRAM_NUMBER1 = 1; // MIDI is zero-based; 1 is Program 02.
  constexpr uint8_t PROGRAM_NUMBER2 = 2; // MIDI is zero-based; 2 is Program 03.
  constexpr uint8_t PROGRAM_NUMBER3 = 3; // MIDI is zero-based; 3 is Program 04.
  constexpr unsigned long DEBOUNCE_MS = 30;

  bool lastReading4;
  bool lastReading5;
  bool lastReading6;
  bool lastReading7;
  bool stablePressed4;
  bool stablePressed5;
  bool stablePressed6;
  bool stablePressed7;
  unsigned long lastChangeTime4;
  unsigned long lastChangeTime5;
  unsigned long lastChangeTime6;
  unsigned long lastChangeTime7;

  uint16_t loadUsbMidiDescriptor(uint8_t *buffer, uint8_t *interfaceNumber)
  {
    const uint8_t endpoint = tinyusb_get_free_duplex_endpoint();
    if (endpoint == 0)
    {
      return 0;
    }

    const uint8_t descriptor[] = {
        TUD_MIDI_DESCRIPTOR(*interfaceNumber, tinyusb_add_string_descriptor("TSC MIDI Pedal"),
                            endpoint, static_cast<uint8_t>(endpoint | 0x80), 64)};
    memcpy(buffer, descriptor, sizeof(descriptor));
    *interfaceNumber += 2;
    return sizeof(descriptor);
  }

  void sendUsbProgramChange(uint8_t programNumber)
  {
    const uint8_t message[] = {
        static_cast<uint8_t>(0xC0 | (MIDI_CHANNEL - 1)),
        programNumber};
    if (tud_midi_stream_write(0, message, sizeof(message)) != sizeof(message))
    {
      log_e("Failed to send USB MIDI program change");
    }
  }
} // namespace

void setup()
{
  pinMode(FOOTSWITCH_PIN4, INPUT_PULLUP);
  pinMode(FOOTSWITCH_PIN5, INPUT_PULLUP);
  pinMode(FOOTSWITCH_PIN6, INPUT_PULLUP);
  pinMode(FOOTSWITCH_PIN7, INPUT_PULLUP);
  pinMode(MIDI_MODE_SWITCH_PIN, INPUT_PULLUP);

  lastReading4 = digitalRead(FOOTSWITCH_PIN4) == LOW;
  lastReading5 = digitalRead(FOOTSWITCH_PIN5) == LOW;
  lastReading6 = digitalRead(FOOTSWITCH_PIN6) == LOW;
  lastReading7 = digitalRead(FOOTSWITCH_PIN7) == LOW;

  stablePressed4 = lastReading4; // Initialize to the state of the first footswitch
  stablePressed5 = lastReading5;
  stablePressed6 = lastReading6;
  stablePressed7 = lastReading7;

  lastChangeTime4 = millis();
  lastChangeTime5 = millis();
  lastChangeTime6 = millis();
  lastChangeTime7 = millis();

  if (tinyusb_enable_interface(USB_INTERFACE_MIDI, TUD_MIDI_DESC_LEN,
                               loadUsbMidiDescriptor) != ESP_OK)
  {
    log_e("Failed to register USB MIDI interface");
    return;
  }

  if (!USB.productName("TSC MIDI Pedal"))
  {
    log_e("Failed to set USB product name");
    return;
  }

  if (!USB.begin())
  {
    log_e("Failed to start USB");
    return;
  }
  MIDI.begin(MIDI_CHANNEL_OMNI);
}

void loop()
{
  MIDI.read();
  const bool usbMidi = digitalRead(MIDI_MODE_SWITCH_PIN) == LOW;
  const bool reading4 = digitalRead(FOOTSWITCH_PIN4) == LOW;
  const bool reading5 = digitalRead(FOOTSWITCH_PIN5) == LOW;
  const bool reading6 = digitalRead(FOOTSWITCH_PIN6) == LOW;
  const bool reading7 = digitalRead(FOOTSWITCH_PIN7) == LOW;

  if (reading4 != lastReading4)
  {
    lastReading4 = reading4;
    lastChangeTime4 = millis();
  }
  if (millis() - lastChangeTime4 >= DEBOUNCE_MS && reading4 != stablePressed4)
  {
    stablePressed4 = reading4;
    if (stablePressed4)
    {
      if (usbMidi)
      {
        sendUsbProgramChange(PROGRAM_NUMBER0);
      }
      else
      {
        MIDI.sendProgramChange(PROGRAM_NUMBER0, MIDI_CHANNEL);
      }      
    }
  }
  if (reading5 != lastReading5)
  {
    lastReading5 = reading5;
    lastChangeTime5 = millis();
  }
  if (millis() - lastChangeTime5 >= DEBOUNCE_MS && reading5 != stablePressed5)
  {
    stablePressed5 = reading5;
    if (stablePressed5)
    {
      if (usbMidi)
      {
        sendUsbProgramChange(PROGRAM_NUMBER1);
      }
      else
      {
        MIDI.sendProgramChange(PROGRAM_NUMBER1, MIDI_CHANNEL);
      }
    }
  }
  if (reading6 != lastReading6)
  {
    lastReading6 = reading6;
    lastChangeTime6 = millis();
  }
  if (millis() - lastChangeTime6 >= DEBOUNCE_MS && reading6 != stablePressed6)
  {
    stablePressed6 = reading6;
    if (stablePressed6)
    {
      if (usbMidi)
      {
        sendUsbProgramChange(PROGRAM_NUMBER2);
      }
      else
      {
        MIDI.sendProgramChange(PROGRAM_NUMBER2, MIDI_CHANNEL);
      }
    }
  }
  if (reading7 != lastReading7)
  {
    lastReading7 = reading7;
    lastChangeTime7 = millis();
  }
  if (millis() - lastChangeTime7 >= DEBOUNCE_MS && reading7 != stablePressed7)
  {
    stablePressed7 = reading7;
    if (stablePressed7)
    {
      if (usbMidi)
      {
        sendUsbProgramChange(PROGRAM_NUMBER3);
      }
      else
      {
        MIDI.sendProgramChange(PROGRAM_NUMBER3, MIDI_CHANNEL);
      }
    }
  }
}
