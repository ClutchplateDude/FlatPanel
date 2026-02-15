#pragma once
#include <Arduino.h>
#include "AccelStepper.h"
#include "SaturnSerial.hpp"
#include "TmcStepper.h"

#include "../Configuration.hpp"

class Command {
    String _command;
    String _description;
    void (*_execFunction)(String parameter);

public:
    Command(String command, void (*execFunction)(String parameter), String description) 
    {   
        this->_command = command;
        this->_description = description;
        this->_execFunction = execFunction;
    }
    String getCommand() { return _command; }
    String getDescription() { return _description; }
    void (*getExecFunction())(String parameter) { return _execFunction; }    
};

