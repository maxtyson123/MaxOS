//
// Created by 98max on 12/04/2023.
//

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
