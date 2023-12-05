//
// Created by 98max on 11/12/2022.
//

#ifndef MAXOS_HARDWARECOMMUNICATION_SERIAL_H
#define MAXOS_HARDWARECOMMUNICATION_SERIAL_H

#include <stdint.h>
#include <hardwarecommunication/port.h>
#include <hardwarecommunication/interrupts.h>

namespace maxOS{

    namespace hardwarecommunication{

        //Note: '_' is used to detect the end of the line, instead of \0 as is used at the start for colour codewords
        class Colour{

            public:
                string defaultColour_fg = "\033[39m_";
                string defaultColour_bg = "\033[49m_";

                string black_fg = "\033[30m_";
                string black_bg = "\033[40m_";

                string dark_red_fg = "\033[31m_";
                string dark_red_bg = "\033[41m_";

                string dark_green_fg = "\033[32m_";
                string dark_green_bg = "\033[42m_";

                string dark_yellow_fg = "\033[33m_";
                string dark_yellow_bg = "\033[43m_";

                string dark_blue_fg = "\033[34m_";
                string dark_blue_bg = "\033[44m_";

                string dark_magenta_fg = "\033[35m_";
                string dark_magenta_bg = "\033[45m_";

                string dark_cyan_fg = "\033[36m_";
                string dark_cyan_bg = "\033[46m_";

                string dark_grey_fg = "\033[37m_";
                string dark_grey_bg = "\033[47m_";

                string light_grey_fg = "\033[90m_";
                string light_grey_bg = "\033[100m_";

                string red_fg = "\033[91m_";
                string red_bg = "\033[101m_";

                string green_fg = "\033[92m_";
                string green_bg = "\033[102m_";

                string yellow_fg = "\033[93m_";
                string yellow_bg = "\033[103m_";

                string blue_fg = "\033[94m_";
                string blue_bg = "\033[104m_";

                string magenta_fg = "\033[95m_";
                string magenta_bg = "\033[105m_";

                string cyan_fg = "\033[96m_";
                string cyan_bg = "\033[106m_";

                string white_fg = "\033[97m_";
                string white_bg = "\033[107m_";


        };

        class Type{

            public:
                string bold = "\033[1m_";
                string italic = "\033[3m_";
                string underline = "\033[4m_";
                string strikethrough = "\033[9m_";
                string blink = "\033[5m_";
                string reverse = "\033[7m_";
                string hidden = "\033[8m_";
                string reset = "\033[0m_";
                string none = "_";


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
                void printHeader(string col, string type, string msg);

            public:
                serial(InterruptManager* interruptManager);
                ~serial();

                void HandleInterrupt();

                void Test();
                char Read();
                void Write(string str, int type = 0);


        };

    }

}



#endif //MAXOS_HARDWARECOMMUNICATION_SERIAL_H
