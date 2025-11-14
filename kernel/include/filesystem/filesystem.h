/**
 * @file filesystem.h
 * @brief Defines a generic API for a file system, including classes for File, Directory, and FileSystem
 *
 * @date 5th January 2023
 * @author Max Tyson
 */

#ifndef MAXOS_FILESYSTEM_FILESYSTEM_H
#define MAXOS_FILESYSTEM_FILESYSTEM_H

#include <stdint.h>
#include <stddef.h>
#include <common/string.h>
#include <common/vector.h>
#include <common/buffer.h>
#include <filesystem/path.h>
#include <syscore/include/filesystem/file.h>

namespace MaxOS {

	namespace filesystem {


		typedef uint32_t lba_t;                             ///< Logical Block Addressing type
		typedef syscore::filesystem::SeekType SeekType;     ///< Seek type for file operations

		/**
		 * @class File
		 * @brief Handles file operations and information
		 */
		class File {

			protected:
				uint32_t m_offset;  ///< The current offset in the file
				string m_name;      ///< The name of the file
				size_t m_size;      ///< The size of the file

			public:
				File();
				virtual ~File();

				virtual void write(common::buffer_t* data, size_t size);
				virtual void read(common::buffer_t* data, size_t size);
				virtual void flush();

				void seek(SeekType seek_type, size_t offset);
				uint32_t position();

				size_t size();
				string name();
		};

		/**
		 * @class Directory
		 * @brief Handles a group of files (directory)
		 */
		class Directory {
			protected:
				common::Vector<File*> m_files;                  ///< The files in this directory
				common::Vector<Directory*> m_subdirectories;    ///< The subdirectories in this directory

				string m_name;                                  ///< The name of this directory

			public:
				Directory();
				virtual ~Directory();

				virtual void read_from_disk();

				common::Vector<File*> files();
				common::Vector<Directory*> subdirectories();

				File* open_file(const string &name);
				Directory* open_subdirectory(const string &name);

				virtual File* create_file(const string &name);
				virtual void remove_file(const string &name);

				void rename_file(File* file, const string &new_name);
				virtual void rename_file(const string &old_name, const string &new_name);

				void rename_subdirectory(Directory* directory, const string &new_name);
				virtual void rename_subdirectory(const string &old_name, const string &new_name);

				virtual Directory* create_subdirectory(const string &name);
				virtual void remove_subdirectory(const string &name);

				string name();
				size_t size();
		};

		/**
		 * @class FileSystem
		 * @brief Handles the disk operations and file system information
		 */
		class FileSystem {
			protected:
				Directory* m_root_directory;    ///< The fist directory in the file system (not be confused with the system root)

			public:
				FileSystem();
				virtual ~FileSystem();

				Directory* root_directory();
				Directory* get_directory(const string &path);

				bool exists(const string &path);
		};

	}

}

#endif //MAXOS_FILESYSTEM_FILESYSTEM_H
