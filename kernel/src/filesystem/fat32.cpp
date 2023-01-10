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


    FatDirectoryTraverser rootTraverser(hd, rootStart, dataStart, bpb.sectorsPerCluster, fatStart);
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


///__DirectoryTraverser__///

FatDirectoryTraverser::FatDirectoryTraverser(drivers::AdvancedTechnologyAttachment* ataDevice, common::uint32_t directorySector, common::uint32_t dataStart, common::uint32_t clusterSectorCount, common::uint32_t fatLoc) {

    //TODO: Add multiple sector support, and multiple cluster support

    hd = ataDevice;

    //Read the directory entry's from the directory sector
    hd -> Read28(directorySector, (uint8_t*)&dirent[0], 16*sizeof(DirectoryEntry));

    //Clear the enumerators
    currentFileEnumerator = 0;
    currentDirectoryEnumerator = 0;

    //Set the data start and cluster sector count and fat location
    dataStartSector = dataStart;
    sectorsPrCluster = clusterSectorCount;
    fatLocation = fatLoc;

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

    //Intialize the reader and its buffer
    FatFileReader* fr = (FatFileReader*)currentFileEnumerator -> getReader();
    uint8_t* readBuffer = (uint8_t*)MemoryManager::activeMemoryManager -> malloc(fr->GetFileSize()/2);

    //Read all the data from the file and then print it to the screen
    fr -> Read(readBuffer, fr->GetFileSize()/2);
    printf((char *)readBuffer);

    //Test the setting of the seek positioning
    fr -> Seek(0, SEEK_SET);

    //Read the second half (split for testing of cursor moving)
    fr -> Read(readBuffer, fr->GetFileSize()/2);
    printf((char *)readBuffer);

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
    DirectoryTraverser::makeDirectory(name);
}

void FatDirectoryTraverser::removeDirectory(char *name) {
    DirectoryTraverser::removeDirectory(name);
}

void FatDirectoryTraverser::makeFile(char *name) {
    DirectoryTraverser::makeFile(name);
}

void FatDirectoryTraverser::removeFile(char *name) {
    DirectoryTraverser::removeFile(name);
}

FileEnumerator* FatDirectoryTraverser::getFileEnumerator() {
    return currentFileEnumerator;
}

DirectoryEnumerator* FatDirectoryTraverser::getDirectoryEnumerator() {
    return currentDirectoryEnumerator;
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
char *FatDirectoryEnumerator::changeDirectoryName(char *newDirectoryName) {

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
    //TODO: Create a writer object once the writer is implemented

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

FatFileWriter::FatFileWriter() {

}

FatFileWriter::~FatFileWriter() {

}

common::uint32_t FatFileWriter::Write(common::uint8_t *data, common::uint32_t size) {
    return FileWriter::Write(data, size);
}

common::uint32_t FatFileWriter::Seek(common::uint32_t position, SeekType seek) {
    return FileWriter::Seek(position, seek);
}

bool FatFileWriter::Close() {
    return FileWriter::Close();
}

bool FatFileWriter::Flush() {
    return FileWriter::Flush();
}

common::uint32_t FatFileWriter::GetPosition() {
    return FileWriter::GetPosition();
}

common::uint32_t FatFileWriter::GetFileSize() {
    return FileWriter::GetFileSize();
}


