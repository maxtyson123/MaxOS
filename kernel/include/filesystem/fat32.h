//
// Created by 98max on 1/01/2023.
//

#ifndef MAXOS_FILESYSTEM_FAT32_H
#define MAXOS_FILESYSTEM_FAT32_H

#include <common/types.h>
#include <drivers/ata.h>
#include <filesystem/filesystem.h>
#include <common/vector.h>
#include <memory/memorymanagement.h>
#include <common/printf.h>

namespace maxOS{

    namespace filesystem{


        struct BiosParameterBlock32{

            common::uint8_t    jump[3];
            common::uint8_t    OEMName[8];
            common::uint16_t   bytesPerSector;
            common::uint8_t    sectorsPerCluster;
            common::uint16_t   reservedSectors;
            common::uint8_t    tableCopies;
            common::uint16_t   rootEntries;
            common::uint16_t   totalSectors16;
            common::uint8_t    mediaType;
            common::uint16_t   fatSectorCount;
            common::uint16_t   sectorsPerTrack;
            common::uint16_t   headCount;
            common::uint32_t   hiddenSectors;
            common::uint32_t   totalSectors32;

            common::uint32_t   tableSize32;
            common::uint16_t   extendedFlags;
            common::uint16_t   fatVersion;
            common::uint32_t   rootCluster;
            common::uint16_t   fatInfo;
            common::uint16_t   backupSector;
            common::uint8_t    reserved0[12];
            common::uint8_t    driveNumber;
            common::uint8_t    reserved1;
            common::uint8_t    bootSignature;
            common::uint32_t   volumeID;
            common::uint8_t    volumeLabel[11];
            common::uint8_t    fileSystemType[8];

        } __attribute__((packed));

        struct DirectoryEntry{

            common::uint8_t    name[8];
            common::uint8_t    extension[3];
            common::uint8_t    attributes;
            common::uint8_t    reserved;

            common::uint8_t    creationTimeTenth;
            common::uint16_t   creationTime;
            common::uint16_t   creationDate;
            common::uint16_t   lastAccessDate;

            common::uint16_t   firstClusterHigh;

            common::uint16_t  lastWriteTime;
            common::uint16_t  lastWriteDate;

            common::uint16_t  firstClusterLow;

            common::uint32_t  size;

        } __attribute__((packed));


        class FatDirectoryTraverser;

        class FatFileWriter : public FileWriter {
                DirectoryEntry* fileInfo;
                FatDirectoryTraverser* traverser;
                common::uint32_t offsetPosition;
            public:
                FatFileWriter(FatDirectoryTraverser* parent, DirectoryEntry file);
                ~FatFileWriter();

                common::uint32_t Write(common::uint8_t *data, common::uint32_t size);
                common::uint32_t Seek(common::uint32_t position, SeekType seek);

                bool Close();
                bool Flush();

                common::uint32_t GetPosition();
                common::uint32_t GetFileSize();
        };



        class FatFileReader : public FileReader{

            private:
                DirectoryEntry* fileInfo;
                FatDirectoryTraverser* traverser;
                common::uint32_t offsetPosition;

            public:
                FatFileReader(FatDirectoryTraverser* parent, DirectoryEntry file);
                ~FatFileReader();

                common::uint32_t Read(common::uint8_t* data, common::uint32_t size);
                common::uint32_t Seek(common::uint32_t position, SeekType seek);

                common::uint32_t GetPosition();
                common::uint32_t GetFileSize();

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

                char* getFileName();
                char* changeFileName(char* newFileName);

                FileReader* getReader();
                FileWriter* getWriter();

                bool hasNext();
                FileEnumerator*
                next();

        };

        class FatDirectoryEnumerator : public DirectoryEnumerator{

            private:
                FatDirectoryTraverser* traverser;
                DirectoryEntry* directoryInfo;
                int index;

            public:
                FatDirectoryEnumerator(FatDirectoryTraverser* parent, DirectoryEntry directory, int id);
                ~FatDirectoryEnumerator();

                char* getDirectoryName();
                char* changeDirectoryName(char* newDirectoryName);

                bool hasNext();
                DirectoryEnumerator* next();

        };

        class FatDirectoryTraverser : public DirectoryTraverser{
            friend class FatDirectoryEnumerator;
            friend class FatFileEnumerator;

        private:

                DirectoryEntry dirent[16];



                FatDirectoryEnumerator* currentDirectoryEnumerator;
                common::uint32_t currentDirectoryIndex;


                FatFileEnumerator* currentFileEnumerator;
                common::uint32_t currentFileIndex;

            public:
                drivers::AdvancedTechnologyAttachment* hd;
                common::uint32_t dataStartSector;
                common::uint32_t sectorsPrCluster;
                common::uint32_t fatLocation;
                common::uint32_t fatSize;
                common::uint32_t directorySector;

                FatDirectoryTraverser(drivers::AdvancedTechnologyAttachment* ataDevice, common::uint32_t directorySector, common::uint32_t dataStart, common::uint32_t clusterSectorCount, common::uint32_t fatLoc, common::uint32_t fat_size);
                ~FatDirectoryTraverser();


                void changeDirectory(DirectoryEnumerator directory);

                void makeDirectory(char* name);
                void removeDirectory(char* name);

                void makeFile(char* name);
                void removeFile(char* name);

                void WriteDirectoryInfoChange(DirectoryEntry* entry);
	            
                FileEnumerator* getFileEnumerator();
                DirectoryEnumerator* getDirectoryEnumerator();


        };

        class Fat32 : public FileSystem{
            private:
                drivers::AdvancedTechnologyAttachment* drive;
                common::uint32_t partOffset;

                FatDirectoryTraverser* currentTraverser;

            public:
                Fat32(drivers::AdvancedTechnologyAttachment *hd, common::uint32_t partitionOffset);
                ~Fat32();

                static common::uint32_t AllocateCluster(drivers::AdvancedTechnologyAttachment *hd, common::uint32_t currentCluster, common::uint32_t fatLocation, common::uint32_t fat_size);
                static void DeallocateCluster(drivers::AdvancedTechnologyAttachment *hd, common::uint32_t firstCluster, common::uint32_t fatLocation, common::uint32_t fat_size);

                static void UpdateEntryInFat(drivers::AdvancedTechnologyAttachment *hd, common::uint32_t cluster, common::uint32_t newFatValue, common::uint32_t fatLocation);

                static bool IsValidFAT32Name(char* name);

                DirectoryTraverser* getDirectoryTraverser();


        };

        void ReadBiosBlock(drivers::AdvancedTechnologyAttachment *hd, common::uint32_t partitionOffset);

    }

}

#endif //MAXOS_FILESYSTEM_FAT32_H
