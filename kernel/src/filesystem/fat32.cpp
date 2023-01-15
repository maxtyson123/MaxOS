//
// Created by 98max on 1/01/2023.
//

#include <filesystem/fat32.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::drivers;
using namespace maxOS::filesystem;
using namespace maxOS::memory;

void printf(char* str, bool clearLine = false); // Forward declaration
void printfHex(uint8_t key);                    // Forward declaration
char printfInt( int i);                         // Forward declaration
int i = 0;

//TODO: Fix folders after lib not found error

/**
 * Intialize the FAT32 filesystem
 * @param hd The hard disk to initialize the FAT32 filesystem on
 * @param partitionOffset The offset of the partition to initialize the FAT32 filesystem on
 */
Fat32::Fat32(drivers::AdvancedTechnologyAttachment *hd, common::uint32_t partitionOffset) {

    drive = hd;
    partOffset = partitionOffset;

    //Read the BIOS block from the first sector of the partition
    BiosParameterBlock32 bpb;
    hd->Read28(partitionOffset, (uint8_t*)&bpb, sizeof(BiosParameterBlock32));

    //Read information from the BIOS block and store it in the global variables
    uint32_t fatStart = partitionOffset + bpb.reservedSectors;                      //The FAT is located after the reserved sectors
    uint32_t fatSize = bpb.tableSize32;
    uint32_t dataStart = fatStart + fatSize*bpb.tableCopies;                        //The data is located after the FAT (which is fatSize * tableCopies sectors long)
    uint32_t rootStart = dataStart + bpb.sectorsPerCluster*(bpb.rootCluster - 2);   //rootCluster is offset 2 by default


    FatDirectoryTraverser rootTraverser(hd, rootStart, dataStart, bpb.sectorsPerCluster, fatStart, fatSize);
    currentTraverser = &rootTraverser;


}

Fat32::~Fat32() {

}

/**
 * Get the current directory traverser
 * @return The current directory traverser
 */
DirectoryTraverser* Fat32::getDirectoryTraverser() {

    return currentTraverser;

}

/**
 * Allocate a new cluster in the FAT
 * @param currentCluster The current cluster to allocate a new cluster after
 * @param fatLocation The location of the FAT table
 * @param fat_size The size of the FAT table
 * @return The current directory traverser
 */
uint32_t Fat32::AllocateCluster(drivers::AdvancedTechnologyAttachment *hd, uint32_t currentCluster, uint32_t fatLocation, uint32_t fat_size){

    //Find and store the first free cluster
    uint32_t  fatBuffer[512 / sizeof(uint32_t)];                                                                                        //Create a buffer to store a sector of the FAT table
    uint32_t nextFreeCluster = -1;
    for (int sector = 0; sector < fat_size; ++sector) {                                                                     //Loop the sectors of the FAT table

        hd -> Read28(fatLocation + sector, (uint8_t*)fatBuffer, sizeof(fatBuffer));          //Read the sector of the FAT table
        for (int j = 0; j < 512 / sizeof(uint32_t); ++j) {                                                                              //Loop through each entry in the sectort

            if(fatBuffer[i] == 0){                                                                                                      //Fat entries are available if it is 0x000000

                nextFreeCluster = sector * (512 / sizeof(uint32_t)) + j;                                                                //Calculate the cluster number
                break;
            }
        }
    }

    //If there is no space left on the drive
    if(nextFreeCluster == -1) return nextFreeCluster;                                                                                        //If there is no free clusters then return

    //Mark the next free cluster as used
    UpdateEntryInFat(hd, nextFreeCluster, nextFreeCluster, fatLocation);

    //Mark the newly allocated cluster as the last cluster of the file
    UpdateEntryInFat(hd, nextFreeCluster, 0x0FFFFFFF, fatLocation);

    //Modify the FAT table to update the last cluster to point to the new cluster
    UpdateEntryInFat(hd, currentCluster, nextFreeCluster, fatLocation);

    return nextFreeCluster;
};

/**
 * Update the entry in the FAT
 * @param cluster The cluster to update
 * @param newFatValue The new value to set the entry to
 * @param fatLocation The location of the FAT table
 */
