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

            private:
                uint32_t m_offset;
                string m_name;

            public:
                File();
                ~File();

                virtual void write(const uint8_t* data, size_t size);
                virtual void read(uint8_t* data, size_t size);
                virtual void flush();

                virtual void seek(SeekType seek_type, size_t offset);
                virtual uint32_t position();

                virtual size_t size();
                virtual string name();
        };

        /**
         * @class Directory
         * @brief Handles a group of files (directory)
         */
        class Directory
        {
            private:
                common::Vector<File*> m_files;
                common::Vector<Directory*> m_subdirectories;

                string m_name;

            public:
                Directory();
                ~Directory();

                virtual File* create_file(const string& name);
                virtual File* open_file(const string& name);
                virtual void remove_file(const string& name);
                common::Vector<File*> files();

                virtual Directory* create_subdirectory(const string& name);
                virtual Directory* open_subdirectory(const string& name);
                virtual void remove_subdirectory(const string& name);
                common::Vector<Directory*> subdirectories();

                virtual string name();
                virtual size_t size();
        };

        /**
         * @class FileSystem
         * @brief Handles the disk operations and file system information
         */
        class FileSystem
        {

            public:
                FileSystem();
                ~FileSystem();

                virtual Directory* root_directory();
                Directory* get_directory(const string& path);

                bool exists(const string& path);
        };

    }

}

#endif //MAXOS_FILESYSTEM_FILESYSTEM_H
