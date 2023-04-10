//
// Created by 98max on 8/04/2023.
//

namespace maxOS{

    namespace gui {

        /**
         * @details WidgetMoverResizer is a template class that allows you to move and resize a widget
         *
         * @tparam L The left edge of the widget
         * @tparam T The top edge of the widget
         * @tparam W The width of the widget
         * @tparam H The height of the widget
         * @param target The widget to move and resize
         */
        template <int L, int T, int W, int H> WidgetMoverResizer<L,T,W,H>::WidgetMoverResizer(Widget* target): drivers::peripherals::MouseEventHandler()
        {
            this->target = target;
        }

        template<int L, int T, int W, int H> WidgetMoverResizer<L,T,W,H>::~WidgetMoverResizer()
        {
        }

        /**
         * @details OnMouseMoved is called when the mouse is moved. Resizes and moves the widget when this happens.
         *
         * @param x The x position of the mouse
         * @param y The y position of the mouse
         */
        template<int L, int T, int W, int H> void WidgetMoverResizer<L, T, W, H>::OnMouseMove(int x, int y)
        {

            Widget* targ = this->target;

            // If there is actually a size of the widget to change
            if(L != 0 || T != 0)
                // Move the widget to the left or right, and up or down
                targ -> move(targ -> position.left + L * x, targ -> position.top + T * y);

            // If there is actually a size of the widget to change
            if(W != 0 || H != 0)
                // Resize the widget to the left or right, and up or down
                targ -> resize( targ -> position.width + W*x, targ -> position.height + H*y);
        }

    }

}