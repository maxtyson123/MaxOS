/**
 * @file ext2.h
 * @brief Defines structures and enums for the Extended 2 (ext2) filesystem format and relevant Filesystem, Directory and File classes.
 *
 * @date 17th July 2025
 * @author Max Tyson
 */

#ifndef MAXOS_FILESYSTEM_EXT2_H
#define MAXOS_FILESYSTEM_EXT2_H

#include <stdint.h>
#include <common/spinlock.h>
#include <common/macros.h>
#include <drivers/disk/disk.h>
#include <drivers/clock/clock.h>
#include <filesystem/filesystem.h>
#include <memory/memoryIO.h>

namespace MaxOS {
	namespace filesystem {
		namespace format {
			namespace ext2 {

				/**
				 * @struct SuperBlock
				 * @brief The metadata of the ext2 filesystem. Found at an offset of 1024 bytes from the start of the partition.
				 *
				 * @typedef superblock_t
				 * @brief Alias for SuperBlock struct
				 */
				typedef struct PACKED SuperBlock {
					uint32_t total_inodes;                  ///< Total number of inodes
					uint32_t total_blocks;                  ///< Total number of blocks
					uint32_t reserved_blocks;               ///< Number of reserved blocks for superuser
					uint32_t unallocated_blocks;            ///< Number of free blocks
					uint32_t unallocated_inodes;            ///< Number of free inodes
					uint32_t starting_block;                ///< Where the superblock starts (normally 0 but not always)
					uint32_t block_size;                    ///< Block size (as a power of 2, so 1024 << block_size)
					uint32_t fragment_size;                 ///< Fragment size (as a power of 2, so 1024 << fragment_size)
					uint32_t blocks_per_group;              ///< Number of blocks per group
					uint32_t fragments_per_group;           ///< Number of fragments per group
					uint32_t inodes_per_group;              ///< Number of inodes per group
					uint32_t last_mount_time;               ///< Last time the filesystem was mounted (in seconds since the epoch)
					uint32_t late_write_time;               ///< Last time a block was written to (in seconds since the epoch)
					uint16_t mounts_since_check;            ///< Number of mounts since the last consistency check
					uint16_t mounts_until_check;            ///< Number of mounts allowed before a consistency check is needed (fsck)
					uint16_t signature;                     ///< Filesystem magic signature (0xEF53)
					uint16_t state;                         ///< The state of the filesystem (see FileSystemState)
					uint16_t error_operation;               ///< What to do when an error is detected (see ErrorOperation)
					uint16_t version_minor;                 ///< Minor version number (combined with version_major to make version number)
					uint32_t last_check_time;               ///< Time of last consistency check (in seconds since the epoch)
					uint32_t time_until_check;              ///< Maximum time between checks (in seconds)
					uint32_t os_id;                         ///< ID of the OS which created the filesystem (see CreatorOS)
					uint32_t version_major;                 ///< Major version number (combined with version_minor to make version number)
					uint16_t reserved_user;                 ///< User ID that can use reserved blocks
					uint16_t reserved_group;                ///< Group ID that can use reserved blocks

					// Extended Fields (version >= 1)
					uint32_t first_inode;                   ///< Number of the first non-reserved inode (version < 1 this is 11)
					uint16_t inode_size;                    ///< Size of each inode structure (version < 1 this is 128 bytes)
					uint16_t superblock_group;              ///< If there is a backup superblock, which group it is stored in
					uint32_t optional_features;             ///< Features that are not required for read/write support but can help performance (see OptionalFeatures)
					uint32_t required_features;             ///< Features that must be present for read/write support (see RequiredFeatures)
					uint16_t read_only_features;            ///< Features that are only required in read-only mode (see ReadOnlyFeatures)
					uint8_t filesystem_id[16];              ///< A unique identifier for the filesystem
					uint8_t volume_name[16];                ///< The volume name (is null-terminated)
					uint8_t last_mount_path[64];            ///< The path where the filesystem was last mounted (is null-terminated)
					uint32_t compression;                   ///< Compression algorithm used (0 = none, see RequiredFeatures)
					uint8_t file_preallocation_blocks;      ///< How many blocks should be allocated when creating a file
					uint8_t directory_preallocation_blocks; ///< How many blocks should be allocated when creating a directory
					uint16_t unused;                        ///< Reserved
					uint8_t journal_id[16];                 ///< A unique identifier for the journal
					uint32_t journal_inode;                 ///< The inode number of the journal file
					uint32_t journal_device;                ///< The device number of the journal file
					uint32_t orphan_inodes_start;           ///< The start of the list of inodes without a directory entry that need to be deleted
					uint8_t free[276];                      ///< Reserved for future expansion

				} superblock_t;

