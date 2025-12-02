//
// Created by 98max on 9/1/2025.
//


#ifndef SYSCORE_FILESYSTEM_FILE_H
#define SYSCORE_FILESYSTEM_FILE_H

#include <cstdint>
#include <cstddef>
#include <common.h>
#include <syscalls.h>


namespace syscore::filesystem {

	enum class FileFlags {
		DEFAULT,
		READ_SIZE,
		READ_OFFSET,
		WRITE_SEEK_SET,
		WRITE_SEEK_CUR,
		WRITE_SEEK_END,
		WRITE_NAME,
	};

	enum class SeekType {
		SET,
		CURRENT,
		END
	};

	uint64_t open_file(const char* path);
	void close_file(uint64_t handle);

	size_t file_size(uint64_t handle);
	size_t file_offset(uint64_t handle);

	size_t file_read(uint64_t handle, void* buffer, size_t size);
	size_t file_write(uint64_t handle, void* buffer, size_t size);

	void rename_file(uint64_t handle, const char* name);
	void seek_file(uint64_t handle, size_t position, SeekType seek_type);
}


#endif //SYSCORE_FILESYSTEM_FILE_H
