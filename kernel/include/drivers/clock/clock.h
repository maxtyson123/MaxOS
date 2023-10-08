//
// Created by 98max on 10/04/2023.
//

#ifndef MAXOS_DRIVERS_CLOCK_CLOCK_H
#define MAXOS_DRIVERS_CLOCK_CLOCK_H

#include <common/types.h>
#include <common/time.h>
#include <drivers/driver.h>
#include <hardwarecommunication/interrupts.h>
#include <common/vector.h>

namespace maxOS {

    namespace drivers {

        namespace clock {

            class ClockEventHandler {

                public:
                    ClockEventHandler();
                    ~ClockEventHandler();

                    virtual void onTime(const common::Time& time);

            };

            class Clock: public Driver, public hardwarecommunication::InterruptHandler{
                private:
                    volatile common::uint64_t ticks;        // Enusre that the compiler does not optimize this variable out of the code (volatile)

                protected:
                    // Store all the event clockEventHandlers
                    common::Vector<ClockEventHandler*> clockEventHandlers;
                    bool binaryCodedDecimalRepresentation;

                    // Ports
                    hardwarecommunication::Port8Bit dataPort;
                    hardwarecommunication::Port8Bit commandPort;

                    // Time between events
                    common::uint16_t ticksBetweenEvents;
                    common::uint16_t ticksUntilNextEvent;

                    // Other functions
                    common::uint32_t HandleInterrupt(common::uint32_t esp);
                    common::uint8_t readHardwareClock(common::uint8_t address);
                    common::uint8_t binaryRepresentation(common::uint8_t number);

                public:
                    Clock(hardwarecommunication::InterruptManager* interruptManager, common::uint16_t timeBetweenEvents = 10);
                    ~Clock();

                    void activate();
                    void connectClockEventHandler(ClockEventHandler* clockEventHandler);
                    void disconnectClockEventHandler(ClockEventHandler* clockEventHandler);
                    void delay(common::uint32_t milliseconds);

                    common::string getVendorName();
                    common::string getDeviceName();
            };

        }

    }
}

#endif //MAXOS_DRIVERS_CLOCK_CLOCK_H