				/**
				 * @enum FileSystemState
				 * @brief The state of the filesystem
				 */
				enum class FileSystemState {
					CLEAN = 1,
					ERROR = 2,
				};

				/**
				 * @enum ErrorOperation
				 * @brief What to do when an error occurs
				 */
				enum class ErrorOperation {
					IGNORE  = 1,
					REMOUNT = 2,
					PANIC   = 3,
				};

				/**
				 * @enum CreatorOS
				 * @brief The OS which created the filesystem
				 *
				 * @todo Add MaxOS
				 */
				enum class CreatorOS {
					LINUX,
					GNU_HURD,
					MASIX,
					FREE_BSD,
					OTHER_LITES,
				};

				/**
				 * @enum OptionalFeatures
				 * @brief What features that are optional are supported by this filesystem
				 */
				enum class OptionalFeatures {
					PREALLOCATE_DIRECTORY   = 0x1,
					AFS_SERVER_INODES       = 0x2,
					JOURNAL_ENABLED         = 0x4,
					ATTRIBUTES_EXTENDED     = 0x8,
					RESIZEABLE              = 0x10,
					HASH_INDEXING           = 0x20,
				};

				/**
				 * @enum RequiredFeatures
				 * @brief What features that are required are supported by this filesystem
				 */
				enum class RequiredFeatures {
					COMPRESSION           = 0x1,
					DIRECTORY_HAS_TYPE    = 0x2,
					MUST_REPLAY_JOURNAL   = 0x4,
					JOURNAL_DEVICE        = 0x8,
				};

				/**
				 * @enum ReadOnlyFeatures
				 * @brief What features that are only required in read-only mode
				 */
				enum class ReadOnlyFeatures {
					SPARSE_SUPER_BLOCKS     = 0x1,
					FILES_64_BIT            = 0x2,
					BINARY_TREE_DIRECTORIES = 0x4,
				};

				/**
				 * @struct BlockGroupDescriptor
				 * @brief The metadata for a block group in ext2
				 *
				 * @typedef block_group_descriptor_t
				 * @brief Alias for BlockGroupDescriptor struct
				 *
				 * @todo Support updating directory_count when creating/deleting directories
				 */
				typedef struct PACKED BlockGroupDescriptor {
					uint32_t block_usage_bitmap;            ///< The block address of the block usage bitmap
					uint32_t block_inode_bitmap;            ///< The block address of the inode usage bitmap
					uint32_t inode_table_address;           ///< The starting block address of the inode table
					uint16_t free_blocks;                   ///< How many free blocks are in this block group
					uint16_t free_inodes;                   ///< How many free inodes are in this block group
					uint16_t directory_count;               ///< How many directories are in this block group
					uint8_t free[14];                       ///< Reserved for future expansion

				} block_group_descriptor_t;

				/**
				 * @struct Inode
				 * @brief The metadata for a file or directory in ext2
				 *
				 * @typedef inode_t
				 * @brief Alias for Inode struct
				 */
				typedef struct PACKED Inode {
					union {
						uint16_t type_permissions;          ///< The type and permissions of the inode
						struct {
							uint16_t permissions: 12;       ///< The permissions of the inode
							uint16_t type: 4;               ///< The type of the inode
						};
					};
					uint16_t user_id;                       ///< The ID of the user who owns the inode
					uint32_t size_lower;                    ///< The lower 32 bits of the size of the file in bytes
					uint32_t last_access_time;              ///< The seconds (since the epoch) of the last access time
					uint32_t creation_time;                 ///< The seconds (since the epoch) of the time the inode was first allocated
					uint32_t last_modification_time;        ///< The seconds (since the epoch) of the last time the inode was written to
					uint32_t deletion_time;                 ///< The seconds (since the epoch) of the time the inode was marked as not used
					uint16_t group_id;                      ///< The ID of the group who owns the inode
					uint16_t hard_links;                    ///< How many directory entries point to this inode
					uint32_t sectors_used;                  ///< How many 512 byte sectors are used by this inode (not blocks) (not including this struct or directory entries linked to it)
					uint32_t flags;                         ///< Flags for the inode (see InodeFlags)
					uint32_t os_1;                          ///< OS specific value (0 for Linux/HURD, 'transaltor' for Masix)
					uint32_t block_pointers[12];            ///< Pointers to the blocks that store the file's data (0 marks unused)
					uint32_t l1_indirect;                   ///< Pointer to a block that contains more block pointers (see block_pointers)
					uint32_t l2_indirect;                   ///< Pointer to a block that contains more l1_indirect pointers
					uint32_t l3_indirect;                   ///< Pointer to a block that contains more l2_indirect pointers
					uint32_t generation;                    ///< File version (used by NFS)
					uint32_t extended_attribute;            ///< File extended attribute block (Access Control List)
					uint32_t size_upper;                    ///< The upper 32 bits of the size of the file in bytes (for files larger than 4GB). If this is a directory, this is the Directory Access Control List.
					uint32_t os_2[3];                       ///< Second OS specific value (see InodeOS2Linux, ...)
				} inode_t;

