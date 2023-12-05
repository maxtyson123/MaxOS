//
// Created by 98max on 1/01/2023.
//

#ifndef MAXOS_FILESYSTEM_FAT32_H
#define MAXOS_FILESYSTEM_FAT32_H

#include <stdint.h>
#include <drivers/ata.h>
#include <filesystem/filesystem.h>
#include <common/vector.h>
#include <memory/memorymanagement.h>
#include <common/outputStream.h>

namespace maxOS{

    namespace filesystem{


        struct BiosParameterBlock32{

            uint8_t    jump[3];
            uint8_t    OEMName[8];
            uint16_t   bytesPerSector;
            uint8_t    sectorsPerCluster;
            uint16_t   reservedSectors;
            uint8_t    tableCopies;
            uint16_t   rootEntries;
            uint16_t   totalSectors16;
            uint8_t    mediaType;
            uint16_t   fatSectorCount;
            uint16_t   sectorsPerTrack;
            uint16_t   headCount;
            uint32_t   hiddenSectors;
            uint32_t   totalSectors32;

            uint32_t   tableSize32;
            uint16_t   extendedFlags;
            uint16_t   fatVersion;
            uint32_t   rootCluster;
            uint16_t   fatInfo;
            uint16_t   backupSector;
            uint8_t    reserved0[12];
            uint8_t    driveNumber;
            uint8_t    reserved1;
            uint8_t    bootSignature;
            uint32_t   volumeID;
            uint8_t    volumeLabel[11];
            uint8_t    fileSystemType[8];

        } __attribute__((packed));

        struct DirectoryEntry{

            uint8_t    name[8];
            uint8_t    extension[3];
            uint8_t    attributes;
            uint8_t    reserved;

            uint8_t    creationTimeTenth;
            uint16_t   creationTime;
            uint16_t   creationDate;
            uint16_t   lastAccessDate;

            uint16_t   firstClusterHigh;

            uint16_t  lastWriteTime;
            uint16_t  lastWriteDate;

            uint16_t  firstClusterLow;

            uint32_t  size;

        } __attribute__((packed));


        //A struct to hold the extra information about a directory entry
        struct DirectoryEntryExtras{
             char longFileName[256];
        };

        class FatDirectoryTraverser;

        class FatFileWriter : public FileWriter {
                DirectoryEntry* fileInfo;
                FatDirectoryTraverser* traverser;
                uint32_t offsetPosition;
            public:
                FatFileWriter(FatDirectoryTraverser* parent, DirectoryEntry file);
                ~FatFileWriter();

                uint32_t Write(uint8_t *data, uint32_t size);
                uint32_t Seek(uint32_t position, SeekType seek);

                bool Close();
                bool Flush();

                uint32_t GetPosition();
                uint32_t GetFileSize();
        };



        class FatFileReader : public FileReader{

            private:
                DirectoryEntry* fileInfo;
                FatDirectoryTraverser* traverser;
                uint32_t offsetPosition;

            public:
                FatFileReader(FatDirectoryTraverser* parent, DirectoryEntry file);
                ~FatFileReader();

                uint32_t Read(uint8_t* data, uint32_t size);
                uint32_t Seek(uint32_t position, SeekType seek);

                uint32_t GetPosition();
                uint32_t GetFileSize();

        };



        class FatFileEnumerator : public FileEnumerator{
            friend class FatFileReader;
            friend class FatFileWriter;

            private:
                FatDirectoryTraverser* traverser;
                DirectoryEntry* fileInfo;
                int index;

                FatFileReader* reader;
                FatFileWriter* writer;

            public:
                FatFileEnumerator(FatDirectoryTraverser* parent, DirectoryEntry directory, int id);
                ~FatFileEnumerator();

                string getFileName();
                string changeFileName(string newFileName);

                FileReader* getReader();
                FileWriter* getWriter();

                bool hasNext();
                FileEnumerator*
                next();

        };

        class FatDirectoryEnumerator : public DirectoryEnumerator{
            friend class FatDirectoryTraverser;    
            private:
                FatDirectoryTraverser* traverser;
               
                int index;

            public:
                FatDirectoryEnumerator(FatDirectoryTraverser* parent, DirectoryEntry directory, int id);
                ~FatDirectoryEnumerator();

                 DirectoryEntry* directoryInfo;

                string getDirectoryName();
                string changeDirectoryName(string newDirectoryName);

                bool hasNext();
                DirectoryEnumerator* next();

        };

        class FatDirectoryTraverser : public DirectoryTraverser{
            friend class FatDirectoryEnumerator;
            friend class FatFileEnumerator;

        private:

                DirectoryEntry tempDirent[16];
                common::Vector<DirectoryEntry> dirent;        
                common::Vector<DirectoryEntryExtras> dirent_extras;

                common::OutputStream* fat32MessageStream;

                FatDirectoryEnumerator* currentDirectoryEnumerator;
                uint32_t currentDirectoryIndex;


                FatFileEnumerator* currentFileEnumerator;
                uint32_t currentFileIndex;

            public:
                drivers::AdvancedTechnologyAttachment* hd;
                uint32_t dataStartSector;
                uint32_t sectorsPrCluster;
                uint32_t fatLocation;
                uint32_t fatSize;
                uint32_t directorySector;
                uint32_t directoryCluster;

                FatDirectoryTraverser(drivers::AdvancedTechnologyAttachment* ataDevice, uint32_t directorySector, uint32_t dataStart, uint32_t clusterSectorCount, uint32_t fatLoc, uint32_t fat_size, common::OutputStream* fat32MessageStream);
                ~FatDirectoryTraverser();


                void changeDirectory(FatDirectoryEnumerator* directory);

                void makeDirectory(string name);
                void removeDirectory(string name);

                void makeFile(string name);
                void removeFile(string name);

                void WriteDirectoryInfoChange(DirectoryEntry* entry);
	            
                void UpdateDirectoryEntrysToDisk();
                void ReadEntrys();

                FileEnumerator* getFileEnumerator();
                DirectoryEnumerator* getDirectoryEnumerator();


        };

        class Fat32 : public FileSystem{
            private:
                drivers::AdvancedTechnologyAttachment* drive;
                uint32_t partOffset;
                common::OutputStream* fat32MessageStream;

                FatDirectoryTraverser* currentTraverser;

            public:
                Fat32(drivers::AdvancedTechnologyAttachment *hd, uint32_t partitionOffset, common::OutputStream* fat32MessageStream);
                ~Fat32();

                static uint32_t AllocateCluster(drivers::AdvancedTechnologyAttachment *hd, uint32_t currentCluster, uint32_t fatLocation, uint32_t fat_size);
                static void DeallocateCluster(drivers::AdvancedTechnologyAttachment *hd, uint32_t firstCluster, uint32_t fatLocation, uint32_t fat_size);

                static void UpdateEntryInFat(drivers::AdvancedTechnologyAttachment *hd, uint32_t cluster, uint32_t newFatValue, uint32_t fatLocation);

                static bool IsValidFAT32Name(string name);

                DirectoryTraverser* getDirectoryTraverser();


        };

        void ReadBiosBlock(drivers::AdvancedTechnologyAttachment *hd, uint32_t partitionOffset);

    }

}

#endif //MAXOS_FILESYSTEM_FAT32_H
