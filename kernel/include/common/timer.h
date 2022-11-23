//
// Created by 98max on 23/11/2022.
//

#ifndef MAXOS_COMMON_TIMER_H
#define MAXOS_COMMON_TIMER_H

#include <common/types.h>
#include <hardwarecommunication/port.h>

namespace maxOS{

    namespace common{

        class PIT{
            private:

                hardwarecommunication::Port8Bit channel0;
                hardwarecommunication::Port8Bit channel1;
                hardwarecommunication::Port8Bit channel2;
                hardwarecommunication::Port8Bit command;


            public:
                PIT(common::uint32_t frequency);
                ~PIT();

        };

        class Timer{
        protected:



        public:
                Timer();
                ~Timer();

                static Timer* activeTimer;
                static common::uint32_t ticks;

                common::uint32_t GetTicks();
                void Wait(common::uint32_t ms);
                void UpdateTicks();
        };
    }

}

#endif //MAXOS_COMMON_TIMER_H
