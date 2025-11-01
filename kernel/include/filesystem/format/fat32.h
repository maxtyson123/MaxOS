/**
 * @file fat32.h
 * @brief Defines structures and constants for the File Allocation Table 32 (FAT32) filesystem format and the relevant
 * Filesystem, Directory and File classes.
 *
 * @date 1st January 2023
 * @author Max Tyson
 *
 * @todo BUG: Subdirectory seems to write to the disk this end but tools like fatcat complain the that the EOC isn't written (cluster 3037)
 * @todo Implment: create/delete/rename files & directories
 */

#ifndef MAXOS_FILESYSTEM_FAT32_H
#define MAXOS_FILESYSTEM_FAT32_H

#include <stdint.h>
#include <common/macros.h>
#include <drivers/disk/disk.h>
#include <filesystem/filesystem.h>

namespace MaxOS {
	namespace filesystem {
		namespace format {

			/**
			 * @struct BiosParameterBlock32
			 * @brief Stores information about the FAT32 filesystem
			 *
			 * @typedef bpb32_t
			 * @brief Alias for BiosParameterBlock32 struct
			 */
			typedef struct PACKED BiosParameterBlock32 {

				uint8_t jump[3];                    ///< Jump instruction to boot code
				uint8_t OEM_name[8];                ///< OEM Name in ASCII
				uint16_t bytes_per_sector;          ///< Bytes per sector
				uint8_t sectors_per_cluster;        ///< Sectors per cluster
				uint16_t reserved_sectors;          ///< Number of reserved sectors
				uint8_t table_copies;               ///< Number of FAT tables
				uint16_t root_entries;              ///< Number of root directory entries
				uint16_t total_sectors_16;          ///< Total number of sectors (if zero, use total_sectors_32)
				uint8_t media_type;                 ///< Media descriptor
				uint16_t fat_sector_count;          ///< Sectors per FAT (FAT12/FAT16 only)
				uint16_t sectors_per_track;         ///< Sectors per track
				uint16_t head_count;                ///< Number of heads/sides
				uint32_t hidden_sectors;            ///< Number of hidden sectors (LBA of start of partition)
				uint32_t total_sectors_32;          ///< Total number of sectors (if total_sectors_16 is zero)

				uint32_t table_size_32;             ///< Sectors per FAT (FAT32 only)
				uint16_t extended_flags;            ///< Extended flags
				uint16_t fat_version;               ///< FAT version number (high byte major, low byte minor)
				uint32_t root_cluster;              ///< Cluster number of root directory start
				uint16_t fat_info;                  ///< Sector number of FSInfo structure
				uint16_t backup_sector;             ///< Sector number of backup boot sector
				uint8_t reserved0[12];              ///< Should be zero
				uint8_t drive_number;               ///< Drive number (must be identical to BIOS drive number) (0x00 = floppy, 0x80 = first hard disk)
				uint8_t reserved_1;                 ///< Reserved (used by Windows NT)
				uint8_t boot_signature;             ///< Extended boot signature (0x28 or 0x29)
				uint32_t volume_id;                 ///< Volume serial number
				uint8_t volume_label[11];           ///< Volume label in ASCII (spaces padded)
				uint8_t file_system_type[8];        ///< System type in ASCII (not null-terminated)

			} bpb32_t;

			/**
			 * @struct FSInfo
			 * @brief Stores extra information about the FAT32 filesystem
			 *
			 * @typedef fs_info_t
			 * @brief Alias for FSInfo struct
			 */
			typedef struct PACKED FSInfo {
				uint32_t lead_signature;            ///< FSInfo lead signature (0x41615252)
				uint8_t reserved1[480];             ///< Reserved (should be zero)
				uint32_t structure_signature;       ///< FSInfo structure signature (0x61417272)
				uint32_t free_cluster_count;        ///< Number of free clusters (or 0xFFFFFFFF if unknown) (not reliable, must range check)
				uint32_t next_free_cluster;         ///< Cluster number of the next free cluster (or 0xFFFFFFFF if unknown, start searching from cluster 2)
				uint8_t reserved2[12];              ///< Reserved (should be zero)
				uint32_t trail_signature;           ///< FSInfo trail signature (0xAA550000)
			} fs_info_t;