				/**
				 * @enum InodeType
				 * @brief The type of an inode
				 */
				enum class InodeType {
					UNKNOWN,
					FIFO              = 0x1000,
					CHARACTER_DEVICE  = 0x2000,
					DIRECTORY         = 0x4000,
					BLOCK_DEVICE      = 0x6000,
					FILE              = 0x8000,
					SYMBOLIC_LINK     = 0xA000,
					SOCKET            = 0xC000,
				};

				/**
				 * @enum InodePermissions
				 * @brief The permissions that an inode can have
				 */
				enum class InodePermissions{
					OTHER_EXECUTE = 0x1,
					OTHER_WRITE   = 0x2,
					OTHER_READ    = 0x4,
					GROUP_EXECUTE = 0x8,
					GROUP_WRITE   = 0x10,
					GROUP_READ    = 0x20,
					USER_EXECUTE  = 0x40,
					USER_WRITE    = 0x80,
					USER_READ     = 0x100,
					STICKY        = 0x200,
					GROUP_ID      = 0x400,
					USER_ID       = 0x800,
				};

				/**
				 * @enum InodePermissionsDefaults
				 * @brief The default permissions for files and directories
				 *
				 * @todo Document these values
				 */
				enum class InodePermissionsDefaults {
					FILE        = 0x1A4,
					DIRECTORY   = 0x1ED,
				};

				/**
				 * @enum InodeFlags
				 * @brief The flags that can be set on an inode
				 */
				enum class InodeFlags {
					SECURE_DELETE     = 0x1,  // Zero out data on deletion
					KEEP_DATA         = 0x2,
					FILE_COMPRESSION  = 0x4,
					SYNC_UPDATES      = 0x8,
					FILE_IMMUTABLE    = 0x10,
					APPEND_ONLY       = 0x20,
					DONT_DUMP         = 0x40,
					NO_LAST_ACCESS    = 0x80,
					HASH_INDEXED      = 0x10000,
					AFS_DIRECTORY     = 0x20000,
					JOURNAL_FILE_DATA = 0x40000,
				};

				/**
				 * @struct InodeOS2Linux
				 * @brief The OS specific data for Linux created ext2 inodes
				 *
				 * @typedef linux_os_2_t
				 * @brief Alias for InodeOS2Linux struct
				 *
				 * @todo HURD, MASIX
				 */
				typedef struct PACKED InodeOS2Linux {
					uint8_t fragment;                   ///<  The fragment number
					uint8_t fragment_size;              ///<  How many 1024 byte fragments are in the file
					uint16_t high_type_permissions;     ///<  The high bits of the type & permissions
					uint16_t high_user_id;              ///<  The high bits of the user ID
					uint16_t high_group_id;             ///<  The high bits of the group ID
					uint32_t author_id;                 ///<  The user who created the inode (0xFFFFFFFF if use UID/GID fields)
				} linux_os_2_t;

				/**
				 * @struct DirectoryEntry
				 * @brief An entry in a directory that points to a file or subdirectory (the name follows this struct)
				 *
				 * @typedef directory_entry_t
				 * @brief Alias for DirectoryEntry struct
				 */
				typedef struct PACKED DirectoryEntry {
					uint32_t inode;         ///<  The inode number this entry points to (0 = unused)
					uint16_t size;          ///<  The total size of this entry (including name)
					uint8_t name_length;    ///<  The length of the name
					uint8_t type;           ///<  The type of the entry (see EntryType) (only if the high bit of the superblock's optional_features is set)
					// Rest are name chars
				} directory_entry_t;

				/**
				 * @enum EntryType
				 * @brief The type of a directory entry
				 *
				 * @warning MaxOS only supports FILE, DIRECTORY
				 * @todo Support more
				 */
				enum class EntryType {
					UNKNOWN,
					FILE,
					DIRECTORY,
					CHARACTER_DEVICE,
					BLOCK_DEVICE,
					FIFO,
					SOCKET,
					SYMBOLIC_LINK
				};

				/**
				 * @class Ext2Volume
				 * @brief Common operations for an ext2 volume that are used by both files & directories (like block & inode allocation)
				 *
				 * @todo Free blocks
				 */
				class Ext2Volume {

					private:
						common::Vector<uint32_t> allocate_group_blocks(uint32_t block_group, uint32_t amount);
						void free_group_blocks(uint32_t block_group, uint32_t amount, uint32_t start);


						void write_back_block_groups() const;
						void write_back_superblock();

