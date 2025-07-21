//
// Created by 98max on 17/07/2025.
//
#include <filesystem/ext2.h>

using namespace MaxOS;
using namespace MaxOS::filesystem;
using namespace MaxOS::filesystem::ext2;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::disk;
using namespace MaxOS::drivers::clock;

Ext2Volume::Ext2Volume(drivers::disk::Disk *disk, lba_t partition_offset)
: disk(disk),
  partition_offset(partition_offset)
{

	ext2_lock.unlock();

	// Read superblock
	uint8_t buffer[1024];
	disk->read(partition_offset + 2, buffer, 512);
	disk->read(partition_offset + 3, buffer + 512, 512);
	memcpy(&superblock, buffer, sizeof(superblock_t));

	// Validate signature
	ASSERT(superblock.signature == 0xEF53, "Ext2 Filesystem doesnt have a valid signature\n");

	// Version 0 has constant inode info
	if(superblock.version_major < 1){
		superblock.first_inode = 11;
		superblock.inode_size  = 128;
	}

	// Parse the superblock
	block_size = 1024 << superblock.block_size;
	total_block_groups = (superblock.total_blocks + superblock.blocks_per_group - 1) / superblock.blocks_per_group;
	block_group_descriptor_table = (block_size == 1024) ? 2 : 1;
	block_group_descriptor_table_size = total_block_groups * sizeof(block_group_descriptor_t);
	pointers_per_block = block_size / sizeof(uint32_t);
	inodes_per_block = block_size / superblock.block_size;
	sectors_per_block = block_size / 512;
	blocks_per_inode_table = (superblock.inode_size * superblock.inodes_per_group) / block_size;
	sectors_per_inode_table = (superblock.inode_size * superblock.inodes_per_group) / 512;

	// Read the block groups
	block_groups = new block_group_descriptor_t*[total_block_groups] {nullptr};
	uint32_t sectors_to_read = (block_group_descriptor_table_size + block_size - 1) / block_size * sectors_per_block;
	auto* bg_buffer = new uint8_t[sectors_to_read * 512];
	for (uint32_t i = 0; i < sectors_to_read; ++i)
		disk->read(partition_offset + block_group_descriptor_table * sectors_per_block + i, bg_buffer + i * 512, 512);

	// Store the block groups
	for (uint32_t i = 0; i < total_block_groups; ++i) {
		block_groups[i] = new block_group_descriptor_t;
		memcpy(block_groups[i], bg_buffer + i * sizeof(block_group_descriptor_t), sizeof(block_group_descriptor_t));
	}
	delete[] bg_buffer;

}

Ext2Volume::~Ext2Volume() = default;

/**
 * @breif Write a single block from a buffer into onto the disk
 *
 * @param block_num The block to update
 * @param buffer The buffer to read from
 */
void Ext2Volume::write_block(uint32_t block_num, uint8_t *buffer) {

	// Read each sector of the block
	for (size_t i = 0; i < sectors_per_block; ++i)
		disk->write(partition_offset + block_num * sectors_per_block + i, buffer + i * 512, 512);

};

/**
 * @brief Write an inode to the filesystem
 *
 * @param inode_num The inode index
 * @param inode The inode to read from
 */
void Ext2Volume::write_inode(uint32_t inode_num, inode_t* inode) {

	// Locate the inode
	uint32_t group = (inode_num - 1) / superblock.inodes_per_group;
	uint32_t index = (inode_num - 1) % superblock.inodes_per_group;

	// Locate the block
	uint32_t inode_table        = block_groups[group] -> inode_table_address;
	uint32_t offset             = index * superblock.inode_size;
	uint32_t block              = offset / block_size;
	uint32_t in_block_offset    = offset % block_size;

	// Read the inode
	auto* buffer = new uint8_t[block_size];
	read_block(inode_table + block, buffer);
	memcpy(buffer + in_block_offset, inode, sizeof(inode_t));

	// Modify the block
	write_block(inode_table + block, buffer);
	delete[] buffer;
}

/**
 * @breif Reads a single block from the disk into a buffer
 *
 * @param block_num The block to read
 * @param buffer The buffer to read into
 */
