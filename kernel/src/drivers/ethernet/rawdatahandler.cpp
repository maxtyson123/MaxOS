//
// Created by 98max on 12/1/2022.
//

#include <drivers/ethernet/rawdatahandler.h>

using namespace maxOS;
using namespace maxOS::drivers;
using namespace maxOS::drivers::ethernet;

RawDataHandler::RawDataHandler()
{
}

RawDataHandler::~RawDataHandler()
{
}

bool RawDataHandler::HandleRawData(uint8_t*, uint32_t)
{
    return false;
}