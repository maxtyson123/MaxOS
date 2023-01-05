//
// Created by 98max on 5/01/2023.
//

#ifndef MAXOS_FILESYSTEM_FILESYSTEM_H
#define MAXOS_FILESYSTEM_FILESYSTEM_H

#include <common/types.h>


namespace maxOS{

    namespace filesystem{


        class FileWriter {

            public:
                FileWriter();
                ~FileWriter();

                virtual bool Write(common::uint8_t *data, common::uint32_t size);
                virtual bool Seek(common::uint32_t position);
                virtual bool Close();
                virtual bool Flush();

                common::uint32_t GetPosition();
                common::uint32_t GetFileSize();
        };

        class FileReader{

            public:
                FileReader();
                ~FileReader();

                virtual void Read(common::uint8_t* data, common::uint32_t size);
                virtual bool Seek(common::uint32_t position);

                common::uint32_t GetPosition();
                common::uint32_t GetFileSize();

        };

        class FileEnumerator{

        public:
            FileEnumerator();
            ~FileEnumerator();

            virtual void getFileName();
            virtual FileReader getReader();
            virtual FileWriter getWriter();
            virtual FileEnumerator next();

        };

        class DirectoryEnumerator{

        public:
            DirectoryEnumerator();
            ~DirectoryEnumerator();

            virtual void getDirectoryName();
            virtual DirectoryEnumerator next();

        };

        class DirectoryTraverser{
            public:

                DirectoryTraverser();
                ~DirectoryTraverser();

                virtual bool hasNext();
                virtual DirectoryTraverser next();
                virtual void changeDirectory();
                virtual void makeDirectory();

                virtual FileEnumerator getFileEnumerator();
                virtual DirectoryEnumerator getDirectoryEnumerator();

        };

        class FileSystem{

            public:
                FileSystem();
                ~FileSystem();

                virtual DirectoryTraverser getDirectoryTraverser();
        };

    }

}

#endif //MAXOS_FILESYSTEM_FILESYSTEM_H
