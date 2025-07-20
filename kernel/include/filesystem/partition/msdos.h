//
// Created by Max Tyson on 18/04/2025.
//

#ifndef MAXOS_FILESYSTEM_PARTITION_MSDOS_H
#define MAXOS_FILESYSTEM_PARTITION_MSDOS_H

#include <stdint.h>
#include <drivers/disk/disk.h>
#include <filesystem/fat32.h>
#include <filesystem/ext2.h>
#include <filesystem/vfs.h>

namespace MaxOS{

  namespace filesystem{

    namespace partition{


      /// Credit: http://www.osdever.net/documents/pdf/partitiontypes.pdf
      enum class PartitionType {
            EMPTY,                 // 0x00
            FAT12,                 // 0x01
            XENIX_ROOT,            // 0x02
            XENIX_USR,             // 0x03
            FAT16,                 // 0x04
            EXTENDED,              // 0x05
            FAT16_32MB,            // 0x06
            NTFS,                  // 0x07
            QNX_QNY,               // 0x08
            AIX_DATA,              // 0x09
            OS2_BOOT_MANAGER,      // 0x0A
            FAT32,                 // 0x0B
            FAT32_LBA,             // 0x0C
            UNUSED_1,              // 0x0D
            FAT16_LBA,             // 0x0E
            EXTENDED_LBA,          // 0x0F
            OPUS,                  // 0x10
            FAT12_HIDDEN,          // 0x11
            COMPAQ,                // 0x12
            UNUSED_2,              // 0x13
            FAT16_HIDDEN,          // 0x14
            UNUSED_3,              // 0x15
            FAT16_32MB_HIDDEN,     // 0x16
            IFS_HIDDEN,            // 0x17
            AST_SMART_SLEEP,       // 0x18
            UNUSED_4,              // 0x19
            UNUSED_5,              // 0x1A
            FAT32_HIDDEN,          // 0x1B
            FAT32_LBA_HIDDEN,      // 0x1C
            UNUSED_6,              // 0x1D
            FAT16_LBA_HIDDEN,      // 0x1E

            UNUSED_7,              // 0x1F
            UNUSED_8,              // 0x20
            RESERVED_1,            // 0x21
            UNUSED_10,             // 0x22
            RESERVED_2,            // 0x23
            NEC_DOS,               // 0x24
            UNUSED_11,             // 0x25
            RESERVED_3,            // 0x26

            // 0x27 - 0x30: unassigned
            UNUSED_12,             // 0x27
            UNUSED_13,             // 0x28
            UNUSED_14,             // 0x29
            UNUSED_15,             // 0x2A
            UNUSED_16,             // 0x2B
            UNUSED_17,             // 0x2C
            UNUSED_18,             // 0x2D
            UNUSED_19,             // 0x2E
            UNUSED_20,             // 0x2F
            UNUSED_21,             // 0x30

            RESERVED_4,            // 0x31
            NOS,                   // 0x32
            RESERVED_5,            // 0x33
            RESERVED_6,            // 0x34
            JFS,                   // 0x35
            RESERVED_7,            // 0x36
            UNUSED_22,             // 0x37
            THEOS_3_2,             // 0x38
            PLAN9,                 // 0x39
            THEOS_4_4GB,           // 0x3A
            THEOS_4_EXTENDED,      // 0x3B
            PARTITIONMAGIC_RECOVERY, // 0x3C
            HIDDEN_NETWARE,        // 0x3D
            UNUSED_23,             // 0x3E
            UNUSED_24,             // 0x3F

            VENIX,                 // 0x40
            LINUX_MINIX,           // 0x41
            LINUX_SWAP,            // 0x42
            LINUX_NATIVE,          // 0x43
            GOBACK,                // 0x44
            BOOT_US,               // 0x45
            EUMEL_ELAN_46,         // 0x46
            EUMEL_ELAN_47,         // 0x47
            EUMEL_ELAN_48,         // 0x48
            UNUSED_25,             // 0x49
            ADAOS,                 // 0x4A
            UNUSED_26,             // 0x4B
            OBERON,                // 0x4C
            QNX4,                  // 0x4D
            QNX4_SECOND,           // 0x4E
            QNX4_THIRD,            // 0x4F

            ONTRACK_DM,            // 0x50
            ONTRACK_DM_RW,         // 0x51
            CPM,                   // 0x52
            DISK_MANAGER_AUX3,     // 0x53
            DISK_MANAGER_DDO,      // 0x54
            EZ_DRIVE,              // 0x55
            GOLDEN_BOW,            // 0x56
            DRIVE_PRO,             // 0x57

