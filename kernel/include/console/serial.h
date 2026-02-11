/**
 * @file serial.h
 * @brief Defines a SerialConsole driver for serial output
 *
 * @date 14th January 2024
 * @author Max Tyson
 */

#ifndef MAXOS_SERIAL_H
#define MAXOS_SERIAL_H

#include <port.h>
#include <common/logger.h>


namespace MaxOS::console {


	/**
	 * @class SerialConsole
	 * @brief A driver for the serial output
	 */
	class SerialConsole : public common::OutputStream {

		private:
			common::Port8Bit m_data_port;
			common::Port8Bit m_interrupt_enable_port;
			common::Port8Bit m_fifo_control_port;
			common::Port8Bit m_line_control_port;
			common::Port8Bit m_modem_control_port;
			common::Port8Bit m_line_status_port;

		public:
			explicit SerialConsole(Logger* logger);
			~SerialConsole();

			void put_character(char c);
			void write_char(char c) final;

	};

}


#endif // MAXOS_SERIAL_H
