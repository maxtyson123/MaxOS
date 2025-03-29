//
// Created by 98max on 10/04/2023.
//

#ifndef MAXOS_DRIVERS_CLOCK_CLOCK_H
#define MAXOS_DRIVERS_CLOCK_CLOCK_H

#include <stdint.h>
#include <common/time.h>
#include <drivers/driver.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/apic.h>
#include <common/vector.h>
#include <common/eventHandler.h>

namespace MaxOS {

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


            enum Channel {
              CHANNEL_0 = 0,
              CHANNEL_1 = 1,
              CHANNEL_2 = 2,
            };

            enum AccessMode {
              LATCH_COUNT = 0,
              LOW_BYTE = 1,
              HIGH_BYTE = 2,
              LOW_HIGH_BYTE = 3,
            };

            enum OperatingMode {
              MODE_0 = 0, // Interrupt on Terminal Count
              MODE_1 = 1, // Hardware Re-triggerable One-Shot
              MODE_2 = 2, // Rate Generator
              MODE_3 = 3, // Square Wave Generator
              MODE_4 = 4, // Software Triggered Strobe
              MODE_5 = 5, // Hardware Triggered Strobe
            };

            enum BCDMode {
              BINARY = 0,
              BCD = 1,
            };

            typedef struct {
              uint8_t bcd_mode        : 1; // Bit 0: BCD/Binary mode
              uint8_t operating_mode  : 3; // Bits 1-3: Mode of operation
              uint8_t access_mode     : 2; // Bits 4-5: Access mode
              uint8_t channel         : 2; // Bits 6-7: Channel
            } PITCommand;


            // Forward declaration
            class Clock;

            /**
             * @class PIT
             * @brief Driver for the Programmable Interval Timer
             */
            class PIT: public hardwarecommunication::InterruptHandler{
              friend Clock;

              private:

                uint64_t m_ticks { 0 };
                static const uint16_t s_calibrate_ticks = 10;

              protected:

                // Ports
                hardwarecommunication::Port8Bit m_data_port;
                hardwarecommunication::Port8Bit m_command_port;

                // APIC
                hardwarecommunication::LocalAPIC* m_local_apic;
                hardwarecommunication::IOAPIC* m_io_apic;

                // Funcs
                void handle_interrupt() final;
                uint32_t ticks_per_ms();

              public:
                PIT(hardwarecommunication::InterruptManager* interrupt_manager, hardwarecommunication::AdvancedProgrammableInterruptController* apic);
                ~PIT();

            };

            /**
             * @class Clock
             * @brief Driver for the APIC clock
             */
            class Clock: public Driver, public hardwarecommunication::InterruptHandler, public common::EventManager<ClockEvents>{
                private:

                  uint64_t m_ticks { 0 };

                protected:
                  
                    bool m_binary;
                    bool m_24_hour_clock;

                    // Ports
                    hardwarecommunication::Port8Bit m_data_port;
                    hardwarecommunication::Port8Bit m_command_port;

                    // APIC
                    hardwarecommunication::AdvancedProgrammableInterruptController* m_apic;

                    // Time between events
                    uint16_t m_ticks_between_events { 0 };
                    uint16_t m_ticks_until_next_event { 1 };

                    // Other functions
                    void handle_interrupt() final;
                    uint8_t read_hardware_clock(uint8_t address);
                    uint8_t binary_representation(uint8_t number);

                public:
                    Clock(hardwarecommunication::InterruptManager* interrupt_manager, hardwarecommunication::AdvancedProgrammableInterruptController* apic, uint16_t time_between_events = 10);
                    ~Clock();

                    void activate() override;
                    void delay(uint32_t milliseconds);

                    void calibrate(uint64_t ms_per_tick = 1);
                    common::Time get_time();

                    string get_vendor_name() final;
                    string get_device_name() final;
            };

        }

    }
}

#endif //MAXOS_DRIVERS_CLOCK_CLOCK_H
