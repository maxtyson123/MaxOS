//
// Created by 98max on 5/01/2023.
//

#ifndef MAXOS_FILESYSTEM_FILESYSTEM_H
#define MAXOS_FILESYSTEM_FILESYSTEM_H

#include <common/types.h>


namespace maxOS{

    namespace filesystem{


        enum SeekType{
            SEEK_SET,
            SEEK_CUR,
            SEEK_END
        };

        class FileWriter {

            public:
                FileWriter();
                ~FileWriter();

                virtual common::uint32_t Write(common::uint8_t *data, common::uint32_t size);
                virtual common::uint32_t Seek(common::uint32_t position, SeekType seek);

                virtual bool Close();
                virtual bool Flush();

                virtual common::uint32_t GetPosition();
                virtual common::uint32_t GetFileSize();
        };

        class FileReader{

            public:
                FileReader();
                ~FileReader();

                virtual common::uint32_t Read(common::uint8_t* data, common::uint32_t size);
                virtual common::uint32_t Seek(common::uint32_t position, SeekType seek);

                virtual common::uint32_t GetPosition();
                virtual common::uint32_t GetFileSize();

        };

        class FileEnumerator{

        public:
            FileEnumerator();
            ~FileEnumerator();

            virtual char* getFileName();
            virtual FileReader* getReader();
            virtual FileWriter* getWriter();

            virtual bool hasNext();
            virtual FileEnumerator* next();

        };

        class DirectoryEnumerator{

        public:
            DirectoryEnumerator();
            ~DirectoryEnumerator();

            virtual char* getDirectoryName();

            virtual bool hasNext();
            virtual DirectoryEnumerator* next();

        };

        class DirectoryTraverser{
            public:

                DirectoryTraverser();
                ~DirectoryTraverser();

                virtual void changeDirectory(DirectoryEnumerator directory);

                virtual void makeDirectory(char* name);
                virtual void removeDirectory(char* name);

                virtual void makeFile(char* name);
                virtual void removeFile(char* name);

                virtual FileEnumerator* getFileEnumerator();
                virtual DirectoryEnumerator* getDirectoryEnumerator();

        };

        class FileSystem{

            public:
                FileSystem();
                ~FileSystem();

                virtual DirectoryTraverser* getDirectoryTraverser();
        };

    }

}

#endif //MAXOS_FILESYSTEM_FILESYSTEM_H
