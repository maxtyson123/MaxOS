//
// Created by 98max on 9/1/2025.
//
#include <filesystem/vfsresource.h>

using namespace MaxOS;
using namespace MaxOS::filesystem;
using namespace MaxOS::processes;
using namespace MaxOS::common;
using namespace syscore::filesystem;

FileResource::FileResource(string const& name, size_t flags, processes::resource_type_t type)
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
 * @brief Write to a file resource
 *
 * @param buffer The buffer to write from
 * @param size The number of bytes to write
 * @param flags The flags to pass to the writing
 * @return The number of bytes successfully written or -1 on error
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
			Resource* parent = GlobalResourceRegistry::get_registry(resource_type_t::FILESYSTEM)->get_resource(Path::parent_directory(name()));
			auto parent_directory = ((DirectoryResource*)parent)->directory;

			// Rename
			string new_name = string((uint8_t*)buffer, size);
			parent_directory->rename_file(file, new_name);
			break;
		}

		default:
			return -1;

	}

	return size;
}

DirectoryResource::DirectoryResource(string const& name, size_t flags, processes::resource_type_t type)
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
 * @brief Read from a directory resource
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
 * @brief Write to a directory resource
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

	auto registry = GlobalResourceRegistry::get_registry(resource_type_t::FILESYSTEM);

	switch ((DirectoryFlags)flags){

		case DirectoryFlags::WRITE_NAME : {

			// Open the parent
			Resource* parent = registry->get_resource(Path::parent_directory(name()));
			auto parent_directory = ((DirectoryResource*)parent)->directory;

			// Rename
			string new_name = string((uint8_t*)buffer, size);
			parent_directory->rename_subdirectory(directory, new_name);
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

VFSResourceRegistry::VFSResourceRegistry(VirtualFileSystem* vfs)
: BaseResourceRegistry(resource_type_t::FILESYSTEM),
  m_vfs(vfs)
{

}

VFSResourceRegistry::~VFSResourceRegistry() = default;

/**
 * @brief Open a directory as a resource
 *
 * @param name The name to call the resource
 * @param directory The directory to open
 * @return The new resource or nullptr if it failed to open
 */
Resource* VFSResourceRegistry::open_as_resource(const string& name, Directory* directory) {

	// Doesnt exist
	if(!directory)
		return nullptr;

	// Create the resource
	auto resource = new DirectoryResource(name, 0, resource_type_t::FILESYSTEM);
	resource->directory = directory;

	register_resource(resource);
	return resource;
}

/**
 * @brief Open a file as a resource
 *
 * @param name The name to call the resource
 * @param file The file to open
 * @return The new resource or nullptr if it failed to open
 */
Resource* VFSResourceRegistry::open_as_resource(string const& name, File* file) {

	// Doesnt exist
	if(!file)
		return nullptr;

	// Create the resource
	auto resource = new FileResource(name, 0, resource_type_t::FILESYSTEM);
	resource->file = file;

	register_resource(resource);
	return resource;
}


Resource* VFSResourceRegistry::get_resource(string const& name) {

	// Resource already opened
	auto resource = BaseResourceRegistry::get_resource(name);
	if(resource != nullptr)
		return resource;

	// Open the resource
	bool is_file = Path::is_file(name);
	if(is_file)
		return open_as_resource(name, m_vfs->open_file(name));
	else
		return open_as_resource(name, m_vfs->open_directory(name));
}

Resource* VFSResourceRegistry::create_resource(string const& name, size_t flags) {

	// Resource already opened
	auto resource = BaseResourceRegistry::get_resource(name);
	if(resource != nullptr)
		return resource;

	// Open the resource
	bool is_file = Path::is_file(name);
	if(is_file)
		return open_as_resource(name, m_vfs->create_file(name));
	else
		return open_as_resource(name, m_vfs->create_directory(name));

}