			/**
			 * @struct DirectoryEntry
			 * @brief Stores information about a file or directory
			 *
			 * @typedef dir_entry_t
			 * @brief Alias for DirectoryEntry struct
			 */
			typedef struct PACKED DirectoryEntry {

				uint8_t name[8];                    ///< File name
				uint8_t extension[3];               ///< File extension
				uint8_t attributes;                 ///< File attributes (see DirectoryEntryAttributes)
				uint8_t reserved;                   ///< Reserved for Windows NT

				uint8_t creation_time_tenth;        ///< Creation time in tenths of a second (or 100ths of a second) (0-199 on Windows, 0/100 on Linux)
				uint16_t creation_time;             ///< Creation time (HH:MM:SS) (multiply seconds by 2)
				uint16_t creation_date;             ///< Creation date (YYYY-MM-DD)
				uint16_t last_access_date;          ///< Last access date (YYYY-MM-DD)

				uint16_t first_cluster_high;        ///< High word of this entry's first cluster number (usually 0 for FAT12/FAT16)

				uint16_t last_write_time;           ///< Last write time (HH:MM:SS) (multiply seconds by 2)
				uint16_t last_write_date;           ///< Last write date (YYYY-MM-DD)

				uint16_t first_cluster_low;         ///< Low word of this entry's first cluster number

				uint32_t size;                      ///< File size in bytes

			} dir_entry_t;

			/**
			 * @enum DirectoryEntryAttributes
			 * @brief Flags for a directory entry in a FAT32 System
			 */
			enum class DirectoryEntryAttributes {
				FREE = 0x00,
				READ_ONLY = 0x01,
				HIDDEN = 0x02,
				SYSTEM = 0x04,
				VOLUME_ID = 0x08,
				DIRECTORY = 0x10,
				ARCHIVE = 0x20,
				LONG_NAME = READ_ONLY | HIDDEN | SYSTEM | VOLUME_ID,
			};

			/**
			 * @enum DirectoryEntryType
			 * @brief The type of a directory entry
			 */
			enum class DirectoryEntryType {
				LAST = 0x00,
				FREE = 0xE5,
			};

			/**
			 * @struct LongFileNameEntry
			 * @brief Directory entry for a long file name
			 *
			 * @typedef long_file_name_entry_t
			 * @brief Alias for LongFileNameEntry struct
			 */
			typedef struct PACKED LongFileNameEntry {
				uint8_t order;          ///< Index of this entry in the sequence of LFN entries
				uint16_t name1[5];      ///< First 5 characters of the long file name
				uint8_t attributes;     ///< Attributes (always 0x0F for LFN entries)
				uint8_t type;           ///< Type (always 0 for LFN entries)
				uint8_t checksum;       ///< Checksum of the corresponding short file name
				uint16_t name2[6];      ///< Next 6 characters of the long file name
				uint16_t zero;          ///< Must be zero
				uint16_t name3[2];      ///< Last 2 characters of the long file name

			} long_file_name_entry_t;

			/**
			 * @enum ClusterState
			 * @brief The state of a cluster in the FAT32 filesystem
			 */
			enum class ClusterState : uint32_t {
				FREE            = 0x00000000,
				BAD             = 0x0FFFFFF7,
				END_OF_CHAIN    = 0xFFFFFFFF,
			};

			/// Highest number of characters in a file/directory name
			constexpr uint16_t MAX_NAME_LENGTH = 255;

