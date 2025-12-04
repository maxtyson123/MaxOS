//
// Created by 98max on 9/1/2025.
//

#include <filesystem/file.h>


namespace MaxOS::KPI::filesystem {

	uint64_t open_file(const char* path) {
		return resource_open(ResourceType::FILESYSTEM, path, 0);
	}

	void close_file(uint64_t handle) {
		resource_close(handle, 0);
	}

	size_t file_size(uint64_t handle) {
		return resource_read(handle, nullptr, 0, (size_t) FileFlags::READ_SIZE);
	}

	size_t file_offset(uint64_t handle) {
		return resource_read(handle, nullptr, 0, (size_t) FileFlags::READ_OFFSET);
	}

	size_t file_read(uint64_t handle, void* buffer, size_t size) {
		return resource_read(handle, buffer, size, (size_t) FileFlags::DEFAULT);
	}

	size_t file_write(uint64_t handle, void* buffer, size_t size) {
		return resource_write(handle, buffer, size, (size_t) FileFlags::DEFAULT);
	}

	void rename_file(uint64_t handle, const char* name) {

		resource_write(handle, name, strlen(name), (size_t) FileFlags::WRITE_NAME);
	}

	void seek_file(uint64_t handle, size_t position, SeekType seek_type) {

		switch(seek_type) {

			case SeekType::SET:
				resource_write(handle, nullptr, position, (size_t) FileFlags::WRITE_SEEK_SET);
				break;

			case SeekType::CURRENT:
				resource_write(handle, nullptr, position, (size_t) FileFlags::WRITE_SEEK_CUR);
				break;

			case SeekType::END:
				resource_write(handle, nullptr, position, (size_t) FileFlags::WRITE_SEEK_END);
				break;

		}
	}

}
