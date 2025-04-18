//
// Created by 98max on 12/04/2023.
//

#include <common/inputStream.h>

using namespace MaxOS;
using namespace MaxOS::common;

InputStream::InputStream(InputStreamEventHandler<string> *input_stream_event_handler)
: GenericInputStream<string>::GenericInputStream( input_stream_event_handler)
{

}
