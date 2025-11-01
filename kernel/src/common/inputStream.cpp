/**
 * @file inputStream.cpp
 * @brief Implements an InputStream class for handling string input streams
 *
 * @date 12th April 2023
 * @author Max Tyson
 */

#include <common/inputStream.h>

using namespace MaxOS;
using namespace MaxOS::common;

/**
 * @brief Creates a new InputStream that handles strings
 *
 * @param input_stream_event_handler The event handler to handle the data
 */
InputStream::InputStream(InputStreamEventHandler<string> *input_stream_event_handler)
: GenericInputStream<string>::GenericInputStream( input_stream_event_handler)
{

}
