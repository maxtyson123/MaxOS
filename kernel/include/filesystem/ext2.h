//
// Created by 98max on 17/07/2025.
//

#ifndef MAXOS_FILESYSTEM_EXT2_H
#define MAXOS_FILESYSTEM_EXT2_H

#include <drivers/disk/disk.h>
#include <filesystem/filesystem.h>
#include <stdint.h>
#include <memory/memoryIO.h>
#include <common/spinlock.h>
#include <drivers/clock/clock.h>

namespace MaxOS {
  namespace filesystem {
	  namespace ext2{


		  typedef struct SuperBlock{
			  uint32_t total_inodes;
			  uint32_t total_blocks;
			  uint32_t reserved_blocks;
			  uint32_t unallocated_blocks;
			  uint32_t unallocated_inodes;
			  uint32_t starting_block;
			  uint32_t block_size;
			  uint32_t fragment_size;
			  uint32_t blocks_per_group;
			  uint32_t fragments_per_group;
			  uint32_t inodes_per_group;
			  uint32_t last_mount_time;
			  uint32_t late_write_time;
			  uint16_t mounts_since_check;
			  uint16_t mounts_until_check;
			  uint16_t signature;
			  uint16_t state;
			  uint16_t error_operation;
			  uint16_t version_minor;
			  uint32_t last_check_time;
			  uint32_t time_until_check;
			  uint32_t os_id;
			  uint32_t version_major;
			  uint16_t reserved_user;
			  uint16_t reserved_group;

			  // Extended Fields (version >= 1)
			  uint32_t first_inode;
			  uint16_t inode_size;
			  uint16_t superblock_group;
			  uint32_t optional_features;
			  uint32_t required_features;
			  uint16_t read_only_features;
			  uint8_t  filesystem_id[16];
			  uint8_t  volume_name[16];
			  uint8_t  last_mount_path[64];
			  uint32_t compression;
			  uint8_t  file_preallocation_blocks;
			  uint8_t  directory_preallocation_blocks;
			  uint16_t unused;
			  uint8_t  journal_id[16];
			  uint32_t journal_inode;
			  uint32_t journal_device;
			  uint32_t orphan_inodes_start;
			  uint8_t  free[276];

		  } __attribute__((packed)) superblock_t;


		  enum class FileSystemState{
			  CLEAN = 1,
			  ERROR = 2,
		  };

		  enum class ErrorOperation{
			  IGNORE    = 1,
			  REMOUNT   = 2,
			  PANIC     = 3,
		  };

		  enum class CreatorOS{
			  LINUX,
			  GNU_HURD,
			  MASIX,
			  FREE_BSD,
			  OTHER_LITES,
		  };

		  enum class OptionalFeatures{
			  PREALLOCATE_DIRECTORY = 0x1,
			  AFS_SERVER_INODES     = 0x2,
			  JOURNAL_ENABLED       = 0x4,
			  ATTRIBUTES_EXTENDED   = 0x8,
			  RESIZEABLE            = 0x10,
			  HASH_INDEXING         = 0x20,
		  };

		  enum class RequiredFeatures {
			  COMPRESSION           = 0x1,
			  DIRECTORY_HAS_TYPE    = 0x2,
			  MUST_REPLAY_JOURNAL   = 0x4,
			  JOURNAL_DEVICE        = 0x8,
		  };

		  enum class ReadOnlyFeatures {
			  SPARSE_SUPER_BLOCKS       = 0x1,
			  FILES_64_BIT              = 0x2,
			  BINARY_TREE_DIRECTORIES   = 0x4,
		  };

		  typedef struct BlockGroupDescriptor{
			  uint32_t block_usage_bitmap;
			  uint32_t block_inode_bitmap;
			  uint32_t inode_table_address;
			  uint16_t free_blocks;
			  uint16_t free_inodes;
			  uint16_t directory_count;
			  uint8_t free[14];

		  } __attribute__((packed)) block_group_descriptor_t;

		  typedef struct Inode{
			  union {
				  uint16_t type_permissions;
				  struct {
					  uint16_t permissions : 12;
					  uint16_t type        : 4;
				  };
			  };
			  uint16_t user_id;
			  uint32_t size_lower;
			  uint32_t last_access_time;
			  uint32_t creation_time;
			  uint32_t last_modification_time;
			  uint32_t deletion_time;
			  uint16_t group_id;
			  uint16_t hard_links;
			  uint32_t sectors_used;
			  uint32_t flags;
			  uint32_t os_1;
			  uint32_t block_pointers[12];
			  uint32_t l1_indirect;
			  uint32_t l2_indirect;
			  uint32_t l3_indirect;
			  uint32_t generation;
			  uint32_t extended_attribute; // File ACL
			  uint32_t size_upper;         // Dir  ACL
			  uint32_t os_2[3];
		  } __attribute__((packed)) inode_t;

