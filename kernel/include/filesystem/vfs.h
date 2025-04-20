//
// Created by Max Tyson on 20/04/2025.
//

#ifndef MAXOS_FILESYSTEM_VFS_H
#define MAXOS_FILESYSTEM_VFS_H

#include <common/map.h>
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

          VirtualFileSystem* current_file_system();

          void mount_filesystem(FileSystem* filesystem);
          void mount_filesystem(FileSystem* filesystem, string mount_point);
          void unmount_filesystem(FileSystem* filesystem);
          void unmount_filesystem(string mount_point);
          void unmount_all();

          FileSystem* root_filesystem();
          FileSystem* get_filesystem(string mount_point);
          FileSystem* find_filesystem(string path);
          string get_relative_path(FileSystem* filesystem, string path);

          Directory* root_directory();
          Directory* open_directory(string path);
          Directory* create_directory(string path);
          void delete_directory(string path);

          File* create_file(string path);
          File* open_file(string path);
          File* open_file(string path, size_t offset);
          void delete_file(string path);
      };
  }
}

#endif //MAXOS_FILESYSTEM_VFS_H
