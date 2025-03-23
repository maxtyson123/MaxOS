//
// Created by 98max on 21/03/2025.
//
#include <stdint.h>

extern "C" void _start(void)
{

  // For now loop forever
  while(true)
    asm("nop");
}