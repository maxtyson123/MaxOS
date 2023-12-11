//
// Created by 98max on 10/15/2022.
//

#ifndef MaxOS_GUI_WINDOW_H
#define MaxOS_GUI_WINDOW_H

#include <stdint.h>
#include <gui/widget.h>
#include <gui/widgets/text.h>

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

                uint8_t windowFrameThickness;
                uint8_t windowTitleBarHeight;

                common::Colour windowAreaColour;
                common::Colour windowFrameColour;
                common::Colour windowFrameBorderColour;

                Window(int32_t left, int32_t top, uint32_t width, uint32_t height, string titleText);
                Window(Widget* containedWidget, string titleText);
                ~Window();

                void drawSelf(common::GraphicsContext* gc, common::Rectangle<int32_t>& area);
                void addChild(Widget* child);

                drivers::peripherals::MouseEventHandler* onMouseButtonPressed(uint32_t x, uint32_t y, uint8_t button);



        };

    }

}

#endif //MaxOS_GUI_WINDOW_H
