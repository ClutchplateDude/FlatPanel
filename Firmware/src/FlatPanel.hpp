#pragma once
#include <Arduino.h>
#include "AccelStepper.h"
#include "SaturnSerial.hpp"
#include "TmcStepper.h"
#include "Command.hpp"

#include "../Configuration.hpp"

typedef enum CoverState_t {
    STATE_IDLE,
    STATE_COVER_OPENING_MOVING_OFF_SENSOR,
    STATE_COVER_OPENING,
    STATE_COVER_OPENING_STOPPING,
    STATE_COVER_CLOSING_MOVING_OFF_SENSOR,
    STATE_COVER_CLOSING,
    STATE_COVER_CLOSING_STOPPING,
} CoverState_t;


class FlatPanel {
    AccelStepper *_coverStepper;
    TMC2209Stepper *_coverDriver;
    SaturnSerial* _serialHandler;
    int _switchState;
    int _lightState;
    int _lightBrightness;
    CoverState_t _coverState;
    bool _coverOpen;

    Command** _allCommands;
    static FlatPanel* _instance;

public:
    FlatPanel();
    void setup();
    void loop();
    void realTimeLoop();
    void createCommands();
    
    // Direct command handlers
    static void getVersion(String parameter);
    static void getDeviceId(String parameter);
    static void getSwitchState(String parameter);
    static void openLid(String parameter);
    static void closeLid(String parameter);
    static void turnLightOn(String parameter);
    static void turnLightOff(String parameter);
    static void stopStepper(String parameter);
    static void sendHelp(String parameter);
    static void moveTo(String parameter);
    static void getCurrentPosition(String parameter);
    static void setClosedPosition(String parameter);

    void switchActivated();
    void switchDeactivated();
    void processCommands();
    void processEndSwitch();
    //void processStepper();

    bool isCoverMoving();
    void openCover();
    void closeCover();
    void stopCover();
    int getLightState();
    void setLightState(int state);
    void setLightBrightness(int brightness);
    void getLightBrightness();
    void getMaxLightBrightness();
    bool isConnected();
};

