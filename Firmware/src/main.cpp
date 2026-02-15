#include <Arduino.h>
#include "AccelStepper.h"
#include "TmcStepper.h"

#include "../Configuration.hpp"

#include "FlatPanel.hpp"

#if (WIFI_ENABLED == 1)
    #include "SaturnWifi.hpp"
#endif



#if (WIFI_ENABLED == 1)
SaturnWifi *wifiHandler = nullptr;
#endif

FlatPanel *_flatPanel = nullptr;

TaskHandle_t RealtimeLoopTask;

// This is the task for simulating periodic interrupts on ESP32 platforms.
// It should do very minimal work, only calling Mount::interruptLoop() to step the stepper motors as needed.
// This task function is run on Core 0 of the ESP32 and never returns
void IRAM_ATTR realtimeLoopTask(void *payload)
{
    FlatPanel *flatPanelCopy = reinterpret_cast<FlatPanel *>(payload);
    for (;;)
    {
        flatPanelCopy->realTimeLoop();
        // vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

/*
void sendHelp(String parameter)
{
    serialHandler.sendReply(F("\nUsage:"));
    int i = 0;
    while (Command::allCommands[i].getCommand() != "")
    {
        serialHandler.sendReply("\n" + Command::allCommands[i].getCommand() + " - " + Command::allCommands[i].getDescription());
        i++;
    }
    serialHandler.sendReply("\n");
}

void getVersion(String parameter)
{
    serialHandler.sendReply(F(VERSION));
}

void getDeviceId(String parameter)
{
    serialHandler.sendReply(F("OpenFlatPanel"));
}

void getSwitchState(String parameter)
{
    if (ENDSWITCH_PIN > 0)
    {
        serialHandler.sendReply(String(_switchState == ENDSWITCH_ACTIVE_STATE ? "1" : "0"));
    }
    else
    {
        serialHandler.sendReply(F("-1"));
    }
}

void getCurrentPosition(String parameter)
{
    serialHandler.sendReply(String(_stepperPanel->currentPosition()));
}

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

void openLid(String parameter)
{
    _stepperPanel->move(-STEPPER_STEPS_FOR_100_DEGREES );
}

void closeLid(String parameter)
{
    _stepperPanel->move(STEPPER_STEPS_FOR_100_DEGREES );
}

void testConnection(String parameter)
{
    serialHandler.sendReply(String(_driver->test_connection()));
}





void processCommands()
{
    String newCommand;
    String parameter;
    serialHandler.loop();
    if (serialHandler.isCommandAvailable(newCommand, parameter))
    {
        int i = 0;
        while (Command::allCommands[i].getCommand() != "")
        {
            if (Command::allCommands[i].getCommand() == newCommand)
            {
                Command::allCommands[i].getExecFunction()(parameter);
                break;
            }
            i++;
        }
    }
}
*/

void setup()
{
    _flatPanel = new FlatPanel();
    _flatPanel->setup();

    disableCore0WDT();
    xTaskCreatePinnedToCore(realtimeLoopTask,  // Function to run on this core
                            "RealtimeLoop",    // Name of this task
                            32767,               // Stack space in bytes
                            (void*)_flatPanel,              // payload
                            1,                   // Priority (2 is higher than 1)
                            &RealtimeLoopTask,        // The location that receives the thread id
                            0);                  // The core to run this on

    // #if (WIFI_ENABLED == 1)
    //     wifiHandler = new SaturnWifi();
    //     wifiHandler->init();
    //     wifiHandler->mountFileSystem();

    //     wifiHandler->registerRoute("/", SaturnHttpMethod::Get, []() {
    //         if (!wifiHandler->tryServeStaticFile("/")) {
    //             wifiHandler->send(503, "text/plain", "Static content not available");
    //         }
    //     });

    //     wifiHandler->registerRoute("/api/status", SaturnHttpMethod::Get, []() {
    //         String payload = "{";
    //         payload += "\"uptimeMs\":" + String(millis());
    //         payload += ",\"stepperRunning\":" + String(_stepperPanel->isRunning() ? "true" : "false");
    //         payload += ",\"currentPosition\":" + String(_stepperPanel->currentPosition());
    //         payload += ",\"targetPosition\":" + String(_stepperPanel->targetPosition());
    //         payload += ",\"remainingDistance\":" + String(_stepperPanel->distanceToGo());
    //         payload += ",\"maxSpeed\":" + String(_stepperPanel->maxSpeed(), 3);
    //         payload += ",\"maxAcceleration\":" + String(_stepperPanel->acceleration(), 3);
    //         payload += ",\"microsteps\":" + String(_driver->microsteps());
    //         payload += ",\"rmsCurrent\":" + String(_driver->rms_current(), 3);
    //         payload += ",\"endSwitchActive\":" + String((_switchState == ENDSWITCH_ACTIVE_STATE) ? "true" : "false");
    //         payload += "}";
    //         wifiHandler->send(200, "application/json", payload);
    //     });

    //     wifiHandler->registerRouteNotFound();

    //     if (!wifiHandler->connectToWiFi(WIFI_HOSTNAME, WIFI_SSID, WIFI_WPAKEY))
    //     {
    //         Serial.println("Failed to connect to WiFi");
    //         //     Serial.println("Failed to connect as station.");
    //         //     if (WIFI_AP_FALLBACK_ENABLED) {
    //         //         Serial.println("Starting WiFi access point fallback.");
    //         //         WiFi.mode(WIFI_AP);
    //         //         WiFi.softAPsetHostname(WIFI_HOSTNAME);

    //         //         IPAddress apIP(192, 168, 4, 1);
    //         //         IPAddress apGateway(192, 168, 4, 1);
    //         //         IPAddress apSubnet(255, 255, 255, 0);
    //         //         if (!WiFi.softAPConfig(apIP, apGateway, apSubnet)) {
    //         //             Serial.println("Failed to configure SoftAP IP");
    //         //         }

    //         //         bool apStarted = WiFi.softAP(WIFI_SSID, WIFI_WPAKEY);
    //         //         if (!apStarted) {
    //         //             Serial.println("Failed to start WiFi access point");
    //         //         } else {
    //         //             IPAddress fallbackIP = WiFi.softAPIP();
    //         //             Serial.println("WiFi access point started");
    //         //             Serial.println("SSID: " + String(WIFI_SSID));
    //         //             Serial.print("AP IP address: ");
    //         //             Serial.println(fallbackIP);
    //         //             webServerReady = true;
    //         //         }
    //         //     }
    //         // }

    //         // if (webServerReady) {
    //         //     _webServer.begin();
    //         // } else {
    //         //     Serial.println("Web server not started (no network available).");
    //         // }
    //     }
    //     else
    //     {
    //         digitalWrite(GREEN_LED, HIGH);
    //         delay(10);
    //         digitalWrite(GREEN_LED, LOW);
    //         delay(50);
    //         digitalWrite(GREEN_LED, HIGH);
    //         delay(10);
    //         digitalWrite(GREEN_LED, LOW);
    //     }
    // #endif
}

/*
void switchActivated()
{
    Serial.println("Switch activated");
    _stepperPanel->stop();
}

void switchDeactivated()
{
    Serial.println("Switch deactivated");
}

void processEndSwitch()
{
    const int state = digitalRead(ENDSWITCH_PIN);
    if (state != _switchState) {
        if (state == ENDSWITCH_ACTIVE_STATE) {
            digitalWrite(GREEN_LED, HIGH);
            switchActivated();
        }
        else 
        {
            digitalWrite(GREEN_LED, LOW);
            switchDeactivated();
        }
        _switchState = state;
    }
}

void processWifi(){
    #if WIFI_ENABLED == 1
    if (wifiHandler != nullptr) {
        wifiHandler->loop();
    }
    #endif
}

void processStepper()
{
    if (millis() - timestamp > 20)
    {
        timestamp = millis();
        if (_stepperPanel->distanceToGo() != 0)
        {
            digitalWrite(BLUE_LED, HIGH);
        }
        else
        {
            digitalWrite(BLUE_LED, LOW);
        }
    }
}
*/
void loop()
{
    // if (ENDSWITCH_PIN > 0)
    // {   
    //     processEndSwitch();
    // }
    
    // processWifi();
    // processStepper();
    // processCommands();
    _flatPanel->loop();
}
