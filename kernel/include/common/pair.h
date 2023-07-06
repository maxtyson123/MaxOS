//
// Created by 98max on 9/04/2023.
//

#ifndef MAXOS_PAIR_H
#define MAXOS_PAIR_H

namespace maxOS {

    namespace common {

        template<class First, class Second> class Pair {
            public:
                First first;
                Second second;

                Pair();
                Pair(First, Second);
                ~Pair();
        };

        //_____________________________Implementation___________________________________________
        template<class First, class Second> Pair<First,Second>::Pair()
        {
        }

        template<class First, class Second> Pair<First,Second>::Pair(First first, Second second)
        {
            this->first = first;
            this->second = second;
        }

        template<class First, class Second> Pair<First,Second>::~Pair()
        {
        }


        //Note, The GUI fucked it self soley because pair was implmented inline ffs

    }

}
#endif //MAXOS_PAIR_H
