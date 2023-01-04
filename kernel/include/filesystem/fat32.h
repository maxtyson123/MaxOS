//
// Created by 98max on 1/01/2023.
//

#ifndef MAXOS_FILESYSTEM_FAT32_H
#define MAXOS_FILESYSTEM_FAT32_H

#include <common/types.h>
#include <drivers/ata.h>

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

             void ReadBiosBlock(drivers::AdvancedTechnologyAttachment *hd, common::uint32_t partitionOffset);

    }

}

#endif //MAXOS_FILESYSTEM_FAT32_H
