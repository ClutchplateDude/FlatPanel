/*
  SaturnSerial.hpp - Parser for a LX200-like command protocol.
  Copyright (c) 2025 Lutz Kretzschmar.  All right reserved.

  Remarks:

    A command begins with the semiolon character ':' and ends with the hash character '#'.
    All commands have 3 characters and are ASCII encoded.
    Commands may have one parameter directly following the command characters.
    Commands can have replies, which end with the hash character '#'.
*/

#ifndef SATURN_SERIAL_HPP
#define SATURN_SERIAL_HPP

#if ARDUINO < 100
    #include <Wprogram.h>
#else
    #include <Arduino.h>
#endif

#define SIZE_INPUT_BUFFER  16  // Incoming command buffer size. No command is longer than 16 characters.
#define SIZE_OUTPUT_BUFFER 8   // Outgoing answer buffer size. No answer is longer than 8 characters.

class SaturnSerial
{
  public:
    SaturnSerial();

    // Initialize the serial protocol with the given baud rate    
    void init(int baudRate);

    // Check whether a new command is available and return it if available. No leading ':' and trailing '#' included.
    bool isCommandAvailable(String& commandBuffer, String& parameterBuffer);

    // Send a reply with the given data. Appends the trailing '#' automatically.
    void sendReply(String reply);

    // Process the serial port for new commands
    void loop();

  private:
    bool _isCommandAvailable;
    char _currentCommand[SIZE_INPUT_BUFFER];
    char _currentReply[SIZE_OUTPUT_BUFFER];
    int _readIndex;

    void readNewAscii();
 };

#endif  // SaturnSerial_h
