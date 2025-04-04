//
// Created by 98max on 15/01/2024.
//

#include <drivers/console/serial.h>

using namespace MaxOS;
using namespace MaxOS::drivers;

SerialConsole::SerialConsole()
: m_data_port(0x3F8),
  m_interrupt_enable_port(0x3F9),
  m_fifo_control_port(0x3FA),
  m_line_control_port(0x3FB),
  m_modem_control_port(0x3FC),
  m_line_status_port(0x3FD)
{

  // Disable all interrupts
  m_interrupt_enable_port.write(0x00);

  // Enable DLAB (set baud rate divisor)
  m_line_control_port.write(0x80);

  // Set divisor to 3
  m_data_port.write(0x03);
  m_interrupt_enable_port.write(0x00);

  // 8 bits, no parity, one stop bit
  m_line_control_port.write(0x03);

  // Enable FIFO, clear them, with 14-byte threshold
  m_fifo_control_port.write(0xC7);

  // IRQs enabled, RTS/DSR set
  m_modem_control_port.write(0x0B);

  // Test serial chip
  m_modem_control_port.write(0x1E);
  m_data_port.write(0xAE);
  if (m_data_port.read() != 0xAE)
    return;

  // Enable serial chip
  m_modem_control_port.write(0x0F);

  // Set the active serial console
  s_active_serial_console = this;

}

SerialConsole::~SerialConsole() {

    // If this is the active serial console, set it to null
    if (s_active_serial_console == this)
            s_active_serial_console = nullptr;

}

/**
 * @brief Waits for the serial port to be ready, then writes a character to it
 *
 * @param c The character to write
 */
void SerialConsole::put_character(char c) {

    // Wait for the serial port to be ready
    while (0 == (m_line_status_port.read() & 0x20));

    // Write the character
    m_data_port.write(c);

}
void SerialConsole::write_char(char c) {
  put_character(c);
}
