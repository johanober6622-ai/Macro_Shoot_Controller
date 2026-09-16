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

enum OpticalMode {
    OPTICAL_MACRO_LENS,
    OPTICAL_OBJECTIVE_LENS,
    OPTICAL_REVERSE_LENS
};

struct ControllerSettings {
    float stepsPerMicron;
    float magnification;
    float macroMagnification;
    float aperture;
    float effectiveAperture;
    float numericalAperture;
    float objectiveBaseTubeLength;
    float objectiveActualTubeLength;
    float objectiveDesignMagnification;
    float reverseFrontFocalLength;
    float reverseRearFocalLength;
    int depthOfField;
    int shootDistance;
    int delaySeconds;
    int stepsPerShot;
    int totalShots;
    bool stepsMode;
    OpticalMode opticalMode;
    int sensorType;
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
void controllerSetAperture(float value);
void controllerSetNumericalAperture(float value);
void controllerSetObjectiveBaseTubeLength(float value);
void controllerSetObjectiveActualTubeLength(float value);
void controllerSetObjectiveDesignMagnification(float value);
void controllerSetReverseFrontFocalLength(float value);
void controllerSetReverseRearFocalLength(float value);
void controllerSetStepsMode(bool enabled);
void controllerSetOpticalMode(OpticalMode mode);
void controllerSetSensorType(int value);
void controllerRecalculate();

bool controllerStartRun();
void controllerStop();
bool controllerStartManualMove(int direction, const char *speedMode);
bool controllerStartTravelTest(int direction, float distanceMm);
void controllerCameraTest();
void controllerClearEndpoints();
void controllerSaveEndpointStart();
void controllerSaveEndpointEnd();

#endif
