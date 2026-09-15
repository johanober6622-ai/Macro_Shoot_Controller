#ifndef MACROCONTROLLER_CONTROLLER_H
#define MACROCONTROLLER_CONTROLLER_H

#include <Arduino.h>

enum ControllerState {
    CONTROLLER_IDLE,
    CONTROLLER_MANUAL_MOVE,
    CONTROLLER_TRIGGER,
    CONTROLLER_WAIT_CAMERA,
    CONTROLLER_RUN_MOVE,
    CONTROLLER_WAIT_SETTLE,
    CONTROLLER_RETURNING,
    CONTROLLER_STOPPING,
    CONTROLLER_ERROR
};

struct ControllerSettings {
    float stepsPerMicron;
    float magnification;
    float fStop;
    float numericalAperture;
    int depthOfField;
    int shootDistance;
    int delaySeconds;
    int stepsPerShot;
    int totalShots;
    bool stepsMode;
    bool objectiveMode;
    bool frontAperture;
};

struct ControllerStatus {
    ControllerState state;
    int currentShot;
    int remainingShots;
    int distanceTravelled;
    int endpointSteps;
    String error;
};

void controllerBegin();
void controllerTick();
const ControllerSettings &controllerSettings();
const ControllerStatus &controllerStatus();
const char *controllerStateName();

void controllerSetDistance(int value);
void controllerSetDelay(int value);
void controllerSetStepsPerMicron(float value);
void controllerSetMagnification(float value);
void controllerSetFStop(float value);
void controllerSetNumericalAperture(float value);
void controllerSetStepsMode(bool enabled);
void controllerSetObjectiveMode(bool enabled);
void controllerSetFrontAperture(bool enabled);
void controllerRecalculate();

bool controllerStartRun();
void controllerStop();
bool controllerStartManualMove(int direction, const char *speedMode);
void controllerCameraTest();
void controllerClearEndpoints();
void controllerSaveEndpointStart();
void controllerSaveEndpointEnd();

#endif
