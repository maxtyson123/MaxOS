//
// Created by 98max on 9/04/2023.
//

#ifndef MAXOS_PAIR_H
#define MAXOS_PAIR_H

namespace MaxOS {

    namespace common {

        /**
         * @class Pair
         * @brief A pair of two objects
         *
         * @tparam First The type of the m_first_memory_chunk object
         * @tparam Second The type of the second object
         */
        template<class First, class Second> class Pair {
            public:
                First first;
                Second second;

                Pair();
                Pair(First, Second);
                ~Pair();
        };

        ///_____________________________Implementation___________________________________________///
        template<class First, class Second> Pair<First,Second>::Pair()
        {
        }

        template<class First, class Second> Pair<First,Second>::Pair(First first, Second second)
        : first(first),
          second(second)
        {

        }

        template<class First, class Second> Pair<First,Second>::~Pair()
        {
        }
    }

}
#endif //MAXOS_PAIR_H