void Ext2Volume::read_block(uint32_t block_num, uint8_t *buffer) const {

	// Read each sector of the block
	for (size_t i = 0; i < sectors_per_block; ++i)
		disk->read(partition_offset + block_num * sectors_per_block + i, buffer + i * 512, 512);

}

/**
 * @brief Read an inode from the filesystem
 *
 * @param inode_num The inode index
 */
inode_t Ext2Volume::read_inode(uint32_t inode_num) const {

	inode_t inode;

	// Locate the inode
	uint32_t group = (inode_num - 1) / superblock.inodes_per_group;
	uint32_t index = (inode_num - 1) % superblock.inodes_per_group;

	// Locate the block
	uint32_t inode_table        = block_groups[group] -> inode_table_address;
	uint32_t offset             = index * superblock.inode_size;
	uint32_t block              = offset / block_size;
	uint32_t in_block_offset    = offset % block_size;

	// Read the inode
	auto* buffer = new uint8_t[block_size];
	read_block(inode_table + block, buffer);
	memcpy(&inode, buffer + in_block_offset, sizeof(inode_t));

	delete[] buffer;
	return inode;
}

/**
 * @brief Read a blockgroup from the disk
 *
 * @param group_num The group to read
 * @return The block group descriptor for the specified group
 */
block_group_descriptor_t Ext2Volume::read_block_group(uint32_t group_num) {
	block_group_descriptor_t descriptor;

	// Locate the group
	uint32_t offset             = group_num * sizeof(block_group_descriptor_t);
	uint32_t block              = offset / block_size;
	uint32_t in_block_offset    = offset % block_size;

	// Read the block group
	auto* buffer = new uint8_t[block_size];
	read_block(2 + block, buffer);
	memcpy(&descriptor, buffer + in_block_offset, sizeof(block_group_descriptor_t));

	delete[] buffer;
	return descriptor;
}

Ext2File::Ext2File(Ext2Volume *volume, uint32_t inode, string const &name)
: m_volume(volume),
  m_inode_number(inode),
  m_inode()
{

	// Set up the base information
	m_name = name;
	m_inode = m_volume ->read_inode(m_inode_number);
	m_size = (size_t)m_inode.size_upper | m_inode.size_lower;

	// Read the block pointers
	for (uint32_t direct_pointer = 0; direct_pointer < 12; ++direct_pointer)
		m_block_pointers.push_back(m_inode.block_pointers[direct_pointer]);

	auto* buffer = new uint8_t[m_volume -> block_size];
	parse_indirect(1, m_inode.l1_indirect, buffer);
	parse_indirect(2, m_inode.l2_indirect, buffer);
	parse_indirect(3, m_inode.l3_indirect, buffer);
	delete[] buffer;
}

/**
 * @brief Write data to the file (at the current seek position, updated to be += amount)
 *
 * @param data The byte buffer to write
 * @param amount The amount of data to write
 */
void Ext2File::write(uint8_t const *data, size_t amount) {

	// Nothing to write
	if(m_size == 0)
		return;

	// Prepare for writing
	m_volume -> ext2_lock.lock();
	auto* buffer = new uint8_t[m_volume -> block_size];

	// Force bounds
	if(m_offset + amount > m_size)
		amount = m_size - m_offset;

	// Convert bytes to blocks
	uint32_t block_start    = m_offset / m_volume -> block_size;
	uint32_t block_offset   = m_offset % m_volume -> block_size;

	// TODO: Expand the file

	// Save the updated metadata
	m_inode.size_lower = (uint32_t)(m_size  & 0xFFFFFFFFULL);
	m_inode.size_upper = (uint32_t)(m_size  >> 32);
	m_inode.last_modification_time = time_to_epoch(Clock::active_clock() -> get_time());
	m_volume -> write_inode(m_inode_number, &m_inode);

	// Read each block
	size_t current_block = block_start;
	for (size_t remaining = amount; remaining > 0; remaining -= m_volume -> block_size) {

		// Read the block
		uint32_t block = m_block_pointers[current_block];
		m_volume -> read_block(block, buffer);
		current_block++;

		// Data may be part-way through the first block
		if((current_block - 1) == block_start){

			size_t block_remaining = m_volume -> block_size - block_offset;

			// Replacement ends partway through block
			if(amount < block_remaining){
				memcpy(buffer + block_offset, data, amount);
				m_volume ->write_block(block, buffer);
				break;
			}

			// Replace the entire rest of the block
			memcpy(buffer + block_offset, data, block_remaining);
			m_volume ->write_block(block, buffer);
			remaining += block_offset;
			continue;
		}

		size_t block_remaining = amount - remaining;

		// Last part of the data to be replaced is part-way through the block
		if(remaining < m_volume -> block_size){
			memcpy(buffer + block_remaining, data, remaining);
			m_volume ->write_block(block, buffer);
			break;
		}

		// Replace the entire block
		memcpy(buffer + block_remaining, data, m_volume -> block_size);
		m_volume ->write_block(block, buffer);
	}

	// Clean up
	m_offset += amount;
	m_volume -> ext2_lock.unlock();
	delete[] buffer;
}