		  enum class InodeType {
			  UNKNOWN,
			  FILE,
			  DIRECTORY,
			  CHARACTER_DEVICE,
			  BLOCK_DEVICE,
			  FIFO,
			  SOCKET,
			  SYMBOLIC_LINK
		  };

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

		  enum class InodeFlags{
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

		  // TODO: Also HURD, MASIX
		  typedef struct InodeOS2Linux{
			  uint8_t fragment;
			  uint8_t fragment_size;
			  uint16_t high_type_permissions;
			  uint16_t high_user_id;
			  uint16_t high_group_id;
			  uint32_t author_id;   //0xFFFFFFFF = use user_id
		  } __attribute__((packed)) linux_os_2_t;

		  typedef struct DirectoryEntry{
			  uint32_t inode;
			  uint16_t size;
			  uint8_t name_length;
			  uint8_t type;
			  // Rest are name chars
		  } __attribute__((packed)) directory_entry_t;

		  class Ext2Volume {

			  private:
			     common::Vector<uint32_t>  allocate_group_blocks(uint32_t block_group, uint32_t amount);
				 void write_back_block_groups();
				 void write_back_superblock();

			  public:
				  Ext2Volume(drivers::disk::Disk* disk, lba_t partition_offset);
				  ~Ext2Volume();

				  drivers::disk::Disk* disk;
				  lba_t partition_offset;

				  superblock_t superblock;
				  block_group_descriptor_t** block_groups;

				  size_t    block_size;
				  uint32_t  block_group_descriptor_table;
				  uint32_t  block_group_descriptor_table_size;
				  uint32_t  total_block_groups;
				  size_t    pointers_per_block;
				  uint32_t  inodes_per_block;
				  uint32_t  sectors_per_block;

				  uint32_t  blocks_per_inode_table;
				  uint32_t  sectors_per_inode_table;

				  common::Spinlock ext2_lock;

				  void                       write_block(uint32_t block_num, uint8_t* buffer);
				  void                       write_inode(uint32_t inode_num, inode_t* inode);
			      [[nodiscard]] uint32_t     create_inode(bool is_directory);

				  void                      read_block(uint32_t block_num, uint8_t* buffer) const;
				  [[nodiscard]] inode_t     read_inode(uint32_t inode_num) const;
				  block_group_descriptor_t  read_block_group(uint32_t group_num);

			      [[nodiscard]] uint32_t                  allocate_block();
			      [[nodiscard]] common::Vector<uint32_t>  allocate_blocks(uint32_t amount);
			      [[nodiscard]] uint32_t                  bytes_to_blocks(size_t bytes) const;

			  // TODO: free blocks
		  };

		  /**
		   * @class InodeHandler
		   * @brief Simplfies the management of an inode & its blocks
		   */
		   class InodeHandler {

			   private:
			        Ext2Volume* m_volume = nullptr;

				   void parse_indirect(uint32_t level, uint32_t block, uint8_t* buffer);
				   void write_indirect(uint32_t level, uint32_t& block, size_t& index);
				   void store_blocks(const common::Vector<uint32_t>& blocks);

			   public:
			        InodeHandler(Ext2Volume* volume, uint32_t inode);
					~InodeHandler();

					uint32_t inode_number;
					inode_t inode;
					common::Vector<uint32_t> block_cache;

					[[nodiscard]] size_t size() const;
					void set_size(size_t size);
					size_t grow(size_t amount, bool flush = true);

					void save();

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
				  Ext2File(Ext2Volume* volume, uint32_t inode, const string& name);
				  ~Ext2File() final;

				  void write(const uint8_t* data, size_t amount) final;
				  void read(uint8_t* data, size_t amount) final;
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
			      directory_entry_t create_entry(const string& name, uint32_t inode, bool is_directory = false);

				  void parse_block(uint8_t* buffer);

			  public:
				  Ext2Directory(Ext2Volume* volume, uint32_t inode, const string& name);
				  ~Ext2Directory() final;

				  void read_from_disk() final;

				  File* create_file(const string& name) final;
				  void remove_file(const string& name) final;

				  Directory* create_subdirectory(const string& name) final;
				  void remove_subdirectory(const string& name) final;
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

#endif // MAXOS_FILESYSTEM_EXT2_H