void Fat32::UpdateEntryInFat(drivers::AdvancedTechnologyAttachment *hd, uint32_t cluster, uint32_t newFatValue, uint32_t fatLocation){
    
    uint32_t  fatBuffer[512 / sizeof(uint32_t)];  
    uint32_t sector = cluster / (512 / sizeof(uint32_t));                                                                                //Calculate the sector of the FAT table
    uint32_t offset = cluster % (512 / sizeof(uint32_t));                                                                                //Calculate the offset of the FAT table
    hd -> Read28(fatLocation + sector, (uint8_t*)fatBuffer, sizeof(fatBuffer));              //Read the sector of the FAT table
    fatBuffer[offset] = newFatValue;                                                                                                //Set the entry to 0x0FFFFFFF
    hd -> Write28(fatLocation + sector, (uint8_t*)fatBuffer, sizeof(fatBuffer));             //Write the changes to the disk                                    //Write the sector of the FAT table

};

///__DirectoryTraverser__///

FatDirectoryTraverser::FatDirectoryTraverser(drivers::AdvancedTechnologyAttachment* ataDevice, common::uint32_t dirSec, common::uint32_t dataStart, common::uint32_t clusterSectorCount, common::uint32_t fatLoc, common::uint32_t fat_size) {

    //TODO: For reading directotys Add multiple sector support, and multiple cluster support
    //TODO: Add error checks

    hd = ataDevice;

    //Set the data start and cluster sector count and fat location
    dataStartSector = dataStart;
    sectorsPrCluster = clusterSectorCount;
    fatLocation = fatLoc;
    directorySector = dirSec;
    fatSize = fat_size;

    //Read the directory entry's from the directory sector
    hd -> Read28(directorySector, (uint8_t*)&dirent[0], 16*sizeof(DirectoryEntry));

    //Clear the enumerators
    currentFileEnumerator = 0;
    currentDirectoryEnumerator = 0;


    for(int i = 0; i < 16; i++) {

        //If the name is 0x00 then there are no more entries
        if (dirent[i].name[0] == 0x00)
            break;

        //If the atrribute is 0x0F then this is a long file name entry, skip it.
        if ((dirent[i].attributes & 0x0F) == 0x0F || (dirent[i].attributes & 0x10) != 0x10)
            continue;

        if(currentDirectoryEnumerator == 0){                                                                                            //If this is the first directory entry then set the current directory to this entry
            currentDirectoryIndex = i;                                                                                                  //Set the current directory index to the current entry
            FatDirectoryEnumerator* dir = new FatDirectoryEnumerator(this, dirent[i], currentDirectoryIndex);         //Create a new directory enumerator
            currentDirectoryEnumerator = dir;                                                                                           //Set the current directory enumerator to the new directory enumerator
        }
    }

    for(int i = 0; i < 16; i++) {

        //If the name is 0x00 then there are no more entries
        if (dirent[i].name[0] == 0x00)
            break;

        //If the atrribute is 0x0F then this is a long file name entry, skip it.
        if ((dirent[i].attributes & 0x0F) == 0x0F || (dirent[i].attributes & 0x10) == 0x10)
            continue;

        if(currentFileEnumerator == 0){
            currentFileIndex = i;
            FatFileEnumerator* file = new FatFileEnumerator(this, dirent[i], currentFileIndex);
            currentFileEnumerator = file;
        }


    }

    //Intialize the reader, writer and its buffer
    FatFileReader* fr = (FatFileReader*)currentFileEnumerator -> getReader();
    FatFileWriter* fw = (FatFileWriter*)currentFileEnumerator -> getWriter();

    uint32_t fileSize = fr -> GetFileSize() +512;                                                   //Test what happens if we try to write more then whats allready allocated for the file

    uint8_t* fileBuffer = (uint8_t*)MemoryManager::activeMemoryManager ->malloc(fileSize);




    //Write some dummy text to the file
    for (int j = 0; j < fileSize; ++j) {
        unsigned char a = 'b';
        fileBuffer[j] = (uint8_t)a;
    }


    //Write to the file
    fw -> Write(fileBuffer, fileSize);
    printf("\nData written: ");
    printf((char*)fileBuffer);


    MemoryManager::activeMemoryManager -> free(fileBuffer);

    //TODO: Fix bug that the physical drive doesnt get updated, probabbly has also got something to do with the reading error



}

