/**
 * @file logger.cpp
 * @brief Implements a Logger class for logging messages to multiple output streams
 *
 * @date 4th April 2025
 * @author Max Tyson
 */

#include <common/logger.h>
#include <stdarg.h>
#include <drivers/console/vesaboot.h>
#include <common/version.h>
#include <system/cpu.h>
#include <processes/scheduler.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::drivers::console;
using namespace MaxOS::processes;
using namespace MaxOS::system;

/**
 * @brief Constructs a Logger object and sets it as the active logger
 */
Logger::Logger()
: m_log_writers()
{

	s_active_logger = this;

	// The following line is generated automatically by the MaxOS build system.
	s_progress_total = 22;

}


Logger::~Logger() = default;

/**
 * @brief Adds an output stream to the logger
 *
 * @param log_writer The output stream to add
 */
void Logger::add_log_writer(OutputStream* log_writer) {

	// If the list is not empty
	if (m_log_writer_count >= MAX_LOG_WRITERS)
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
 * @param log_writer The output stream to remove
 */
void Logger::disable_log_writer(OutputStream* log_writer) {

	// If the list is empty
	if (m_log_writer_count == 0)
		return;

	// Find the output stream in the list
	for (int i = 0; i < m_log_writer_count; i++)
		if (m_log_writers[i] == log_writer)
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

	// Update the progress bar
	if (log_level == LogLevel::INFO) {
		VESABootConsole::update_progress_bar((m_progress_current * 100) / s_progress_total);
		m_progress_current++;
	}

	// Print the header
	switch (log_level) {

		case LogLevel::HEADER:
			*this << ANSI_COLOURS[ANSIColour::FG_Blue] << "[  BOOT    ] ";
			break;

		case LogLevel::INFO:
			*this << ANSI_COLOURS[ANSIColour::FG_Cyan] << "[  INFO    ]" << ANSI_COLOURS[ANSIColour::FG_White] << " ";
			break;

		case LogLevel::TEST:
			*this << ANSI_COLOURS[ANSIColour::FG_Green] << "[  TEST    ]" << ANSI_COLOURS[ANSIColour::FG_White] << " ";
			break;

		case LogLevel::DEBUG:
			*this << ANSI_COLOURS[ANSIColour::FG_Yellow] << "[  DEBUG   ]" << ANSI_COLOURS[ANSIColour::Reset] << " ";
			break;

		case LogLevel::WARNING:
			*this << ANSI_COLOURS[ANSIColour::BG_Yellow] << ANSI_COLOURS[FG_White] << "[ WARNING  ]" << ANSI_COLOURS[ANSIColour::Reset] << " ";
			break;

		case LogLevel::ERROR:
			*this << ANSI_COLOURS[ANSIColour::BG_Red] << "[  ERROR   ]" << ANSI_COLOURS[ANSIColour::Reset] << " ";
			break;
	}

	GlobalScheduler::print_running_header();
}


/**
 * @brief Writes a character to the logger
 *
 * @param c The character to write
 */
void Logger::write_char(char c) {

	// Ensure logging at this level is enabled
	if (m_log_level > MAX_LOG_LEVEL)
		return;

	// Write the character to all output streams
	for (int i = 0; i < m_log_writer_count; i++)
		if (m_log_writers_enabled[i])
			m_log_writers[i]->write_char(c);

}

/**
 * @brief Gets the active logger
 *
 * @return The active logger
 */
Logger &Logger::Out() {

	return *active_logger();
}

/**
 * @brief Gets active logger set to task level
 *
 * @return The task logger
 */
Logger Logger::HEADER() {

	s_active_logger->set_log_level(LogLevel::HEADER);
	return Out();

}

/**
 * @brief Gets active logger set to info level
 *
 * @return The info logger
 */
Logger Logger::INFO() {

	s_active_logger->set_log_level(LogLevel::INFO);
	return Out();
}

/**
 * @brief Gets active logger set to test level
 *
 * @return The test logger
 */
Logger Logger::TEST() {

	s_active_logger->set_log_level(LogLevel::TEST);
	return Out();
}

/**
 * @brief Gets active logger set to DEBUG level
 *
 * @return The debug logger
 */
Logger Logger::DEBUG() {

	s_active_logger->set_log_level(LogLevel::DEBUG);
	return Out();

}

/**
 * @brief Gets active logger set to WARNING level
 *
 * @return The warning logger
 */
Logger Logger::WARNING() {

	s_active_logger->set_log_level(LogLevel::WARNING);
	return Out();
}

/**
 * @brief Gets active logger set to ERROR level
 *
 * @return The error logger
 */
Logger Logger::ERROR() {

	s_active_logger->set_log_level(LogLevel::ERROR);
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

	write(String::formatted(format, parameters));
}

/**
 * @brief Puts the system into a panic state if the condition is false, printing the message first
 *
 * @param condition The condition to check if is met
 * @param message The message to print if the condition fails
 */
void Logger::ASSERT(bool condition, char const *message, ...) {

	// If the condition is met then everything is ok
	if (condition)
		return;

	va_list parameters;
	va_start(parameters, message);

	// Hang the system
	system::CPU::PANIC(String::formatted(message, parameters).c_str());
}

/**
 * @brief Sets the log level of the logger
 *
 * @param log_level The log level to set
 * @return This logger
 */
Logger &Logger::operator<<(LogLevel log_level) {

	set_log_level(log_level);
	return *this;

}