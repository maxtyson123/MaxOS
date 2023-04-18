//
// Created by 98max on 10/15/2022.
//

#ifndef MaxOS_GUI_WINDOW_H
#define MaxOS_GUI_WINDOW_H

#include <gui/widget.h>
#include <gui/widgets/text.h>
#include <common/types.h>

namespace maxOS{

    namespace gui{

        class Window : public CompositeWidget{

            protected:
                widgets::Text title;

                // Resizers
                WidgetMover windowWidgetMover;
                WidgetMoverResizerTop windowWidgetMoverResizerTop;
                WidgetMoverResizerBottom windowWidgetMoverResizerBottom;
                WidgetMoverResizerLeft windowWidgetMoverResizerLeft;
                WidgetMoverResizerRight windowWidgetMoverResizerRight;
                WidgetMoverResizerTopLeft windowWidgetMoverResizerTopLeft;
                WidgetMoverResizerTopRight windowWidgetMoverResizerTopRight;
                WidgetMoverResizerBottomLeft windowWidgetMoverResizerBottomLeft;
                WidgetMoverResizerBottomRight windowWidgetMoverResizerBottomRight;


            public:

                common::uint8_t windowFrameThickness;
                common::uint8_t windowTitleBarHeight;

                common::Colour windowAreaColour;
                common::Colour windowFrameColour;
                common::Colour windowFrameBorderColour;

                Window(common::int32_t left, common::int32_t top, common::uint32_t width, common::uint32_t height, common::string titleText);
                Window(Widget* containedWidget, common::string titleText);
                ~Window();

                void drawSelf(common::GraphicsContext* gc, common::Rectangle<int>& area);
                void addChild(Widget* child);

                drivers::peripherals::MouseEventHandler* onMouseButtonPressed(common::uint32_t x, common::uint32_t y, common::uint8_t button);



        };

    }

}

#endif //MaxOS_GUI_WINDOW_H
