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

## Calculations

The controller calculates the capture spacing from the selected optical mode. In the table below, $M$ is magnification, $N$ is nominal aperture, $N_{eff}$ is effective aperture, $NA$ is numerical aperture, $c$ is the sensor circle of confusion in mm, and $S$ is the configured motor calibration in steps per micrometer.

| Calculation | Formula used by the controller | Notes |
| --- | --- | --- |
| Macro lens magnification | $M = M_{macro}$ | Uses the entered macro-lens magnification. |
| Microscope objective magnification | $M = M_{design} \times \frac{L_{actual} - 10}{L_{base} - 10}$ | The result is limited to $0.1$ through $100$. |
| Stacked-lens magnification | $M = \frac{f_{rear}}{f_{front}}$ | Rear focal length divided by front focal length. |
| Effective aperture: macro or stacked lenses | $N_{eff} = N \times (M + 1)$ | Uses the nominal aperture and calculated magnification. |
| Effective aperture: microscope objective | $N_{eff} = \frac{M}{2 \times NA}$ | Uses the calculated objective magnification and numerical aperture. |
| Circle of confusion | Full Frame: $c = 0.03$; APS-C: $c = 0.02$; Micro Four Thirds: $c = 0.015$ | Values are in mm. |
| Depth of field | $DoF_{\mu m} = \frac{2 \times c \times N_{eff}}{M^2} \times 1000$ | Converted to an integer micrometer value and limited to $1$ through $1,000,000$. |
| Step length | $Step_{\mu m} = \lfloor DoF_{\mu m} \times 0.90 \rfloor$ | Creates 10% overlap between adjacent focus positions. |
| Motor steps per shot | $Steps_{shot} = \lfloor Step_{\mu m} \times S \rfloor$ | Multiply the calculated step length in micrometers by the configured motor calibration ($S$, steps per micrometer). The result is limited to $1$ through $1,000,000$ motor steps. |
| Shots: Stacking Distance mode | $Shots = \lceil \frac{Distance_{\mu m}}{Step_{\mu m}} \rceil$ | Uses the configured stacking distance. |
| Shots: Start/Stop mode | $Endpoint_{\mu m} = \frac{|Endpoint_{steps}|}{S}$; $Shots = \lceil \frac{Endpoint_{\mu m}}{Step_{\mu m}} \rceil$ | Divides the saved endpoint distance in motor steps by the calibration ($S$) to get micrometers, then uses the absolute distance between endpoints. |

The calculated step length is shown in the status header and determines both the motor movement per shot and the total shot count.

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
