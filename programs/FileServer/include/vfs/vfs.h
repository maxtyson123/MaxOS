/**
 * @file vfs.h
 * @brief Defines a Virtual File System (VFS) class for managing multiple filesystems and providing a unified interface
 * for file and directory operations.
 *
 * @date 20th April 2025
 * @author Max Tyson
 */

#ifndef FILESERVER_VFS_H
#define FILESERVER_VFS_H

#include <libcommon/map.h>
#include <libcommon/pair.h>

#include <filesystem/filesystem.h>


namespace FileServer::vfs {

	typedef MaxOS::string string;
	typedef filesystem::File File;
	typedef filesystem::Directory Directory;
	typedef filesystem::FileSystem FileSystem;

	/**
	 * @class VirtualFileSystem
	 * @brief Combines all the filesystems across the partitions on each disk into a single filesystems and exposes
	 * a single API to interact with them
	 *
	 * @todo mulithread safe cache
	 */
	class VirtualFileSystem {

		private:
			MaxOS::common::Map<FileSystem*, string> filesystems;
			inline static VirtualFileSystem* s_current_file_system = nullptr;

			MaxOS::common::Map<string, Directory*>	m_directory_cache;
			MaxOS::common::Map<string, File*>		m_file_cache;

		public:
			VirtualFileSystem();
			~VirtualFileSystem();

			static VirtualFileSystem* current_file_system();

			void mount_filesystem(FileSystem* filesystem);
			void mount_filesystem(FileSystem* filesystem, const string& mount_point);
			void unmount_filesystem(FileSystem* filesystem);
			void unmount_filesystem(const string& mount_point);
			void unmount_all();

			Directory* root_directory();
			FileSystem* root_filesystem();

			FileSystem* get_filesystem(const string& mount_point);
			FileSystem* find_filesystem(string path);
			string get_relative_path(FileSystem* filesystem, string path);

			Directory* open_directory(const string& path);
			static Directory* open_directory(Directory* parent, const string& name);

			Directory* create_directory(string path);
			static Directory* create_directory(Directory* parent, const string& name);

			void delete_directory(string path);
			static void delete_directory(Directory* parent, const string& name);
			static void delete_directory(Directory* parent, Directory* directory);

			File* create_file(const string& path);
			static File* create_file(Directory* parent, const string& name);

			File* open_file(const string& path, size_t offset = 0);
			static File* open_file(Directory* parent, const string& name, size_t offset = 0);

			void delete_file(const string& path);
			static void delete_file(Directory* parent, const string& name);

		    void update_cache(string old_path, string new_path);
			void remove_cache(string old_path);

	};
}


#endif //FILESERVER_VFS_H
