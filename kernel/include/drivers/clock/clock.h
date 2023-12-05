//
// Created by 98max on 10/04/2023.
//

#ifndef MAXOS_DRIVERS_CLOCK_CLOCK_H
#define MAXOS_DRIVERS_CLOCK_CLOCK_H

#include <stdint.h>
#include <common/time.h>
#include <drivers/driver.h>
#include <hardwarecommunication/interrupts.h>
#include <common/vector.h>
#include <common/eventHandler.h>

namespace maxOS {

    namespace drivers {

        namespace clock {

            enum ClockEvents{
                TIME
            };

            class TimeEvent : public common::Event<ClockEvents>{
                public:
                    common::Time* time;
                    TimeEvent(common::Time* time);
                    ~TimeEvent();
            };
            class ClockEventHandler : public common::EventHandler<ClockEvents>{

                public:
                    ClockEventHandler();
                    ~ClockEventHandler();

                    common::Event<ClockEvents>* onEvent(common::Event<ClockEvents>* event);

                    virtual void onTime(const common::Time& time);

            };

            class Clock: public Driver, public hardwarecommunication::InterruptHandler, public common::EventManager<ClockEvents>{
                private:
                    volatile uint64_t ticks;        // Ensure that the compiler does not optimize this variable out of the code (volatile)

                protected:
                    // Store all the event clockEventHandlers
                    common::Vector<ClockEventHandler*> clockEventHandlers;
                    bool binaryCodedDecimalRepresentation;

                    // Ports
                    hardwarecommunication::Port8Bit dataPort;
                    hardwarecommunication::Port8Bit commandPort;

                    // Time between events
                    uint16_t ticksBetweenEvents;
                    uint16_t ticksUntilNextEvent;

                    // Other functions
                    void HandleInterrupt();
                    uint8_t readHardwareClock(uint8_t address);
                    uint8_t binaryRepresentation(uint8_t number);

                public:
                    Clock(hardwarecommunication::InterruptManager* interruptManager, uint16_t timeBetweenEvents = 10);
                    ~Clock();

                    void activate();
                    void delay(uint32_t milliseconds);

                    string getVendorName();
                    string getDeviceName();
            };

        }

    }
}

#endif //MAXOS_DRIVERS_CLOCK_CLOCK_H
