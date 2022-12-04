//
// Created by 98max on 11/11/2022.
//

#include "test.h"

using namespace Application;
using namespace Application::test;


int main(int argc, char** argv, char** envp) {

   //Init the ui lib

    LibUI::init();

    //Make window
    Window window = LibUI::Window("Test Application");


    //Run event loop (THIS will set proc to waiting, and then when an event is triggered, it will be handled)
    LibUI::run();



    //Event loop exited, exit program
    return 0;
}