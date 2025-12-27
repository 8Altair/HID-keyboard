# HID-keyboard

This repository contains an embedded firmware project for the STM32F411E-Discovery development board, demonstrating the use of the USB Human Interface Device (HID) protocol in keyboard mode.

The firmware configures the microcontroller as a USB HID keyboard device and generates predefined keystroke sequences in response to a hardware button input.

---

## Overview

The application implements a USB HID keyboard using the STM32 HAL library and the USB Device stack. A single GPIO input (the on-board blue user button) is used to control the transmission of keyboard data to the host system.

The device enumerates as a standard USB keyboard and sends HID reports that are interpreted by the operating system according to the active keyboard layout.

---

## Hardware Platform

- **Board:** STM32F411E-Discovery  
- **Microcontroller:** STM32F411VE  
- **Input:**  
  - PA0 (blue user button)
- **Interface:**  
  - USB Full-Speed Device (HID class)

---

## USB HID Device Configuration

- **USB Class:** Human Interface Device (HID)
- **Subclass:** Boot Interface
- **Protocol:** Keyboard
- **Product String:** `KeyboardGhost`

The device follows the USB HID specification for keyboard devices and communicates with the host using standard HID reports.

---

## Firmware Behavior

The firmware maintains an internal state that toggles on each valid button press.

### Odd Button Presses
- Sends the fixed character sequence: 29402
- Characters are transmitted sequentially.
- A fixed delay of **245 ms** is applied between each character.

### Even Button Presses
- Sends characters corresponding to the ASCII table range: 36 to 124
- Characters are transmitted sequentially in ascending order.
- A fixed delay of **245 ms** is applied between each character.

Each button press results in exactly one complete sequence being sent.

---

## HID Report Handling

Keystrokes are transmitted using standard HID keyboard reports. Each character is converted into its corresponding keycode according to the HID Usage Tables and combined with the required modifier state.

Character interpretation depends on the keyboard language and layout configured in the host operating system. The firmware targets a single keyboard layout and does not perform runtime layout detection.

---

 ## Keyboard Layout Considerations

This firmware is designed and tested specifically with the **Bosnian QWERTZ keyboard layout** configured on the host operating system.

USB HID keyboards do not transmit characters directly. Instead, the device sends **key usage codes** defined by the HID Usage Tables, along with modifier states (such as Shift). The conversion of these keycodes into visible characters is performed entirely by the host operating system based on the active keyboard layout.

The keycode-to-character mapping implemented in this firmware assumes the Bosnian QWERTZ layout. When a different keyboard layout is selected on the host system, the same HID reports may result in different characters being displayed. For this reason, the firmware does not attempt to adapt to multiple layouts dynamically; instead, it targets a single, known configuration to ensure deterministic behavior.

---

## Standards and References

This implementation is based on the official USB HID specifications:

- **Device Class Definition for Human Interface Devices (HID), Version 1.11**  
https://www.usb.org/sites/default/files/documents/hid1_11.pdf

- **HID Usage Tables, Version 1.12**  
https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf

These documents define the HID report format, keyboard usage IDs, and device behavior required for compatibility with standard operating systems.

---

## Project Structure

The project follows the standard STM32CubeMX and STM32CubeIDE directory layout, including:

- Core application source and header files
- STM32 HAL drivers
- USB Device middleware and HID class implementation
- CubeMX configuration file for project regeneration

---

## Notes

- The firmware is designed to be regenerated from the CubeMX configuration file.
- No external drivers are required on the host system.
- The device operates as a standard USB keyboard recognized by modern operating systems.
