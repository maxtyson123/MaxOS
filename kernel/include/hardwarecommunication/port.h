//
// Created by 98max on 10/4/2022.
//

#ifndef MAX_OS_HARDWARECOMMUNICATION_PORT_H
#define MAX_OS_HARDWARECOMMUNICATION_PORT_H

#include <stdint.h>
namespace maxOS {
    namespace hardwarecommunication {

        /**
         * @class Port
         * @brief base class for all ports
         */
        class Port {
            protected:
                uint16_t m_port_number;

                Port(uint16_t port_number);
                ~Port();
        };

        /**
         * @class Port8Bit
         * @brief Handles 8 bit ports
         */
        class Port8Bit : public Port {
            public:
                Port8Bit(uint16_t port_number);
                ~Port8Bit();

                virtual void write(uint8_t data);
                virtual uint8_t read();
        };

        /**
         * @class Port8BitSlow
         * @brief Handles 8 bit ports (slow)
         */
        class Port8BitSlow : public Port8Bit {
            public:
                Port8BitSlow(uint16_t port_number);
                ~Port8BitSlow();

                virtual void write(uint8_t data) final;
        };

        /**
         * @class Port16Bit
         * @brief Handles 16 bit ports
         */
        class Port16Bit : public Port {
            public:
                Port16Bit(uint16_t port_number);
                ~Port16Bit();

                virtual void write(uint16_t data);
                virtual uint16_t read();
        };

        /**
         * @class Port32Bit
         * @brief Handles 32 bit ports
         */
        class Port32Bit : public Port {
            public:
                Port32Bit(uint16_t port_number);
                ~Port32Bit();

                virtual void write(uint32_t data);
                virtual uint32_t read();
        };
    }
}

#endif //MAX_OS_HARDWARECOMMUNICATION_PORT_H