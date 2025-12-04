/**
 * @file pair.h
 * @brief Defines a Pair class for storing two related objects together.
 *
 * @date 9th April 2023
 * @author Max Tyson
 */

#ifndef MAXOS_PAIR_H
#define MAXOS_PAIR_H


namespace MaxOS::common {

	/**
	 * @class Pair
	 * @brief A pair of two objects
	 *
	 * @tparam First The type of the first object
	 * @tparam Second The type of the second object
	 */
	template<class First, class Second> class Pair {

		public:
			First first;                                    ///< The first object (often the key)
			Second second;                                  ///< The second object (often the value)

			Pair();
			Pair(First, Second);
			~Pair();
	};

	///_____________________________Implementation___________________________________________///
	template<class First, class Second> Pair<First, Second>::Pair() = default;

	/**
	 * @brief Creates a new pair
	 *
	 * @tparam First The type of the first object
	 * @tparam Second The type of the second object
	 * @param first The first object
	 * @param second The second object
	 */
	template<class First, class Second> Pair<First, Second>::Pair(First first, Second second)
			: first(first),
			second(second) {

	}

	template<class First, class Second> Pair<First, Second>::~Pair()
	= default;
}


#endif //MAXOS_PAIR_H