            UNUSED_27,             // 0x58
            UNUSED_28,             // 0x59
            UNUSED_29,             // 0x5A
            UNUSED_30,             // 0x5B
            PRIAM_EDISK,           // 0x5C
            UNUSED_31,             // 0x5D
            UNUSED_32,             // 0x5E
            UNUSED_33,             // 0x5F

            UNUSED_34,             // 0x60
            SPEEDSTOR,             // 0x61
            UNUSED_35,             // 0x62
            UNIX,                  // 0x63
            PC_ARMOUR,             // 0x64
            NOVELL_NETWARE386,     // 0x65
            NOVELL_SMS,            // 0x66
            NOVELL,                // 0x67
            NOVELL_OLD,            // 0x68
            NOVELL_NETWARE_NSS,    // 0x69

            UNUSED_36,             // 0x6A
            UNUSED_37,             // 0x6B
            UNUSED_38,             // 0x6C
            UNUSED_39,             // 0x6D
            UNUSED_40,             // 0x6E
            UNUSED_41,             // 0x6F

            DISKSECURE,            // 0x70
            RESERVED_8,            // 0x71
            UNUSED_42,             // 0x72
            RESERVED_9,            // 0x73
            SCRAMDISK,             // 0x74
            IBM_PCIX,              // 0x75
            RESERVED_10,           // 0x76
            M2FS,                  // 0x77
            XOSL_FS,               // 0x78
            UNUSED_43,             // 0x79
            UNUSED_44,             // 0x7A
            UNUSED_45,             // 0x7B
            UNUSED_46,             // 0x7C
            UNUSED_47,             // 0x7D
            UNUSED_48,             // 0x7E
            UNUSED_49,             // 0x7F

            MINIX,                 // 0x80
            MINIX2,                // 0x81
            LINUX_SWAP_ALT,        // 0x82
            LINUX_EXT2,            // 0x83
            HIBERNATION,           // 0x84
            LINUX_EXTENDED,        // 0x85
            LINUX_RAID,            // 0x86
            NTFS_VOLUME_SET,       // 0x87
            UNUSED_50,             // 0x88
            UNUSED_51,             // 0x89
            LINUX_KERNEL,          // 0x8A
            FAULT_TOLERANT_FAT32,  // 0x8B
            FT_FAT32_LBA,          // 0x8C
            FREEFDISK_FAT12,       // 0x8D
            LINUX_LVM,             // 0x8E
            UNUSED_52,             // 0x8F
            FREEFDISK_FAT16,       // 0x90
            FREEFDISK_EXTENDED,    // 0x91
            FREEFDISK_FAT16_LARGE, // 0x92
            HIDDEN_LINUX_NATIVE,   // 0x93
            AMOEBA_BAD_BLOCK,      // 0x94
            MIT_EXOPC,             // 0x95
            UNUSED_53,             // 0x96
            FREEFDISK_FAT32,       // 0x97
            FREEFDISK_FAT32_LBA,   // 0x98
            DCE376,                // 0x99
            FREEFDISK_FAT16_LBA,   // 0x9A
            FREEFDISK_EXTENDED_LBA,// 0x9B

            UNUSED_54,             // 0x9C
            UNUSED_55,             // 0x9D
            UNUSED_56,             // 0x9E

            BSD_OS,                // 0x9F
            LAPTOP_HIBERNATION,    // 0xA0
            HP_VOLUME_EXPANSION,   // 0xA1
            UNUSED_57,             // 0xA2
            RESERVED_11,           // 0xA3
            RESERVED_12,           // 0xA4
            BSD_386,               // 0xA5
            OPENBSD,               // 0xA6
            NEXTSTEP,              // 0xA7
            MAC_OS_X,              // 0xA8
            NETBSD,                // 0xA9
            OLIVETTI_SERVICE,      // 0xAA
            MAC_OS_X_BOOT,         // 0xAB
            UNUSED_58,             // 0xAC
            UNUSED_59,             // 0xAD
            SHAGOS,                // 0xAE
            SHAGOS_SWAP,           // 0xAF
            BOOTSTAR,              // 0xB0

            RESERVED_13,           // 0xB1
            UNUSED_60,             // 0xB2
            RESERVED_14,           // 0xB3
            RESERVED_15,           // 0xB4
            UNUSED_61,             // 0xB5
            RESERVED_16,           // 0xB6