FatDirectoryTraverser::~FatDirectoryTraverser() {

}

/**
 * Changes the current directory to the specified directory, re reads the directory entries
 * @param directory The directory to change to
 */
void FatDirectoryTraverser::changeDirectory(DirectoryEnumerator directory) {

    //Get the first sector of the directory
    //Set this to the directory sector
    //Re-read the directory entries
    //Print for debuging


}

void FatDirectoryTraverser::makeDirectory(char *name) {
    //Check if the name is a valid FAT32 name
    //Check if the name is already in use
    //Create a new directory entry with the name
	//Allocate a cluster for the directory
    //Write the directory entry to the disk

}

void FatDirectoryTraverser::removeDirectory(char *name) {
    //Loop  throuugh all the dirictory entrys
	//Check if the name is the same as parameter
	//De allocate any clusters  
	//Clear the directory entry

}

void FatDirectoryTraverser::makeFile(char *name) {
   //Check if the name is a valid FAT32 name
    //Check if the name is already in use
    //Create a new directory entry with the name
	//Allocate a cluster for the file
    //Write the directory entry to the disk
}

void FatDirectoryTraverser::removeFile(char *name) {
    //Loop  throuugh all the dirictory entrys
	//Check if the name is the same as parameter
	//De allocate any clusters  
	//Clear the directory entry
}

FileEnumerator* FatDirectoryTraverser::getFileEnumerator() {
    return currentFileEnumerator;
}

DirectoryEnumerator* FatDirectoryTraverser::getDirectoryEnumerator() {
    return currentDirectoryEnumerator;
}

void FatDirectoryTraverser::WriteDirectoryInfoChange(DirectoryEntry* e) {

    DirectoryEntry entry = *e;
    for (int j = 0; j < 16; ++j) {

        //Calculate the first cluster position in the FAT
        uint32_t  firstFileCluster = ((uint32_t) entry.firstClusterHigh << 16)       //Shift the high cluster number 16 bits to the left
                                     | entry.firstClusterLow;                        //Add the low cluster number

        uint32_t  afirstFileCluster = ((uint32_t) dirent[i].firstClusterHigh << 16)       //Shift the high cluster number 16 bits to the left
                                     | dirent[i].firstClusterLow;                        //Add the low cluster number

        //Check if they are the same
        if(firstFileCluster == afirstFileCluster){
            dirent[i] = entry;
            break;
        }

    }

    hd -> Write28(directorySector, (uint8_t*)dirent, sizeof(dirent));

}

///__DirectoryEnumerator__///

FatDirectoryEnumerator::FatDirectoryEnumerator(FatDirectoryTraverser* parent, DirectoryEntry directory, int id) {

    //Set the directory entry
    traverser = parent;
    directoryInfo = &directory;
    index = id;




}

FatDirectoryEnumerator::~FatDirectoryEnumerator() {

}

/**
 * Get the name of the directory
 * @return The name of the directory
 */
char* FatDirectoryEnumerator::getDirectoryName() {
    char *foo = "        ";
    for (int j = 0; j < 8; j++)
        foo[j] = directoryInfo -> name[j];

    return foo;
}

/**
 * Change the name of the directory
 * @param newDirectoryName The new name of the directory
 * @return The old name of the directory
 */
char *FatDirectoryEnumerator::changeDirectoryName(char* newDirectoryName) {

    //TODO: Check if the new name is acceptable
    //TODO: Write the new name into the directory entry
    //TODO: Save the changes to the disk

}

/**
 * Check if there is another directory in the parent directory
 * @return True if there is another directory, false if not
 */
bool FatDirectoryEnumerator::hasNext() {

    return next() != 0;
}

/**
 * Get the next directory in the parent directory
 * @return The next directory in the parent directory
 */