/**
* @brief Read data from the file (at the current seek position, updated to be += amount)
*
* @param data The byte buffer to read into
* @param amount The amount of data to read
*/
void Ext2File::read(uint8_t* data, size_t amount) {

	// Nothing to read
	if(m_size == 0 || amount == 0)
		return;

	// Prepare for reading
	m_volume -> ext2_lock.lock();
	auto* buffer = new uint8_t[m_volume -> block_size];

	// Force bounds
	if(m_offset + amount > m_size)
		amount = m_size - m_offset;

	// Convert bytes to blocks
	uint32_t block_start    = m_offset / m_volume -> block_size;
	uint32_t block_offset   = m_offset % m_volume -> block_size;

	// Read each block
	size_t current_block = block_start;
	for (size_t remaining = amount; remaining > 0; remaining -= m_volume -> block_size) {

		// Read the block
		m_volume -> read_block(m_block_pointers[current_block], buffer);

		// Data may be part-way through the first block
		if(current_block == block_start){

			size_t block_remaining = m_volume -> block_size - block_offset;

			// Read ends partway through block
			if(amount < block_remaining){
				memcpy(data, buffer + block_offset, amount);
				break;
			}

			// Copy the entire rest of the block
			memcpy(data, buffer + block_offset, block_remaining);
			remaining += block_offset;
			continue;
		}

		size_t block_remaining = amount - remaining;

		// Last part of the data is part-way through the block
		if(remaining < m_volume -> block_size){
			memcpy(data, buffer + block_remaining, remaining);
			break;
		}

		// Copy the entire block
		memcpy(data, buffer + block_remaining, m_volume -> block_size);
	}

	// Clean up
	m_offset += amount;
	m_volume -> ext2_lock.unlock();
	delete[] buffer;
}

/**
 * @brief Flush the file to the disk
 */
void Ext2File::flush() {
	File::flush();
}

/**
 * @brief Caches the indirect layers block pointers
 *
 * @param level Recursion level
 * @param block The block number to parse from
 * @param buffer Buffer to read into
 */
void Ext2File::parse_indirect(uint32_t level, uint32_t block, uint8_t *buffer) {

	// Invalid
	if(block == 0)
		return;

	// Read the block
	m_volume -> read_block(block, buffer);
	auto* pointers = (uint32_t*)buffer;

	// Parse the pointers
	for (size_t i = 0; i < m_volume->pointers_per_block; ++i) {
		uint32_t pointer = pointers[i];

		// Invaild
		if(pointer == 0)
			break;

		// Has indirect sub entries
		if(level > 1){
			parse_indirect(level - 1, pointer, buffer);
			continue;
		}

		// Parse the entry
		m_block_pointers.push_back(pointer);
	}
}

Ext2File::~Ext2File() = default;


Ext2Directory::Ext2Directory(Ext2Volume *volume, uint32_t inode, const string& name)
: m_volume(volume),
  m_inode_number(inode),
  m_inode()
{
	m_name = name;
}

/**
 * @brief Store all entries from a buffer and convert to File or Directory objects
 */
