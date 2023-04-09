//
// Created by 98max on 8/04/2023.
//

#ifndef MAXOS_COMMON_RECTANGLE_H
#define MAXOS_COMMON_RECTANGLE_H

#include <common/vector.h>

namespace maxOS{

    namespace common{

        // A rectangle template for use in the GUI system
        template<class T> class Rectangle{
            public:
                T left;
                T top;
                T width;
                T height;

                Rectangle();
                Rectangle(T left, T top, T width, T height);
                ~Rectangle();

                bool intersects(const Rectangle<T>& other);
                Rectangle<T> intersection(const Rectangle<T>& other);
                Vector<Rectangle<T> > subtract(const Rectangle<T>& other);
                bool contains(const Rectangle<T>& other);
                bool contains(T x, T y);
        };

    }

}


#include "../../src/common/rectangle_templates.cpp"
#endif //MAXOS_COMMON_RECTANGLE_H
