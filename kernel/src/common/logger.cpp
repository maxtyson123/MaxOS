//
// Created by 98max on 4/04/2025.
//
#include <common/logger.h>
#include <stdarg.h>

using namespace MaxOS;
using namespace MaxOS::common;


Logger::Logger()
:m_log_writers()
{

  // Set the logger to this
  s_active_logger = this;

}


Logger::~Logger() {


}

/**
 * @brief Adds an output stream to the logger
 *
 * @param output_stream The output stream to add
 */
void Logger::add_log_writer(OutputStream* log_writer) {

  // If the list is not empty
  if(m_log_writer_count >= m_max_log_writers)
    return;

  // Add the output stream to the list
  m_log_writers[m_log_writer_count] = log_writer;
  m_log_writers_enabled[m_log_writer_count] = true;
  m_log_writer_count++;

  // Print the setup info
  *this << LogLevel::INFO << "Logger setup: " << m_log_writer_count << "\n";
  *this << LogLevel::HEADER << "MaxOS v" << VERSION_STRING << " [ build " << BUILD_NUMBER << " ]\n";

}

/**
 * @brief Removes an output stream from the logger
 *
 * @param output_stream The output stream to remove
 */
void Logger::disable_log_writer(OutputStream* log_writer) {

  // If the list is empty
  if(m_log_writer_count == 0)
    return;

  // Find the output stream in the list
  for(int i = 0; i < m_log_writer_count; i++)
      if(m_log_writers[i] == log_writer)
        m_log_writers_enabled[i] = false;
}

/**
 * @brief Sets the log level of the logger
 *
 * @param log_level The log level to set
 */
void Logger::set_log_level(LogLevel log_level) {

  // Set the log level
  m_log_level = log_level;

  // Print the header
  switch (log_level) {

    case LogLevel::HEADER:
        *this << ANSI_COLOURS[ANSIColour::FG_Cyan] << "[\n[  \t\t\t ";
        break;
    case LogLevel::INFO:
      *this << ANSI_COLOURS[ANSIColour::FG_Green] << "[  INFO    ]" << ANSI_COLOURS[ANSIColour::FG_White] << " ";
      break;

    case LogLevel::DEBUG:
      *this << ANSI_COLOURS[ANSIColour::FG_Yellow] << "[  DEBUG   ]" << ANSI_COLOURS[ANSIColour::Reset] << "\t";
      break;

    case LogLevel::WARNING:
      *this << ANSI_COLOURS[ANSIColour::FG_Yellow] << ANSI_COLOURS[FG_White] << "[ WARNING  ]" << ANSI_COLOURS[ANSIColour::Reset] << " ";
      break;

    case LogLevel::ERROR:
      *this << ANSI_COLOURS[ANSIColour::BG_Red] << "[  ERROR   ]" << ANSI_COLOURS[ANSIColour::Reset] << " ";
      break;
  }
}

/**
 * @brief Writes a character to the logger
 *
 * @param c The character to write
 */
void Logger::write_char(char c) {

  // Write the character to all output streams
  for(int i = 0; i < m_log_writer_count; i++)
    if(m_log_writers_enabled[i] || m_log_level == LogLevel::ERROR)
      m_log_writers[i]->write_char(c);

}

/**
 * @brief Sets the log level of the logger
 *
 * @param log_level The log level to set
 * @return This logger
 */
Logger &Logger::operator<<(LogLevel log_level) {

    // Set the log level
    set_log_level(log_level);

    // Return this logger
    return *this;

}

/**
 * @brief Gets the active logger
 * @return The active logger
 */
Logger Logger::Out() {
    return *s_active_logger;
}

/**
 * @brief Gets active logger set to task level
 * @return The task logger
 */
Logger Logger::HEADER(){

    // Set the log level to task
    s_active_logger->set_log_level(LogLevel::HEADER);

    // Return the logger
    return Out();

}

/**
 * @brief Gets active logger set to info level
 * @return The info logger
 */
Logger Logger::INFO() {

    // Set the log level to info
    s_active_logger->set_log_level(LogLevel::INFO);

    // Return the logger
    return Out();

}

/**
 * @brief Gets active logger set to DEBUG level
 * @return The debug logger
 */
Logger Logger::DEBUG() {

    // Set the log level to debug
    s_active_logger->set_log_level(LogLevel::DEBUG);

    // Return the logger
    return Out();

}

/**
 * @brief Gets active logger set to WARNING level
 * @return The warning logger
 */
Logger Logger::WARNING() {

    // Set the log level to warning
    s_active_logger->set_log_level(LogLevel::WARNING);

    // Return the logger
    return Out();
}

/**
 * @brief Gets active logger set to ERROR level
 *
 * @return The error logger
 */
Logger Logger::ERROR() {

    // Set the log level to error
    s_active_logger->set_log_level(LogLevel::ERROR);

    // Return the logger
    return Out();

}

/**
 * @brief Gets the active logger (to be used to modify the logger)
 *
 * @return The active logger
 */
Logger *Logger::active_logger() {
    return s_active_logger;
}

/**
 * @brief Prints a formatted string to the logger
 *
 * @param format The format string
 * @param ... The arguments to format
 */
void Logger::printf(char const *format, ...) {

  // Create a pointer to the data
  va_list parameters;
  va_start(parameters, format);


  // Loop through the format string
  for (; *format != '\0'; format++)
  {

    // If it is not a %, print the character
    if (*format != '%')
    {
      write_char(*format);
      continue;
    }

    // Move to the next character
    format++;

    switch (*format)
    {
      case 'd':
      {
        // Print a decimal
        int number = va_arg (parameters, int);
        write_int(number);
        break;
      }
      case 'x':
      {
        // Print a hex
        uint64_t number = va_arg (parameters, uint64_t );
        write_hex(number);
        break;
      }
      case 's':
      {
        // Print a string
        char* str = va_arg (parameters, char*);
        write(str);
        break;
      }
    }
  }
}

#include <system/cpu.h>
void Logger::ASSERT(bool condition, char const *message, ...) {

  // If the condition is met then everything is ok
  if(condition)
    return;

  // Print the message
  s_active_logger -> set_log_level(LogLevel::ERROR);
  s_active_logger -> printf(message);

  // Hang the system
  system::CPU::PANIC("Check previous logs for more information");
}

/**
 * @brief Prints a new line to the logger
 *
 * @return This logger
 */
Logger Logger::Endline() {


    // Print a new line
    switch (s_active_logger -> m_log_level) {
        case LogLevel::HEADER:
            *s_active_logger << ANSI_COLOURS[ANSIColour::FG_Cyan] << "\n[" << ANSI_COLOURS[ANSIColour::Reset] << "\n";
            break;
        case LogLevel::INFO:
        case LogLevel::DEBUG:
        case LogLevel::WARNING:
        case LogLevel::ERROR:
            *s_active_logger << ANSI_COLOURS[ANSIColour::Reset] << "\n";
            break;
    
    }

    // Return this logger
    return Out();

}