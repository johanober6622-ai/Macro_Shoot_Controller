#include "controller.h"
#include "SpeedyStepper.h"
#include <cstring>

namespace {
constexpr uint8_t CAMERA_TRIGGER_PIN = 27;
constexpr uint8_t MOTOR_STEP_PIN = 23;
constexpr uint8_t MOTOR_DIRECTION_PIN = 18;
constexpr uint8_t MOTOR_ENABLE_PIN = 19;
constexpr int MIN_DISTANCE = 0;
constexpr int MAX_DISTANCE = 5000;
constexpr int MIN_DELAY = 0;
constexpr int MAX_DELAY = 3600;
constexpr int SHUTTER_PULSE_MS = 500;
constexpr int CAMERA_SETTLE_MS = 2000;
constexpr int SLOW_SPEED = 100;
constexpr int FAST_SPEED = 400;
constexpr int TRAVEL_TEST_SPEED = 800;
constexpr int LONG_MOVE_STEPS = 3200;
constexpr float SENSOR_CIRCLE_OF_CONFUSION[] = {0.03f, 0.02f, 0.015f};

ControllerSettings settings = {
    0.4f, 2.1f, 2.1f, 5.6f, 11.2f, 0.1f, 160.0f, 160.0f, 4.0f, 50.0f, 50.0f,
    55, 49.5f, 10, 5, 22, 18, false, OPTICAL_MACRO_LENS, 0};
ControllerStatus status = {CONTROLLER_IDLE, false, 0, 0, 0, 0, ""};
ControllerState state = CONTROLLER_IDLE;
unsigned long stateStartedAt = 0;
long endpointDistanceSteps = 0;
long returnSteps = 0;
long manualMoveSteps = 0;
long runTravelledSteps = 0;
long runStartPositionSteps = 0;

int clampInt(int value, int minimum, int maximum)
{
    if (value < minimum) return minimum;
    if (value > maximum) return maximum;
    return value;
}

float clampFloat(float value, float minimum, float maximum)
{
    if (value < minimum) return minimum;
    if (value > maximum) return maximum;
    return value;
}

bool isBusy()
{
    return state != CONTROLLER_IDLE && state != CONTROLLER_ERROR;
}

void setState(ControllerState nextState)
{
    state = nextState;
    status.state = nextState;
    stateStartedAt = millis();
}

void enableMotor(bool enabled)
{
    digitalWrite(MOTOR_ENABLE_PIN, enabled ? LOW : HIGH);
}

void beginMove(long steps, int speed, ControllerState nextState)
{
    enableMotor(true);
    setSpeedInStepsPerSecond(speed);
    setAccelerationInStepsPerSecondPerSecond(speed);
    setupRelativeMoveInSteps(steps);
    setState(nextState);
}

void updateShotCount()
{
    if (settings.depthOfField <= 0)
    {
        settings.totalShots = 0;
        settings.stepsPerShot = 0;
        settings.stepDistance = 0.0f;
        return;
    }

    settings.stepDistance = floorf(static_cast<float>(settings.depthOfField) * 0.9f);
    settings.stepsPerShot = clampInt(static_cast<int>(floorf(settings.stepDistance * settings.stepsPerMicron)), 1, 1000000);
    if (settings.stepsMode)
    {
        float endpointMicrons = static_cast<float>(labs(endpointDistanceSteps)) / settings.stepsPerMicron;
        settings.totalShots = static_cast<int>(ceilf(endpointMicrons / settings.stepDistance));
    }
    else
    {
        settings.totalShots = static_cast<int>(ceilf(static_cast<float>(settings.shootDistance) / settings.stepDistance));
    }
}

long nextEndpointMove()
{
    long totalSteps = labs(endpointDistanceSteps);
    long intervals = settings.totalShots - 1;
    if (intervals <= 0) return 0;

    long interval = status.currentShot + 1;
    long targetSteps = static_cast<long>(
        (static_cast<long long>(totalSteps) * interval + intervals - 1) / intervals);
    long moveSteps = targetSteps - runTravelledSteps;
    if (moveSteps <= 0) moveSteps = 1;
    runTravelledSteps = targetSteps;
    return endpointDistanceSteps < 0 ? -moveSteps : moveSteps;
}

void finishWithError(const char *message)
{
    digitalWrite(CAMERA_TRIGGER_PIN, LOW);
    enableMotor(false);
    status.captureSequenceActive = false;
    status.error = message;
    setState(CONTROLLER_ERROR);
}

void triggerCamera()
{
    digitalWrite(CAMERA_TRIGGER_PIN, HIGH);
    setState(CONTROLLER_WAIT_CAMERA);
}

void finishRun()
{
    digitalWrite(CAMERA_TRIGGER_PIN, LOW);
    enableMotor(false);
    status.captureSequenceActive = false;
    status.remainingShots = 0;
    setState(CONTROLLER_IDLE);
}

}

