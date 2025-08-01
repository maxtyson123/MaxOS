//
// Created by 98max on 4/04/2025.
//

#ifndef MAXOS_COMMON_LOGGER_H
#define MAXOS_COMMON_LOGGER_H

#include <common/outputStream.h>
#include <common/colour.h>



        /**
         * @enum LogLevel
         * @brief Priority levels for logging messages. Different levels may be used to filter messages based on their importance depending on build type.
         */
         enum class LogLevel
         {
             WARNING,
             ERROR,
             HEADER,
             INFO,
             DEBUG,
         };


        /**
         * @class Logger
         * @brief A class that handles logging messages to the console and files.
         */
         class Logger : public MaxOS::common::OutputStream
         {
            private:

                // Cant use vector as this needs to be init before the heap
                uint8_t m_log_writer_count = 0;
                static const uint8_t m_max_log_writers = 5;
                OutputStream* m_log_writers[m_max_log_writers] = {nullptr, nullptr, nullptr, nullptr, nullptr};
                bool m_log_writers_enabled[m_max_log_writers] = {false, false, false, false, false};

                // Progress bar
                static inline uint8_t s_progress_total = 100;
                uint8_t m_progress_current = 0;

                static inline Logger* s_active_logger = nullptr;

                LogLevel m_log_level = LogLevel::INFO;
                inline static LogLevel s_max_log_level = LogLevel::DEBUG;

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

                static Logger& Out();

                static Logger HEADER();
                static Logger INFO();
                static Logger DEBUG();
                static Logger WARNING();
                static Logger ERROR();

                using OutputStream::operator<<;
                Logger& operator << (LogLevel log_level);
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

#endif // MAXOS_COMMON_LOGGER_H
