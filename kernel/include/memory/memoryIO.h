//
// Created by 98max on 11/29/2022.
//

#ifndef MAXOS_MEMORY_MEMORYINPUTOUTPUT_H
#define MAXOS_MEMORY_MEMORYINPUTOUTPUT_H

#include <stdint.h>
#include <stddef.h>

namespace maxOS{
    namespace memory{

        //Base class for memory I/o
        class MemIO {
            protected:  //Protected so that it cant be instantiated bc its purely virtual
                uint64_t address;

                MemIO(uint64_t address);
                ~MemIO();
            };

        class MemIO8Bit : public MemIO {
            public:
                //Constructor / Deconstructor
                MemIO8Bit(uint64_t address);
                ~MemIO8Bit();

                //Read / Write function
                virtual void Write(uint8_t data);

                virtual uint8_t Read();
            };


        class MemIO16Bit : public MemIO {
            public:
                //Constructor / Deconstructor
                MemIO16Bit(uint64_t address);
                ~MemIO16Bit();

                //Read / Write function
                virtual void Write(uint16_t data);

                virtual uint16_t Read();
            };

        class MemIO32Bit : public MemIO {
            public:
                //Constructor / Deconstructor
                MemIO32Bit(uint64_t address);
                ~MemIO32Bit();

                //Read / Write function
                virtual void Write(uint32_t data);

                virtual uint32_t Read();
            };

        class MemIO64Bit : public MemIO {
        public:
            //Constructor / Deconstructor
            MemIO64Bit(uint64_t address);
            ~MemIO64Bit();

            //Read / Write function
            virtual void Write(uint64_t data);

            virtual uint64_t Read();
        };


        void* memcpy(void* destination, const void* source, uint32_t num);


    }
}

#endif //MAXOS_MEMORY_MEMORYINPUTOUTPUT_H
