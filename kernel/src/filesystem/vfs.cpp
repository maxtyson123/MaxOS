//
// Created by Max Tyson on 20/04/2025.
//
#include <filesystem/vfs.h>
#include <common/logger.h>

using namespace MaxOS;
using namespace MaxOS::filesystem;

VirtualFileSystem::VirtualFileSystem()
{

    // Set the current file system to this instance
    s_current_file_system = this;

}

VirtualFileSystem::~VirtualFileSystem()
{
    // Remove all mounted filesystems
    unmount_all();

    // Set the current file system to null
    s_current_file_system = nullptr;

}

/**
 * @brief Get the active virtual file system
 *
 * @return The current virtual file system or null if none is set
 */
VirtualFileSystem* VirtualFileSystem::current_file_system()
{
    return s_current_file_system;
}

/**
 * @brief Add a filesystem to the virtual file system
 *
 * @param filesystem The filesystem to add
 */
void VirtualFileSystem::mount_filesystem(FileSystem* filesystem)
{

    // Check if the filesystem is already mounted
    if (filesystems.find(filesystem) != filesystems.end())
    {
        Logger::WARNING() << "Filesystem already mounted\n";
        return;
    }

    // Get the mount point for the filesystem
    string mount_point = "/filesystem_" + filesystems.size();

    // If this is the first filesystem to be mounted, set the root filesystem
    if (filesystems.size() == 0)
        mount_point = "/";

    // Add the filesystem to the map
    filesystems.insert(filesystem, mount_point);
}

/**
 * @brief Add a filesystem to the virtual file system at a given mount point
 *
 * @param filesystem The filesystem to add
 * @param mount_point The mount point for the filesystem
 */
void VirtualFileSystem::mount_filesystem(FileSystem* filesystem, string mount_point)
{

    // Check if the filesystem is already mounted
    if (filesystems.find(filesystem) != filesystems.end())
    {
        Logger::WARNING() << "Filesystem already mounted at " << mount_point << "\n";
        return;
    }

    // Add the filesystem to the map
    filesystems.insert(filesystem, mount_point);
}

/**
 * @brief Remove a filesystem from the virtual file system & delete it
 *
 * @param filesystem The filesystem to remove
 */
void VirtualFileSystem::unmount_filesystem(FileSystem* filesystem)
{

    // Check if the filesystem is mounted
    if (filesystems.find(filesystem) == filesystems.end())
        return;

    // Remove the filesystem from the map
    filesystems.erase(filesystem);

    // Delete the filesystem
    delete filesystem;
}

/**
 * @brief Remove a filesystem from the virtual file system
 *
 * @param mount_point Where the filesystem is mounted
 */
void VirtualFileSystem::unmount_filesystem(string mount_point)
{
    // Check if the filesystem is mounted
    auto it = filesystems.begin();
    while (it != filesystems.end())
    {
        if (it->second == mount_point)
        {
            // Remove the filesystem from the map
            return unmount_filesystem(it->first);
        }
        ++it;
    }

    // Filesystem not found
    Logger::WARNING() << "Filesystem not found at " << mount_point << "\n";
}

/**
 * @brief Remove all mounted filesystems
 */
void VirtualFileSystem::unmount_all()
{

    // Loop through the filesystems and unmount them
    for (auto it = filesystems.begin(); it != filesystems.end();)
    {
        unmount_filesystem(it->first);
        it = filesystems.begin();
    }
}

/**
 * @brief Get the filesystem mounted at the root
 *
 * @return The file system mounted "/" or null if none is mounted
 */
FileSystem* VirtualFileSystem::root_filesystem()
{

    // Ensure there is at least one filesystem mounted
    if (filesystems.size() == 0)
        return nullptr;

    // It is always the first filesystem mounted
    return filesystems.begin()->first;
}

/**
 * @brief Get a specific filesystem mounted at a given mount point
 *
 * @param mount_point The mount point to search for
 * @return The filesystem mounted at the given mount point or null if none is found
 */
FileSystem* VirtualFileSystem::get_filesystem(string mount_point)
{

    // Check if the filesystem is mounted
    auto it = filesystems.begin();
    while (it != filesystems.end())
    {
        if (it->second == mount_point)
            return it->first;
        ++it;
    }

    // Filesystem not found
    return nullptr;
}

/**
 * @brief Find the filesystem that is responsible for a given path
 *
 * @param path The path to search for
 * @return The filesystem that contains the path or null if none is found
 */
FileSystem* VirtualFileSystem::find_filesystem(string path)
{

    // Longest matching path will be where the filesystem is mounted
    string longest_match = "";
    FileSystem* longest_match_fs = nullptr;

    // Search through the filesystems
    for (auto it = filesystems.begin(); it != filesystems.end(); ++it)
    {
        // Get the filesystem and mount point
        FileSystem* fs = it->first;
        string mount_point = it->second;

        // Check if the path starts with the mount point
        if (path.starts_with(mount_point) && mount_point.length() > longest_match.length())
        {
            longest_match = mount_point;
            longest_match_fs = fs;
        }
    }

    return longest_match_fs;
}

