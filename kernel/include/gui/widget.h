//
// Created by 98max on 10/15/2022.
//

#ifndef MaxOS_GUI_WIDGET_H
#define MaxOS_GUI_WIDGET_H

#include <stdint.h>
#include <common/rectangle.h>
#include <common/vector.h>
#include <common/graphicsContext.h>
#include <common/coordinates.h>
#include <drivers/peripherals/keyboard.h>
#include <drivers/peripherals/mouse.h>


namespace maxOS{

    namespace gui{

        /**
         * A widget is a graphical element that can be drawn on the screen.
         */
        class Widget : public drivers::peripherals::KeyboardEventHandler{
            template<int Left, int Top, int Width, int Height> friend class WidgetMoverResizer;
            friend class CompositeWidget;

            private:
                common::Rectangle<int32_t> position;

            protected:
                Widget* parent;
                bool valid;

                virtual void setFocus(Widget* widget);
                virtual void bringToFront(Widget* widget);

                uint32_t minWidth;
                uint32_t minHeight;

                uint32_t maxWidth;
                uint32_t maxHeight;

            public:

                // Initializing functions
                Widget();
                Widget(int32_t left, int32_t top, uint32_t width, uint32_t height);
                ~Widget();

                // Drawing functions
                virtual void draw(common::GraphicsContext* gc, common::Rectangle<int32_t>& area);
                void invalidate();
                virtual void invalidate(common::Rectangle<int32_t>& area);
                virtual void addChild(Widget* child);

                // Positioning functions
                virtual common::Coordinates absoluteCoordinates(common::Coordinates coordinates);
                virtual bool containsCoordinate(uint32_t x, uint32_t y);
                common::Rectangle<int32_t> getPosition();
                void move(int32_t left, int32_t top);
                void resize(int32_t width, int32_t height);

                // Focus functions
                void focus();
                virtual void onFocus();
                virtual void onFocusLost();
                void bringToFront();

                // Mouse functions
                virtual void onMouseEnterWidget(uint32_t toX, uint32_t toY);
                virtual void onMouseLeaveWidget(uint32_t fromX, uint32_t fromY);
                virtual void onMouseMoveWidget(uint32_t fromX, uint32_t fromY, uint32_t toX, uint32_t toY);
                virtual drivers::peripherals::MouseEventHandler* onMouseButtonPressed(uint32_t x, uint32_t y, uint8_t button);
                virtual void onMouseButtonReleased(uint32_t x, uint32_t y, uint8_t button);

        };

        /**
         * A widget that can contain other widgets.
         */
        class CompositeWidget : public Widget{

            protected:

                common::Vector<Widget*> children;
                void draw(common::GraphicsContext* gc, common::Rectangle<int32_t>& area, common::Vector<Widget*>::iterator start);
                virtual void drawSelf(common::GraphicsContext* gc, common::Rectangle<int32_t>& area);


            public:

                // Initializing functions
                CompositeWidget();
                CompositeWidget(int32_t left, int32_t top, uint32_t width, uint32_t height);
                ~CompositeWidget();

                // Drawing functions
                virtual void draw(common::GraphicsContext* gc, common::Rectangle<int32_t>& area);
                virtual void addChild(Widget* child);

                // Mouse functions
                virtual void onMouseEnterWidget(uint32_t toX, uint32_t toY);
                virtual void onMouseLeaveWidget(uint32_t fromX, uint32_t fromY);
                virtual void onMouseMoveWidget(uint32_t fromX, uint32_t fromY, uint32_t toX, uint32_t toY);
                virtual drivers::peripherals::MouseEventHandler* onMouseButtonPressed(uint32_t x, uint32_t y, uint8_t button);
                virtual void onMouseButtonReleased(uint32_t x, uint32_t y, uint8_t button);


        };


        /**
         * A resizable widget.
         *
         * @tparam Left Left
         * @tparam Top Top
         * @tparam Width Width
         * @tparam Height Height
         */
        template<int Left, int Top, int Width, int Height> class WidgetMoverResizer : public drivers::peripherals::MouseEventHandler{
            protected:
                Widget* targettedWidget;
            public:
                WidgetMoverResizer(Widget* widget);
                ~WidgetMoverResizer();

                void onMouseMoveEvent(int8_t x, int8_t y);

        };

        typedef WidgetMoverResizer<1, 1, 0, 0> WidgetMover;

        typedef WidgetMoverResizer<0, 1, 0, -1> WidgetMoverResizerTop;
        typedef WidgetMoverResizer<0, 0, 0, 1> WidgetMoverResizerBottom;
        typedef WidgetMoverResizer<1, 0, -1, 0> WidgetMoverResizerLeft;
        typedef WidgetMoverResizer<0, 0, 1, 0> WidgetMoverResizerRight;

        typedef WidgetMoverResizer<1, 1, -1, -1> WidgetMoverResizerTopLeft;
        typedef WidgetMoverResizer<0, 1, 1, -1> WidgetMoverResizerTopRight;
        typedef WidgetMoverResizer<1, 0, -1, 1> WidgetMoverResizerBottomLeft;
        typedef WidgetMoverResizer<0, 0, 1, 1> WidgetMoverResizerBottomRight;


        ///________________________________Tempaltes________________________________///
        /**


        * @details WidgetMoverResizer is a template class that allows you to move and resize a widget
        *
        * @tparam Left The left edge of the widget
        * @tparam Top The top edge of the widget
        * @tparam Width The width of the widget
        * @tparam Height The height of the widget
        * @param target The widget to move and resize
        */
        template <int Left, int Top, int Width, int Height> WidgetMoverResizer<Left,Top,Width,Height>::WidgetMoverResizer(Widget* target)
                : drivers::peripherals::MouseEventHandler()
        {
            this -> targettedWidget = target;
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
        template<int Left, int Top, int Width, int Height> void WidgetMoverResizer<Left, Top, Width, Height>::onMouseMoveEvent(int8_t x, int8_t y)
        {

            Widget* targ = this->targettedWidget;

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
#endif //MaxOS_GUI_WIDGET_H
