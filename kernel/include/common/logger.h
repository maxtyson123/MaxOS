/**
 * @file logger.h
 * @brief Defines a Logger class for logging messages with different severity levels to multiple output streams.
 *
 * @date 4th April 2025
 * @author Max Tyson
 *
 * @todo move somewhere else (maybe even commonlib), rename common/ to generated/
 */

#ifndef MAXOS_COMMON_LOGGER_H
#define MAXOS_COMMON_LOGGER_H

#include <outputStream.h>
#include <colour.h>
#include <spinlock.h>

namespace MaxOS {


	/**
	 * @enum LogLevel
	 * @brief Priority levels for logging messages. Different levels may be used to filter messages based on their importance depending on build type.
	 */
	enum class LogLevel {
		ERROR,
		WARNING,
		HEADER,
		INFO,
		TEST,
		DEBUG,
	};


	constexpr uint8_t MAX_LOG_WRITERS = 5;                  ///< The maximum number of log writers that can be added to the logger
#ifdef NDEBUG
	constexpr LogLevel MAX_LOG_LEVEL = LogLevel::INFO;      ///< The maximum log level for this build (messages above this level will not be logged)
#else
	constexpr LogLevel MAX_LOG_LEVEL = LogLevel::DEBUG;     ///< The maximum log level for this build (messages above this level will not be logged)
#endif

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

			static void assert(bool condition, const char* message, ...);

			static Logger* active_logger();

			static Logger& Out();

			static Logger ERROR();
			static Logger WARNING();
			static Logger HEADER();
			static Logger INFO();
			static Logger TEST();
			static Logger DEBUG();

			using OutputStream::operator <<;
			Logger& operator <<(LogLevel log_level);
	};
}

#endif // MAXOS_COMMON_LOGGER_H
