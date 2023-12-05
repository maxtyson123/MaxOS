//
// Created by 98max on 5/01/2023.
//

#ifndef MAXOS_FILESYSTEM_FILESYSTEM_H
#define MAXOS_FILESYSTEM_FILESYSTEM_H

#include <stdint.h>


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

                virtual uint32_t Write(uint8_t *data, uint32_t size);
                virtual uint32_t Seek(uint32_t position, SeekType seek);

                virtual bool Close();
                virtual bool Flush();

                virtual uint32_t GetPosition();
                virtual uint32_t GetFileSize();
        };

        class FileReader{

            public:
                FileReader();
                ~FileReader();

                virtual uint32_t Read(uint8_t* data, uint32_t size);
                virtual uint32_t Seek(uint32_t position, SeekType seek);

                virtual uint32_t GetPosition();
                virtual uint32_t GetFileSize();

        };

        class FileEnumerator{

        public:
            FileEnumerator();
            ~FileEnumerator();

            virtual string getFileName();
            virtual FileReader* getReader();
            virtual FileWriter* getWriter();

            virtual bool hasNext();
            virtual FileEnumerator* next();

        };

        class DirectoryEnumerator{

        public:
            DirectoryEnumerator();
            ~DirectoryEnumerator();

            virtual string getDirectoryName();

            virtual bool hasNext();
            virtual DirectoryEnumerator* next();

        };

        class DirectoryTraverser{
            public:

                DirectoryTraverser();
                ~DirectoryTraverser();

                virtual void changeDirectory(DirectoryEnumerator directory);

                virtual void makeDirectory(string name);
                virtual void removeDirectory(string name);

                virtual void makeFile(string name);
                virtual void removeFile(string name);

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