DirectoryEnumerator* FatDirectoryEnumerator::next() {

    for(int i = index+1; i < 16; i++) {

        //If the name is 0x00 then there are no more entries
        if (traverser -> dirent[i].name[0] == 0x00)
            break;

        //If the atrribute is 0x0F then this is a long file name entry, skip it. Or if it isnt a directory  then skip it
        if ((traverser -> dirent[i].attributes & 0x0F) == 0x0F || (traverser -> dirent[i].attributes & 0x10) != 0x10)
            continue;

        FatDirectoryEnumerator* dir = new FatDirectoryEnumerator(traverser, traverser -> dirent[i], i);
        return dir;
    }
    return 0;
}

///__FileEnumerator__///

FatFileEnumerator::FatFileEnumerator(FatDirectoryTraverser* parent, DirectoryEntry file, int id) {
    //Set the directory entry
    traverser = parent;
    fileInfo = &file;
    index = id;

    //Create a new reader and writer
    reader = new FatFileReader(parent,file);
    writer = new FatFileWriter(parent,file);


}

FatFileEnumerator::~FatFileEnumerator() {

}

/**
 * Get the name of the file
 * @return The name of the file
 */
char *FatFileEnumerator::getFileName() {
    char *foo = "        ";
    for (int j = 0; j < 8; j++)
        foo[j] = fileInfo -> name[j];

    return foo;
}

/**
 * Changes the name of the currently enumerated file
 * @param newFileName The new filename
 * @return The old filename
 */
char *FatFileEnumerator::changeFileName(char *newFileName) {
    //TODO: Check if the new name is acceptable
    //TODO: Write the changes to the directory entry
    //TODO: Write the changes to disk
    return nullptr;
}

/**
 * Gets a reference to the current reader object
 * @return The current reader object
 */
FileReader* FatFileEnumerator::getReader() {
    return reader;
}

/**
 * Gets a reference to the current writer object
 * @return The current writer object
 */
FileWriter* FatFileEnumerator::getWriter() {
    return writer;
}

/**
 * Checks if there is a another file enumerater after this one
 * @return True if there is another file, false if not
 */
bool FatFileEnumerator::hasNext() {
    return next() != 0;
}

/**
 * Creates a new FatFileEnumerator object for the next file in the directory (Note: this is why you should store the return value in a varible and refernce that value instead of calling x -> next() -> doY(); )
 * @return The next file in the directory
 */
FileEnumerator* FatFileEnumerator::next() {
    for(int i = index+1; i < 16; i++) {

        //If the name is 0x00 then there are no more entries
        if (traverser -> dirent[i].name[0] == 0x00)
            break;

        //If the atrribute is 0x0F then this is a long file name entry, skip it. Or if it is a directory  then skip it
        if ((traverser -> dirent[i].attributes & 0x0F) == 0x0F || (traverser -> dirent[i].attributes & 0x10) == 0x10)
            continue;

        FatFileEnumerator* file = new FatFileEnumerator(traverser, traverser -> dirent[i], i);
        return file;
    }
    return 0;
}

///__FileReader__///
FatFileReader::FatFileReader(FatDirectoryTraverser* parent, DirectoryEntry file) {

    fileInfo = &file;
    traverser = parent;

}

FatFileReader::~FatFileReader() {

}

/**
 * Read a number of bytes from the file
 * @param data The buffer to read the data into
 * @param size The number of bytes to read
 * @return The number of bytes read. If this is less than size then the end of the file has been reached.
 */
