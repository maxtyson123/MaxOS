//
// Created by 98max on 10/15/2022.
//

#ifndef MaxOS_GUI_DESKTOP_H
#define MaxOS_GUI_DESKTOP_H

#include <common/types.h>
#include <common/graphicsContext.h>
#include <drivers/peripherals/mouse.h>
#include <gui/widget.h>
#include <drivers/clock/clock.h>

namespace maxOS{

    namespace gui{

        class Desktop : public CompositeWidget, public drivers::peripherals::MouseEventHandler, public drivers::clock::ClockEventHandler{     //NTS: it is not a good idea to hardcode the mouse into the desktop as a tablet or touch screen device wont have a mouse cursor

            protected:
                common::uint32_t mouseX;
                common::uint32_t mouseY;

                common::GraphicsContext* graphicsContext;
                Widget* focussedWidget;
                drivers::peripherals::MouseEventHandler* draggedWidget;

                void setFocus(Widget* widget);
                void bringToFront(Widget* frontWidget);
                void invertMouseCursor();

                common::Vector<common::Rectangle<common::int32_t> > invalidAreas;
                void internalInvalidate(common::Rectangle<common::int32_t>& area, common::Vector<common::Rectangle<common::int32_t> >::iterator start, common::Vector<common::Rectangle<common::int32_t> >::iterator stop);
                void drawSelf(common::GraphicsContext* gc, common::Rectangle<common::int32_t>& area);

            public:
                common::Colour colour;

                Desktop(common::GraphicsContext* gc);
                ~Desktop();

                void addChild(Widget* childWidget);
                void onTime(const common::Time& time);
                void invalidate(common::Rectangle<common::int32_t>& area);

                void onMouseMoveEvent(int x, int y);
                void onMouseDownEvent(maxOS::common::uint8_t button);
                void onMouseUpEvent(maxOS::common::uint8_t button);

                void onKeyDown(drivers::peripherals::KeyCode keyDownCode, drivers::peripherals::KeyboardState keyDownState);
                void onKeyUp(drivers::peripherals::KeyCode keyUpCode, drivers::peripherals::KeyboardState keyUpState);

        };
    }

}

#endif //MaxOS_GUI_DESKTOP_H
