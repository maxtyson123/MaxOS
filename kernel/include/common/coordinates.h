/**
 * @file coordinates.h
 * @brief Defines a simple Coordinates type as a pair of int32_t values.
 *
 * @date 9th April 2023
 * @author Max Tyson
 */

#ifndef MAXOS_COMMON_COORDINATES_H
#define MAXOS_COMMON_COORDINATES_H

#include <cstdint>
#include <common/pair.h>


namespace MaxOS::common {

	/**
	 * @typedef Coordinates
	 * @brief A type representing 2D coordinates as a pair of int32_t
	 */
	typedef Pair<int32_t, int32_t> Coordinates;

}


#endif //MAXOS_COMMON_COORDINATES_H
