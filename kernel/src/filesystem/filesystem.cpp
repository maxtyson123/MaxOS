/**
 * @file filesystem.cpp
 * @brief Implementation of a generic filesystem interface
 *
 * @date 5th January 2023
 * @author Max Tyson
 */

#include <filesystem/filesystem.h>
#include <common/logger.h>

using namespace MaxOS;
using namespace MaxOS::filesystem;
using namespace MaxOS::common;

File::File() = default;

File::~File() = default;

/**
 * @brief write data to the file
 *
 * @param data The byte buffer to write
 * @param amount The amount of data to write
 */
void File::write(buffer_t* data, size_t amount) {
}

/**
 * @brief read data from the file
 *
 * @param data The byte buffer to read into
 * @param amount The amount of data to read
 */
void File::read(common::buffer_t* data, size_t amount) {
}

/**
 * @brief Flush the file to the disk
 */
void File::flush() {
}

/**
 * @brief Seek to a position in the file
 *
 * @param seek_type The type of seek to perform (where to seek to)
 * @param offset The amount to seek
 */
void File::seek(SeekType seek_type, size_t offset) {

	// Seek based on the type
	switch (seek_type) {
		case SeekType::SET:
			m_offset = offset;
			break;

		case SeekType::CURRENT:
			m_offset += offset;
			break;

		case SeekType::END:
			m_offset = size() - offset;
			break;
	}
}

/**
 * @brief Get where the file is currently at (amount read/write/seeked)
 *
 * @return The current position in the file
 */
uint32_t File::position() const {
	return m_offset;
}

/**
 * @brief Get the name of the file
 *
 * @return The name of the file
 */
string File::name() {
	return m_name;
}

/**
 * @brief Get the size of the file
 *
 * @return The size of the file (in bytes)
 */
size_t File::size() const {
	return m_size;
}

Directory::Directory() = default;

/**
 * @brief Destructor for Directory, frees all files and subdirectories
 */
Directory::~Directory() {

	// Free the files
	for (auto &file: m_files)
		delete file;

	// Free the subdirectories
	for (auto &subdirectory: m_subdirectories)
		delete subdirectory;

}

/**
 * @brief read the directory from the disk
 */
void Directory::read_from_disk() {

}

/**
 * @brief Get the files in the directory
 *
 * @return A list of all the files in the directory
 */
common::Vector<File*> Directory::files() {
	return m_files;
}

/**
 * @brief Open a file in the directory
 *
 * @param name The name of the file to open
 * @return
 */
File* Directory::open_file(const string &name) {

	// Try to find the file
	for (auto &file: m_files)
		if (file->name() == name)
			return file;

	// File not found
	return nullptr;
}

/**
 * @brief Create a file in the directory
 *
 * @param name The name of the file to create
 * @return A new file object or null if it could not be created
 */
File* Directory::create_file(const string &name) {
	return nullptr;
}

/**
 * @brief Delete a file in the directory
 *
 * @param name The name of the file to remove
 */
void Directory::remove_file(const string &name) {
}

/**
 * @brief Get the subdirectories in the directory
 *
 * @return The subdirectories in the directory
 */
common::Vector<Directory*> Directory::subdirectories() {
	return m_subdirectories;
}

/**
 * @brief Open a directory in the directory
 *
 * @param name The name of the directory to open
 * @return The directory object or null if it could not be opened
 */
Directory* Directory::open_subdirectory(const string &name) {

	// Try to find the directory
	for (auto &subdirectory: m_subdirectories)
		if (subdirectory->name() == name) {
			subdirectory->read_from_disk();
			return subdirectory;
		}

	// Directory not found
	return nullptr;
}

/**
 * @brief Create a directory in the directory
 *
 * @param name The name of the directory to create
 * @return The new directory object or null if it could not be created
 */
Directory* Directory::create_subdirectory(const string &name) {
	return nullptr;
}

/**
 * @brief Try to remove a directory in the directory
 * @param name The name of the directory to remove
 */
void Directory::remove_subdirectory(const string &name) {
}


/**
 * @brief Get the name of the directory
 *
 * @return The name of the directory
 */
string Directory::name() {
	return m_name;
}

/**
 * @brief Get the size of the directory
 *
 * @return The size of the directory (sum of all the files in bytes)
 */
size_t Directory::size() {

	// Sum the size of all the files
	size_t size = 0;
	for (auto &file: m_files)
		size += file->size();

	return size;

}

/**
 * @brief Rename a file in the directory
 *
 * @param file The file to rename
 * @param new_name The new name of the file
 */
void Directory::rename_file(File* file, string const &new_name) {

	rename_file(file->name(), new_name);

}

/**
 * @brief Rename a file in the directory
 *
 * @param old_name The file to rename
 * @param new_name The new name of the file
 */
void Directory::rename_file(string const &old_name, string const &new_name) {

	ASSERT(false, "not implemented");

}

/**
 * @brief Rename a subdirectory in the directory
 *
 * @param directory The directory to rename
 * @param new_name The new name of the directory
 */
void Directory::rename_subdirectory(Directory* directory, string const &new_name) {

	rename_subdirectory(directory->name(), new_name);

}

/**
 * @brief Rename a subdirectory in the directory
 *
 * @param old_name The directory to rename
 * @param new_name The new name of the directory
 */
void Directory::rename_subdirectory(string const &old_name, string const &new_name) {
	ASSERT(false, "not implemented");
}

FileSystem::FileSystem() = default;

/**
 * @brief Destructor for FileSystem, frees the root directory (Which in turn frees all files and subdirectories)
 */
FileSystem::~FileSystem() {

	// Free the root directory
	delete m_root_directory;

}

/**
 * @brief Get the directory at "/"
 *
 * @return The root directory of the filesystem
 */
Directory* FileSystem::root_directory() {
	return m_root_directory;
}

/**
 * @brief Get the directory at a given path
 *
 * @param path The path to the directory
 * @return The directory object or null if it could not be opened
 */
Directory* FileSystem::get_directory(const string &path) {

	// Check if the path is the root
	if (path == "/")
		return root_directory();

	// Recursively open the directory
	Directory* directory = root_directory();
	string directory_path = path;
	while (directory_path.length() > 0) {

		// Get the name of the directory
		string directory_name = Path::top_directory(directory_path);

		// Open the directory
		Directory* subdirectory = directory->open_subdirectory(directory_name);
		if (!subdirectory)
			return nullptr;

		// Set the new directory
		directory = subdirectory;

		// Get the path to the next directory
		directory_path = directory_path.substring(directory_name.length() + 1, directory_path.length() - directory_name.length() - 1);
	}

	return directory;
}

/**
 * @brief Check if a path exists on the filesystem
 *
 * @param path The path to check
 * @return True if the path exists, false otherwise
 */
bool FileSystem::exists(const string &path) {

	// Get the directory at the path
	string directory_path = Path::file_path(path);
	Directory* directory = get_directory(directory_path);

	// Check if the directory exists
	if (!directory)
		return false;

	// Check if the file exists
	const string file_name = Path::file_name(path);
	return directory->open_file(file_name) != nullptr;
}