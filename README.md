# MacroController

MacroController is an ESP32 focus-stacking controller for a motorized camera rail. It calculates a depth of field, moves a stepper motor between shots, triggers the camera, and provides a browser-based control panel. The TFT displays status and the controller address; configuration and controls are available in the web interface.

## Hardware

- ESP32 CYD board (the PlatformIO target is `esp32dev`)
- Stepper motor and compatible driver
- Motorized camera rail
- Camera trigger input and compatible trigger cable
- CYD 240 x 320 TFT display
- USB cable for programming and serial diagnostics

### GPIO assignments

| Function | ESP32 GPIO |
| --- | ---: |
| Camera trigger | 27 |
| Stepper pulse | 23 |
| Stepper direction | 18 |
| Stepper enable | 19 |
| TFT backlight | 21 |

The motor enable output is active-low. Confirm the driver's enable polarity and logic levels before connecting the motor.

## Build and upload

1. Open the project in VS Code with PlatformIO installed.
2. Copy [src/secrets.h.example](src/secrets.h.example) to `src/secrets.h`.
3. Set the Wi-Fi and fallback access-point values in `src/secrets.h`.
4. Connect the ESP32 by USB.
5. Run **PlatformIO: Build**, then **PlatformIO: Upload**.
6. Open the serial monitor at `115200` baud.

The firmware starts the web server during boot and prints its URL to the serial monitor. The same address is shown on the TFT.

## Network setup

By default, the controller joins the configured Wi-Fi network using DHCP. Optional static-IP settings are available in `src/secrets.h`.

If the station connection fails, or `WIFI_FORCE_AP` is set to `1`, the controller starts a fallback access point:

- SSID: `MacroController`
- Password: `macrocontrol`
- Address: `http://192.168.4.1/`

Connect a phone or computer to that access point and open the address above. Keep real Wi-Fi credentials out of version control.

## Web interface

The web interface updates every 500 ms and provides:

- Start and stop controls for a stacking run
- Camera trigger test
- Slow step, fast, and long manual movement in both directions
- Distance mode, which calculates shots from the configured travel distance
- Start/stop mode, which calculates shots from recorded rail endpoints
- Macro lens, objective lens, and reverse lens calculation modes
- Sensor type, aperture, magnification, numerical aperture, tube length, focal length, and delay settings
- Live state, remaining shots, distance travelled, calculated step length, and calculated steps per shot

### Step Length And Overlap

The controller uses a step length equal to 90% of the calculated depth of field to create overlap between adjacent images:

`Step length (&#956;m) = floor(Depth of field (&#956;m) x 0.90)`

This calculated step length is rounded down to a whole micrometer, shown in the status header, and used to calculate the motor steps per shot. The remaining 10% provides overlap between consecutive focus positions.

### Distance mode

Set the travel distance, calibration value in steps per &#956;m, optical values, and delay. The controller calculates the number of shots using a step length equal to 90% of the depth of field, then returns the rail to its starting position when the run finishes.

Microscope objective mode defaults to an actual tube length of 160 mm. Adjust it when the measured tube length differs from the objective's marked base tube length.

### Start/stop mode

1. Select **Start / stop** mode.
2. Use the jog controls to move to the first endpoint.
3. Select **Save start**.
4. Jog to the second endpoint.
5. Select **Save end + return**. The controller records the travel and returns to the start.
6. Start the run.

## Operating checklist

1. Test movement with the camera and subject clear of the rail.
2. Confirm forward and reverse directions.
3. Verify the driver enable behavior and the rail's usable travel.
4. Test the camera trigger with the camera disconnected or protected from unintended exposure.
5. Check the calculated shot count and delay.
6. Start the capture only after the rail, camera, and subject are secure.

The TFT is a status display only. Use the browser interface for setup and control.

## Project layout

- `src/main.cpp` - Arduino setup and main loop
- `src/controller.*` - motion, capture state machine, and focus-stack calculations
- `src/web_server.*` - Wi-Fi setup and browser interface
- `src/display.*` - TFT status display
- `src/SpeedyStepper.*` - stepper motion support
