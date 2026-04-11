/**
 * @file vfsresource.cpp
 * @brief Implementation of virtual file system resources for files and directories
 *
 * @date 1st September 2025
 * @author Max Tyson
 */

#include <vfsresource.h>

using namespace MaxOS;
using namespace FileServer;
using namespace MaxOS::common;
using namespace MaxOS::KPI;
using namespace LibFS;

/**
 * @brief Construct a new File Resource object
 *
 * @param name The name of the resource
 * @param flags The flags for the resource when opened
 * @param type The type of the resource
 */
FileResource::FileResource(string const& name, size_t flags, ResourceType type)
: Resource(name, flags, type),
  file(nullptr)	// Initialised by the registry
{

}

FileResource::~FileResource() = default;

/**
 * @brief Read from a file resource
 *
 * @param buffer The buffer to read into
 * @param size The number of bytes to read
 * @param flags The flags to pass to the reading
 * @return The number of bytes successfully read or -1 on error
 */
int FileResource::read(void* buffer, size_t size, size_t flags) {

	// File not found
	if(!file)
		return -1;

	// Handle the operation
	switch ((FileFlags)flags) {

		case FileFlags::DEFAULT:{

			buffer_t file_buffer(buffer, size);
			file->read(&file_buffer, size);
			break;
		}

		case FileFlags::READ_SIZE:{
			return file->size();
		}

		case FileFlags::READ_OFFSET:{

			return file->position();
		}

		default:
			return -1;
	}
	return size;
}

/**
 * @brief write to a file resource
 *
 * @param buffer The buffer to write from
 * @param size The number of bytes to write
 * @param flags The flags to pass to the writing
 * @return The number of bytes successfully written or -1 on error
 *
 * @warning may cause two resources, current one with old path and duplicate one with new path, as the Bridge will still
 * store the resource name as the old name. Due to the cache they will point to the same opened file object and in theory
 * not cause issues with each other
 *
 * @todo May want to fix the out of sync of renaming a resource
 */
int FileResource::write(void const* buffer, size_t size, size_t flags) {

	// File not found
	if(!file)
		return -1;

	// Handle the operation
	switch ((FileFlags)flags) {
		case FileFlags::DEFAULT:{

			buffer_t file_buffer((void*)buffer, size);
			file->write(&file_buffer, size);
			break;
		}

		case FileFlags::WRITE_SEEK_SET:
			file->seek(SeekType::SET, size);
			break;

		case FileFlags::WRITE_SEEK_CUR:
			file->seek(SeekType::CURRENT, size);
			break;

		case FileFlags::WRITE_SEEK_END:
			file->seek(SeekType::END, size);
			break;

		case FileFlags::WRITE_NAME:{

			// Open the parent
			auto parent_directory = VirtualFileSystem::current_file_system()->open_directory(Path::parent_directory(name()));

			// Rename in the parent
			string new_name = string((uint8_t*)buffer, size);
			parent_directory->rename_file(file, new_name);

			// Update name references
			new_name = Path::join_path(Path::parent_directory(name()), new_name);
			VirtualFileSystem::current_file_system()->update_cache(name(), new_name);
			// rename_resource();

			break;
		}

		default:
			return -1;

	}

	return size;
}

/**
 * @brief Construct a new Directory Resource object
 *
 * @param name The name of the resource
 * @param flags The flags for the resource when opened
 * @param type The type of the resource
 */
DirectoryResource::DirectoryResource(string const& name, size_t flags, ResourceType type)
: Resource(name, flags, type),
  directory(nullptr)
{

}

DirectoryResource::~DirectoryResource() = default;

/**
 * @brief Copies all the entries in this directory into a buffer
 *
 * @param buffer The buffer to copy into
 * @param size The size of the buffer
 */
void DirectoryResource::write_entries(void const* buffer, size_t size) const {

	size_t amount_written = 0;

	entry_information_t* entry = nullptr;
	size_t entry_capacity = 0;

	auto write_single_entry = [&](const string& name, size_t entry_size, bool is_file) {

		// Make sure there is enough space
		size_t required_size = sizeof(entry_information_t) + name.length() + 1;
		if (required_size > entry_capacity) {
			delete[] (uint8_t*)entry;
			entry = (entry_information_t*)(new uint8_t[required_size]);
			entry_capacity = required_size;
		}

		// Create the entry
		entry->is_file = is_file;
		entry->size = entry_size;
		entry->entry_length = required_size;
		memcpy(entry->name, name.c_str(), name.length());
		entry->name[name.length()] = '\0';

		// Not enough space
		if (amount_written + entry->entry_length > size)
			return false;

		// Copy the entry
		memcpy((uint8_t*)buffer + amount_written, entry, entry->entry_length);
		amount_written += entry->entry_length;
		return true;
	};

	// Write files
	for (const auto& file : directory->files()) {
		if (!write_single_entry(file->name(), file->size(), true))
			break;
	}

	// Write directories
	for (const auto& dir : directory->subdirectories()) {
		if (!write_single_entry(dir->name(), dir->size(), false))
			break;
	}

	delete[] (uint8_t*)entry;
}

/**
 * @brief Gets the size required to store all the entries
 *
 * @return The total size
 */
size_t DirectoryResource::entries_size() const {

	size_t size = 0;

	// Files
	for (const auto& file : directory->files()) {
		size_t entry_size = sizeof(entry_information_t) + file->name().length() + 1;
		size += entry_size;
	}

	// Subdirectories
	for (const auto& dir : directory->subdirectories()) {
		size_t entry_size = sizeof(entry_information_t) + dir->name().length() + 1;
		size += entry_size;
	}

	return size;
}

