#include <TFT_eSPI.h>
#include "controller.h"
#include "display.h"
#include "web_server.h"

namespace {
TFT_eSPI display = TFT_eSPI();
unsigned long lastDraw = 0;
String lastStatus;
}

void displayBegin()
{
    display.init();
    display.setRotation(1);
    display.fillScreen(TFT_BLACK);
    display.setTextDatum(TL_DATUM);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.drawString("MacroController", 8, 8, 2);
}

void displayTick()
{
    if (millis() - lastDraw < 250) return;
    lastDraw = millis();

    const ControllerSettings &settings = controllerSettings();
    const ControllerStatus &status = controllerStatus();
    String text = String(controllerStateName()) + ":" + status.currentShot + ":" + status.remainingShots + ":" + settings.totalShots + ":" + String(settings.stepsPerShot) + ":" + webServerAddress();
    if (text == lastStatus) return;
    lastStatus = text;

    display.fillScreen(TFT_BLACK);
    display.setTextColor(TFT_CYAN, TFT_BLACK);
    display.drawString("MacroController", 8, 8, 2);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.drawString("State: " + String(controllerStateName()), 8, 38, 2);
    display.drawString("Capture Frames: " + String(status.currentShot) + "/" + String(settings.totalShots), 8, 64, 2);
    display.drawString("Capture Frames Left: " + String(status.remainingShots), 8, 90, 2);
    display.drawString("Distance Moved: " + String(status.distanceTravelled), 8, 116, 2);
    display.drawString("DoF: " + String(settings.depthOfField) + " um", 8, 142, 2);
    display.drawString("Stepper Steps/Captured Frame: " + String(settings.stepsPerShot), 8, 168, 2);
    display.drawString("Web: " + webServerAddress(), 8, 194, 2);
    if (status.error.length() > 0)
    {
        display.setTextColor(TFT_RED, TFT_BLACK);
        display.drawString(status.error, 8, 220, 2);
    }
}