void controllerBegin()
{
    pinMode(CAMERA_TRIGGER_PIN, OUTPUT);
    pinMode(MOTOR_STEP_PIN, OUTPUT);
    pinMode(MOTOR_DIRECTION_PIN, OUTPUT);
    pinMode(MOTOR_ENABLE_PIN, OUTPUT);
    digitalWrite(CAMERA_TRIGGER_PIN, LOW);
    enableMotor(false);
    connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);
    controllerRecalculate();
}

void controllerRecalculate()
{
    settings.macroMagnification = clampFloat(settings.macroMagnification, 0.1f, 20.0f);
    settings.aperture = clampFloat(settings.aperture, 0.1f, 64.0f);
    settings.numericalAperture = clampFloat(settings.numericalAperture, 0.01f, 1.0f);
    settings.objectiveBaseTubeLength = clampFloat(settings.objectiveBaseTubeLength, 10.01f, 1000.0f);
    settings.objectiveActualTubeLength = clampFloat(settings.objectiveActualTubeLength, 10.01f, 1000.0f);
    settings.objectiveDesignMagnification = clampFloat(settings.objectiveDesignMagnification, 0.1f, 100.0f);
    settings.reverseFrontFocalLength = clampFloat(settings.reverseFrontFocalLength, 0.1f, 1000.0f);
    settings.reverseRearFocalLength = clampFloat(settings.reverseRearFocalLength, 0.1f, 1000.0f);
    settings.sensorType = clampInt(settings.sensorType, 0, 2);
    settings.stepsPerMicron = clampFloat(settings.stepsPerMicron, 0.01f, 100.0f);
    settings.shootDistance = clampInt(settings.shootDistance, MIN_DISTANCE, MAX_DISTANCE);
    settings.delaySeconds = clampInt(settings.delaySeconds, MIN_DELAY, MAX_DELAY);

    float calculationMagnification = settings.macroMagnification;
    if (settings.opticalMode == OPTICAL_OBJECTIVE_LENS)
    {
        calculationMagnification = settings.objectiveDesignMagnification *
            (settings.objectiveActualTubeLength - 10.0f) /
            (settings.objectiveBaseTubeLength - 10.0f);
        calculationMagnification = clampFloat(calculationMagnification, 0.1f, 100.0f);
    }
    else if (settings.opticalMode == OPTICAL_REVERSE_LENS)
    {
        calculationMagnification = settings.reverseRearFocalLength /
            settings.reverseFrontFocalLength;
    }

    settings.magnification = calculationMagnification;
    settings.effectiveAperture = settings.opticalMode == OPTICAL_OBJECTIVE_LENS
        ? calculationMagnification / (2.0f * settings.numericalAperture)
        : settings.aperture * (calculationMagnification + 1.0f);
    float circleOfConfusion = SENSOR_CIRCLE_OF_CONFUSION[settings.sensorType];
    float dof = 2.0f * circleOfConfusion * settings.effectiveAperture /
                (calculationMagnification * calculationMagnification) * 1000.0f;

    settings.depthOfField = clampInt(static_cast<int>(dof), 1, 1000000);
    updateShotCount();
    status.error = "";
}

const ControllerSettings &controllerSettings()
{
    return settings;
}

const ControllerStatus &controllerStatus()
{
    return status;
}

