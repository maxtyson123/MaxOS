//
// Created by 98max on 10/15/2022.
//

#ifndef MaxOS_GUI_WIDGET_H
#define MaxOS_GUI_WIDGET_H

#include <common/types.h>
#include <common/graphicsContex.h>

namespace maxOS{

    namespace gui{

        class Widget{

            protected:

                Widget* parent;
                common::int32_t x;
                common::int32_t y;
                common::int32_t w;
                common::int32_t h;

                common::uint8_t r;
                common::uint8_t g;
                common::uint8_t b;
                bool Focussable;

            public:

                Widget( Widget* parent, common::int32_t x, common::int32_t y, common::int32_t w, common::int32_t h, common::uint8_t r, common::uint8_t g, common::uint8_t b);
                ~Widget();

                virtual void Draw(GraphicsContext* gc);
                virtual void GetFocus(Widget* widget);
                virtual void ModelToScreen(common::int32_t &x, common::int32_t &y);

                virtual void OnMouseDown(common::int32_t x, common::int32_t y);
                virtual void OnMouseUp(common::int32_t x, common::int32_t y);
                virtual void OnMouseMove(common::int32_t old_x, common::int32_t old_y, common::int32_t new_x, common::int32_t new_y);

                virtual void OnKeyDown(common::int32_t x, common::int32_t y);
                virtual void OnKeyUp(common::int32_t x, common::int32_t y);

        };

        class CompositeWidget : public Widget{

            private:

                int numChildren;
                Widget* children[100];
                Widget* focussedChild;

        public:

            CompositeWidget( Widget* parent, common::int32_t x, common::int32_t y, common::int32_t w, common::int32_t h, common::uint8_t r, common::uint8_t g, common::uint8_t b);
            ~CompositeWidget();

            virtual void Draw(GraphicsContext* gc);
            virtual void GetFocus(Widget* widget);

            virtual void OnMouseDown(common::int32_t x, common::int32_t y);
            virtual void OnMouseUp(common::int32_t x, common::int32_t y);
            virtual void OnMouseMove(common::int32_t old_x, common::int32_t old_y, common::int32_t new_x, common::int32_t new_y);

            virtual void OnKeyDown(common::int32_t x, common::int32_t y);
            virtual void OnKeyUp(common::int32_t x, common::int32_t y);


        };


    }

}

#endif //MaxOS_GUI_WIDGET_H
