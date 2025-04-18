//
// Created by 98max on 12/1/2022.
//

#include <drivers/ethernet/rawdatahandler.h>

using namespace MaxOS;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::ethernet;

RawDataHandler::RawDataHandler() = default;

RawDataHandler::~RawDataHandler() = default;

bool RawDataHandler::HandleRawData(uint8_t*, uint32_t)
{
    return false;
}