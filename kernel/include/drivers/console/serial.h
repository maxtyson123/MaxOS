//
// Created by 98max on 14/01/2024.
//

#ifndef MAXOS_SERIAL_H
#define MAXOS_SERIAL_H

#include <stdint.h>
#include <hardwarecommunication/port.h>
#include <drivers/driver.h>

namespace MaxOS {

  namespace drivers {


      /**
       * @class SerialConsole
       * @brief A driver for the serial output
       */
      class SerialConsole : public Driver {

        private:
            hardwarecommunication::Port8Bit m_data_port;
            hardwarecommunication::Port8Bit m_interrupt_enable_port;
            hardwarecommunication::Port8Bit m_fifo_control_port;
            hardwarecommunication::Port8Bit m_line_control_port;
            hardwarecommunication::Port8Bit m_modem_control_port;
            hardwarecommunication::Port8Bit m_line_status_port;

        public:
          static SerialConsole* s_active_serial_console;

          SerialConsole();
          ~SerialConsole();

          void put_character(char c);

      };

  }
}



#endif // MAXOS_SERIAL_H
