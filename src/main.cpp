#include "controller.h"
#include "display.h"
#include "web_server.h"

void setup()
{
    Serial.begin(115200);
    controllerBegin();
    displayBegin();
    webServerBegin();
}

void loop()
{
    controllerTick();
    webServerTick();
    displayTick();
    delay(1);
}
