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
    String text = String(controllerStateName()) + ":" + status.remainingShots + ":" + String(settings.stepDistance, 0) + ":" + webServerAddress();
    if (text == lastStatus) return;
    lastStatus = text;

    display.fillScreen(TFT_BLACK);
    display.setTextColor(TFT_CYAN, TFT_BLACK);
    display.drawString("MacroController", 8, 8, 2);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.drawString("State: " + String(controllerStateName()), 8, 38, 2);
    display.drawString("Shots: " + String(status.currentShot) + "/" + String(settings.totalShots), 8, 64, 2);
    display.drawString("Remaining: " + String(status.remainingShots), 8, 90, 2);
    display.drawString("Distance: " + String(status.distanceTravelled), 8, 116, 2);
    display.drawString("DoF: " + String(settings.depthOfField), 8, 142, 2);
    display.drawString("Step: " + String(settings.stepDistance, 0) + " um", 8, 168, 2);
    display.drawString("Web: " + webServerAddress(), 8, 194, 2);
    if (status.error.length() > 0)
    {
        display.setTextColor(TFT_RED, TFT_BLACK);
        display.drawString(status.error, 8, 220, 2);
    }
}
