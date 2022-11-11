//
// Created by 98max on 24/10/2022.
//

#include <drivers/ata.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::hardwarecommunication;
using namespace maxOS::drivers;

void printf(char* str, bool clearLine = false); // Forward declaration
void printfHex(uint8_t key);                    // Forward declaration

AdvancedTechnologyAttachment::AdvancedTechnologyAttachment(uint16_t portBase, bool master)
: dataPort(portBase),
  errorPort(portBase + 1),
  sectorCountPort(portBase + 2),
  LBAlowPort(portBase + 3),
  LBAmidPort(portBase + 4),
  LBAHiPort(portBase + 5),
  devicePort(portBase + 6),
  commandPort(portBase + 7),
  controlPort(portBase + 0x206)
{
    bytesPerSector = 512;
    this -> master = master;
}

AdvancedTechnologyAttachment::~AdvancedTechnologyAttachment() {

}

/**
 * @details This function Identifiers the ATA device
 */
void AdvancedTechnologyAttachment::Identify() {

    devicePort.Write(master ? 0xA0 : 0xB0);     //Select Device Master(0xA0) / Slave(0xB0)
    controlPort.Write(0);                       //Clear HOB bit (HOB : Set this to read back the High Order Byte of the last LBA48 value sent to an IO port.)

    //Floating Bus check : First you select the master, then read in the value of the status register and then compare it with 0xFF (is an invalid status value).
    devicePort.Write(0xA0);                     //Select Master (0xA0)
    uint8_t status = commandPort.Read();             //Read Status
    if(status == 0xFF){                              //IF status is 0xFF then there is no device
        printf("Invalid Status");
        return;                                      //Return, beacuse if there is no master then there wont be a slave either
    }

    devicePort.Write(master ? 0xA0 : 0xB0);     //Select Device Master(0xA0) / Slave(0xB0)
    sectorCountPort.Write(0);                   //Sector Doesn't Matter when Identifying so select sec 0
    LBAlowPort.Write(0);                        //Same here
    LBAmidPort.Write(0);                        //Same here
    LBAHiPort.Write(0);                         //Same here
    commandPort.Write(0x0EC);                   //Command For Identifying

    status = commandPort.Read();                     //Read Status
    if(status == 0x00){                              //IF status is 0x00 then there is no device
        printf("No Device");
        return;                                      //There is no slave/master
    }

    //Can take a while for there to be an answer to the identify command,
    while (
            ((status & 0x80) == 0x80)                 //Device is busy
            &&
            ((status & 0x01) != 0x01)                 //There was an error
          )
    {
        status = commandPort.Read();
    }

    //Check for any errors
    if(status & 0x01){

        printf("ERROR");
        return;

    }

    //We are reading 2 bytes from data port so , it will be 256 , so total bytes read are 512 (512 is bytes per sector)
    bool stopPrint = false;
    for (uint16_t i = 0; i < 256; ++i) {

        uint16_t data = dataPort.Read();
        char *text = "  \0";
        text[0] = (data >> 8) & 0xFF;
        text[1] = data & 0xFF;
        if(text[0] == 'K'){
            printf(text);
            stopPrint = true;       //Stop the messed up text from showing
        }

        if(!stopPrint) printf(text);

    }


}

/**
 * @details This function reads a sector from the ATA device
 * @param sector The sector to read
 * @param count The amount of data to read to that sector
 */
