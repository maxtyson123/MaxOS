//
// Created by 98max on 8/04/2023.
//

#ifndef MAXOS_COMMON_RECTANGLE_H
#define MAXOS_COMMON_RECTANGLE_H

#include <common/vector.h>

namespace maxOS{

    namespace common{

        // A rectangle template for use in the GUI system
        template<class Type> class Rectangle{
            public:
                Type left;
                Type top;
                Type width;
                Type height;

                Rectangle();
                Rectangle(Type left, Type top, Type width, Type height);
                ~Rectangle();

                bool intersects(const Rectangle<Type>& other);
                Rectangle<Type> intersection(const Rectangle<Type>& other);
                Vector<Rectangle<Type> > subtract(const Rectangle<Type>& other);
                bool contains(const Rectangle<Type>& other);
                bool contains(Type x, Type y);
        };

    }

}


#include "../../src/common/rectangle_templates.cpp"
#endif //MAXOS_COMMON_RECTANGLE_H
