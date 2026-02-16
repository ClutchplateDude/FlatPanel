#pragma once
#include <Arduino.h>
#include "AccelStepper.h"
#include "SaturnSerial.hpp"
#include "TmcStepper.h"
#include "Command.hpp"

#include "../Configuration.hpp"

typedef enum CoverState_t {
    STATE_IDLE,
    STATE_COVER_MOVING,
    STATE_COVER_STOPPING,
} CoverState_t;


class FlatPanel {
    AccelStepper *_coverStepper;
    TMC2209Stepper *_coverDriver;
    SaturnSerial* _serialHandler;
    int _lightState;
    int _lightBrightness;
    CoverState_t _coverState;

    Command** _allCommands;
    static FlatPanel* _instance;

public:
    FlatPanel();
    void setup();
    void loop();
    void realTimeLoop();
    void createCommands();
    void setLightState(int state);
    
    void stopCover();
    void openCover();
    void closeCover();
    void moveTo(long targetPosValue);

    // Direct command handlers
    static void getVersion(String parameter);
    static void getDeviceId(String parameter);
    static void openLid(String parameter);
    static void closeLid(String parameter);
    static void turnLightOn(String parameter);
    static void turnLightOff(String parameter);
    static void isLightOn(String parameter);
    static void isCoverMoving(String parameter);
    static void stopStepper(String parameter);
    static void moveTo(String parameter);
    static void getCurrentPosition(String parameter);
    static void setClosedPosition(String parameter);
    static void sendHelp(String parameter);

    void processCommands();
};

