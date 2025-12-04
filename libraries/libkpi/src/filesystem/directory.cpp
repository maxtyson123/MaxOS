//
// Created by 98max on 9/1/2025.
//

#include <filesystem/directory.h>


namespace MaxOS::KPI::filesystem {


	/**
	 * @brief Opens a directory from a path (must end in /)
	 *
	 * @param path The path to the directory
	 * @return The handle of the opened directory or 0 if it failed
	 */
	uint64_t open_directory(const char* path) {
		return resource_open(ResourceType::FILESYSTEM, path, 0);
	}

	/**
	 * @brief Rename the directory
	 *
	 * @param handle The directory to rename
	 * @param name What to rename the directory to
	 */
	void rename_directory(uint64_t handle, const char* name) {
		resource_write(handle, name, strlen(name), (size_t) DirectoryFlags::WRITE_NAME);
	}

	/**
	 * @brief Closes an open directory
	 *
	 * @param handle The directory open
	 */
	void close_directory(uint64_t handle) {
		resource_close(handle, 0);
	}

	/**
	 * @brief Gets the size required to hold the list of directory entries
	 *
	 * @param handle The directory to list
	 * @return The size of the total entries
	 */
	size_t directory_entries_size(uint64_t handle) {
		return resource_read(handle, nullptr, 0, (size_t) DirectoryFlags::READ_ENTRIES_SIZE);
	}

	/**
	 * @brief Reads the contents of a directory (as a list of entry_information_t)
	 *
	 * @param handle The directory to read
	 * @param buffer Where to read the list
	 * @param size How big is the buffer
	 */
	void directory_entries(uint64_t handle, void* buffer, size_t size) {

		resource_read(handle, buffer, size, (size_t) DirectoryFlags::READ_ENTRIES);
	}

	/**
	 * @brief Creates a new file in the directory
	 *
	 * @param handle The directory to create the file in
	 * @param name The name of the new file
	 */
	void new_file(uint64_t handle, const char* name) {

		resource_write(handle, name, strlen(name), (size_t) DirectoryFlags::WRITE_NEW_FILE);
	}

	/**
	 * @brief Creates a new subdirectory in the directory
	 *
	 * @param handle The directory to create the subdirectory in
	 * @param name The name of the new directory
	 */
	void new_directory(uint64_t handle, const char* name) {

		resource_write(handle, name, strlen(name), (size_t) DirectoryFlags::WRITE_NEW_DIR);
	}

	/**
	 * @brief Removes a file from the directory
	 *
	 * @param handle The directory to remove the file from
	 * @param name The name of the file to remove
	 */
	void remove_file(uint64_t handle, const char* name) {

		resource_write(handle, name, strlen(name), (size_t) DirectoryFlags::WRITE_REMOVE_FILE);
	}

	/**
	 * @brief Removes a subdirectory from the directory
	 *
	 * @param handle The directory to remove the subdirectory from
	 * @param name The name of the subdirectory to remove
	 */
	void remove_directory(uint64_t handle, const char* name) {

		resource_write(handle, name, strlen(name), (size_t) DirectoryFlags::WRITE_REMOVE_DIR);
	}

}
