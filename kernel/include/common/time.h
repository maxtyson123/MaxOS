//
// Created by 98max on 10/04/2023.
//

#ifndef MAXOS_COMMON_TIME_H
#define MAXOS_COMMON_TIME_H

#include <stdint.h>

namespace MaxOS{

    namespace common{

        /**
         * @struct Time
         * @brief Stores the year, month, day, hour, minute and second of a time.
         */
        typedef struct Time{

            uint16_t year;
            uint8_t month;
            uint8_t day;

            uint8_t hour;
            uint8_t minute;
            uint8_t second;

            [[nodiscard]] bool is_leap_year() const {
                return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
            }

        } time_t;

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

        static const char* Days[] = {
            "Sunday",
            "Monday",
            "Tuesday",
            "Wednesday",
            "Thursday",
            "Friday",
            "Saturday"
        };

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

        constexpr uint16_t DAYS_PER_YEAR = 365;
	    constexpr uint16_t DAYS_PER_LEAP_YEAR = 366;

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
