//
// Created by 98max on 12/04/2023.
//

#include <common/inputStream.h>

using namespace maxOS;
using namespace maxOS::common;

InputStream::InputStream(InputStreamEventHandler<string> *inputStreamEventHandler)
: GenericInputStream<string>::GenericInputStream(inputStreamEventHandler) {

}
