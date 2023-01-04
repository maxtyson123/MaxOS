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

int i = 0;

/**
 * Read the FAT32 BIOS block of a given hard disk
 * @param hd The hard disk to read the BIOS block from
 * @param partitionOffset The offset of the partition to read the BIOS block from
 */
void maxOS::filesystem::ReadBiosBlock(drivers::AdvancedTechnologyAttachment *hd, common::uint32_t partitionOffset) {

    //Read the BIOS block from the first sector of the partition
    BiosParameterBlock32 bpb;
    hd->Read28(partitionOffset, (uint8_t*)&bpb, sizeof(BiosParameterBlock32));

    //Log sector size
    printf("sectors per cluster: ");
    printfHex(bpb.sectorsPerCluster);
    printf("\n");


    //Read information from the BIOS block and store it in the global variables
    uint32_t fatStart = partitionOffset + bpb.reservedSectors;
    uint32_t fatSize = bpb.tableSize32;
    uint32_t dataStart = fatStart + fatSize*bpb.tableCopies;
    uint32_t rootStart = dataStart + bpb.sectorsPerCluster*(bpb.rootCluster - 2);   //rootCluster is offset 2 by default

    //16 Root entries per sector
    DirectoryEntry dirent[16];
    hd->Read28(rootStart, (uint8_t*)&dirent[0], 16*sizeof(DirectoryEntry));

    //Loop through the found root entries
    for(int i = 0; i < 16; i++)
    {
        //If the name is 0x00 then there are no more entries
        if(dirent[i].name[0] == 0x00)
            break;

        //If the atrribute is 0x0F then this is a long file name entry, skip it
        if((dirent[i].attributes & 0x0F) == 0x0F)
            continue;

        //Print the name of the file
        char* foo = "        ";
        for(int j = 0; j < 8; j++)
            foo[j] = dirent[i].name[j];
        printf(foo);

        if((dirent[i].attributes & 0x10) == 0x10)                                       //If the attribute has 5th bit on 1 then it is a directory
        {

            printf(" (directory)\n");

        }
        else{

            printf(" (file)\n");

            //Read the first cluster of the file
            uint32_t  fileCluster = ((uint32_t) dirent[i].firstClusterHigh << 16)       //Shift the high cluster number 16 bits to the left
                                              | dirent[i].firstClusterLow;              //Add the low cluster number

            //Convert file cluster into a sector
            uint32_t fileSector = dataStart + bpb.sectorsPerCluster*(fileCluster - 2);  //*Offset by 2

            //Read the first sector of the file
            uint8_t fileBuffer[512];                                                    //Create a buffer to store the file contents
            hd -> Read28(fileSector, fileBuffer, 512);                 //Read the first sector of the file
            fileBuffer[dirent[i].size = '\0'];                                          //Add a null terminator to the end of the file
            printf((char*)fileBuffer);                                              //Print the file contents

        }


    }
}
