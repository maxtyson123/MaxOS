/**
 * @file rawdatahandler.cpp
 * @brief Implementation of a RawDataHandler class for handling raw Ethernet data
 *
 * @date 1st December 2022
 * @author Max Tyson
 */

#include <drivers/ethernet/rawdatahandler.h>

using namespace MaxOS;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::ethernet;

RawDataHandler::RawDataHandler() = default;

RawDataHandler::~RawDataHandler() = default;

/**
 * @brief Handles raw data received from the Ethernet driver
 *
 * @param data Pointer to the raw data
 * @param size Size of the raw data in bytes
 * @return True if the data was handled, false otherwise
 */
bool RawDataHandler::handle_raw_data(uint8_t* data, uint32_t size)
{
    return false;
}