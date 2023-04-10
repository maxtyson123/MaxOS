//
// Created by 98max on 8/04/2023.
//

namespace maxOS{

    namespace common{

        /**
         * @details A rectangle template for use in the GUI system
         *
         * @tparam T Type of the rectangle
         */
        template<class T> Rectangle<T>::Rectangle(){
            left = 0;
            top = 0;
            height = 0;
            width = 0;
        }

        /**
         * @details A rectangle template for use in the GUI system
         *
         * @tparam T The type of the rectangle
         * @param left
         * @param top
         * @param width
         * @param height
         */
        template<class T> Rectangle<T>::Rectangle(T left, T top, T width, T height){
            // If the width is negative, adjust the left
            if(width < 0)
            {
                left += width;      // Add the width to the left, what this does is move the left to the right
                width *= -1;        // Multiply the width by -1, what this does is make the width positive
            }

            // If the height is negative, adjust the top
            if(height < 0)
            {
                top += height;     // Add the height to the top, what this does is move the top down
                height *= -1;      // Multiply the height by -1, what this does is make the height positive
            }

            // Set the values
            this->left = left;
            //this -> right = left + width; # The right is not needed, it can be calculated from the left and width
            this->top = top;
            //this -> bottom = top + height; # The bottom is not needed, it can be calculated from the top and height
            this->width = width;
            this->height = height;
        }

        /**
         * @details Destructor
         *
         * @tparam T The type of the rectangle
         */
        template<class T> Rectangle<T>::~Rectangle()
        {
        }


        /**
         * @details Checks if the rectangle intersects with another rectangle
         *
         * @tparam T The type of the rectangle
         * @param other The other rectangle
         * @return True if the rectangles intersect, false otherwise
         */
        template<class T> bool Rectangle<T>::intersects(const Rectangle<T>& other){
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
         * @details Returns the intersection of this rectangle and another rectangle
         *
         * @tparam T The type of the rectangle
         * @param other The other rectangle
         * @return The intersection of the two rectangles
         */
        template<class T> Rectangle<T> Rectangle<T>::intersection(const Rectangle<T>& other){
            // If the rectangles don't intersect, return an empty rectangle
            if(!intersects(other))
                return Rectangle<T>();

            // Get the left and top of the intersection
            T left = this -> left > other.left ? this -> left : other.left;       // Check if the left of this rectangle is greater than the left of the other rectangle, if it is, use the left of this rectangle, otherwise use the left of the other rectangle
            T top = this -> top > other.top ? this -> top : other.top;            // Check if the top of this rectangle is greater than the top of the other rectangle, if it is, use the top of this rectangle, otherwise use the top of the other rectangle

            // Get the right and bottom of the intersection
            T right = this -> left + this -> width < other.left + other.width ? this -> left + this -> width : other.left + other.width;    // Check if the right of this rectangle is less than the right of the other rectangle, if it is, use the right of this rectangle, otherwise use the right of the other rectangle
            T bottom = this -> top + this -> height < other.top + other.height ? this -> top + this -> height : other.top + other.height;   // Check if the bottom of this rectangle is less than the bottom of the other rectangle, if it is, use the bottom of this rectangle, otherwise use the bottom of the other rectangle

            // Return the intersection
            return Rectangle<T>(left, top, right - left, bottom - top);
        }

        /**
         * @details Subtracts a rectangle from this rectangle
         *
         * @tparam T The type of the rectangle
         * @param other The rectangle to subtract
         * @return A vector of rectangles that represent the subtraction
         */
        template<class T> Vector<Rectangle<T> > Rectangle<T>::subtract(const Rectangle<T>& other){

            // Store the result rectangle
            Vector<Rectangle<T> > result;

            // If the rectangles don't intersect, return this rectangle
            if(!intersects(other))
            {
                result.pushBack(*this);
                return result;
            }

            // Get the intersection of the two rectangles
            T left = left > other.left ? left : other.left;
            T right = (left+width < other.left+other.width ? left+width : other.left+other.width);
            T bottom = (top+height < other.top+other.height ? top+height : other.top+other.height);

            // If the top of the other rectangle is above the top of this rectangle then this rectangle must be inside the other rectangle
            if(top < other.top)
            {
                // Create a rectangle from the top of this rectangle to the top of the other rectangle (the block above the other rectangle)
                result.pushBack(Rectangle<T>(left, top, right - left, other.top - top));
            }

            // If the left of this rectangle is to the left of the left of the other rectangle then this rectangle must be inside the other rectangle
            if(this -> left < other.left)
            {
                // Create a rectangle from the left of this rectangle to the left of the other rectangle (the block to the left of the other rectangle)
                result.pushBack(Rectangle<T>(this->left, top, other.left - this->left, bottom - top));
            }

            // If the right of this rectangle is to the right of the right of the other rectangle then this rectangle must be inside the other rectangle
            if(this -> left + this -> width > other.left + other.width)
            {
                // Create a rectangle from the right of the other rectangle to the right of this rectangle (the block to the right of the other rectangle)
                result.pushBack(Rectangle<T>(other.left + other.width, top,
                                             this->left + this->width - (other.left + other.width), bottom - top));
            }

            // If the bottom of this rectangle is below the bottom of the other rectangle then this rectangle must be inside the other rectangle
            if(this -> top + this -> height > other.top + other.height)
            {
                // Create a rectangle from the bottom of the other rectangle to the bottom of this rectangle (the block below the other rectangle)
                result.pushBack(Rectangle<T>(left, other.top + other.height, right - left,
                                             this->top + this->height - (other.top + other.height)));
            }
        }

        /**
         * @details  Checks if this rectangle contains another rectangle
         *
         * @tparam T  The type of the rectangle
         * @param other The other rectangle
         * @return True if this rectangle contains the other rectangle, false otherwise
         */

        template<class T> bool Rectangle<T>::contains(const Rectangle<T>& other){
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
         * @tparam T The type of the rectangle
         * @param x The x coordinate of the point
         * @param y The y coordinate of the point
         * @return True if this rectangle contains the point, false otherwise
         */
        template<class T> bool Rectangle<T>::contains(T x, T y){
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