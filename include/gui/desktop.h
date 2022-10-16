//
// Created by 98max on 10/15/2022.
//

#ifndef MaxOS_GUI_DESKTOP_H
#define MaxOS_GUI_DESKTOP_H

#include <common/types.h>
#include <common/graphicsContex.h>
#include <drivers/mouse.h>
#include <gui/widget.h>

namespace maxOS{

    namespace gui{

        class Desktop : public CompositeWidget, public drivers::MouseEventHandler{     //NTS: it is not a good idea to hardcode the mouse into the desktop as a tablet or touch screen device wont have a mouse cursor

            protected:
                common::uint32_t MouseX;
                common::uint32_t MouseY;

            public:
                Desktop(common::int32_t w, common::int32_t h, common::uint8_t r, common::uint8_t g, common::uint8_t b);
                ~Desktop();

                void Draw(common::GraphicsContext* gc);

            void OnMouseDown(maxOS::common::uint8_t button);
            void OnMouseUp(maxOS::common::uint8_t button);
            void OnMouseMove(int x, int y);

        };


    }

}

#endif //MaxOS_GUI_DESKTOP_H