const char *controllerStateName()
{
    switch (state)
    {
    case CONTROLLER_IDLE: return "idle";
    case CONTROLLER_MANUAL_MOVE: return "manual_move";
    case CONTROLLER_TRIGGER: return "trigger";
    case CONTROLLER_WAIT_CAMERA: return "camera_wait";
    case CONTROLLER_RUN_MOVE: return "moving";
    case CONTROLLER_WAIT_SETTLE: return "settling";
    case CONTROLLER_RETURNING: return "returning";
    case CONTROLLER_STOPPING: return "stopping";
    case CONTROLLER_ERROR: return "error";
    }
    return "unknown";
}

void controllerSetDistance(int value)
{
    settings.shootDistance = clampInt(value, MIN_DISTANCE, MAX_DISTANCE);
    controllerRecalculate();
}

void controllerSetDelay(int value)
{
    settings.delaySeconds = clampInt(value, MIN_DELAY, MAX_DELAY);
    controllerRecalculate();
}

void controllerSetStepsPerMicron(float value)
{
    settings.stepsPerMicron = value;
    controllerRecalculate();
}

void controllerSetMagnification(float value)
{
    settings.macroMagnification = value;
    controllerRecalculate();
}

void controllerSetAperture(float value)
{
    settings.aperture = value;
    controllerRecalculate();
}

void controllerSetNumericalAperture(float value)
{
    settings.numericalAperture = value;
    controllerRecalculate();
}

void controllerSetObjectiveBaseTubeLength(float value)
{
    settings.objectiveBaseTubeLength = value;
    controllerRecalculate();
}

void controllerSetObjectiveActualTubeLength(float value)
{
    settings.objectiveActualTubeLength = value;
    controllerRecalculate();
}

void controllerSetObjectiveDesignMagnification(float value)
{
    settings.objectiveDesignMagnification = value;
    controllerRecalculate();
}

void controllerSetReverseFrontFocalLength(float value)
{
    settings.reverseFrontFocalLength = value;
    controllerRecalculate();
}

void controllerSetReverseRearFocalLength(float value)
{
    settings.reverseRearFocalLength = value;
    controllerRecalculate();
}

void controllerSetStepsMode(bool enabled)
{
    settings.stepsMode = enabled;
    controllerRecalculate();
}

void controllerSetOpticalMode(OpticalMode mode)
{
    settings.opticalMode = mode;
    controllerRecalculate();
}

void controllerSetSensorType(int value)
{
    settings.sensorType = value;
    controllerRecalculate();
}

bool controllerStartRun()
{
    if (isBusy()) return false;
    controllerRecalculate();
    if (settings.totalShots <= 0 || settings.stepsPerShot <= 0)
    {
        status.error = "Set a positive distance and valid calibration first";
        setState(CONTROLLER_ERROR);
        return false;
    }

    status.currentShot = 0;
    status.remainingShots = settings.totalShots;
    status.distanceTravelled = 0;
    returnSteps = 0;
    runTravelledSteps = 0;
    runStartPositionSteps = getCurrentPositionInSteps();
    status.captureSequenceActive = true;
    triggerCamera();
    return true;
}

void controllerStop()
{
    if (!isBusy())
    {
        status.captureSequenceActive = false;
        setState(CONTROLLER_IDLE);
        return;
    }

    digitalWrite(CAMERA_TRIGGER_PIN, LOW);
    if (state == CONTROLLER_MANUAL_MOVE || state == CONTROLLER_RUN_MOVE || state == CONTROLLER_RETURNING)
    {
        setupStop();
        setState(CONTROLLER_STOPPING);
    }
    else
    {
        enableMotor(false);
        status.captureSequenceActive = false;
        setState(CONTROLLER_IDLE);
    }
}

bool controllerStartManualMove(int direction, const char *speedMode)
{
    if (isBusy()) return false;

    int speed = SLOW_SPEED;
    long steps = settings.stepsPerShot;
    if (strcmp(speedMode, "fast") == 0)
    {
        speed = FAST_SPEED;
        steps *= 40;
    }
    else if (strcmp(speedMode, "long") == 0)
    {
        speed = FAST_SPEED;
        steps = LONG_MOVE_STEPS;
    }

    if (steps <= 0) return false;
    if (settings.stepsMode && endpointDistanceSteps == 0)
    {
        status.endpointSteps = 0;
    }
    manualMoveSteps = direction < 0 ? -steps : steps;
    beginMove(direction < 0 ? -steps : steps, speed, CONTROLLER_MANUAL_MOVE);
    return true;
}

