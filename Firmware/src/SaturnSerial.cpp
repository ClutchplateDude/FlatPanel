/*
  SaturnSerial.cpp - Parser for a LX200-like command protocol.
  Copyright (c) 2025 Lutz Kretzschmar.  All right reserved.

*/

#include "SaturnSerial.hpp"

///////////////////////////////////////////////////////////////////
SaturnSerial::SaturnSerial()
{
    int i;

    _isCommandAvailable = false;
    _readIndex = 0;

    for (i = 0; i < SIZE_INPUT_BUFFER; i++)
    {
        _currentCommand[i] = 0;
    }

    for (i = 0; i < SIZE_OUTPUT_BUFFER; i++)
    {
        this->_currentReply[i] = 0;
    }
}

///////////////////////////////////////////////////////////////////
void SaturnSerial::init(int baudRate)
{
    // Start the serial port at the given baudrate.
    Serial.begin(baudRate);

}

///////////////////////////////////////////////////////////////////
// Check whether a new command is available and return it if available. 
// No leading ':' and trailing '#' included.
bool SaturnSerial::isCommandAvailable(String& command, String& parameter)
{
    command = "";
    parameter = "";
    if (_isCommandAvailable)
    {
        // Copy command to the provided buffers
        int i = 0;
        char commandBuffer[SIZE_INPUT_BUFFER];
        char parameterBuffer[SIZE_INPUT_BUFFER];
        if (_readIndex < 3)
        {
            // Ignore this invalid command (too short)
            _isCommandAvailable = false;
            _readIndex = 0;
            return false; 
        }
        commandBuffer[0] = _currentCommand[0]; 
        commandBuffer[1] = _currentCommand[1];
        commandBuffer[2] = _currentCommand[2];
        commandBuffer[3] = 0; // Null-terminate the string

        // If more characters are present, they are the parameter
        int j = 0;
        while (_currentCommand[3 + j] != 0)
        {
            parameterBuffer[j] = _currentCommand[3 + j];
            j++;
        }
        parameterBuffer[j] = 0; // Null-terminate the string

        _isCommandAvailable = false; // Reset flag
        _readIndex = 0;

        command = String(commandBuffer);
        parameter = String(parameterBuffer);
        return true;
    }
    return false;
}

// Send a reply with the given data. Appends the trailing '#' automatically.
void SaturnSerial::sendReply(String reply)
{
    int i = 0;
    while (reply[i] != 0)    
    {
        Serial.write(reply[i]);
        i++;
    }

    // Send the terminating '#'
    Serial.write('#');
}

// Process the serial port for new commands
void SaturnSerial::loop()
{
    if (Serial.available() > 0)
    {
        int i;
        char newByte = 0;

        newByte = Serial.read();
        
        // Check for message begin ':' character 
        if (newByte == ':')
        {
            _isCommandAvailable = false;
            _readIndex = 0;
            // Ignore the byte
        }
        else
        {
            // Check for message end '#' character 
            if (newByte == '#')
            {
                _isCommandAvailable = true;
                // Null-terminate the command string
                if (_readIndex < SIZE_INPUT_BUFFER)
                {
                    _currentCommand[_readIndex] = 0;
                    _readIndex++;
                }
                else
                {
                    _currentCommand[SIZE_INPUT_BUFFER - 1] = 0;
                }
            }
            else
            {
                // Assume we are still reading the command
                if (_readIndex < SIZE_INPUT_BUFFER)
                {
                    _currentCommand[_readIndex] = newByte;
                    _readIndex++;
                }
            }
        }
    }
}



