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
        template <int L, int T, int W, int H> WidgetMoverResizer<L,T,W,H>::WidgetMoverResizer(Widget* target): drivers::mouse::MouseEventHandler()
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
        template<int L, int T, int W, int H> void WidgetMoverResizer<L,T,W,H>::OnMouseMoved(int8_t x, int8_t y)
        {

            // If there is actually a size of the widget to change
            if(L != 0 || T != 0)
                // Move the widget to the left or right, and up or down
                target -> Move(target -> position.left + L * x, target -> position.top + T * y);

            // If there is actually a size of the widget to change
            if(W != 0 || H != 0)
                // Resize the widget to the left or right, and up or down
                target->Resize( target->position.width + W*x, target->position.height + H*y);
        }

    }

}