void AdvancedTechnologyAttachment::Read28(uint32_t sector, int count) {

    //Don't Allow reading More then a sector
    if(sector > 0x0FFFFFFF)
        return;
    if(count > 512)
        return;



    devicePort.Write(master ? 0xE0 : 0xF0 | ((sector & 0x0F000000) >> 24) );     //Select Device Master(0xE0) / Slave(0xF0), and add spare bits
    errorPort.Write(0);                                                          //Clear Previous Errors
    sectorCountPort.Write(1);                                                    // For now only read/write a single sector         TODO: Fix this


    LBAlowPort.Write(  sector & 0x000000FF );              //Split the sector into the port (put the low 8 bits ito this port)
    LBAmidPort.Write( (sector & 0x0000FF00) >> 8);         //Split the sector into the port  (put the mid 8 bits ito this port)
    LBAHiPort.Write( (sector & 0x00FF0000) >> 16);         //Split the sector into the port (put the hi 8 bits ito this port)
    commandPort.Write(0x20);                              //Command For Reading


    uint8_t status = commandPort.Read();                     //Read Status
    if(status == 0x00){                                     //IF status is 0x00 then there is no device
        printf("No Device");
        return;                                              //There is no slave/master
    }

    //Can take a while for the disk to be fully read
    while (
            ((status & 0x80) == 0x80)                 //Device is busy
            &&
            ((status & 0x01) != 0x01)                 //There was an error
            )
    {
        status = commandPort.Read();
    }

    //Check for any errors
    if(status & 0x01){

        printf("ERROR");
        return;

    }

    printf("Reading from ATA: ");


    //We are reading 2 bytes to the data port so , it will be 256 , so has to be incremented by 2
    for (uint16_t i = 0; i < bytesPerSector ; i+= 2) {

        uint16_t readData = dataPort.Read();

        char *text = "  \0";
        text[0] = readData & 0xFF;

        if(i+1 < count)
            text[1] = (readData >> 8) & 0xFF;
        else
            text[1] = '\0';


        if(i < count)                                           //Prevent random shit from throwing up on my screen
            printf(text);
    }

    //Hard Drive must have a full sector read, even if the data isnt the size of a full sector
    for(int i = count + (count%2); i < bytesPerSector; i += 2)      //If count isnt an even number then it would have already had it's byte read
        dataPort.Read();

}

/**
 * @details This function writes a sector to the ATA device
 * @param sector The sector to write to
 * @param count The amount of data to write to that sector
 */
void AdvancedTechnologyAttachment::Write28(uint32_t sector, uint8_t *data, int count) {

    //Don't Allow Writing More then a sector
    if(sector > 0x0FFFFFFF)
        return;
    if(count > 512)
        return;



    devicePort.Write(master ? 0xE0 : 0xF0 | ((sector & 0x0F000000) >> 24) );     //Select Device Master(0xE0) / Slave(0xF0), and add spare bits
    errorPort.Write(0);                                                          //Clear Previous Errors
    sectorCountPort.Write(1);                                                    // For now only read/write a single sector         TODO: Fix this


    LBAlowPort.Write(  sector & 0x000000FF );              //Split the sector into the port (put the low 8 bits ito this port)
    LBAmidPort.Write( (sector & 0x0000FF00) >> 8);         //Split the sector into the port  (put the mid 8 bits ito this port)
    LBAHiPort.Write( (sector & 0x00FF0000) >> 16);         //Split the sector into the port (put the hi 8 bits ito this port)
    commandPort.Write(0x30);                              //Command For Writing

    printf("Writing to ATA: ");


    //We are write 2 bytes to the data port so , it will be 256 , so has to be incremented by 2
    for (uint16_t i = 0; i < bytesPerSector ; i+= 2) {

        uint16_t  writeData = data[i];                          //Get the i'th byte from the data
        if(i+1 < count)                                         //Check if next byte is there also
            writeData |= ((uint16_t)data[i+1]) << 8;            //Write that byte
        dataPort.Write(writeData);                         //Write the data
        char *text = "  \0";
        text[1] = (writeData >> 8) & 0xFF;
        text[0] = writeData & 0xFF;

        if(i < count)                                           //Prevent random shit from throwing up on my screen
             printf(text);
    }

    //Hard Drive must have a full sector written, even if the data isnt the size of a full sector
    for(int i = count + (count%2); i < bytesPerSector; i += 2)      //If count isnt an even number then it would have already had it's byte written
        dataPort.Write(0x0000);

}
/**
 * @details Flush the ATA device
 */
void AdvancedTechnologyAttachment::Flush() {

    devicePort.Write(master ? 0xE0 : 0xF0);     //Select Device Master(0xE0) / Slave(0xF0)

    commandPort.Write(0xE7);                   //Command For Flushing

    uint8_t status = commandPort.Read();                     //Read Status
    if(status == 0x00){                              //IF status is 0x00 then there is no device
        printf("No Device");
        return;                                      //There is no slave/master
    }

    //Can take a while for there to be an answer to the flush command,
    while (
            ((status & 0x80) == 0x80)                 //Device is busy
            &&
            ((status & 0x01) != 0x01)                 //There was an error
            )
    {
        status = commandPort.Read();
    }

    //Check for any errors
    if(status & 0x01){

        printf("ERROR");
        return;

    }

    //Flush complete
}


