#include <Arduino.h>
#include "AccelStepper.h"
#include "TmcStepper.h"

#include "../Configuration.hpp"

#include "FlatPanel.hpp"

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

}

void loop()
{
    _flatPanel->loop();
}
