
#include "FlatPanel.hpp"

#define R_SENSE 0.11f

FlatPanel* FlatPanel::_instance = nullptr;

FlatPanel::FlatPanel()
{
    _instance = this;
    _serialHandler = new SaturnSerial();
    pinMode(GREEN_LED, OUTPUT);
    pinMode(BLUE_LED, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(STEPPER_ENABLE_PIN, OUTPUT);

    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(GREEN_LED, LOW);
    _lightState = 0;
    _lightBrightness = 0;
    _coverState = STATE_IDLE;
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

void FlatPanel::moveTo(long targetPosValue)
{
    if (_coverState == STATE_IDLE)    
    {
        _coverState = STATE_COVER_MOVING;
        _coverStepper->moveTo(targetPosValue);
        digitalWrite(BLUE_LED, HIGH);
    }
}

void FlatPanel::openCover()
{
    if (_coverState == STATE_IDLE)    
    {
        _coverState = STATE_COVER_MOVING;
        _coverStepper->moveTo(STEPPER_STEPS_FOR_100_DEGREES);
        digitalWrite(BLUE_LED, HIGH);
    }
}

void FlatPanel::stopCover()
{
    if (_coverState == STATE_COVER_MOVING)
    {
        _coverState = STATE_COVER_STOPPING;
        _coverStepper->stop();
    }
}

void FlatPanel::closeCover()
{
    if (_coverState == STATE_IDLE)    
    {
        _coverState = STATE_COVER_MOVING;
        _coverStepper->moveTo(0);
        digitalWrite(BLUE_LED, HIGH);
    }
}

void FlatPanel::setLightState(int state)
{
    digitalWrite(RELAY_PIN, state == 1 ? HIGH : LOW);
    digitalWrite(GREEN_LED, state == 1 ? HIGH : LOW);
    _lightState = state;
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

void FlatPanel::openLid(String parameter)
{
    _instance->openCover();
}

void FlatPanel::closeLid(String parameter)
{
    _instance->closeCover();
}

void FlatPanel::isCoverMoving(String parameter)
{
    _instance->_serialHandler->sendReply(String(_instance->_coverState == STATE_COVER_MOVING ? "1" : "0"));
}

void FlatPanel::stopStepper(String parameter)
{
    _instance->stopCover();
}

void FlatPanel::isLightOn(String parameter)
{
    _instance->_serialHandler->sendReply(String(_instance->_lightState == 1 ? "1" : "0"));
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
    _instance->moveTo(targetPosValue);
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
                if (_allCommands[i]->getExecFunction() != nullptr)
                {
                    _allCommands[i]->getExecFunction()(parameter);
                }
                break;
            }
            i++;
        }
    }
}

void FlatPanel::realTimeLoop()
{
    _coverStepper->run();
}

void FlatPanel::loop()
{
    switch (_coverState)    
    {
        case STATE_IDLE:
            break;
        
        case STATE_COVER_MOVING:
            if (_coverStepper->distanceToGo() == 0)
            {
                _coverState = STATE_IDLE;
                digitalWrite(BLUE_LED, LOW);
            }
            break;  

        case STATE_COVER_STOPPING:
            if (_coverStepper->isRunning() == false)
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
    _allCommands[2] = new Command("OPN", FlatPanel::openLid, "Open the cover");
    _allCommands[3] = new Command("CLS", FlatPanel::closeLid, "Close the cover");
    _allCommands[4] = new Command("LON", FlatPanel::turnLightOn, "Turn the light on");
    _allCommands[5] = new Command("LOF", FlatPanel::turnLightOff, "Turn the light off");
    _allCommands[6] = new Command("ILO", FlatPanel::isLightOn, "Get the state of the light");
    _allCommands[7] = new Command("ICM", FlatPanel::isCoverMoving, "Is the cover in motion?");
    _allCommands[8] = new Command("STP", FlatPanel::stopStepper, "Stop the stepper");
    _allCommands[9] = new Command("MVT", FlatPanel::moveTo, "Move to a given step position");
    _allCommands[10] = new Command("GCP", FlatPanel::getCurrentPosition, "Get the current stepper position");
    _allCommands[11] = new Command("STC", FlatPanel::setClosedPosition, "Sets the current position as the closed position");
    _allCommands[12] = new Command("HLP", FlatPanel::sendHelp, "Show this help message");
    _allCommands[13] = new Command("", nullptr, "");
};

