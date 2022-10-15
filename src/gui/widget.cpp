//
// Created by 98max on 10/15/2022.
//
#include <gui/widget.h>

using namespace maxOS::common;
using namespace maxOS::gui;

///__DEFAULT WIDGET__

Widget::Widget( Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h, uint8_t r, uint8_t g, uint8_t b){

    this->parent = parent;
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
    this->r = r;
    this->g = g;
    this->b = b;
    this->Focussable = true;

}

Widget::~Widget(){

}

void Widget::GetFocus(Widget* widget){

    //Default Implementation is to pass on to the parent unless there inst any

    if(parent != 0){                //IF there's a parent
        parent->GetFocus(widget);   //Pass on to parent

    }

    //pass the call to the parent and the parent passes it to its parent till it hits the root parent and the root parent must override this function and give the most child the focus .

}

void Widget::ModelToScreen(int32_t &x, int32_t &y){

    //Get the offset from the parent and then add own offset to it

    if(parent != 0){                         //IF there's a parent
        parent->ModelToScreen(x,y);   //Pass on to parent

    }

    x += this->x;
    y += this->y;

}

void Widget::Draw(GraphicsContext* gc){
    //Default Implementation

    //Set To 0 as ModelToScreen ads own x and y to it
    int X = 0;
    int Y = 0;

    //Get absolute position
    ModelToScreen(X,Y);

    gc->FillRectangle(X,Y,w,h,r,g,b);
}

void Widget::OnMouseDown(int32_t x, int32_t y){
    //If the object can actually be focussed
    if(Focussable){
      GetFocus(this);
    }

}


//Ignore these by default
void Widget::OnMouseUp(int32_t x, int32_t y){

}

void Widget::OnMouseMove(int32_t old_x, int32_t old_y, int32_t new_x, int32_t new_y){

}

void Widget::OnKeyDown(char* str){

}

void Widget::OnKeyUp(char* str){

}

///__COMPOSITE WIDGET__

CompositeWidget::CompositeWidget( Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h, uint8_t r, uint8_t g, uint8_t b){
    focussedChild = 0;
    numChildren = 0;
}

CompositeWidget::~CompositeWidget() {

}

void CompositeWidget::GetFocus(Widget *widget) {
    this->focussedChild = widget;
    if(parent != 0){
        parent->GetFocus(this);
    }

}

void CompositeWidget::Draw(GraphicsContext *gc) {

    //Draw its own bg
    Widget::Draw(gc);

    //loop from "number of children -1" to 0 because the children deep in the z direction into the screen must be drawn first and the deeper children have the higher indices
    for (int i = numChildren-1; i >= 0; i--) {
        children[i]->Draw(gc);
    }
}

void CompositeWidget::OnMouseDown(int32_t x, int32_t y) {

    //Pass the event to the child that contains the co-ordinate

    //Children with the smaller index is first on the screen therefore should receive the MouseEvent
    for (int i = 0; i < numChildren; i++) {

        //Pass the event to the child that contains the co-ordinate
        if(children[i]->ContainsCoordinate(x - this->x, y - this->y)){

            //(Subtracting x and y turns it into relative coords)
            children[i]->OnMouseDown(x - this->x, y - this->y);          //Pass the Mouse Event
            break;                                                             //Stop looping as the click has been handled

        }

    }
}

void CompositeWidget::OnMouseUp(int32_t x, int32_t y) {


    //Children with the smaller index is first on the screen therefore should receive the MouseEvent
    for (int i = 0; i < numChildren; i++) {

        //Pass the event to the child that contains the co-ordinate
        if(children[i]->ContainsCoordinate(x - this->x, y - this->y)){

            //(Subtracting x and y turns it into relative coords)
            children[i]->OnMouseUp(x - this->x, y - this->y);          //Pass the Mouse Event
            break;                                                           //Stop looping as the click has been handled

        }

    }
}

void CompositeWidget::OnMouseMove(int32_t old_x, int32_t old_y, int32_t new_x, int32_t new_y) {

    //Handle Old Coord
    int firstChild = -1;
    //Children with the smaller index is first on the screen therefore should receive the MouseEvent
        for (int i = 0; i < numChildren; i++) {

            //Pass the event to the child that contains the co-ordinate
            if(children[i]->ContainsCoordinate(old_x - this->x, old_y - this->y))
            {
                //                                   (Subtracting x and y turns it into relative coords)
                children[i]->OnMouseMove(old_x - this->x, old_y - this->y, new_x - this->x, new_y - this->y);           //Pass the Mouse Event
                firstChild = i;
                break;                                                                                                                          //Stop looping as the click has been handled

            }

        }

    //Handle New Coord

    //Children with the smaller index is first on the screen therefore should receive the MouseEvent
    for (int i = 0; i < numChildren; i++) {

        //Pass the event to the child that contains the co-ordinate
        if(children[i]->ContainsCoordinate(new_x - this->x, new_y - this->y))
        {

            if(firstChild != i){                                                                                                                //If the mouse is moved inside the same widget, dont give the event twice
                //                                   (Subtracting x and y turns it into relative coords)
                children[i]->OnMouseMove(old_x - this->x, old_y - this->y, new_x - this->x, new_y - this->y);           //Pass the Mouse Event

            }
            break;                                                                                                                              //Stop looping as the click has been handled

        }

    }

}


void CompositeWidget::OnKeyDown(char* str) {

    if(focussedChild != 0){             //IF the key event should happen on a child
        focussedChild->OnKeyDown(str);  //Pass to child
    }
}

void CompositeWidget::OnKeyUp(char* str){

    if(focussedChild != 0){             //IF the key event should happen on a child
        focussedChild->OnKeyUp(str);    //Pass to child
    }
}

