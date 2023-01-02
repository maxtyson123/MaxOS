//
// Created by 98max on 1/01/2023.
//

#include <filesystem/fat32.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::drivers;
using namespace maxOS::filesystem;

void printf(char* str, bool clearLine = false); // Forward declaration
void printfHex(uint8_t key);                    // Forward declaration

void FileAllocationTable32::ReadBiosBlock(drivers::AdvancedTechnologyAttachment *hd, common::uint32_t partitionOffset) {

    //Read the first sector of the partition
    BiosParameterBlock32 bpb;
    hd -> Read28(partitionOffset, (uint8_t*)&bpb, sizeof(BiosParameterBlock32));

    printf("bpb: ");
    for (int i = 0x00; i < sizeof(BiosParameterBlock32); ++i) {                           //Loop through the MBR and print it
        printfHex(((uint8_t*)&bpb)[i]);                                             //Print the buffer at the current index
        printf(" ");
    }
    printf(" \n");
}
