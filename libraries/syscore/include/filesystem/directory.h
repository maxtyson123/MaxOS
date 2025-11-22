//
// Created by 98max on 9/1/2025.
//

#ifndef SYSCORE_FILESYSTEM_DIRECTORY_H
#define SYSCORE_FILESYSTEM_DIRECTORY_H

#include <cstdint>
#include <cstddef>
#include <common.h>
#include <syscalls.h>


namespace syscore {
	namespace filesystem {

		enum class DirectoryFlags {
			READ_ENTRIES,
			READ_ENTRIES_SIZE,
			WRITE_NAME,
			WRITE_NEW_FILE,
			WRITE_NEW_DIR,
			WRITE_REMOVE_FILE,
			WRITE_REMOVE_DIR
		};

		typedef struct EntryInformation {
			size_t entry_length;
			size_t size;
			bool is_file;
			char name[];
		} entry_information_t;

		uint64_t open_directory(const char* path);
		void rename_directory(uint64_t handle, const char* name);
		void close_directory(uint64_t handle);

		size_t directory_entries_size(uint64_t handle);
		void directory_entries(uint64_t handle, void* buffer, size_t size);

		void new_file(uint64_t handle, const char* name);
		void new_directory(uint64_t handle, const char* name);

		void remove_file(uint64_t handle, const char* name);
		void remove_directory(uint64_t handle, const char* name);

	}
}

#endif //SYSCORE_FILESYSTEM_DIRECTORY_H
