

#ifndef SpeedyStepper_h
#define SpeedyStepper_h

#include <Arduino.h>
#include <stdlib.h>


//
// the SpeedyStepper class
//
#ifdef __cplusplus
extern "C" {
#endif
 
    //
    // public functions
    //
   
    extern void connectToPins(byte stepPinNumber, byte directionPinNumber);
    
    void setStepsPerMillimeter(float motorStepPerMillimeter);
    float getCurrentPositionInMillimeters();
    void setCurrentPositionInMillimeters(float currentPositionInMillimeter);
    void setSpeedInMillimetersPerSecond(float speedInMillimetersPerSecond);
    void setAccelerationInMillimetersPerSecondPerSecond(float accelerationInMillimetersPerSecondPerSecond);
    bool moveToHomeInMillimeters(long directionTowardHome, float speedInMillimetersPerSecond, long maxDistanceToMoveInMillimeters, int homeLimitSwitchPin);
    void moveRelativeInMillimeters(float distanceToMoveInMillimeters);
    void setupRelativeMoveInMillimeters(float distanceToMoveInMillimeters);
    void moveToPositionInMillimeters(float absolutePositionToMoveToInMillimeters);
    void setupMoveInMillimeters(float absolutePositionToMoveToInMillimeters);
    float getCurrentVelocityInMillimetersPerSecond();
    void setStepsPerRevolution(float motorStepPerRevolution);
    float getCurrentPositionInRevolutions();
    void setSpeedInRevolutionsPerSecond(float speedInRevolutionsPerSecond);
    void setCurrentPositionInRevolutions(float currentPositionInRevolutions);
    void setAccelerationInRevolutionsPerSecondPerSecond(float accelerationInRevolutionsPerSecondPerSecond);
    bool moveToHomeInRevolutions(long directionTowardHome, float speedInRevolutionsPerSecond, long maxDistanceToMoveInRevolutions, int homeLimitSwitchPin);
    void moveRelativeInRevolutions(float distanceToMoveInRevolutions);
    void setupRelativeMoveInRevolutions(float distanceToMoveInRevolutions);
    void moveToPositionInRevolutions(float absolutePositionToMoveToInRevolutions);
    void setupMoveInRevolutions(float absolutePositionToMoveToInRevolutions);
    float getCurrentVelocityInRevolutionsPerSecond();

    //void enableStepper(void);
    //void disableStepper(void);
    void setCurrentPositionInSteps(long currentPositionInSteps);
    long getCurrentPositionInSteps();
    void setupStop();
    void setSpeedInStepsPerSecond(float speedInStepsPerSecond);
    void setAccelerationInStepsPerSecondPerSecond(float accelerationInStepsPerSecondPerSecond);
    bool moveToHomeInSteps(long directionTowardHome, float speedInStepsPerSecond, long maxDistanceToMoveInSteps, int homeSwitchPin);
    void moveRelativeInSteps(long distanceToMoveInSteps);
    void setupRelativeMoveInSteps(long distanceToMoveInSteps);
    void moveToPositionInSteps(long absolutePositionToMoveToInSteps);
    void setupMoveInSteps(long absolutePositionToMoveToInSteps);
    bool motionComplete();
    float getCurrentVelocityInStepsPerSecond(); 
    bool processMovement(void);


 
    //
    // private member variables
    //
    extern byte stepPin;
    extern byte directionPin;
    extern float desiredSpeed_InStepsPerSecond;
    extern float acceleration_InStepsPerSecondPerSecond;
    extern long targetPosition_InSteps;
    extern float stepsPerMillimeter;
    extern float stepsPerRevolution;
    extern bool startNewMove;
    extern float desiredStepPeriod_InUS;
    extern long decelerationDistance_InSteps;
    extern int direction_Scaler;
    extern float ramp_InitialStepPeriod_InUS;
    extern float ramp_NextStepPeriod_InUS;
    extern unsigned long ramp_LastStepTime_InUS;
    extern float acceleration_InStepsPerUSPerUS;
    extern float currentStepPeriod_InUS;
    extern long currentPosition_InSteps;

  
// ------------------------------------ End ---------------------------------
#ifdef __cplusplus
}
#endif

#endif 