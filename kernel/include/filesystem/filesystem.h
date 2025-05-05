//
// Created by 98max on 5/01/2023.
//

#ifndef MAXOS_FILESYSTEM_FILESYSTEM_H
#define MAXOS_FILESYSTEM_FILESYSTEM_H

#include <stdint.h>
#include <stddef.h>
#include <common/string.h>
#include <common/vector.h>

namespace MaxOS{

    namespace filesystem{


        enum class SeekType{
            SET,
            CURRENT,
            END
        };

        /**
         * @class Path
         * @brief Handles file & directory paths
         */
        class Path
        {
            public:
                static bool   vaild(string path);
                static string file_name(string path);
                static string file_extension(string path);
                static string file_path(string path);

        };

        /**
         * @class File
         * @brief Handles file operations and information
         */
        class File
        {

            protected:
                uint32_t m_offset;
                string m_name;
                size_t m_size;

            public:
                File();
                virtual ~File();

                virtual void write(const uint8_t* data, size_t size);
                virtual void read(uint8_t* data, size_t size);
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
        class Directory
        {
            protected:
                common::Vector<File*> m_files;
                common::Vector<Directory*> m_subdirectories;

                string m_name;

            public:
                Directory();
                virtual ~Directory();

                virtual void read_from_disk();

                common::Vector<File*> files();
                File* open_file(const string& name);
                virtual File* create_file(const string& name);
                virtual void remove_file(const string& name);

                common::Vector<Directory*> subdirectories();
                Directory* open_subdirectory(const string& name);
                virtual Directory* create_subdirectory(const string& name);
                virtual void remove_subdirectory(const string& name);

                string name();
                size_t size();

                void debug_print(int level = 0);
        };

        /**
         * @class FileSystem
         * @brief Handles the disk operations and file system information
         */
        class FileSystem
        {
            protected:
                Directory* m_root_directory;

            public:
                FileSystem();
                virtual ~FileSystem();

                Directory* root_directory();
                Directory* get_directory(const string& path);

                bool exists(const string& path);
        };

    }

}

#endif //MAXOS_FILESYSTEM_FILESYSTEM_H
