//
// Created by 98max on 10/15/2022.
//

#ifndef MaxOS_GUI_WIDGET_H
#define MaxOS_GUI_WIDGET_H

#include <common/types.h>
#include <common/graphicsContext.h>
#include <drivers/peripherals/keyboard.h>


namespace maxOS{

    namespace gui{

        class Widget : public drivers::peripherals::KeyboardEventHandler{

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

                virtual void Draw(common::GraphicsContext* gc);
                virtual void GetFocus(Widget* widget);
                virtual void ModelToScreen(common::int32_t &x, common::int32_t &y);
                virtual bool ContainsCoordinate(common::int32_t x, common::int32_t y);

                virtual void OnMouseDown(common::int32_t x, common::int32_t y, common::uint8_t button);
                virtual void OnMouseUp(common::int32_t x, common::int32_t y, common::uint8_t button);
                virtual void OnMouseMove(common::int32_t old_x, common::int32_t old_y, common::int32_t new_x, common::int32_t new_y);


        };

        class CompositeWidget : public Widget{

            private:

                int numChildren;
                Widget* children[100];
                Widget* focussedChild;

        public:

            CompositeWidget( Widget* parent, common::int32_t x, common::int32_t y, common::int32_t w, common::int32_t h, common::uint8_t r, common::uint8_t g, common::uint8_t b);
            ~CompositeWidget();

            virtual void Draw(common::GraphicsContext* gc);
            virtual void GetFocus(Widget* widget);
            virtual bool AddChild(Widget* child);

            virtual void OnMouseDown(common::int32_t x, common::int32_t y, common::uint8_t button);
            virtual void OnMouseUp(common::int32_t x, common::int32_t y, common::uint8_t button);
            virtual void OnMouseMove(common::int32_t old_x, common::int32_t old_y, common::int32_t new_x, common::int32_t new_y);

            virtual void OnKeyDown(char*);
            virtual void OnKeyUp(char*);


        };


    }

}

#endif //MaxOS_GUI_WIDGET_H
