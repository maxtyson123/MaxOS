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

                Pair() {

                };

                Pair(First, Second) {
                    this->first = first;
                    this->second = second;
                };

                ~Pair() {

                };
            };
    }
}
#endif //MAXOS_PAIR_H