            BSDI_BSD386_FS,        // 0xB7
            BSDI_BSD386_SWAP,      // 0xB8
            UNUSED_62,             // 0xB9
            UNUSED_63,             // 0xBA
            BOOT_WIZARD_HIDDEN,    // 0xBB
            UNUSED_64,             // 0xBC
            UNUSED_65,             // 0xBD
            SOLARIS8_BOOT,         // 0xBE
            UNUSED_66,             // 0xBF
            CTOS,                  // 0xC0
            DRDOS_FAT12,           // 0xC1
            RESERVED_17,           // 0xC2
            HIDDEN_LINUX_SWAP,     // 0xC3
            DRDOS_FAT16,           // 0xC4
            DRDOS_EXTENDED,        // 0xC5
            DRDOS_FAT16_LARGE,     // 0xC6
            NTFS_CORRUPT,          // 0xC7

            UNUSED_67,             // 0xC8
            UNUSED_68,             // 0xC9
            UNUSED_69,             // 0xCA

            DRDOS_FAT32,           // 0xCB
            DRDOS_FAT32_LBA,       // 0xCC
            CTOS_MEMDUMP,          // 0xCD
            DRDOS_FAT16_LBA,       // 0xCE
            UNUSED_70,             // 0xCF
            REAL32_SECURE_BIG,     // 0xD0
            OLD_MULTIUSER_DOS_FAT12,// 0xD1
            UNUSED_71,             // 0xD2
            UNUSED_72,             // 0xD3
            OLD_MULTIUSER_DOS_FAT16,// 0xD4
            OLD_MULTIUSER_DOS_EXTENDED,// 0xD5
            OLD_MULTIUSER_DOS_FAT16_LARGE,// 0xD6
            UNUSED_73,             // 0xD7
            CPM86,                 // 0xD8
            UNUSED_74,             // 0xD9
            NON_FS_DATA,           // 0xDA
            DIGITAL_RESEARCH_CPM,  // 0xDB
            UNUSED_75,             // 0xDC
            HIDDEN_CTOS_MEMDUMP,   // 0xDD
            DELL_POWEREDGE,        // 0xDE
            DGUX,                  // 0xDF
            STM_AVFS,              // 0xE0
            SPEEDSTOR_FAT_EXTENDED,// 0xE1
            UNUSED_76,             // 0xE2
            SPEEDSTOR_RO,          // 0xE3
            SPEEDSTOR_EXTENDED,    // 0xE4
            TANDY_DOS,             // 0xE5

            RESERVED_18,           // 0xE6
            UNUSED_77,             // 0xE7
            UNUSED_78,             // 0xE8
            UNUSED_79,             // 0xE9

            BEFS,                  // 0xEA
            SPRYTIX,               // 0xEB
            EFI_PROTECTIVE,        // 0xEE
            EFI_SYSTEM,            // 0xEF
            LINUX_PA_RISC,         // 0xF0
            SPEEDSTOR_2,           // 0xF1
            DOS_3_3_SECONDARY,     // 0xF2
            RESERVED_19,           // 0xF3
            SPEEDSTOR_LARGE,       // 0xF4
            PROLOGUE_MULTI,        // 0xF5
            RESERVED_20,           // 0xF6
            UNUSED_80,             // 0xF7
            UNUSED_81,             // 0xF8
            UNUSED_82,             // 0xF9
            BOCHS,                 // 0xFA
            VMFS,                  // 0xFB
            VMWARE_SWAP,           // 0xFC
            LINUX_RAID_AUTO,       // 0xFD
            NT_HIDDEN,             // 0xFE
            XENIX_BAD_BLOCK        // 0xFF
        };

        /**
         * @struct PartitionTableEntry
         * @brief Stores information about a partition
         */
        struct PartitionTableEntry{

          uint8_t     bootable;              // 0x80 = bootable, 0x00 = not bootable

          uint8_t     start_head;
          uint8_t     start_sector : 6;
          uint16_t    start_cylinder : 10;

          uint8_t     type;

          uint8_t     end_head;
          uint8_t     end_sector : 6;
          uint16_t    end_cylinder : 10;

          uint32_t    start_LBA;
          uint32_t    length;

        } __attribute__((packed));

        /**
         * @struct MasterBootRecord
         * @brief Stores information about the master boot record
         */
        struct MasterBootRecord{

          uint8_t     bootloader[440];
          uint32_t    disk_signature;
          uint16_t    unused;

          PartitionTableEntry primary_partition[4];

          uint16_t    magic;

        } __attribute__((packed));


        class MSDOSPartition
        {
          public:
            static void mount_partitions(drivers::disk::Disk* disk);
        };


        // TODO: Abstract some of this into a base class and use it for GPT and other partition tables
    }
  }
}

#endif //MSDOS_H