bool controllerStartTravelTest(int direction, float distanceMm)
{
    if (isBusy() || distanceMm <= 0.0f || !isfinite(distanceMm)) return false;

    controllerRecalculate();
    long steps = lroundf(distanceMm * 1000.0f * settings.stepsPerMicron);
    if (steps <= 0) return false;

    manualMoveSteps = direction < 0 ? -steps : steps;
    beginMove(manualMoveSteps, TRAVEL_TEST_SPEED, CONTROLLER_MANUAL_MOVE);
    return true;
}

void controllerCameraTest()
{
    if (isBusy()) return;
    digitalWrite(CAMERA_TRIGGER_PIN, HIGH);
    delay(SHUTTER_PULSE_MS);
    digitalWrite(CAMERA_TRIGGER_PIN, LOW);
}

void controllerClearEndpoints()
{
    if (isBusy()) return;
    endpointDistanceSteps = 0;
    status.endpointSteps = 0;
    settings.stepsMode = true;
    updateShotCount();
    status.error = "";
}

void controllerSaveEndpointStart()
{
    if (isBusy()) return;
    endpointDistanceSteps = 0;
    status.endpointSteps = 0;
}

void controllerSaveEndpointEnd()
{
    if (isBusy()) return;
    settings.stepsMode = true;
    updateShotCount();
    returnSteps = -endpointDistanceSteps;
    if (returnSteps != 0)
    {
        beginMove(returnSteps, SLOW_SPEED, CONTROLLER_RETURNING);
    }
}

void controllerTick()
{
    unsigned long now = millis();
    switch (state)
    {
    case CONTROLLER_IDLE:
    case CONTROLLER_ERROR:
        return;

    case CONTROLLER_MANUAL_MOVE:
        if (processMovement())
        {
            enableMotor(false);
            endpointDistanceSteps += manualMoveSteps;
            status.endpointSteps = static_cast<int>(endpointDistanceSteps);
            setState(CONTROLLER_IDLE);
        }
        break;

    case CONTROLLER_TRIGGER:
        triggerCamera();
        break;

    case CONTROLLER_WAIT_CAMERA:
        if (now - stateStartedAt >= SHUTTER_PULSE_MS)
        {
            digitalWrite(CAMERA_TRIGGER_PIN, LOW);
            if (status.currentShot >= settings.totalShots - 1)
            {
                status.remainingShots = 0;
                returnSteps = runStartPositionSteps - getCurrentPositionInSteps();
                if (returnSteps == 0)
                {
                    finishRun();
                }
                else
                {
                    beginMove(returnSteps, SLOW_SPEED, CONTROLLER_RETURNING);
                }
            }
            else
            {
                long moveSteps = settings.stepsMode ? nextEndpointMove() : -settings.stepsPerShot;
                beginMove(moveSteps, SLOW_SPEED, CONTROLLER_RUN_MOVE);
            }
        }
        break;

    case CONTROLLER_RUN_MOVE:
        if (processMovement())
        {
            enableMotor(false);
            status.currentShot++;
            status.remainingShots = settings.totalShots - status.currentShot;
            status.distanceTravelled = static_cast<int>(settings.stepDistance * status.currentShot);
            setState(CONTROLLER_WAIT_SETTLE);
        }
        break;

    case CONTROLLER_WAIT_SETTLE:
        if (now - stateStartedAt >= static_cast<unsigned long>(settings.delaySeconds) * 1000UL)
        {
            setState(CONTROLLER_TRIGGER);
        }
        break;

    case CONTROLLER_RETURNING:
        if (processMovement())
        {
            enableMotor(false);
            setState(CONTROLLER_IDLE);
        }
        break;

    case CONTROLLER_STOPPING:
        if (processMovement())
        {
            enableMotor(false);
            status.captureSequenceActive = false;
            setState(CONTROLLER_IDLE);
        }
        break;
    }
}
