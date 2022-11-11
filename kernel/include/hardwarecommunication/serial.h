//
// Created by 98max on 11/12/2022.
//

#ifndef MAXOS_HARDWARECOMMUNICATION_SERIAL_H
#define MAXOS_HARDWARECOMMUNICATION_SERIAL_H

#include <hardwarecommunication/port.h>
#include <hardwarecommunication/interrupts.h>
#include <common/types.h>

namespace maxOS{

    namespace hardwarecommunication{

        //Note: '_' is used to detect the end of the line, instead of \0 as is used at the start for colour codewords
        class Colour{

            public:
                char* defaultColour_fg = "\033[39m_";
                char* defaultColour_bg = "\033[49m_";

                char* black_fg = "\033[30m_";
                char* black_bg = "\033[40m_";

                char* dark_red_fg = "\033[31m_";
                char* dark_red_bg = "\033[41m_";

                char* dark_green_fg = "\033[32m_";
                char* dark_green_bg = "\033[42m_";

                char* dark_yellow_fg = "\033[33m_";
                char* dark_yellow_bg = "\033[43m_";

                char* dark_blue_fg = "\033[34m_";
                char* dark_blue_bg = "\033[44m_";

                char* dark_magenta_fg = "\033[35m_";
                char* dark_magenta_bg = "\033[45m_";

                char* dark_cyan_fg = "\033[36m_";
                char* dark_cyan_bg = "\033[46m_";

                char* dark_grey_fg = "\033[37m_";
                char* dark_grey_bg = "\033[47m_";

                char* light_grey_fg = "\033[90m_";
                char* light_grey_bg = "\033[100m_";

                char* red_fg = "\033[91m_";
                char* red_bg = "\033[101m_";

                char* green_fg = "\033[92m_";
                char* green_bg = "\033[102m_";

                char* yellow_fg = "\033[93m_";
                char* yellow_bg = "\033[103m_";

                char* blue_fg = "\033[94m_";
                char* blue_bg = "\033[104m_";

                char* magenta_fg = "\033[95m_";
                char* magenta_bg = "\033[105m_";

                char* cyan_fg = "\033[96m_";
                char* cyan_bg = "\033[106m_";

                char* white_fg = "\033[97m_";
                char* white_bg = "\033[107m_";


        };

        class Type{

            public:
                char* bold = "\033[1m_";
                char* italic = "\033[3m_";
                char* underline = "\033[4m_";
                char* strikethrough = "\033[9m_";
                char* blink = "\033[5m_";
                char* reverse = "\033[7m_";
                char* hidden = "\033[8m_";
                char* reset = "\033[0m_";
                char* none = "_";


        };

        class serial : public InterruptHandler {
            private:
                Port8Bit dataPort;
                Port8Bit interruptEnableRegisterPort;
                Port8Bit fifoCommandPort;
                Port8Bit lineCommandPort;
                Port8Bit modemCommandPort;
                Port8Bit lineStatusPort;
                Port8Bit modemStatusPort;
                Port8Bit scratchPort;

                int receive();
                int isTransmitEmpty();
                void printHeader(char* col, char* type, char* msg);

            public:
                serial(InterruptManager* interruptManager);
                ~serial();

                common::uint32_t    HandleInterrupt(common::uint32_t esp);

                void Test();
                char Read();
                void Write(char* str, int type = 0);


        };

    }

}



#endif //MAXOS_HARDWARECOMMUNICATION_SERIAL_H
