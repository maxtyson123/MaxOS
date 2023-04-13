//
// Created by 98max on 8/04/2023.
//

namespace maxOS{

    namespace gui {

        /**
         * @details WidgetMoverResizer is a template class that allows you to move and resize a widget
         *
         * @tparam Left The left edge of the widget
         * @tparam Top The top edge of the widget
         * @tparam Width The width of the widget
         * @tparam Height The height of the widget
         * @param target The widget to move and resize
         */
        template <int Left, int Top, int Width, int Height> WidgetMoverResizer<Left,Top,Width,Height>::WidgetMoverResizer(Widget* target): drivers::peripherals::handler()
        {
            this->target = target;
        }

        template<int Left, int Top, int Width, int Height> WidgetMoverResizer<Left,Top,Width,Height>::~WidgetMoverResizer()
        {
        }

        /**
         * @details OnMouseMoved is called when the mouse is moved. Resizes and moves the widget when this happens.
         *
         * @param x The x position of the mouse
         * @param y The y position of the mouse
         */
        template<int Left, int Top, int Width, int Height> void WidgetMoverResizer<Left, Top, Width, Height>::onMouseMoveEvent(int x, int y)
        {

            Widget* targ = this->target;

            // If there is actually a size of the widget to change
            if(Left != 0 || Top != 0)
                // Move the widget to the left or right, and up or down
                targ -> move(targ -> position.left + Left * x, targ -> position.top + Top * y);

            // If there is actually a size of the widget to change
            if(Width != 0 || Height != 0)
                // Resize the widget to the left or right, and up or down
                targ -> resize( targ -> position.width + Width*x, targ -> position.height + Height*y);
        }

    }

}