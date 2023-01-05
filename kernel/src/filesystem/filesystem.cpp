//
// Created by 98max on 5/01/2023.
//

#include <filesystem/filesystem.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::filesystem;

///__FileSystem__///

FileSystem::FileSystem() {
    //Base implementation:
    //Create a new Directory Traverser and store it in the FileSystem object
}

FileSystem::~FileSystem() {

}

/**
 * Get the directory traverser for the filesystem
 * @return
 */
DirectoryTraverser FileSystem::getDirectoryTraverser() {
    return DirectoryTraverser();
}

///__DirectoryTraverser__///

DirectoryTraverser::DirectoryTraverser() {
    //A directory traverser provides a way to traverse a directory and get the files and subdirectories
    //It allows to change what directory is being traversed

}

DirectoryTraverser::~DirectoryTraverser() {

}

/**
 * Change what directory is being traversed
 * @param directory the directory to traverse
 */
void DirectoryTraverser::changeDirectory(DirectoryEnumerator directory) {

    //e.g. change the current sector to the sector of the directory
    //e.g. reset the directoryEnumerator and fileEnumerator

}

/**
 * Creates a new directory
 * @param name the name of the directory
 */
void DirectoryTraverser::makeDirectory(char* name) {

}

/**
 * Removes a directory and all its contents
 * @param name the name of the directory to remove
 */
void DirectoryTraverser::removeDirectory(char *name) {

}




/**
 * Gets the file enumerator for the current directory
 * @return the file enumerator
 */
FileEnumerator DirectoryTraverser::getFileEnumerator() {
    return FileEnumerator();
}

/**
 * Gets the directory enumerator for the current directory
 * @return  the directory enumerator
 */
DirectoryEnumerator DirectoryTraverser::getDirectoryEnumerator() {
    return DirectoryEnumerator();
}

/**
 * Creates a new file
 * @param name the name of the file
 */
void DirectoryTraverser::makeFile(char *name) {

}

/**
 * Removes a file
 * @param name the name of the file
 */
void DirectoryTraverser::removeFile(char *name) {

}


///__Directory Enumerator__///


DirectoryEnumerator::DirectoryEnumerator() {

    //A directory enumerator provides a way to enumerate the directories in a directory
    //It allows to get the name of the directory and to get the next directory

}

DirectoryEnumerator::~DirectoryEnumerator() {

}

/**
 * Gets the name of the directory
 * @return the name of the directory
 */
char* DirectoryEnumerator::getDirectoryName() {

}

/**
 * Checks if there is another directory
 * @return true if there is another directory, false otherwise
 */
bool DirectoryEnumerator::hasNext() {
    return false;
}

/**
 * Gets the next directoryEnumerator in this DirectoryTraverser
 * @return the next directoryEnumerator
 */
DirectoryEnumerator DirectoryEnumerator::next() {
    return DirectoryEnumerator();
}

///__FileEnumerator__///

FileEnumerator::FileEnumerator() {
    //A file enumerator provides a way to enumerate the files in a directory
    //It allows to get the name of the file, to get the reader and writer for the file and to get the next file
}

FileEnumerator::~FileEnumerator() {

}

/**
 * Gets the name of the current file
 * @return the name of the file
 */
char* FileEnumerator::getFileName() {
    char* a = "none";
    return a;
}

/**
 * Gets the reader for the current file
 * @return the reader for the file
 */
FileReader FileEnumerator::getReader() {
    return FileReader();
}

/**
 * Gets the writer for the current file
 * @return the writer for the file
 */
FileWriter FileEnumerator::getWriter() {
    return FileWriter();
}

/**
 * Checks if there is another file
 * @return true if there is another file, false otherwise
 */
bool FileEnumerator::hasNext() {
    return false;
}

/**
 * Gets the next file in this DirectoryTraverser
 * @return The next file
 */
FileEnumerator FileEnumerator::next() {
    return FileEnumerator();
}

///__FileReader__///

FileReader::FileReader() {
    //A file reader provides a way to read from a file
    //It allows to get the position in the file, to get the size of the file and to read from the file

}

FileReader::~FileReader() {

}

/**
 * Reads from the file and stores the data in the buffer at the given position (default 0 + how many bytes has been read this fileEnumeration)
 * @param data the data pointer to read into
 * @param size the size of the data to read
 * @return the amount of data read
 */
uint32_t FileReader::Read(common::uint8_t *data, common::uint32_t size) {

}

/**
 * Moves the position in the file
 * @param position how many bytes to move the position
 * @return the position in the file
 */
uint32_t FileReader::Seek(common::uint32_t position) {

}

/**
 * Gets the current position in the file
 * @return the current position in the file
 */
common::uint32_t FileReader::GetPosition() {
    return 0;
}

/**
 * Gets the size of the file
 * @return the size of the file
 */
common::uint32_t FileReader::GetFileSize() {
    return 0;
}

///__FileWriter__///

FileWriter::FileWriter() {
    //A file writer provides a way to write to a file
    //It allows to get the position in the file, to get the size of the file and to write to the file

}

FileWriter::~FileWriter() {

}

/**
 * Writes to the file at the given position (default 0 + how many bytes has been read this fileEnumeration)
 * @param data the data pointer to write from
 * @param size the size of the data to write
 * @return the amount of data written
 */
uint32_t FileWriter::Write(common::uint8_t *data, common::uint32_t size) {
    return 0;
}

/**
 * Moves the position in the file
 * @param position  the position to move to
 * @return how many bytes the position has been moved
 */
common::uint32_t FileWriter::Seek(common::uint32_t position) {
    return 0;
}

/**
 * Closes the current file
 * @return True if the file was closed successfully, false otherwise
 */
bool FileWriter::Close() {
    return false;
}

/**
 * Flushes the current file (writes all data to the disk)
 * @return True if the file was flushed successfully, false otherwise
 */
bool FileWriter::Flush() {
    return false;
}

/**
 * Gets the current position in the file
 * @return the current position in the file
 */
common::uint32_t FileWriter::GetPosition() {
    return 0;
}

/**
 * Gets the size of the file
 * @return the size of the file
 */
common::uint32_t FileWriter::GetFileSize() {
    return 0;
}


