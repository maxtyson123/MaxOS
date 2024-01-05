//
// Created by 98max on 11/29/2022.
//

#ifndef MAXOS_MEMORY_MEMORYINPUTOUTPUT_H
#define MAXOS_MEMORY_MEMORYINPUTOUTPUT_H

#include <stdint.h>
#include <stddef.h>

namespace maxOS{
    namespace memory{

        /**
         * @class MemIO
         * @brief base class for all memory IO
         */
        class MemIO {
            protected:
                uint32_t m_address;

                MemIO(uint32_t address);
                ~MemIO();
            };

        /**
         * @class MemIO8Bit
         * @brief Handles 8 bit memory IO
         */
        class MemIO8Bit : public MemIO {
            public:
                MemIO8Bit(uint32_t address);
                ~MemIO8Bit();

                virtual void write(uint8_t data);
                virtual uint8_t read();
            };

        /**
         * @class MemIO16Bit
         * @brief Handles 16 bit memory IO
         */
        class MemIO16Bit : public MemIO {
            public:
                MemIO16Bit(uint32_t address);
                ~MemIO16Bit();

                virtual void write(uint16_t data);
                virtual uint16_t read();
            };

        /**
         * @class MemIO32Bit
         * @brief Handles 32 bit memory IO
         */
        class MemIO32Bit : public MemIO {
            public:
                MemIO32Bit(uint32_t address);
                ~MemIO32Bit();

                virtual void write(uint32_t data);
                virtual uint32_t read();
            };

        /**
         * @class MemIO64Bit
         * @brief Handles 64 bit memory IO
         */
        class MemIO64Bit : public MemIO {
        public:
            MemIO64Bit(uint32_t address);
            ~MemIO64Bit();

            virtual void write(uint64_t data);
            virtual uint64_t read();
        };


        static void* memcpy(void* destination, const void* source, uint32_t num);
        static void* memset(void* ptr, int value, uint32_t num);
        static void* memmove(void* destination, const void* source, uint32_t num);
        static int memcmp(const void* ptr1, const void* ptr2, uint32_t num);
    }
}

#endif //MAXOS_MEMORY_MEMORYINPUTOUTPUT_H