void Ext2Directory::parse_entries(uint8_t* buffer) {

	size_t offset = 0;
	while (offset < m_volume -> block_size){

		// Read the entry
		auto* entry = (directory_entry_t*)(buffer + offset);
		m_entries.push_back(*entry);

		// Not valid
		if (entry -> inode == 0 || entry -> name_length == 0)
			break;

		// Parse
		string filename(buffer + offset + sizeof(directory_entry_t), entry->name_length);
		uint32_t inode = entry->inode;

		// Create the object
		switch ((InodeType)entry->type) {

			case InodeType::FILE:
				m_files.push_back(new Ext2File(m_volume, inode, filename));
				break;

			case InodeType::DIRECTORY:
				m_subdirectories.push_back(new Ext2Directory(m_volume, inode, filename));
				break;

			default:
				Logger::WARNING() << "Unknown entry type: " << entry->type << "\n";

		}

		// Go to next
		offset += entry -> size;
	}

}

/**
 * @brief Recursively parses the indirect entries
 *
 * @param level Recursion level
 * @param block The block number to parse from
 * @param buffer Buffer to read into
 */
void Ext2Directory::parse_indirect(uint32_t level, uint32_t block, uint8_t* buffer) {

	// Invalid
	if(block == 0)
		return;

	// Read the block
	m_volume -> read_block(block, buffer);
	auto* pointers = (uint32_t*)buffer;

	// Parse the pointers
	for (size_t i = 0; i < m_volume->pointers_per_block; ++i) {
		uint32_t pointer = pointers[i];

		// Invaild
		if(pointer == 0)
			break;

		// Has indirect sub entries
		if(level > 1){
			parse_indirect(level - 1, pointer, buffer);
			continue;
		}

		// Parse the entry
		m_volume ->read_block(pointer, buffer);
		parse_entries(buffer);
	}
}

/**
 * @brief Read the directory from the inode on the disk
 */
void Ext2Directory::read_from_disk() {

	m_volume -> ext2_lock.lock();

	// Read the inode
	m_inode = m_volume ->read_inode(m_inode_number);

	// Clear the old files & Directories
	for(auto& file : m_files)
		delete file;
	m_files.clear();

	for(auto& directory : m_subdirectories)
		delete directory;
	m_subdirectories.clear();

	// Read the direct blocks (cant use for( : ))
	auto* buffer = new uint8_t[m_volume -> block_size];
	for (int i = 0; i < 12; ++i) {
		uint32_t block_pointer = m_inode.block_pointers[i];

		// Invalid block
		if(block_pointer == 0)
			break;

		// Parse the block
		m_volume->read_block(block_pointer, buffer);
		parse_entries(buffer);
	}

	// Indirect blocks
	parse_indirect(1, m_inode.l1_indirect, buffer);
	parse_indirect(2, m_inode.l2_indirect, buffer);
	parse_indirect(3, m_inode.l3_indirect, buffer);

	m_volume -> ext2_lock.unlock();
	delete[] buffer;
}

/**
 * @brief Create a new file in the directory
 *
 * @param name The name of the file to create
 * @return The new file object or null if it could not be created
 */
File *Ext2Directory::create_file(string const &name) {
	// TODO
	return nullptr;
}

/**
 * @brief Delete a file from the subdirectory
 *
 * @param name The name of the file to delete
 */
void Ext2Directory::remove_file(string const &name) {
	Directory::remove_file(name);
}

/**
 * @brief Create a new directory in the directory
 *
 * @param name The name of the directory to create
 * @return The new directory object or null if it could not be created
 */
Directory *Ext2Directory::create_subdirectory(string const &name) {
	return Directory::create_subdirectory(name);
}

/**
 * @brief Remove a directory entry from the directory
 *
 * @param name The name of the entry to remove
 */
void Ext2Directory::remove_subdirectory(string const &name) {
	Directory::remove_subdirectory(name);
}

Ext2Directory::~Ext2Directory() = default;


Ext2FileSystem::Ext2FileSystem(Disk *disk, uint32_t partition_offset)
: m_volume(disk, partition_offset)
{

	// Create the root directory
	m_root_directory = new Ext2Directory(&m_volume, 2, "/");
	m_root_directory -> read_from_disk();

}

Ext2FileSystem::~Ext2FileSystem(){
	delete m_root_directory;
};