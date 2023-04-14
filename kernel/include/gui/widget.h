//
// Created by 98max on 10/15/2022.
//

#ifndef MaxOS_GUI_WIDGET_H
#define MaxOS_GUI_WIDGET_H

#include <common/types.h>
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
            friend class WidgetContainer;

            private:
                common::Rectangle<int> position;

            protected:
                Widget* parent;
                bool valid;

                virtual void setFocus(Widget* widget);
                virtual void bringToFront(Widget* widget);

                common::uint32_t minWidth;
                common::uint32_t minHeight;

                common::uint32_t maxWidth;
                common::uint32_t maxHeight;

            public:

                // Initializing functions
                Widget();
                Widget(common::int32_t left, common::int32_t top, common::uint32_t width, common::uint32_t height);
                ~Widget();

                // Drawing functions
                virtual void draw(common::GraphicsContext* gc, common::Rectangle<int>& area);
                void invalidate();
                virtual void invalidate(common::Rectangle<int>& area);
                virtual void addChild(Widget* child);

                // Positioning functions
                virtual common::Coordinates absoluteCoordinates(common::Coordinates coordinates);
                virtual bool containsCoordinate(common::uint32_t x, common::uint32_t y);
                common::Rectangle<int> getPosition();
                void move(common::int32_t left, common::int32_t top);
                void resize(common::int32_t width, common::int32_t height);

                // Focus functions
                void focus();
                virtual void onFocus();
                virtual void onFocusLost();
                void bringToFront();

                // Mouse functions
                virtual void onMouseEnterWidget(common::uint32_t toX, common::uint32_t toY);
                virtual void onMouseLeaveWidget(common::uint32_t fromX, common::uint32_t fromY);
                virtual void onMouseMoveWidget(common::uint32_t fromX, common::uint32_t fromY, common::uint32_t toX, common::uint32_t toY);
                virtual drivers::peripherals::MouseEventHandler* onMouseButtonPressed(common::uint32_t x, common::uint32_t y, common::uint8_t button);
                virtual void onMouseButtonReleased(common::uint32_t x, common::uint32_t y, common::uint8_t button);

        };

        /**
         * A widget that can contain other widgets.
         */
        class CompositeWidget : public Widget{

            protected:

                common::Vector<Widget*> children;
                void draw(common::GraphicsContext* gc, common::Rectangle<int>& area, common::Vector<Widget*>::iterator start);
                virtual void drawSelf(common::GraphicsContext* gc, common::Rectangle<int>& area);


            public:

                // Initializing functions
                CompositeWidget();
                CompositeWidget(common::int32_t left, common::int32_t top, common::uint32_t width, common::uint32_t height);
                ~CompositeWidget();

                // Drawing functions
                virtual void draw(common::GraphicsContext* gc, common::Rectangle<int>& area);
                virtual void addChild(Widget* child);

                // Mouse functions
                virtual void onMouseEnterWidget(common::uint32_t toX, common::uint32_t toY);
                virtual void onMouseLeaveWidget(common::uint32_t fromX, common::uint32_t fromY);
                virtual void onMouseMoveWidget(common::uint32_t fromX, common::uint32_t fromY, common::uint32_t toX, common::uint32_t toY);
                virtual drivers::peripherals::MouseEventHandler* onMouseButtonPressed(common::uint32_t x, common::uint32_t y, common::uint8_t button);
                virtual void onMouseButtonReleased(common::uint32_t x, common::uint32_t y, common::uint8_t button);


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
                Widget* widget;
            public:
                WidgetMoverResizer(Widget* widget);
                ~WidgetMoverResizer();

                void onMouseMoveEvent(int x, int y);

        };

        typedef WidgetMoverResizer<0, 0, 0, 0> WidgetMover;

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
#endif //MaxOS_GUI_WIDGET_H
