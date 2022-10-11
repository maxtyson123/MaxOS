//
// Created by 98max on 10/4/2022.
//

#ifndef MAX_OS_HARDWARECOMMUNICATION_PORT_H
#define MAX_OS_HARDWARECOMMUNICATION_PORT_H

#include <common/types.h>
namespace maxos {
    namespace hardwarecommunication {
        //Base Class for ports
        class Port {
        protected:  //Protected so that it cant be instantiated bc its purely virtual
            maxos::common::uint16_t portnumber;

            Port(maxos::common::uint16_t portnumber);

            ~Port();
        };

        class Port8Bit : public Port {
        public:
            //Constructor / Deconstructor
            Port8Bit(maxos::common::uint16_t portnumber);

            ~Port8Bit();

            //Read / Write Method
            virtual void Write(maxos::common::uint8_t data);

            virtual maxos::common::uint8_t Read();
        };

        class Port8BitSlow : public Port8Bit {
        public:
            //Constructor / Deconstructor
            Port8BitSlow(maxos::common::uint16_t portnumber);

            ~Port8BitSlow();

            //Read / Write Method
            virtual void Write(maxos::common::uint8_t data);
            //---Inherits read methods
        };

        class Port16Bit : public Port {
        public:
            //Constructor / Deconstructor
            Port16Bit(maxos::common::uint16_t portnumber);

            ~Port16Bit();

            //Read / Write Method
            virtual void Write(maxos::common::uint16_t data);

            virtual maxos::common::uint16_t Read();
        };

        class Port32Bit : public Port {
        public:
            //Constructor / Deconstructor
            Port32Bit(maxos::common::uint16_t portnumber);

            ~Port32Bit();

            //Read / Write Method
            virtual void Write(maxos::common::uint32_t data);

            virtual maxos::common::uint32_t Read();
        };
    }
}

#endif //MAX_OS_HARDWARECOMMUNICATION_PORT_H
