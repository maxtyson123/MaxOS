//
// Created by 98max on 14/01/2024.
//

#ifndef MAXOS_SERIAL_H
#define MAXOS_SERIAL_H

#include <hardwarecommunication/port.h>
#include <drivers/driver.h>
#include <common/logger.h>

namespace MaxOS {

  namespace drivers {


      /**
       * @class SerialConsole
       * @brief A driver for the serial output
       */
        class SerialConsole : public Driver, public common::OutputStream{

        private:
            hardwarecommunication::Port8Bit m_data_port;
            hardwarecommunication::Port8Bit m_interrupt_enable_port;
            hardwarecommunication::Port8Bit m_fifo_control_port;
            hardwarecommunication::Port8Bit m_line_control_port;
            hardwarecommunication::Port8Bit m_modem_control_port;
            hardwarecommunication::Port8Bit m_line_status_port;

        public:
          SerialConsole(Logger* logger);
          ~SerialConsole();

          void put_character(char c);
          void write_char(char c) final;

      };

  }
}



#endif // MAXOS_SERIAL_H