/**
 * @brief Get the relative path on a filesystem for a given VFS path (ie remove the mount point)
 *
 * @param filesystem The filesystem to get the path for
 * @param path The path to get the relative path for
 * @return The relative path on the filesystem or an empty string if none is found
 */
string VirtualFileSystem::get_relative_path(FileSystem* filesystem, string path)
{
    // Find the mount point for the filesystem
    auto it = filesystems.find(filesystem);
    if (it == filesystems.end())
        return "";

    // Get the mount point
    string mount_point = it->second;

    // Make sure that the path points to the filesystem
    if (!path.starts_with(mount_point))
        return "";

    // Get the relative path
    string relative_path = path.substring(mount_point.length(), path.length() - mount_point.length());
    return relative_path;
}

/**
 * @brief Get the root directory of the virtual file system
 *
 * @return The root directory of the virtual file system
 */
Directory* VirtualFileSystem::root_directory()
{
    // Get the root filesystem
    FileSystem* fs = root_filesystem();

    // Check if the filesystem is mounted
    if (!fs)
        return nullptr;

    // Get the root directory
    return fs->root_directory();
}

/**
 * @brief Try to open a directory on the virtual file system and read it's contents
 *
 * @param path The path to the directory
 * @return The directory object or null if it could not be opened
 */
Directory* VirtualFileSystem::open_directory(string path)
{
    // Ensure a valid path is given
    if (!Path::vaild(path))
        return nullptr;

    // Try to find the filesystem that is responsible for the path
    FileSystem* fs = find_filesystem(path);
    if (!fs)
        return nullptr;

    // Get where to open the directory
    string relative_path = get_relative_path(fs, path);
    string directory_path = Path::file_path(relative_path);

    // Open the directory
    Directory* directory = fs -> get_directory(directory_path);
    if (!directory)
        return nullptr;
    directory -> read_from_disk();

    return directory;
}

/**
 * @brief Try to create a directory on the virtual file system & read its contents
 *
 * @param path The path to the directory
 * @return The directory object or null if it could not be opened
 */
Directory* VirtualFileSystem::create_directory(string path)
{
    // Ensure a valid path is given
    if (!Path::vaild(path))
        return nullptr;

    path = path.strip('/');

    // Try to find the filesystem that is responsible for the path
    FileSystem* fs = find_filesystem(path);
    if (!fs)
        return nullptr;

    // Open the parent directory
    Directory* parent_directory = open_directory(path);
    if (!parent_directory)
        return nullptr;

    // Create the directory
    string directory_name = Path::file_name(path);
    Directory* directory =  parent_directory -> create_subdirectory(directory_name);
    directory -> read_from_disk();

    return directory;
}

/**
 * @brief Delete a directory on the virtual file system
 *
 * @param path The path to the directory
 */
void VirtualFileSystem::delete_directory(string path)
{
    // Ensure a valid path is given
    if (!Path::vaild(path))
        return;

    path = path.strip('/');

    // Open the directory
    Directory* directory = open_directory(path);
    if (!directory)
        return;

    // Delete the directory
    string directory_name = Path::file_name(path);
    directory -> remove_subdirectory(directory_name);
}


/**
 * @brief Try to create a file on the virtual file system
 *
 * @param path The path to the file (including the extension)
 * @return The file object or null if it could not be created
 */
File* VirtualFileSystem::create_file(string path)
{
    // Ensure a valid path is given
    if (!Path::vaild(path))
        return nullptr;

    // Open the directory
    Directory* directory = open_directory(path);
    if (!directory)
        return nullptr;

    // Create the file
    string file_name = Path::file_name(path);
    return directory -> create_file(file_name);
}

/**
 * @brief Try to open a file on the virtual file system
 *
 * @param path The path to the file (including the extension)
 * @return The file object or null if it could not be opened
 */
File* VirtualFileSystem::open_file(string path)
{

    // Open the file at the start
    return open_file(path, 0);

}


/**
 * @brief Try to open a file on the virtual file system with a given offset
 *
 * @param path The path to the file (including the extension)
 * @param offset The offset to seek to
 * @return
 */
File* VirtualFileSystem::open_file(string path, size_t offset)
{
    // Ensure a valid path is given
    if (!Path::vaild(path))
        return nullptr;

    // Open the directory
    Directory* directory = open_directory(path);
    if (!directory)
        return nullptr;

    // Open the file
    File* opened_file = directory -> open_file(Path::file_name(path));
    if (!opened_file)
        return nullptr;

    // Seek to the offset
    opened_file -> seek(SeekType::SET, offset);

    // File opened successfully
    return opened_file;
}

/**
 * @brief Delete a file on the virtual file system
 *
 * @param path The path to the file (including the extension)
 */
void VirtualFileSystem::delete_file(string path)
{
    // Ensure a valid path is given
    if (!Path::vaild(path))
        return;

    // Open the directory
    Directory* directory = open_directory(path);
    if (!directory)
        return;

    // Delete the file
    string file_name = Path::file_name(path);
    directory -> remove_file(file_name);
}
