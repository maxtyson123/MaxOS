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
 * Check if there is a directory entry after the current one
 * @return True if there is a directory entry after the current one, false otherwise
 */
bool DirectoryTraverser::hasNext() {
    return false;
}

