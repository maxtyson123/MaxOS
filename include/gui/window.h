//
// Created by 98max on 10/15/2022.
//

#ifndef MaxOS_GUI_WINDOW_H
#define MaxOS_GUI_WINDOW_H

#include <gui/widget.h>
#include <common/types.h>

namespace maxOS{

    namespace gui{

        class Window : public CompositeWidget{

            protected:
                bool Dragging;

            public:
                Window(Widget* parent,
                       common::int32_t x, common::int32_t y, common::int32_t w, common::int32_t h,
                       common::uint8_t r, common::uint8_t g, common::uint8_t b);
                ~Window();

                void Draw(common::GraphicsContext* gc);

                void OnMouseDown(common::int32_t x, common::int32_t y, common::uint8_t button);
                void OnMouseUp(common::int32_t x, common::int32_t y, common::uint8_t button);
                void OnMouseMove(common::int32_t old_x, common::int32_t old_y, common::int32_t new_x, common::int32_t new_y);
        };

    }

}

#endif //MaxOS_GUI_WINDOW_H
