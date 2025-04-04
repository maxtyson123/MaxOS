//
// Created by 98max on 4/04/2025.
//

#ifndef MAXOS_COMMON_LOGGER_H
#define MAXOS_COMMON_LOGGER_H

#include <common/outputStream.h>
#include <common/colour.h>
#include <common/version.h>



        /**
         * @enum LogLevel
         * @brief Enum for the different log levels
         */
         enum class LogLevel
         {
             HEADER,
             INFO,
             DEBUG,
             WARNING,
             ERROR,
         };

        /**
         * @class Logger
         * @brief A class that handles logging messages to the console and files.
         */
         class Logger : public MaxOS::common::OutputStream
        {
            private:

                // Cant use vector as this needs to be init before the heap
                uint8_t m_log_writer_count { 0 };
                static const uint8_t m_max_log_writers { 5 };
                OutputStream* m_log_writers[m_max_log_writers];
                bool m_log_writers_enabled[m_max_log_writers] { false };

                static inline Logger* s_active_logger = nullptr;
                LogLevel m_log_level { LogLevel::INFO };

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

                static Logger Out();
                static Logger Endline();

                static Logger HEADER();
                static Logger INFO();
                static Logger DEBUG();
                static Logger WARNING();
                static Logger ERROR();

                using OutputStream::operator<<;
                Logger& operator << (LogLevel log_level);
        };

        #define ASSERT(condition, format, ...) Logger::ASSERT(condition, format, ##__VA_ARGS__)
#endif // MAXOS_COMMON_LOGGER_H
