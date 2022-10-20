//
// Created by 98max on 10/4/2022.
//

#ifndef MAX_OS_HARDWARECOMMUNICATION_PORT_H
#define MAX_OS_HARDWARECOMMUNICATION_PORT_H

#include <common/types.h>
namespace maxOS {
    namespace hardwarecommunication {
        //Base Class for ports
        class Port {
        protected:  //Protected so that it cant be instantiated bc its purely virtual
            maxOS::common::uint16_t portnumber;

            Port(maxOS::common::uint16_t portnumber);

            ~Port();
        };

        class Port8Bit : public Port {
        public:
            //Constructor / Deconstructor
            Port8Bit(maxOS::common::uint16_t portnumber);

            ~Port8Bit();

            //Read / Write Method
            virtual void Write(maxOS::common::uint8_t data);

            virtual maxOS::common::uint8_t Read();
        };

        class Port8BitSlow : public Port8Bit {
        public:
            //Constructor / Deconstructor
            Port8BitSlow(maxOS::common::uint16_t portnumber);

            ~Port8BitSlow();

            //Read / Write Method
            virtual void Write(maxOS::common::uint8_t data);
            //---Inherits read methods
        };

        class Port16Bit : public Port {
        public:
            //Constructor / Deconstructor
            Port16Bit(maxOS::common::uint16_t portnumber);

            ~Port16Bit();

            //Read / Write Method
            virtual void Write(maxOS::common::uint16_t data);

            virtual maxOS::common::uint16_t Read();
        };

        class Port32Bit : public Port {
        public:
            //Constructor / Deconstructor
            Port32Bit(maxOS::common::uint16_t portnumber);

            ~Port32Bit();

            //Read / Write Method
            virtual void Write(maxOS::common::uint32_t data);

            virtual maxOS::common::uint32_t Read();
        };
    }
}

#endif //MAX_OS_HARDWARECOMMUNICATION_PORT_H

//NOTES
//1-protected : members can be used by objects of the derived classes but not by objects of the base class.
//2-virtual : if the base class and the derived class both have the same function [same name, same input parameters, same return type], then this function is overriden . So to make sure that when calling a member function from an object of the derived class , that this function called is the function that belongs to the derived class not to the base class , we use the virtual keyword to tell the compiler we want to resolve to the most derived class.
//3-static : static member functions can be called without creating an object for this class .
//4-inline : when code is compiled , a call to a function is replaced with the function's code rather than calling the function which improves performance .