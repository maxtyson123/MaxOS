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

            /**
             * @class TimeEvent
             * @brief Event that is triggered when the clock ticks, holds the current time
             */
            class TimeEvent : public common::Event<ClockEvents>{
                public:
                    common::Time* time;
                    TimeEvent(common::Time*);
                    ~TimeEvent();
            };

            /**
             * @class ClockEventHandler
             * @brief Handles the events triggered by the clock
             */
            class ClockEventHandler : public common::EventHandler<ClockEvents>{

                public:
                    ClockEventHandler();
                    ~ClockEventHandler();

                    common::Event<ClockEvents>* on_event(common::Event<ClockEvents>* event) override;

                    virtual void on_time(const common::Time& time);

            };

            /**
             * @class Clock
             * @brief Driver for the CMOS Real Time Clock
             */
            class Clock: public Driver, public hardwarecommunication::InterruptHandler, public common::EventManager<ClockEvents>{
                private:

                  uint64_t m_ticks;

                protected:
                  
                    bool m_binary_coded_decimal_representation;

                    // Ports
                    hardwarecommunication::Port8Bit m_data_port;
                    hardwarecommunication::Port8Bit m_command_port;

                    // Time between events
                    uint16_t m_ticks_between_events { 0 };
                    uint16_t m_ticks_until_next_event { 1 };

                    // Other functions
                    void handle_interrupt() final;
                    uint8_t read_hardware_clock(uint8_t address);
                    uint8_t binary_representation(uint8_t number);

                public:
                    Clock(hardwarecommunication::InterruptManager* interrupt_manager, uint16_t time_between_events = 10);
                    ~Clock();

                    void activate() override;
                    void delay(uint32_t milliseconds);

                    string get_vendor_name() final;
                    string get_device_name() final;
            };

        }

    }
}

#endif //MAXOS_DRIVERS_CLOCK_CLOCK_H