common::uint32_t FatFileReader::Read(common::uint8_t *data, common::uint32_t size) {

    //TODO: fix performance issues

    //Read the first cluster of the file
    uint32_t  firestFileCluster = ((uint32_t) fileInfo -> firstClusterHigh << 16)       //Shift the high cluster number 16 bits to the left
                                  | fileInfo -> firstClusterLow;              //Add the low cluster number


    int32_t fSIZE = fileInfo -> size;
    int32_t nextFileCluster = firestFileCluster;
    uint8_t fileBuffer[513];
    uint8_t fatBuffer[513];

    int dataOffset = 0;
    int readPos = 0;

    while (fSIZE > 0){

        //Convert file cluster into a sector
        uint32_t fileSector = traverser -> dataStartSector + traverser -> sectorsPrCluster*(nextFileCluster - 2);                  //*Offset by 2
        int sectorOffset = 0;


        //Loop the sectors of the file
        for (; fSIZE > 0; fSIZE -= 512) {


            traverser -> hd -> Read28(fileSector + sectorOffset, fileBuffer, 512);    //Read the specified sector of the file
            sectorOffset++;                                                                            //Increment the sector offset


            fileBuffer[fSIZE > 512 ? 512 : fSIZE = '\0'];                                              //Add a null terminator to the end of the sector

            //Copy the sector into the data buffer
            for (int i = 0; i < 512; i++) {

                readPos++;

                //Check that the data being read is after/at the position of the seeker cursor
                if(readPos < offsetPosition){
                   continue;
                }

                //Increament the offset
                offsetPosition++;

                //Check that the data buffer is not full / that the requested size has not been reached
                if(dataOffset >= size)
                    return size;

                //Put the data into the data buffer
                data[dataOffset] = fileBuffer[i];
                dataOffset++;

            }

            //If the next sector is in a different cluster then break
            if (sectorOffset > traverser -> sectorsPrCluster)
                break;
        }


        //Get the next cluster of the file
        uint32_t fatSectorForCurrentCluster = nextFileCluster / (512 / sizeof(uint32_t));               //The FAT sector for the current cluster is the current cluster divided by the number of entries per sector

        //Read the FAT sector for the current cluster
        traverser -> hd -> Read28(traverser -> fatLocation + fatSectorForCurrentCluster, fatBuffer, 512);

        //Get the offset of the current cluster in the FAT sector
        uint32_t fatOffsetInSectorForCurrentCluster = nextFileCluster % (512/sizeof(uint32_t));

        //Set the next cluster to the next cluster in the FAT
        nextFileCluster = ((uint32_t*)&fatBuffer)[fatOffsetInSectorForCurrentCluster] & 0x0FFFFFFF;
    }

    //If we get to here then the user requested to read more bytes then there are in the file
    return dataOffset;
}

/**
 * Moves the file offset to a new position
 * @param position The new position of the file offset
 * @param seek The type of seek to perform. (SEEK_SET, offset = position) (SEEK_CUR, offset = offset + position)  (SEEK_END, offset = fileSize + position)
 * @return The new position of the file offset
 */
common::uint32_t FatFileReader::Seek(common::uint32_t position, SeekType seek) {

    switch (seek) {
        case SEEK_SET:
            offsetPosition = position;
            break;
        case SEEK_CUR:
            offsetPosition += position;
            break;
        case SEEK_END:
            offsetPosition = fileInfo -> size + position;
            break;
        default:
            break;
    }

    return offsetPosition;
}

/**
 * Get the current position of the file offset
 * @return the current position of the file offset
 */
common::uint32_t FatFileReader::GetPosition() {
    return offsetPosition;
}

/**
 * Get the size of the file
 * @return the size of the file
 */
common::uint32_t FatFileReader::GetFileSize() {
    return fileInfo -> size;
}

///__FileWriter__///

//TODO: Implement

FatFileWriter::FatFileWriter(FatDirectoryTraverser* parent, DirectoryEntry file) {

    //Initialize the file writer
    fileInfo = &file;
    traverser = parent;
    offsetPosition = 0;
}

FatFileWriter::~FatFileWriter() {

}

/**
 * Writes data to a file starting at the current seek position and ending at the current seek position + size
 * @param data The buffer to write the data from
 * @param size The size that is to be written
 * @return The number of bytes written, if this is less than size then then there is insufficient space on the disk
 */
