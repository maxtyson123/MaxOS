//
// Created by 98max on 8/04/2023.
//

#ifndef MAXOS_COMMON_RECTANGLE_H
#define MAXOS_COMMON_RECTANGLE_H

#include <common/vector.h>

namespace MaxOS{

    namespace common{

        /**
         * @class Rectangle
         * @brief Stores the left, top, width and height of a rectangle
         *
         * @tparam Type The type of the rectangle (assumed to be numeric)
         */
        template<class Type> class Rectangle{
            public:
                Type left { 0 };
                Type top { 0 };
                Type width { 0 };
                Type height { 0 };

                Rectangle();
                Rectangle(Type left, Type top, Type width, Type height);
                ~Rectangle();

                bool intersects(const Rectangle<Type>&);
                Rectangle<Type> intersection(const Rectangle<Type>&);

                Vector<Rectangle<Type> > subtract(const Rectangle<Type>&);

                bool contains(const Rectangle<Type>&);
                bool contains(Type x, Type y);
        };

        ///_______________________________________________TEMPLATES_________________________________________________________________///

        template<class Type> Rectangle<Type>::Rectangle(){
        }


        template<class Type> Rectangle<Type>::Rectangle(Type left, Type top, Type width, Type height)
        : left(left),
          top(top),
          width(width),
          height(height)
        {

            // If the width is negative, move the left and make the width positive
            if(width < 0)
            {
                left += width;
                width *= -1;
            }

            // If the m_height is negative, move the top and make the height positive
            if(height < 0)
            {
                top += height;
                height *= -1;
            }

            // Set the values
            this->left = left;
            this->top = top;
        }

        template<class Type> Rectangle<Type>::~Rectangle()
        {
        }

        /**
         * @details Checks if the rectangle intersects with another rectangle
         *
         * @tparam Type The type of the rectangle
         * @param other The other rectangle
         * @return True if the rectangles intersect, false otherwise
         */
        template<class Type> bool Rectangle<Type>::intersects(const Rectangle<Type>& other){
            // If the other rectangle is to the left of this rectangle
            if(other.left + other.width <= left)
                return false;

            // If the other rectangle is to the right of this rectangle
            if(other.left >= left + width)
                return false;

            // If the other rectangle is above this rectangle
            if(other.top + other.height <= top)
                return false;

            // If the other rectangle is below this rectangle
            if(other.top >= top + height)
                return false;

            // If none of the above conditions are true, then the rectangles intersect
            return true;
        }

        /**
          * @details Returns a retangle that represents the intersection of this rectangle and another rectangle
          *
          * @tparam Type The type of the rectangle
          * @param other The other rectangle
          * @return The intersection of the two rectangles
          */
        template<class Type> Rectangle<Type> Rectangle<Type>::intersection(const Rectangle<Type>& other){
            // If the rectangles don't intersect, return an empty rectangle
            if(!intersects(other))
                return Rectangle<Type>();

            // Get the left and top of the intersection using the maximum of the two
            Type left = this -> left > other.left ? this -> left : other.left;
            Type top = this -> top > other.top ? this -> top : other.top;

            // Get the right and bottom of the intersection using the minimum of the two
            Type right = this -> left + this -> width < other.left + other.width ? this -> left + this -> width : other.left + other.width;
            Type bottom = this -> top + this -> height < other.top + other.height ? this -> top + this -> height : other.top + other.height;

            // Return the intersection
            return Rectangle<Type>(left, top, right - left, bottom - top);
        }

        /**
         * @details Subtracts a rectangle from this rectangle
         *
         * @tparam Type The type of the rectangle
         * @param other The rectangle to subtract
         * @return A vector of rectangles that represent the subtraction
         */
        template<class Type> Vector<Rectangle<Type> > Rectangle<Type>::subtract(const Rectangle<Type>& other){

            // Store the result rectangle
            Vector<Rectangle<Type> > result;

            // Make sure the rectangles intersect
            if(!intersects(other))
            {
              result.push_back(*this);
                return result;
            }

            // Get the minimum and maximum values for the width and height
            Type maxLeft = left > other.left ? left : other.left;
            Type minRight = (left + width < other.left + other.width ? left + width : other.left + other.width);
            Type minBottom = (top + height < other.top + other.height ? top+height : other.top + other.height);

            // Add the non-intersecting rectangles to the result

            // Add non-overlapping region above if current top is less than other top
            if (top < other.top)
              result.push_back(Rectangle<Type>(maxLeft, top, minRight - maxLeft,
                                               other.top - top));

            // Add non-overlapping region to the left if current left is less than other left
            if (left < other.left)
              result.push_back(
                  Rectangle<Type>(left, top, other.left - left, height));

            // Add non-overlapping region to the right if current right is greater than other right
            if (left + width > other.left + other.width)
              result.push_back(Rectangle<Type>(
                  other.left + other.width, top,
                  (left + width) - (other.left + other.width) + 1, height));

            // Add non-overlapping region below if current bottom is greater than other bottom
            if (this->top + this->height > other.top + other.height)
              result.push_back(Rectangle<Type>(maxLeft, minBottom,
                                               minRight - maxLeft,
                                               top + height - minBottom + 1));

            return result;
        }

        /**
        * @details  Checks if this rectangle contains another rectangle
        *
        * @tparam Type  The type of the rectangle
        * @param other The other rectangle
        * @return True if this rectangle contains the other rectangle, false otherwise
        */
        template<class Type> bool Rectangle<Type>::contains(const Rectangle<Type>& other){

            // If the other rectangle is to the left of this rectangle
            if(other.left + other.width <= left)
                return false;

            // If the other rectangle is to the right of this rectangle
            if(other.left >= left + width)
                return false;

            // If the other rectangle is above this rectangle
            if(other.top + other.height <= top)
                return false;

            // If the other rectangle is below this rectangle
            if(other.top >= top + height)
                return false;

            // If none of the above conditions are true, then the rectangles intersect
            return true;
        }


        /**
         * @details Checks if this rectangle contains a point
         *
         * @tparam Type The type of the rectangle
         * @param x The x coordinate of the point
         * @param y The y coordinate of the point
         * @return True if this rectangle contains the point, false otherwise
         */
        template<class Type> bool Rectangle<Type>::contains(Type x, Type y){

           // If the point is to the left of this rectangle
            if(x < left)
                return false;

            // If the point is to the right of this rectangle
            if(x >= left + width)
                return false;

            // If the point is above this rectangle
            if(y < top)
                return false;

            // If the point is below this rectangle
            if(y >= top + height)
                return false;

            // If none of the above conditions are true, then the point is inside the rectangle
            return true;

        }

    }
}
#endif //MAXOS_COMMON_RECTANGLE_H
