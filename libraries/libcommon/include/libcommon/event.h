/**
 * @file event.h
 * @brief Defines classes for handling events and event managers.
 *
 * @date 9th October 2023
 * @author Max Tyson
 */

#ifndef MAXOS_COMMON_EVENTHANDLER_H
#define MAXOS_COMMON_EVENTHANDLER_H

#include <cstdint>

#include <libcommon/vector.h>


namespace MaxOS::common {

	class EventHandler {

		public:

			EventHandler();
			~EventHandler();

			void on_sample_event(int x, int y);

	};


}


#endif //MAXOS_COMMON_EVENTHANDLER_H
