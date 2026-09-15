# MacroController

This is a simple focus-stacking controller built around an ESP32 CYD board. It moves a camera rail with a stepper motor, triggers the camera, and lets you control the setup from a browser.

## Requirements

- ESP32 CYD board
- Stepper driver and motor
- Camera trigger wiring
- 240 x 320 TFT display for status
- VS Code with PlatformIO
- USB cable

## Setup

1. Open this project in VS Code.
2. Install PlatformIO if needed.
3. Edit [src/secrets.h](src/secrets.h) and add your Wi‑Fi name and password.
4. Connect the ESP32 CYD to your computer.
5. Build and upload the firmware.
6. Open the serial monitor at 115200 baud.
7. Use the web address shown there to open the controller in a browser.

## Wi‑Fi

The controller will try to connect to your Wi‑Fi. If it cannot, it can fall back to its own access point.

Keep your Wi‑Fi settings in [src/secrets.h](src/secrets.h), and do not commit real credentials to Git.

## Basic use

1. Open the web interface.
2. Set the focus-stacking values.
3. Choose the movement mode and delay.
4. Jog the rail to check direction and travel.
5. Test the camera trigger with the camera disconnected.
6. Start the run when everything looks correct.

The device will move the rail and trigger the camera at the right intervals during the stack.

## Notes

- The TFT is only for status; the main controls are in the browser.
- This project is designed around the ESP32 CYD.
- Check motor direction, enable polarity, and trigger wiring before a full run.
- Keep the rail and camera isolated until you are happy with the movement and timing.
