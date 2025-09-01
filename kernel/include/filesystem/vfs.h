//
// Created by Max Tyson on 20/04/2025.
//

#ifndef MAXOS_FILESYSTEM_VFS_H
#define MAXOS_FILESYSTEM_VFS_H

#include <common/map.h>
#include <common/pair.h>
#include <filesystem/filesystem.h>

namespace MaxOS{

  namespace filesystem{

      class VirtualFileSystem{

        private:
          common::Map<FileSystem*, string> filesystems;
          inline static VirtualFileSystem* s_current_file_system = nullptr;

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
      };
  }
}

#endif //MAXOS_FILESYSTEM_VFS_H
