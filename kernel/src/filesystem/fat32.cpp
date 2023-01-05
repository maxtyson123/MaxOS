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
    printf(" contents: \n");


    //Read information from the BIOS block and store it in the global variables
    uint32_t fatStart = partitionOffset + bpb.reservedSectors;                      //The FAT is located after the reserved sectors
    uint32_t fatSize = bpb.tableSize32;
    uint32_t dataStart = fatStart + fatSize*bpb.tableCopies;                        //The data is located after the FAT (which is fatSize * tableCopies sectors long)
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

            printf(" (file) ");

            //Read the first cluster of the file
            uint32_t  firestFileCluster = ((uint32_t) dirent[i].firstClusterHigh << 16)       //Shift the high cluster number 16 bits to the left
                                                    | dirent[i].firstClusterLow;              //Add the low cluster number



            int32_t SIZE = dirent[i].size;
            int32_t nextFileCluster = firestFileCluster;
            uint8_t fileBuffer[513];
            uint8_t fatBuffer[513];

            while (SIZE > 0){

                //Convert file cluster into a sector
                uint32_t fileSector = dataStart + bpb.sectorsPerCluster*(nextFileCluster - 2);                  //*Offset by 2
                int sectorOffset = 0;

                //Loop the sectors of the file
                for (; SIZE > 0; SIZE -= 512) {

                    hd -> Read28(fileSector + sectorOffset, fileBuffer, 512);                 //Read the specified sector of the file
                    sectorOffset++;                                                                            //Increment the sector offset

                    fileBuffer[SIZE > 512 ? 512 : SIZE = '\0'];                                                //Add a null terminator to the end of the sector
                    printf((char*)fileBuffer);                                                             //Print the file contents

                    //If the next sector is in a different cluster then break
                    if (sectorOffset > bpb.sectorsPerCluster)
                        break;
                }

                //Get the next cluster of the file
                uint32_t fatSectorForCurrentCluster = nextFileCluster / (512 / sizeof(uint32_t));               //The FAT sector for the current cluster is the current cluster divided by the number of entries per sector

                //Read the FAT sector for the current cluster
                hd->Read28(fatStart + fatSectorForCurrentCluster, fatBuffer, 512);

                //Get the offset of the current cluster in the FAT sector
                uint32_t fatOffsetInSectorForCurrentCluster = nextFileCluster % (512/sizeof(uint32_t));

                nextFileCluster = ((uint32_t*)&fatBuffer)[fatOffsetInSectorForCurrentCluster] & 0x0FFFFFFF;
            }
        }
    }
    printf("\n");
}


//TODO: Fix folders after lib not found error
