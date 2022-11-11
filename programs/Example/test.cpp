//
// Created by 98max on 11/11/2022.
//

#include "test.h"

using namespace Application;
using namespace Application::test;

Test::Test(){

};

Test::~Test(){

};

void _start(void)
{
    //Print on screen, TEST: Use LIB or kernel wont allow this code
    int i;
    for (i = 0; i < 3; i++) {
        *videomem++ = (0x07 << 8) | ('0' + i);
    }

    while(1);
}