/**
 * @brief read from a directory resource
 *
 * @param buffer The buffer to read into
 * @param size The number of bytes to read
 * @param flags The flags to pass to the reading
 * @return The number of bytes successfully read or -1 on error
 */
int DirectoryResource::read(void* buffer, size_t size, size_t flags) {

	// Directory not found
	if(!directory)
		return -1;

	switch ((DirectoryFlags)flags){
		case DirectoryFlags::READ_ENTRIES:{
			write_entries(buffer, size);
			break;
		}

		case DirectoryFlags::READ_ENTRIES_SIZE:{
			return entries_size();
		}

		default:
			return -1;
	}

	return size;

}

/**
 * @brief write to a directory resource
 *
 * @param buffer The buffer to write from
 * @param size The number of bytes to write
 * @param flags The flags to pass to the writing
 * @return The number of bytes successfully written or -1 on error
 */
int DirectoryResource::write(void const* buffer, size_t size, size_t flags) {

	// Directory not found
	if(!directory)
		return -1;

	switch ((DirectoryFlags)flags){

		case DirectoryFlags::WRITE_NAME : {

			// Open the parent
			auto parent_directory = VirtualFileSystem::current_file_system()->open_directory(Path::parent_directory(name()));

			// Rename
			string new_name = string((uint8_t*)buffer, size);
			parent_directory->rename_subdirectory(directory, new_name);

			// Update name references
			new_name = Path::join_path(Path::parent_directory(name()), new_name);
			VirtualFileSystem::current_file_system()->update_cache(name(), new_name);
			// rename_resource(); - will also need to rename entries below this

			break;
		}

		case DirectoryFlags::WRITE_NEW_FILE:{

			string new_name = string((uint8_t*)buffer, size);
			directory->create_file(new_name);

			break;
		}


		case DirectoryFlags::WRITE_NEW_DIR:{

			string new_name = string((uint8_t*)buffer, size);
			directory->create_subdirectory(new_name);
			break;
		}

		case DirectoryFlags::WRITE_REMOVE_FILE:{

			string new_name = string((uint8_t*)buffer, size);
			directory->remove_file(new_name);
			break;
		}
		case DirectoryFlags::WRITE_REMOVE_DIR:{

			string new_name = string((uint8_t*)buffer, size);
			directory->remove_subdirectory(new_name);
			break;
		}

		default:
			return -1;
	}

	return size;
}

/**
 * @brief Construct a new VFS Resource Registry object
 *
 * @param vfs The virtual file system to use
 */
VFSResourceServer::VFSResourceServer(VirtualFileSystem* vfs)
: ResourceServer("vfs", (size_t)ResourceType::FILESYSTEM),
  m_vfs(vfs)
{

}

VFSResourceServer::~VFSResourceServer() = default;

/**
 * @brief Open a directory as a resource
 *
 * @param name The name to call the resource
 * @param directory The directory to open
 * @return The new resource or nullptr if it failed to open
 */
Resource* VFSResourceServer::open_as_resource(const string& name, Directory* directory) {

	// Doesnt exist
	if(!directory)
		return nullptr;

	// Create the resource
	auto resource = new DirectoryResource(name, 0, ResourceType::FILESYSTEM);
	resource->directory = directory;

	return resource;
}

/**
 * @brief Open a file as a resource
 *
 * @param name The name to call the resource
 * @param file The file to open
 * @return The new resource or nullptr if it failed to open
 */
Resource* VFSResourceServer::open_as_resource(string const& name, File* file) {

	// Doesnt exist
	if(!file)
		return nullptr;

	// Create the resource
	auto resource = new FileResource(name, 0, ResourceType::FILESYSTEM);
	resource->file = file;

	return resource;
}


Resource* VFSResourceServer::get_resource(string const& name) {

	// Get the path from root
	string path = process_relative_path(current_processed_message()->sending_pid, name);

	// Open the resource
	if(Path::is_file(path))
		return open_as_resource(path, m_vfs->open_file(path));


	return open_as_resource(path, m_vfs->open_directory(path));
}

Resource* VFSResourceServer::create_resource(string const& name, size_t flags, uintptr_t data) {

	// Get the path from root
	string path = process_relative_path(current_processed_message()->sending_pid, name);

	// Open the resource
	if(Path::is_file(path))
		return open_as_resource(path, m_vfs->create_file(path));

	return open_as_resource(path, m_vfs->create_directory(path));
}

/**
 * @brief Change the directory in which a process's file operations are relative to
 *
 * @param pid The process of which the working directory needs to change
 * @param dir The working directory for the process
 */
void VFSResourceServer::change_working_dir(uint64_t pid, string dir) {

	// Insert also updates
	m_working_directories.insert(pid, process_relative_path(pid, dir));

}

/**
 * @brief Get the path from root relative to the processes working directory. If the process has not set its working directory it will be assumed root.
 *
 * @param pid The process to use the working directory for
 * @param relative_path The path to compute relative to the process.
 * @return The process working directory + the path.  If the path is already from root it will be returned the same
 */
string VFSResourceServer::process_relative_path(uint64_t pid, const string& relative_path) {

	// Simplify the path
	string path = Path::absolute_path(relative_path);
	string working_path = "/";

	// Try get the working directory
	auto working_directory = m_working_directories.find(pid);
	if (working_directory != m_working_directories.end())
		working_path = working_directory->second;

	// Combine
	return Path::join_path(working_path, path);
}
