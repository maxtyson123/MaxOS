/**
 * @file time.h
 * @brief Defines a Time struct for storing date and time information, along with functions for time manipulation.
 *
 * @date 10th April 2023
 * @author Max Tyson
 */

#ifndef MAXOS_COMMON_TIME_H
#define MAXOS_COMMON_TIME_H

#include <stdint.h>

namespace MaxOS{

    namespace common{

        /**
         * @struct Time
         * @brief Stores the year, month, day, hour, minute and second of a time.
         *
         * @typedef time_t
         * @brief Alias for Time struct
         */
        typedef struct Time{

            uint16_t year;              ///< The year
            uint8_t month;              ///< The month (1-12)
            uint8_t day;                ///< The day (1-31)

            uint8_t hour;               ///< The hour (0-23)
            uint8_t minute;             ///< The minute (0-59)
            uint8_t second;             ///< The second (0-59)

			/**
			 * @brief Checks if the year is a leap year
			 * @return True if the year is a leap year, false otherwise
			 */
            [[nodiscard]] bool is_leap_year() const {
                return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
            }

        } time_t;

		/// List of month names indexed by month number - 1
        static const char* Months[] = {
            "January",
            "February",
            "March",
            "April",
            "May",
            "June",
            "July",
            "August",
            "September",
            "October",
            "November",
            "December"
        };

		/// List of day names starting from Sunday ending on Saturday
        static const char* Days[] = {
            "Sunday",
            "Monday",
            "Tuesday",
            "Wednesday",
            "Thursday",
            "Friday",
            "Saturday"
        };

		/// Number of days in each month indexed by month number - 1
        constexpr uint8_t DAYS_IN_MONTH[] = {
            31, // January
            28, // February
            31, // March
            30, // April
            31, // May
            30, // June
            31, // July
            31, // August
            30, // September
            31, // October
            30, // November
            31  // December
        };

        constexpr uint16_t DAYS_PER_YEAR = 365;         ///< Number of days in a non-leap year
	    constexpr uint16_t DAYS_PER_LEAP_YEAR = 366;    ///< Number of days in a leap year

        /**
         * @brief Converts a time to an epoch
         *
         * @param time The time representation
         * @return The epoch time
         */
        static  uint64_t time_to_epoch(Time time){
                uint64_t epoch = 0;

                // Add the number of years
                for(uint16_t year = 1970; year < time.year; year++)
                        epoch += (time.is_leap_year() ? DAYS_PER_LEAP_YEAR : DAYS_PER_YEAR);


                // Add the number of days in the current year
                for(uint8_t month = 0; month < time.month - 1; month++)
                    epoch += DAYS_IN_MONTH[month];

                // Add the number of days in the current month
                epoch += time.day - 1;

                // Add the number of hours
                epoch *= 24;
                epoch += time.hour;

                // Add the number of minutes
                epoch *= 60;
                epoch += time.minute;

                // Add the number of seconds
                epoch *= 60;
                epoch += time.second;

                return epoch;
        }

    }

}

#endif //MAXOS_COMMON_TIME_H
