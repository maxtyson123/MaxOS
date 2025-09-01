//
// Created by 98max on 5/01/2023.
//

#ifndef MAXOS_FILESYSTEM_FILESYSTEM_H
#define MAXOS_FILESYSTEM_FILESYSTEM_H

#include <stdint.h>
#include <stddef.h>
#include <common/string.h>
#include <common/vector.h>
#include <common/buffer.h>
#include <syscore/include/filesystem/file.h>

namespace MaxOS{

    namespace filesystem{

		// Easier to read
	    typedef uint32_t lba_t;
		typedef syscore::filesystem::SeekType SeekType;

        /**
         * @class Path
         * @brief Handles file & directory paths
         */
        class Path
        {
            public:
                static bool   valid(string path);
				static bool   is_file(const string& path);

                static string file_name(string path);
                static string file_extension(string path);
                static string file_path(string path);

                static string top_directory(string path);
				static string parent_directory(string path);

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

                virtual void write(const common::buffer_t* data, size_t size);
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
                common::Vector<Directory*> subdirectories();

                File* open_file(const string& name);
                Directory* open_subdirectory(const string& name);

                virtual File* create_file(const string& name);
                virtual void remove_file(const string& name);

                void rename_file(File* file, const string& new_name);
                virtual void rename_file(const string& old_name, const string& new_name);

                void rename_subdirectory(Directory* directory, const string& new_name);
                virtual void rename_subdirectory(const string& old_name, const string& new_name);

                virtual Directory* create_subdirectory(const string& name);
                virtual void remove_subdirectory(const string& name);

                string name();
                size_t size();
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