			/**
			 * @class Fat32Volume
			 * @brief Handles the FAT table that stores the information about the files on the disk and operations on the disk
			 */
			class Fat32Volume {
				public:
					Fat32Volume(drivers::disk::Disk* disk, lba_t partition_offset);
					~Fat32Volume();

					bpb32_t bpb;                        ///< The BIOS Parameter Block for the FAT32 volume
					fs_info_t fsinfo;                   ///< The FSInfo structure for the FAT32 volume

					size_t fat_total_clusters;          ///< How many clusters are in the FAT table
					lba_t fat_lba;                      ///< The starting LBA of the FAT table
					lba_t fat_info_lba;                 ///< The LBA of the FSInfo structure
					lba_t fat_copies;                   ///< How many FAT tables are present

					lba_t data_lba;                     ///< The starting LBA of the data region
					lba_t root_lba;                     ///< The starting LBA of the root directory

					drivers::disk::Disk* disk;          ///< The disk that this volume is on

					uint32_t next_cluster(uint32_t cluster);
					uint32_t set_next_cluster(uint32_t cluster, uint32_t next_cluster);
					uint32_t find_free_cluster();

					uint32_t allocate_cluster(uint32_t cluster);
					uint32_t allocate_cluster(uint32_t cluster, size_t amount);

					void free_cluster(uint32_t cluster);
					void free_cluster(uint32_t cluster, size_t amount);
			};

			// Forward def
			class Fat32Directory;

			/**
			 * @class Fat32File
			 * @brief Handles the file operations on the FAT32 filesystem
			 */
			class Fat32File final : public File {

				private:
					Fat32Volume* m_volume;
					Fat32Directory* m_parent_directory;

					dir_entry_t* m_entry;
					uint32_t m_first_cluster;

				public:
					Fat32File(Fat32Volume* volume, Fat32Directory* parent, dir_entry_t* info, const string &name);
					~Fat32File() final;

					void write(const common::buffer_t* data, size_t amount) final;
					void read(common::buffer_t* data, size_t amount) final;
					void flush() final;

					uint32_t first_cluster() const { return m_first_cluster; } ///< Get the first cluster of the file
			};

			/**
			 * @class Fat32Directory
			 * @brief Handles the directory operations on the FAT32 filesystem
			 */
			class Fat32Directory : public Directory {
					friend class Fat32File;

				private:
					Fat32Volume* m_volume;

					lba_t m_first_cluster;
					lba_t m_last_cluster;
					size_t m_current_cluster_length = 0;

					common::Vector<dir_entry_t> m_entries;

					dir_entry_t* create_entry(const string &name, bool is_directory);
					void remove_entry(lba_t cluster, const string &name);
					void read_all_entries();

					int entry_index(lba_t cluster);
					int find_free_entries(size_t amount);
					int expand_directory(size_t amount);

					static common::Vector<long_file_name_entry_t> to_long_filenames(string name);
					static string parse_long_filename(long_file_name_entry_t* entry, const string &current);

				protected:

					void save_entry_to_disk(dir_entry_t* entry);
					void update_entry_on_disk(int index);

				public:
					Fat32Directory(Fat32Volume* volume, lba_t cluster, const string &name);
					~Fat32Directory();

					void read_from_disk() final;

					File* create_file(const string &name) final;
					void remove_file(const string &name) final;

					Directory* create_subdirectory(const string &name) final;
					void remove_subdirectory(const string &name) final;

					[[nodiscard]] lba_t first_cluster() const { return m_first_cluster; }   ///< Get the first cluster of the directory
			};

			/**
			 * @class Fat32FileSystem
			 * @brief Handles the FAT32 filesystem operations
			 */
			class Fat32FileSystem : public FileSystem {
				private:
					Fat32Volume m_volume;

				public:
					Fat32FileSystem(drivers::disk::Disk* disk, uint32_t partition_offset);
					~Fat32FileSystem();
			};

		}
	}
}

#endif //MAXOS_FILESYSTEM_FAT32_H
