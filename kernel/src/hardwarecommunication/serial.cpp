//
// Created by 98max on 11/12/2022.
//

#include "hardwarecommunication/serial.h"

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::hardwarecommunication;



/**
 * @details This function is the constructor for the serial port
 */
serial::serial(InterruptManager* interruptManager)
: InterruptHandler(interruptManager -> HardwareInterruptOffset() + 4, interruptManager),
  dataPort(0x3f8),
  interruptEnableRegisterPort(0x3f9),
  fifoCommandPort(0x3fa),
  lineCommandPort(0x3fb),
  modemCommandPort(0x3fc),
  lineStatusPort(0x3fd),
  modemStatusPort(0x3fe),
  scratchPort(0x3ff)

{

    //Initialise the serial port
    interruptEnableRegisterPort.Write(0x00);                //Disable all interrupts
    lineCommandPort.Write(0x80);                            //Enable DLAB (set baud rate divisor)
    interruptEnableRegisterPort.Write(0x03);                //Set divisor to 3 (lo byte) 38400 baud
    lineCommandPort.Write(0x03);                            //                  (hi byte)
    lineCommandPort.Write(0x03);                            //8 bits, no parity, one stop bit
    fifoCommandPort.Write(0xC7);                            //Enable FIFO, clear them, with 14-byte threshold
    modemCommandPort.Write(0x0B);                           //IRQs enabled, RTS/DSR set
    modemCommandPort.Write(0x1E);                           //Set in loopback mode, test the serial chip
    interruptEnableRegisterPort.Write(0xAE);                // Test serial chip (send byte 0xAE and check if serial returns same byte)


    //Check if the serial port is working
    if(interruptEnableRegisterPort.Read() != 0xAE){
      //HEL{
    }

    //Enable interrupts
    interruptEnableRegisterPort.Write(0x02);                //Enable interrupts

    //If the serial port is working  set it in normal operation mode
    modemCommandPort.Write(0x0F);

}

serial::~serial() {

}

/**
 * @details This function checks if the serial port is ready to read data
 */
int serial::receive() {
    //Check if the data is ready to be read
    return lineStatusPort.Read() & 1;
}

/**
 * @details This function reads a byte from the serial port
 *
 * @return The byte read from the serial port
 */
char serial::Read() {

    //Wait until the data is ready to be read
    while(receive() == 0);

    //Read the data
    return dataPort.Read();
}

/**
 * @details This function checks if the serial port is ready to write data
 */
int serial::isTransmitEmpty() {

    //Check if the transmit buffer is empty
    return lineStatusPort.Read() & 0x20;

}

/**
 * @details This function writes a header for the message
 *
 * @param col The color of the header
 * @param type The type of the text
 * @param msg The message
 */
void serial::printHeader(string col, string type, string msg){

    string message;
    int pos = 0;

    message[pos++] = '[';

    //Colour
    for(int i = 0; col[i] != '_'; ++i){     //Increment through each char as long as it's not the end symbol
        message[pos++] = col[i];
    }

    //Type
    for(int i = 0; type[i] != '_'; ++i){     //Increment through each char as long as it's not the end symbol
        message[pos++] = type[i];
    }

    //Message
    for(int i = 0; msg[i] != '\0'; ++i){     //Increment through each char as long as it's not the end symbol
        message[pos++] = msg[i];
    }

    //Reset Colour
    Colour col_r;
    col = col_r.defaultColour_fg;
    for(int i = 0; col[i] != '_'; ++i){     //Increment through each char as long as it's not the end symbol
        message[pos++] = col[i];
    }

    //Reset Type
    Type type_r;
    type = type_r.reset;
    for(int i = 0; type[i] != '_'; ++i){     //Increment through each char as long as it's not the end symbol
        message[pos++] = type[i];
    }

    message[pos++] = ']';
    message[pos++] = ' ';

    for(int i = 0; i < pos; ++i){                    //Increment through the message
        while(isTransmitEmpty() == 0);               //Wait until the transmit buffer is empty
        dataPort.Write(message[i]);             //Write the char to the serial port
    }

}


/**
 * @details This function tests all the different messages that can be printed
 */
void serial::Test(){

    Write("\n\n===- Testing serial port -===\n\n",-1);

    Write("Test_-1_NoHeader\n",-1);
    Write("Test_0_Default\n",0);
    Write("Test_1_Info\n",1);
    Write("Test_2_Warning\n",2);
    Write("Test_3_Error\n",3);
    Write("Test_4_Fatal\n",4);
    Write("Test_5_Success\n",5);
    Write("Test_6_Debug\n",6);
    Write("Test_7_Kernel\n",7);
    Write("Test_8_Unknown\n",8);

    Write("\n\n===- Testing Complete -===\n\n",-1);

}

/**
 * @details This function writes a message to the serial port
 *
 * @param str The message to be written
 * @param type The type of the message 1 = info, 2 = warning, 3 = error, 4 = fatal, 5 = success, 6 = debug, 7 = kernel, 8 = unknown
 */
void serial::Write(string str, int type) {
    Colour t_colour;
    Type t_type;

    switch (type) {

        //Dont print header
        case -1:
            break;

        //Default
        case 0:
            printHeader(t_colour.defaultColour_fg, t_type.reset, "Message");
            break;

        //Info
        case 1:
            printHeader(t_colour.light_grey_fg, t_type.none, "Info");
            break;

        //Warning
        case 2:
            printHeader(t_colour.yellow_fg, t_type.none, "Warning");
            break;

        //Error
        case 3:
            printHeader(t_colour.red_fg, t_type.none, "Error");
            break;

        //Fatal Error
        case 4:
            printHeader(t_colour.dark_red_fg, t_type.bold, "Fatal Error");
            break;

        //Success
        case 5:
            printHeader(t_colour.green_fg, t_type.blink, "Success");
            break;

        //Debug
        case 6:
            printHeader(t_colour.magenta_fg, t_type.underline, "Debug");
            break;

        //Kernel
        case 7:
            printHeader(t_colour.cyan_fg, t_type.italic, "MaxOS");
            break;

        default:
            printHeader(t_colour.defaultColour_fg, t_type.reset, "Unknown");
            break;
        
    }

    for(int i = 0; str[i] != '\0'; ++i){     //Increment through each char as long as it's not the end symbol
        while(isTransmitEmpty() == 0);      //Wait until the transmit buffer is empty
        dataPort.Write(str[i]);             //Write the char to the serial port
    }



}

/**
 * @details Read from the serial port when the interrupt is triggered
 * 
 * @param esp The stack pointer
 * @return The stack pointer (where the processor will go when the interrupt is finished) (always will be what is passed in)
 */
void serial::HandleInterrupt() {
   //Handle reading from the serial port
}
