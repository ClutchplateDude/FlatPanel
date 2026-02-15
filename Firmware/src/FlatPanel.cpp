
#include "FlatPanel.hpp"

const int ENDSWITCH_ACTIVE_STATE = ENDSWITCH_ACTIVE_LOW ? LOW : HIGH;
#define R_SENSE 0.11f

FlatPanel* FlatPanel::_instance = nullptr;

FlatPanel::FlatPanel()
{
    _instance = this;
    _serialHandler = new SaturnSerial();
    pinMode(ENDSWITCH_PIN, INPUT_PULLUP);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(BLUE_LED, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(STEPPER_ENABLE_PIN, OUTPUT);

    digitalWrite(RELAY_PIN, LOW);
    _switchState = digitalRead(ENDSWITCH_PIN);
    _lightState = 0;
    _lightBrightness = 0;
    _coverState = STATE_IDLE;
    _coverOpen = false;
    _allCommands = new Command*[30];
    createCommands();
}

void FlatPanel::setup()
{
    digitalWrite(BLUE_LED, HIGH);

    pinMode(PIN_STEPPER_MS1, OUTPUT);
    pinMode(PIN_STEPPER_MS2, OUTPUT);
    digitalWrite(PIN_STEPPER_MS1, LOW);
    digitalWrite(PIN_STEPPER_MS2, LOW);

    _serialHandler->init(115200);
    _coverStepper = new AccelStepper(AccelStepper::DRIVER, STEPPER_STEP_PIN, STEPPER_DIRECTION_PIN);
    _coverStepper->setMaxSpeed(STEPPER_MAX_SPEED);
    _coverStepper->setAcceleration(STEPPER_MAX_ACCELERATION);

    Serial2.begin(57600);
    _coverDriver = new TMC2209Stepper(&Serial2, R_SENSE, 0b01);
    _coverDriver->begin();
    if (_coverDriver->test_connection() != 0){
        _serialHandler->sendReply("TMC2209 connection failed!");
    }
    _coverDriver->pdn_disable(true); // keep UART active, not PDN
    _coverDriver->I_scale_analog(false);
    _coverDriver->mstep_reg_select(true);
    _coverDriver->rms_current(STEPPER_MAX_CURRENT, 0.5f);
    _coverDriver->en_spreadCycle(false);
    _coverDriver->microsteps(STEPPER_MICRO_STEPS);

    digitalWrite(STEPPER_ENABLE_PIN, LOW);  // Enable the motor driver

    
    digitalWrite(BLUE_LED, LOW);
}

void FlatPanel::openCover()
{
    if (_coverState == STATE_IDLE)    
    {
        _coverState = STATE_COVER_OPENING_MOVING_OFF_SENSOR;
        _coverStepper->moveTo(STEPPER_STEPS_FOR_100_DEGREES);
        digitalWrite(BLUE_LED, HIGH);
    }
}

void FlatPanel::stopCover()
{
    switch (_coverState)
    {
        case STATE_COVER_OPENING:
        case STATE_COVER_OPENING_MOVING_OFF_SENSOR:
            _coverState = STATE_COVER_OPENING_STOPPING;
            _coverStepper->stop();
            break;
        case STATE_COVER_CLOSING:
        case STATE_COVER_CLOSING_MOVING_OFF_SENSOR:
            _coverState = STATE_COVER_CLOSING_STOPPING;
            _coverStepper->stop();
            break;
        default:
            break;
    }
}

void FlatPanel::closeCover()
{
    if (_coverState == STATE_IDLE)    
    {
        _coverState = STATE_COVER_CLOSING_MOVING_OFF_SENSOR;
        _coverStepper->moveTo(0);
        digitalWrite(BLUE_LED, HIGH);
    }
}

void FlatPanel::setLightState(int state)
{
    digitalWrite(RELAY_PIN, state == 1 ? HIGH : LOW);
}

void FlatPanel::sendHelp(String parameter)
{
    _instance->_serialHandler->sendReply(F("\nUsage:"));
    int i = 0;
    while (_instance->_allCommands[i]->getCommand() != "")
    {
        _instance->_serialHandler->sendReply("\n" + _instance->_allCommands[i]->getCommand() + " - " + _instance->_allCommands[i]->getDescription());
        i++;
    }
    _instance->_serialHandler->sendReply("\n");
}

void FlatPanel::getVersion(String parameter)
{
    _instance->_serialHandler->sendReply(F(VERSION));
}

void FlatPanel::getDeviceId(String parameter)
{
    _instance->_serialHandler->sendReply(F("OpenFlatPanel"));
}

void FlatPanel::getSwitchState(String parameter)
{
    if (ENDSWITCH_PIN > 0)
    {
        _instance->_serialHandler->sendReply(String(_instance->_switchState == ENDSWITCH_ACTIVE_STATE ? "1" : "0"));
    }
    else
    {
        _instance->_serialHandler->sendReply(F("-1"));
    }
}
/*

void getTargetPosition(String parameter)
{
    serialHandler.sendReply(String(_stepperPanel->targetPosition()));
}

void getRemainingDistance(String parameter)
{
    serialHandler.sendReply(String(_stepperPanel->distanceToGo()));
}

void isRunning(String parameter)
{
    serialHandler.sendReply(String(_stepperPanel->isRunning()));
}

void moveBy(String parameter)
{
    long moveByValue = parameter.toInt();    
    _stepperPanel->move(moveByValue);
}

void moveTo(String parameter)
{
    long targetPosValue = parameter.toInt();
    _stepperPanel->moveTo(targetPosValue);
}

void stopStepper(String parameter)
{
    _stepperPanel->stop();
}

void setMaxSpeed(String parameter)
{
    _stepperPanel->setMaxSpeed(parameter.toFloat());
}

void setMaxAcceleration(String parameter)
{
    _stepperPanel->setAcceleration(parameter.toFloat());
}

void getMaxSpeed(String parameter)
{
    serialHandler.sendReply(String(_stepperPanel->maxSpeed()));
}

void getMaxAcceleration(String parameter)
{
    serialHandler.sendReply(String(_stepperPanel->acceleration()));
}   

void setMicrosteps(String parameter)
{
    _driver->microsteps(parameter.toInt());
}

void setRmsCurrent(String parameter)
{
    _driver->rms_current(parameter.toFloat(), 0.5f);
}

void getMicrosteps(String parameter)
{
        serialHandler.sendReply(String(_driver->microsteps()));
}
*/
void FlatPanel::openLid(String parameter)
{
    _instance->openCover();
//    _stepperPanel->move(-STEPPER_STEPS_FOR_100_DEGREES );
}

void FlatPanel::closeLid(String parameter)
{
    _instance->closeCover();
    // _stepperPanel->move(STEPPER_STEPS_FOR_100_DEGREES );
}

// void testConnection(String parameter)
// {
//     serialHandler.sendReply(String(_driver->test_connection()));
// }

void FlatPanel::stopStepper(String parameter)
{
    _instance->stopCover();
}

void FlatPanel::turnLightOn(String parameter)
{
   _instance->setLightState(1);   
}

void FlatPanel::turnLightOff(String parameter)
{
    _instance->setLightState(0);
}

void FlatPanel::moveTo(String parameter)
{
    long targetPosValue = parameter.toInt();
    Serial.println("Moving to position: " + String(targetPosValue));
    _instance->_coverStepper->moveTo(targetPosValue);
}

void FlatPanel::getCurrentPosition(String parameter)
{
    _instance->_serialHandler->sendReply(String(_instance->_coverStepper->currentPosition()));
}
void FlatPanel::setClosedPosition(String parameter)
{
    _instance->_coverStepper->setCurrentPosition(0);
}

void FlatPanel::processCommands()
{
    String newCommand;
    String parameter;
    _serialHandler->loop();
    if (_serialHandler->isCommandAvailable(newCommand, parameter))
    {
        int i = 0;
        while (_allCommands[i]->getCommand() != "")
        {
            if (_allCommands[i]->getCommand() == newCommand)
            {
                _allCommands[i]->getExecFunction()(parameter);
                break;
            }
            i++;
        }
    }
}

// void FlatPanel::processWifi(){
//     #if WIFI_ENABLED == 1
//     if (wifiHandler != nullptr) {
//         wifiHandler->loop();
//     }
//     #endif
// }

void FlatPanel::realTimeLoop()
{
    _coverStepper->run();
}

void FlatPanel::loop()
{
    const int endswitchState = digitalRead(ENDSWITCH_PIN);
    
    // Green LED shows the state of the end switch
    if (endswitchState != _switchState)
    {
        _switchState = endswitchState;
        digitalWrite(GREEN_LED, _switchState == ENDSWITCH_ACTIVE_STATE ? HIGH : LOW);
    }

    switch (_coverState)    
    {
        case STATE_IDLE:
            break;
        
        case STATE_COVER_OPENING_MOVING_OFF_SENSOR:
            if (endswitchState != ENDSWITCH_ACTIVE_STATE)
            {
                _coverState = STATE_COVER_OPENING;
            }
            break;
        
        case STATE_COVER_OPENING:
            if (endswitchState == ENDSWITCH_ACTIVE_STATE)
            {
                _coverState = STATE_COVER_OPENING_STOPPING;
                _coverStepper->stop();
            }
            break;

        case STATE_COVER_OPENING_STOPPING:
            if (_coverStepper->distanceToGo() == 0)
            {
                _coverState = STATE_IDLE;
                digitalWrite(BLUE_LED, LOW);
            }
            break;  

        case STATE_COVER_CLOSING_MOVING_OFF_SENSOR:
            if (endswitchState != ENDSWITCH_ACTIVE_STATE)
            {
                _coverState = STATE_COVER_CLOSING;
            }
            break;
        
        case STATE_COVER_CLOSING:
            if (endswitchState == ENDSWITCH_ACTIVE_STATE)
            {
                _coverState = STATE_COVER_CLOSING_STOPPING;
                _coverStepper->stop();
            }
            break;

        case STATE_COVER_CLOSING_STOPPING:
            if (_coverStepper->distanceToGo() == 0)
            {
                _coverState = STATE_IDLE;
                digitalWrite(BLUE_LED, LOW);
            }
            break;  
      
        default:
            break;
    }

    this->processCommands();
}

void FlatPanel::createCommands()
{
    _allCommands[0] = new Command("GVP", FlatPanel::getVersion, "Get the version of the firmware");
    _allCommands[1] = new Command("GID", FlatPanel::getDeviceId, "Get the device ID");
    _allCommands[2] = new Command("GSS", FlatPanel::getSwitchState, "Get the state of the end switch");
    _allCommands[3] = new Command("STP", FlatPanel::stopStepper, "Stop the stepper");
    _allCommands[4] = new Command("OPN", FlatPanel::openLid, "Open the cover");
    _allCommands[5] = new Command("CLS", FlatPanel::closeLid, "Close the cover");
    _allCommands[6] = new Command("LON", FlatPanel::turnLightOn, "Turn the light on");
    _allCommands[7] = new Command("LOF", FlatPanel::turnLightOff, "Turn the light off");
    _allCommands[8] = new Command("HLP", FlatPanel::sendHelp, "Show this help message");
    _allCommands[9] = new Command("MVT", FlatPanel::moveTo, "Move to a given step position");
    _allCommands[10] = new Command("GCP", FlatPanel::getCurrentPosition, "Get the current stepper position");
    _allCommands[11] = new Command("STC", FlatPanel::setClosedPosition, "Sets the current position as the closed position");
    // _allCommands[5] = new Command("GTP", FlatPanel::getTargetPosition, "Get the target position");
    // _allCommands[6] = new Command("GRD", FlatPanel::getRemainingDistance, "Get the remaining distance");
    // _allCommands[7] = new Command("GST", FlatPanel::getMicrosteps, "Get the current microstepping setting");
    // _allCommands[8] = new Command("ISR", FlatPanel::isRunning, "Return 1 if the stepper is running, 0 otherwise");
    // _allCommands[9] = new Command("MVB", FlatPanel::moveBy, "Move by a given number of steps");
    // _allCommands[16] = new Command("ILO", FlatPanel::isLightOn, "Get the state of the light");
    // _allCommands[17] = new Command("TST", FlatPanel::testConnection, "Test the connection to the stepper driver");
    _allCommands[12] = new Command("", nullptr, "");
};