common::uint32_t FatFileWriter::Write(common::uint8_t *data, common::uint32_t size) {


    //TODO: Write larger then the size of the file (e.g. grow the file)

    //Read the first cluster of the file
    uint32_t  firstFileCluster = ((uint32_t) fileInfo -> firstClusterHigh << 16)       //Shift the high cluster number 16 bits to the left
                                            | fileInfo -> firstClusterLow;              //Add the low cluster number



    int32_t fSIZE = fileInfo -> size;
    int32_t nextFileCluster = firstFileCluster;
    uint8_t fileBuffer[513];
    uint8_t fatBuffer[513];

    int dataOffset = 0;
    int writePos = 0;

    while (fSIZE > 0){

        //Convert file cluster into a sector
        uint32_t fileSector = traverser -> dataStartSector + traverser -> sectorsPrCluster*(nextFileCluster - 2);                  //*Offset by 2
        int sectorOffset = 0;


        //Loop the sectors of the file
        for (; fSIZE > 0; fSIZE -= 512) {

            //Check that the data will be writen at the offset/seek pos
            if(writePos < offsetPosition){

                ///If the data will not be written at the right place then read the sector and then begin writing after the offset
                traverser -> hd -> Read28(fileSector + sectorOffset, fileBuffer, 512);    //Read the specified sector of the file


            }
            sectorOffset++;

            //Copy the data buffer into the sector buffer
            for (int i = 0; i < 512; i++) {
                writePos++;

                //Check if it is a valid place  to put data
                if (writePos < offsetPosition)
                    continue;


                //Increment the offset
                offsetPosition++;

                //Check that the data buffer is not full / that the requested size has not been reached, if it is then write the data early and then
                if (dataOffset >= size){
                    traverser -> hd -> Write28(fileSector + sectorOffset, fileBuffer, i);    //Write the sector to the disk
                    return size;
                }

                //Put the data into the data buffer
                fileBuffer[i] = data[dataOffset];
                dataOffset++;
            }

            //Write the sector to the disk
            traverser -> hd -> Write28(fileSector + sectorOffset, fileBuffer, 512);

            //If the next sector is in a different cluster then break
            if (sectorOffset > traverser -> sectorsPrCluster)
                break;

        }//_For end




        //TODO: Test if it works
        if(writePos >= fSIZE){   //If the data to be written is past the end of the file then we need to update the FAT table with new clusters for the data

            //Increase the file size by one cluster
            uint32_t extendedFileSize = size - dataOffset;                                                                                       //The size of the file minus how much has already been written
            if(extendedFileSize > traverser -> sectorsPrCluster * 512)                                                                           //If the extended filesize is still bigger then the size of one cluster
                extendedFileSize = traverser -> sectorsPrCluster * 512;                                                                          //Then set the extended file size to the size of one cluster
            fSIZE += extendedFileSize;                                                                                                           //Increase the file size by the extended file size
            fileInfo -> size += extendedFileSize;

            //Update the new size on disk
            traverser -> WriteDirectoryInfoChange(fileInfo);            

            //Set the next cluster to the next cluster in the FAT
            nextFileCluster = Fat32::AllocateCluster(traverser -> hd, nextFileCluster, traverser -> fatLocation, traverser -> fatSize);
        
            //If there is no storage (free clusters) left on the partition
            if(nextFileCluster == -1) return dataOffset;
        }
        else
        {                  //Otherwise we can just check the FAT table for the next cluster and continue


            //Get the next cluster of the file
            uint32_t sector = nextFileCluster / (512 / sizeof(uint32_t));                                                                       //Calculate the sector of the FAT table
            uint32_t offset = nextFileCluster % (512 / sizeof(uint32_t));                                                                       //Calculate the offset of the FAT table

            //Read the FAT sector for the current cluster
            traverser -> hd -> Read28(traverser -> fatLocation + sector, fatBuffer, 512);

            //Set the next cluster to the next cluster in the FAT
            nextFileCluster = ((uint32_t*)&fatBuffer)[offset ] & 0x0FFFFFFF;

        }

    }

    //If we get to here then the user requested to read more bytes than there are in the file
    return dataOffset;
}



common::uint32_t FatFileWriter::Seek(common::uint32_t position, SeekType seek) {
    switch (seek) {
        case SEEK_SET:
            offsetPosition = position;
            break;
        case SEEK_CUR:
            offsetPosition += position;
            break;
        case SEEK_END:
            offsetPosition = fileInfo -> size + position;
            break;
        default:
            break;
    }

    return offsetPosition;
}

bool FatFileWriter::Close() {

    //TODO: Implement

}

bool FatFileWriter::Flush() {

    //TODO: Implement

}

common::uint32_t FatFileWriter::GetPosition() {
    return offsetPosition;
}

common::uint32_t FatFileWriter::GetFileSize() {
    return fileInfo -> size;
}