					public:
						Ext2Volume(drivers::disk::Disk* disk, lba_t partition_offset);
						~Ext2Volume();

						drivers::disk::Disk* disk;                          ///< The disk that this volume is on
						lba_t partition_offset;                             ///< How far into the disk this partition starts

						superblock_t superblock;                            ///< The superblock of the ext2 filesystem
						block_group_descriptor_t** block_groups;            ///< The block group descriptors

						size_t block_size;                                  ///< How large each block is (in bytes)
						uint32_t block_group_descriptor_table_block;        ///< Where the block group descriptor table starts
						uint32_t block_group_descriptor_table_size;         ///< How many block groups are in the block group descriptor table
						uint32_t total_block_groups;                        ///< How many block groups are in the filesystem
						size_t pointers_per_block;                          ///< How many block pointers fit in a block
						uint32_t inodes_per_block;                          ///< How many inodes fit in a block
						uint32_t sectors_per_block;                         ///< How many sectors does a block take

						common::Spinlock ext2_lock;                         ///< Lock for synchronised access to the volume (@todo: remove public?)

						void write_block(uint32_t block_num, common::buffer_t* buffer);
						void write_inode(uint32_t inode_num, inode_t* inode);

						[[nodiscard]] uint32_t create_inode(bool is_directory);
						void free_inode(uint32_t inode);

						void read_block(uint32_t block_num, common::buffer_t* buffer) const;
						[[nodiscard]] inode_t read_inode(uint32_t inode_num) const;

						[[nodiscard]] uint32_t allocate_block();
						[[nodiscard]] common::Vector<uint32_t> allocate_blocks(uint32_t amount);
						[[nodiscard]] uint32_t bytes_to_blocks(size_t bytes) const;

						void free_blocks(const common::Vector<uint32_t> &blocks);
				};

				/**
				 * @class InodeHandler
				 * @brief Simplfies the management of an inode & its blocks
				 */
				class InodeHandler {

					private:
						Ext2Volume* m_volume = nullptr;

						void parse_indirect(uint32_t level, uint32_t block, common::buffer_t* buffer);
						void write_indirect(uint32_t level, uint32_t &block, size_t &index);
						void store_blocks(const common::Vector<uint32_t> &blocks);

					public:
						InodeHandler(Ext2Volume* volume, uint32_t inode);
						~InodeHandler();

						uint32_t inode_number;                      ///< The index of the inode
						inode_t inode;                              ///< The inode metadata for this file/directory
						common::Vector<uint32_t> block_cache;       ///< All the blocks used by this inode

						[[nodiscard]] size_t size() const;
						void set_size(size_t size);
						size_t grow(size_t amount, bool flush = true);

						void save();
						void free();

				};

				/**
				 * @class Ext2File
				 * @brief Handles the file operations on the ext2 filesystem
				 */
				class Ext2File final : public File {
					private:
						Ext2Volume* m_volume;
						InodeHandler m_inode;

					public:
						Ext2File(Ext2Volume* volume, uint32_t inode, const string &name);
						~Ext2File() final;

						void write(common::buffer_t* data, size_t amount) final;
						void read(common::buffer_t* data, size_t amount) final;
						void flush() final;
				};

				/**
				 * @class Ext2Directory
				 * @brief Handles the directory operations on the ext2 filesystem
				 */
				class Ext2Directory final : public Directory {

					private:
						Ext2Volume* m_volume;
						InodeHandler m_inode;

						common::Vector<directory_entry_t> m_entries;
						common::Vector<string> m_entry_names;

						void write_entries();
						directory_entry_t create_entry(const string &name, uint32_t inode, bool is_directory = false);

						void parse_block(common::buffer_t* buffer);

						void remove_entry(const string &name, bool is_directory, bool clear = true);
						void rename_entry(const string &old_name, const string &new_name, bool is_directory);

					public:
						Ext2Directory(Ext2Volume* volume, uint32_t inode, const string &name);
						~Ext2Directory() final;

						void read_from_disk() final;

						File* create_file(const string &name) final;
						void remove_file(const string &name) final;
						void rename_file(const string &old_name, const string &new_name) final;

						Directory* create_subdirectory(const string &name) final;
						void remove_subdirectory(const string &name) final;
						void rename_subdirectory(const string &old_name, const string &new_name) final;
				};

				/**
				 * @class Ext2FileSystem
				 * @brief Handles the ext2 filesystem operations
				 */
				class Ext2FileSystem final : public FileSystem {
					private:
						Ext2Volume m_volume;

					public:
						Ext2FileSystem(drivers::disk::Disk* disk, uint32_t partition_offset);
						~Ext2FileSystem() final;
				};

			}
		}
	}
}

#endif // MAXOS_FILESYSTEM_EXT2_H
