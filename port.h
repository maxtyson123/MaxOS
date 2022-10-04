//
// Created by 98max on 10/4/2022.
//

#ifndef MAX_OS_PORT_H
#define MAX_OS_PORT_H

#include "types.h"

    //Base Class for ports
    class Port{
        protected:  //Protected so that it cant be instantiated bc its purely virtual
            uint16_t portnumber;
            Port(uint16_t portnumber);
            ~Port();
    };

    class Port8Bit : public Port{
        public:
            //Constructor / Deconstructor
            Port8Bit(uint16_t portnumber);
            ~Port8Bit();
            //Read / Write Method
            virtual void Write(uint8_t data);
            virtual uint8_t Read();
    };

    class Port8BitSlow : public Port8Bit{
    public:
        //Constructor / Deconstructor
        Port8BitSlow(uint16_t portnumber);
        ~Port8BitSlow();
        //Read / Write Method
        virtual void Write(uint8_t data);
        //---Inherits read methods
    };

    class Port16Bit : public Port{
    public:
        //Constructor / Deconstructor
        Port16Bit(uint16_t portnumber);
        ~Port16Bit();
        //Read / Write Method
        virtual void Write(uint16_t data);
        virtual uint16_t Read();
    };

    class Port32Bit : public Port{
    public:
        //Constructor / Deconstructor
        Port32Bit(uint16_t portnumber);
        ~Port32Bit();
        //Read / Write Method
        virtual void Write(uint32_t data);
        virtual uint32_t Read();
    };

#endif //MAX_OS_PORT_H
