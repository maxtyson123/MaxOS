/**
 * @file logger.h
 * @brief Defines a Logger class for logging messages with different severity levels to multiple output streams.
 *
 * @date 4th April 2025
 * @author Max Tyson
 */

#ifndef MAXOS_COMMON_LOGGER_H
#define MAXOS_COMMON_LOGGER_H

#include <common/outputStream.h>
#include <common/colour.h>
#include <common/spinlock.h>

namespace MaxOS{


/**
 * @enum LogLevel
 * @brief Priority levels for logging messages. Different levels may be used to filter messages based on their importance depending on build type.
 */
	enum class LogLevel {
		WARNING,
		ERROR,
		HEADER,
		INFO,
		DEBUG,
	};


	constexpr uint8_t MAX_LOG_WRITERS = 5;                  ///< The maximum number of log writers that can be added to the logger
	constexpr LogLevel MAX_LOG_LEVEL = LogLevel::DEBUG;     ///< The maximum log level for this build (messages above this level will not be logged)

/**
 * @class Logger
 * @brief A class that handles logging messages to the console and files.
 */
	class Logger : public MaxOS::common::OutputStream {
		private:

			// Cant use vector as this needs to be init before the heap
			uint8_t m_log_writer_count = 0;
			OutputStream* m_log_writers[MAX_LOG_WRITERS] = { nullptr, nullptr, nullptr, nullptr, nullptr };
			bool m_log_writers_enabled[MAX_LOG_WRITERS] = { false, false, false, false, false };

			// Progress bar
			static inline uint8_t s_progress_total = 100;
			uint8_t m_progress_current = 0;

			static inline Logger* s_active_logger = nullptr;

			LogLevel m_log_level = LogLevel::INFO;

		public:
			Logger();
			~Logger();

			void add_log_writer(OutputStream* log_writer);
			void disable_log_writer(OutputStream* log_writer);

			void set_log_level(LogLevel log_level);

			void write_char(char c) final;
			void printf(const char* format, ...);

			static void ASSERT(bool condition, const char* message, ...);

			static Logger* active_logger();

			static Logger &Out();

			static Logger HEADER();
			static Logger INFO();
			static Logger DEBUG();
			static Logger WARNING();
			static Logger ERROR();

			using OutputStream::operator<<;
			Logger &operator<<(LogLevel log_level);
	};

	/**
	* @brief If the specified condition is not met then the kernel will crash with the specified message.
	*
	* This macro wraps Logger::ASSERT and supports printf-style formatting with variadic arguments.
	*
	* @param condition The condition to check.
	* @param format The format string (like printf).
	* @param ... Additional arguments to format.
	*
	* @see Logger::ASSERT
	*/
	#define ASSERT(condition, format, ...) Logger::ASSERT(condition, format, ##__VA_ARGS__)

}

#endif // MAXOS_COMMON_LOGGER_H
