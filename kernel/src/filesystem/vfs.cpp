/**
 * @file vfs.cpp
 * @brief Implementation of a Virtual File System (VFS) for managing multiple file systems
 *
 * @date 20th April 2025
 * @author Max Tyson
 */

#include <filesystem/vfs.h>
#include <common/logger.h>

using namespace MaxOS;
using namespace MaxOS::filesystem;
using namespace MaxOS::common;

VirtualFileSystem::VirtualFileSystem() {

	// Set the current file system to this instance
	s_current_file_system = this;

}

VirtualFileSystem::~VirtualFileSystem() {

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
VirtualFileSystem* VirtualFileSystem::current_file_system() {
	return s_current_file_system;
}

/**
 * @brief Add a filesystem to the virtual file system
 *
 * @param filesystem The filesystem to add
 */
void VirtualFileSystem::mount_filesystem(FileSystem* filesystem) {

	// Check if the filesystem is already mounted
	if (filesystems.find(filesystem) != filesystems.end()) {
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
void VirtualFileSystem::mount_filesystem(FileSystem* filesystem, const string& mount_point) {

	// Check if the filesystem is already mounted
	if (filesystems.find(filesystem) != filesystems.end()) {
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
void VirtualFileSystem::unmount_filesystem(FileSystem* filesystem) {

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
void VirtualFileSystem::unmount_filesystem(const string& mount_point) {

	// Remove the filesystem from the map
	for(const auto& filesystem : filesystems)
		if (filesystem.second == mount_point)
			return unmount_filesystem(filesystem.first);

	// Filesystem not found
	Logger::WARNING() << "Filesystem not found at " << mount_point << "\n";
}

/**
 * @brief Remove all mounted filesystems
 */
void VirtualFileSystem::unmount_all() {

	// Loop through the filesystems and unmount them
	for(const auto& filesystem : filesystems)
		unmount_filesystem(filesystem.first);

}

/**
 * @brief Get the root directory of the virtual file system
 *
 * @return The root directory of the virtual file system
 */
Directory* VirtualFileSystem::root_directory() {

	// Get the root filesystem
	FileSystem* fs = root_filesystem();
	if (!fs)
		return nullptr;

	// Get the root directory
	return fs->root_directory();
}

/**
 * @brief Get the filesystem mounted at the root
 *
 * @return The file system mounted "/" or null if none is mounted
 */
FileSystem* VirtualFileSystem::root_filesystem() {

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
FileSystem* VirtualFileSystem::get_filesystem(const string& mount_point) {

	// Check if the filesystem is mounted
	for(const auto& filesystem : filesystems)
		if (filesystem.second == mount_point)
			return filesystem.first;

	// Filesystem not found
	return nullptr;
}

/**
 * @brief Find the filesystem that is responsible for a given path
 *
 * @param path The path to search for
 * @return The filesystem that contains the path or null if none is found
 */
FileSystem* VirtualFileSystem::find_filesystem(string path) {

	// Longest matching path will be where the filesystem is mounted
	string longest_match = "";
	FileSystem* longest_match_fs = nullptr;

	// Search through the filesystems
	for (const auto& filesystem : filesystems) {
		// Get the filesystem and mount point
		FileSystem* fs = filesystem.first;
		string mount_point = filesystem.second;

		// Check if the path starts with the mount point
		if (path.starts_with(mount_point) && mount_point.length() > longest_match.length()) {
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
string VirtualFileSystem::get_relative_path(FileSystem* filesystem, string path) {

	// Find the mount point for the filesystem
	auto fs = filesystems.find(filesystem);
	if (fs == filesystems.end())
		return "";

	// Get the mount point
	string mount_point = fs->second;

	// Make sure that the path points to the filesystem
	if (!path.starts_with(mount_point))
		return "";

	// Get the relative path
	string relative_path = path.substring(mount_point.length(), path.length() - mount_point.length());
	return relative_path;
}

/**
 * @brief Try to open a directory on the virtual file system and read it's contents
 *
 * @param path The path to the directory
 * @return The directory object or null if it could not be opened
 */
Directory* VirtualFileSystem::open_directory(const string &path) {

	// Ensure a valid path is given
	if (!Path::valid(path))
		return nullptr;

	// Try to find the filesystem that is responsible for the path
	FileSystem* fs = find_filesystem(path);
	if (!fs)
		return nullptr;

	// Get where to open the directory
	string relative_path = get_relative_path(fs, path);
	string directory_path = Path::file_path(relative_path);

	// Open the directory
	Directory* directory = fs->get_directory(directory_path);
	if (!directory)
		return nullptr;
	directory->read_from_disk();

	return directory;
}

/**
 * @brief Opens a subdirectory from a parent directory
 *
 * @param parent The parent directory
 * @param name The name of the subdirectory
 * @return The opened subdirectory or null if it could not be opened
 */
Directory* VirtualFileSystem::open_directory(Directory* parent, string const& name) {

	// Open the file
	Directory* opened_directory = parent->open_subdirectory(name);
	if (!opened_directory)
		return nullptr;

	opened_directory->read_from_disk();
	return opened_directory;
}

/**
 * @brief Attempts to open the parent directory and creates the sub directory at the end of the path
 *
 * @param path The path to the directory
 * @return The directory object or null if it could not be opened
 */
Directory* VirtualFileSystem::create_directory(string path) {

	// Ensure a valid path is given
	if (!Path::valid(path))
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

	string directory_name = Path::file_name(path);
	return create_directory(parent_directory, directory_name);
}

/**
 * @brief Creates a subdirectory in the specified directory and
 *
 * @param parent Where to create the directory
 * @param name The name of the new directory
 * @return The created directory
 */
Directory* VirtualFileSystem::create_directory(Directory* parent, string const &name) {

	// Create the directory
	Directory* directory = parent->create_subdirectory(name);
	directory->read_from_disk();

	return directory;
}


/**
 * @brief Attempts to open the parent directory and deletes the sub directory at the end of the path
 *
 * @param path The path to the directory
 */
void VirtualFileSystem::delete_directory(string path) {

	// Ensure a valid path is given
	if (!Path::valid(path))
		return;

	path = path.strip('/');

	// Open the directory
	Directory* parent_directory = open_directory(path);
	if (!parent_directory)
		return;

	// Delete the directory
	string directory_name = Path::file_name(path);
	delete_directory(parent_directory, directory_name);
}

/**
 * @brief Delete a directory on the virtual file system and it's sub contents
 *
 * @param parent The directory that contains the reference to the directory being deleted
 * @param name The name of the directory to delete
 */
void VirtualFileSystem::delete_directory(Directory* parent, const string &name) {

	// Find the directory and delete it
	for (const auto& directory: parent->subdirectories())
		if (directory->name() == name)
			delete_directory(parent, directory);

}

/**
 * @brief Delete a directory on the virtual file system and it's sub contents
 *
 * @param parent The directory that contains the reference to the directory being deleted
 * @param directory The the directory to delete
 */
void VirtualFileSystem::delete_directory(Directory* parent, Directory* directory) {

	// Nothing to delete
	if (!directory)
		return;

	// Store a reference to each subdirectory and its parent
	Map<Directory*, Directory*> stack;
	Vector<Pair<Directory*, Directory*>> to_delete;
	stack.push_back(parent, directory);

	while (!stack.empty()) {

		// Save the current
		auto current = stack.pop_back();
		auto current_directory = current.second;
		current_directory->read_from_disk();
		to_delete.push_back({current.first, current_directory});

		// Empty the directory
		for (const auto &file: current_directory->files())
			delete_file(current_directory, file->name());

		// Process the subdirectories
		for (const auto &subdir: current_directory->subdirectories())
			if (subdir->name() != "." && subdir->name() != "..")
				stack.push_back(current_directory, subdir);

	}

	// Delete the directory from the bottom of the tree
	for (int i = to_delete.size() - 1; i >= 0; --i) {

		// Get the parent and child
		const auto &current = to_delete[i];
		Directory* owner = current.first;
		Directory* subdirectory = current.second;

		owner->remove_subdirectory(subdirectory->name());
	}
}


/**
 * @brief Attempts to open the parent directory and create the file at the end of the path
 *
 * @param path The path to the file (including the extension)
 * @return The file object or null if it could not be created
 */
File* VirtualFileSystem::create_file(const string &path) {

	// Ensure a valid path is given
	if (!Path::valid(path))
		return nullptr;

	// Open the directory
	Directory* directory = open_directory(path);
	if (!directory)
		return nullptr;

	// Create the file
	string file_name = Path::file_name(path);
	return create_file(directory, file_name);
}

/**
 * @brief Create a file in a directory
 *
 * @param parent The directory where the file should be created
 * @param name The name of the file to create
 */
File* VirtualFileSystem::create_file(Directory* parent, string const &name) {
	return parent->create_file(name);
}

/**
 * @brief Try to open a file on the virtual file system with a given offset
 *
 * @param path The path to the file (including the extension)
 * @param offset The offset to seek to (default = 0)
 * @return The file or null pointer if not found
 */
File* VirtualFileSystem::open_file(const string &path, size_t offset) {

	// Ensure a valid path is given
	if (!Path::valid(path))
		return nullptr;

	// Open the directory
	Directory* directory = open_directory(path);
	if (!directory)
		return nullptr;

	return open_file(directory, Path::file_name(path), offset);
}

/**
 * @brief Opens a file in a directory with the given offset
 *
 * @param parent The directory containing the file
 * @param name The name of the file to open
 * @param offset How far in the file to open (default = 0)
 * @return The file or null pointer if not found
 */
File* VirtualFileSystem::open_file(Directory* parent, string const &name, size_t offset) {

	// Open the file
	File* opened_file = parent->open_file(name);
	if (!opened_file)
		return nullptr;

	// Seek to the offset
	opened_file->seek(SeekType::SET, offset);

	return opened_file;
}

/**
 * @brief Opens a directory on the vfs and deletes the file at the end of the path
 *
 * @param path The path to the file (including the extension)
 */
void VirtualFileSystem::delete_file(const string &path) {

	// Ensure a valid path is given
	if (!Path::valid(path))
		return;

	// Open the directory
	Directory* directory = open_directory(path);
	if (!directory)
		return;

	// Delete the file
	string file_name = Path::file_name(path);
	delete_file(directory, file_name);
}

/**
 * @brief Delete a file in the given directory
 *
 * @param parent The directory containing the file
 * @param name The name of the file
 */
void VirtualFileSystem::delete_file(Directory* parent, string const &name) {

	// Delete the file
	parent->remove_file(name);
}