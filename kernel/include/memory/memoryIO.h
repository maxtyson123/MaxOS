//
// Created by 98max on 11/29/2022.
//

#ifndef MAXOS_MEMORY_MEMORYINPUTOUTPUT_H
#define MAXOS_MEMORY_MEMORYINPUTOUTPUT_H

#include <common/types.h>

namespace maxOS{
    namespace memory{

        //Base class for memory I/o
        class MemIO {
            protected:  //Protected so that it cant be instantiated bc its purely virtual
                common::uint64_t address;

                MemIO(common::uint64_t address);
                ~MemIO();
            };

        class MemIO8Bit : public MemIO {
            public:
                //Constructor / Deconstructor
                MemIO8Bit(common::uint64_t address);
                ~MemIO8Bit();

                //Read / Write function
                virtual void Write(common::uint8_t data);

                virtual common::uint8_t Read();
            };


        class MemIO16Bit : public MemIO {
            public:
                //Constructor / Deconstructor
                MemIO16Bit(common::uint64_t address);
                ~MemIO16Bit();

                //Read / Write function
                virtual void Write(common::uint16_t data);

                virtual common::uint16_t Read();
            };

        class MemIO32Bit : public MemIO {
            public:
                //Constructor / Deconstructor
                MemIO32Bit(common::uint64_t address);
                ~MemIO32Bit();

                //Read / Write function
                virtual void Write(common::uint32_t data);

                virtual common::uint32_t Read();
            };

        class MemIO64Bit : public MemIO {
        public:
            //Constructor / Deconstructor
            MemIO64Bit(common::uint64_t address);
            ~MemIO64Bit();

            //Read / Write function
            virtual void Write(common::uint64_t data);

            virtual common::uint64_t Read();
        };


        void* memcpy(void* destination, const void* source, common::uint32_t num);


    }
}

#endif //MAXOS_MEMORY_MEMORYINPUTOUTPUT_H
