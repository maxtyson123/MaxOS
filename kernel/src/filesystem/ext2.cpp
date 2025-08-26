//
// Created by 98max on 17/07/2025.
//
#include <filesystem/ext2.h>

using namespace MaxOS;
using namespace MaxOS::filesystem;
using namespace MaxOS::common;
using namespace MaxOS::filesystem::ext2;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::disk;
using namespace MaxOS::drivers::clock;

Ext2Volume::Ext2Volume(drivers::disk::Disk *disk, lba_t partition_offset)
: disk(disk),
  partition_offset(partition_offset)
{

	// Read superblock
	buffer_t superblock_buffer(&superblock, 1024);
	disk->read(partition_offset + 2, &superblock_buffer, 512);
	disk->read(partition_offset + 3, &superblock_buffer, 512);

	// Validate signature
	ASSERT(superblock.signature == 0xEF53, "Ext2 Filesystem doesnt have a valid signature\n");

	// Version 0 has constant inode info
	if (superblock.version_major < 1) {
		superblock.first_inode = 11;
		superblock.inode_size = 128;
	}

	// Parse the superblock
	block_size = 1024 << superblock.block_size;
	total_block_groups = (superblock.total_blocks + superblock.blocks_per_group - 1) / superblock.blocks_per_group;
	block_group_descriptor_table_block = superblock.starting_block + 1;
	block_group_descriptor_table_size = total_block_groups * sizeof(block_group_descriptor_t);
	pointers_per_block = block_size / sizeof(uint32_t);
	inodes_per_block = block_size / superblock.inode_size;
	sectors_per_block = block_size / 512;
	blocks_per_inode_table = (superblock.inode_size * superblock.inodes_per_group + (block_size - 1)) / block_size;
	sectors_per_inode_table = (superblock.inode_size * superblock.inodes_per_group + (512 - 1)) / 512;

	// Read the block groups
	block_groups = new block_group_descriptor_t *[total_block_groups]{nullptr};
	uint32_t bgdt_lba = partition_offset + block_group_descriptor_table_block * sectors_per_block;
	uint32_t sectors_to_read = (block_group_descriptor_table_size + block_size - 1) / block_size * sectors_per_block;
	buffer_t bg_buffer(sectors_to_read * 512);
	for (uint32_t i = 0; i < sectors_to_read; ++i)
		disk->read(bgdt_lba + i, &bg_buffer, 512);

	// Store the block groups
	for (uint32_t i = 0; i < total_block_groups; ++i) {
		block_groups[i] = new block_group_descriptor_t;
		memcpy(block_groups[i], bg_buffer.raw() + i * sizeof(block_group_descriptor_t), sizeof(block_group_descriptor_t));
	}
}

Ext2Volume::~Ext2Volume() = default;

/**
 * @breif Write a single block from a buffer into onto the disk
 *
 * @param block_num The block to update
 * @param buffer The buffer to read from
 */
void Ext2Volume::write_block(uint32_t block_num, buffer_t *buffer) {

	// Ensure the buffer is in the right format
	buffer->set_offset(0);
	bool old = buffer->update_offset;
	buffer->update_offset = true;

	// Read each sector of the block
	for (size_t i = 0; i < sectors_per_block; ++i)
		disk->write(partition_offset + block_num * sectors_per_block + i, buffer, 512);

	// Reset buffer
	buffer->set_offset(0);
	buffer->update_offset = old;
};

/**
 * @brief Write an inode to the filesystem
 *
 * @param inode_num The inode index
 * @param inode The inode to read from
 */
void Ext2Volume::write_inode(uint32_t inode_num, inode_t *inode) {

	// Locate the inode
	uint32_t group = (inode_num - 1) / superblock.inodes_per_group;
	uint32_t index = (inode_num - 1) % superblock.inodes_per_group;

	// Locate the block
	uint32_t inode_table = block_groups[group]->inode_table_address;
	uint32_t offset = index * superblock.inode_size;
	uint32_t block = offset / block_size;
	uint32_t in_block_offset = offset % block_size;

	// Read the inode
	buffer_t buffer(block_size);
	read_block(inode_table + block, &buffer);
	buffer.copy_from(inode, sizeof(inode_t), in_block_offset);

	// Modify the block
	write_block(inode_table + block, &buffer);
}

/**
 * @breif Reads a single block from the disk into a buffer
 *
 * @param block_num The block to read
 * @param buffer The buffer to read into
 */
void Ext2Volume::read_block(uint32_t block_num, buffer_t *buffer) const {

	// Ensure the buffer is in the right format
	buffer->set_offset(0);

	// Read each sector of the block
	for (size_t i = 0; i < sectors_per_block; ++i)
		disk->read(partition_offset + block_num * sectors_per_block + i, buffer, 512);

	// Reset buffer
	buffer->set_offset(0);

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
	uint32_t inode_table = block_groups[group]->inode_table_address;
	uint32_t offset = index * superblock.inode_size;
	uint32_t block = offset / block_size;
	uint32_t in_block_offset = offset % block_size;

	// Read the block
	buffer_t buffer(block_size);
	read_block(inode_table + block, &buffer);

	// Read the inode from the block
	buffer.copy_to(&inode, sizeof(inode_t), in_block_offset);
	return inode;
}

/**
 * @brief Allocates a single block to be used by an inode
 *
 * @return The new block number or 0 if the allocation failed
 */
uint32_t Ext2Volume::allocate_block() {

	return allocate_blocks(1)[0];
}

/**
 * @brief Allocates a set of blocks to be used by an inode
 *
 * @param amount The amount of blocks to allocate
 * @return A list of the allocated blocks or [0] if the allocation failed
 */
Vector<uint32_t> Ext2Volume::allocate_blocks(uint32_t amount) {

	// No blocks to allocate
	if (!amount)
		return {1, 0};

	// Find the block group with enough free blocks
	block_group_descriptor_t *block_group = block_groups[0];
	for (uint32_t bg_index = 0; bg_index < total_block_groups; block_group = block_groups[++bg_index])
		if (block_group->free_blocks >= amount)
			return allocate_group_blocks(bg_index, amount);

	// No block group can contain the block so split across multiple
	Vector<uint32_t> result{};
	while (amount > 0) {

		// Find the block group with most free blocks
		block_group = block_groups[0];
		uint32_t bg_index = 0;
		for (; bg_index < total_block_groups; ++bg_index)
			if (block_groups[bg_index]->free_blocks > block_group->free_blocks)
				block_group = block_groups[bg_index];

		// No space
		if (block_group->free_blocks == 0)
			return {1, 0};

		// Allocate the remaining blocks
		auto allocated = allocate_group_blocks(bg_index, 1);
		amount -= allocated.size();
		for (auto block: allocated)
			result.push_back(block);
	}

	return result;
}

/**
 * @brief Allocate a certain amount of blocks in a block group
 *
 * @param block_group The block group where the allocation is performed
 * @param amount The amount of blocks to allocate
 * @return The block numbers allocated
 */
common::Vector<uint32_t> Ext2Volume::allocate_group_blocks(uint32_t block_group, uint32_t amount) {

	// Ensure enough space
	block_group_descriptor_t *descriptor = block_groups[block_group];
	if (amount > descriptor->free_blocks)
		return {1, 0};

	// Prepare
	Vector<uint32_t> result{};
	buffer_t zeros(block_size);
	zeros.clear();

	// Read bitmap
	buffer_t bitmap(block_size);
	read_block(descriptor->block_usage_bitmap, &bitmap);

	// Allocate the blocks
	for (uint32_t i = 0; i < superblock.blocks_per_group; ++i) {

		// Block is already used
		if ((bitmap.raw()[i / 8] & (1u << (i % 8))) != 0)
			continue;

		// Mark as used
		descriptor->free_blocks--;
		superblock.unallocated_blocks--;
		bitmap.raw()[i / 8] |= (uint8_t) (1u << (i % 8));

		// Zero out data
		uint32_t block = block_group * superblock.blocks_per_group + superblock.starting_block + i;
		write_block(block, &zeros);
		result.push_back(block);

		// All done
		amount--;
		if (!amount)
			break;
	}

	// Save the changed metadata
	write_block(descriptor->block_usage_bitmap, &bitmap);
	write_back_block_groups();
	write_back_superblock();

	return result;
}

/**
 * @brief Free a certain amount of blocks in a block group and zero's them out.
 *
 * @param block_group The block group where the blocks exist
 * @param amount The amount of blocks to free
 */
void Ext2Volume::free_group_blocks(uint32_t block_group, uint32_t amount, uint32_t start) {

	// Read bitmap
	block_group_descriptor_t *descriptor = block_groups[block_group];
	buffer_t bitmap(block_size);
	read_block(descriptor->block_usage_bitmap, &bitmap);

	// Convert start to be index based on the group instead of global
	start -= (block_group * superblock.blocks_per_group + superblock.starting_block);

	// Free the blocks
	for (uint32_t i = start; i < start + amount; ++i) {

		// Block is already free (shouldn't happen)
		if ((bitmap.raw()[i / 8] & (1u << (i % 8))) == 0)
			continue;

		// Mark as free
		descriptor->free_blocks++;
		superblock.unallocated_blocks++;
		bitmap.raw()[i / 8] &= ~(1u << (i % 8));

		// TODO: Decide whether to zero out or not, my implementation zeros on allocation but idk about others
	}

	// Save the changed metadata
	write_block(descriptor->block_usage_bitmap, &bitmap);
	write_back_block_groups();
	write_back_superblock();

}


/**
 * @brief Save any changes of the block groups to the disk
 */
void Ext2Volume::write_back_block_groups() const {

	// Locate the block groups
	uint32_t bgdt_blocks = (block_group_descriptor_table_size + block_size - 1) / block_size;
	uint32_t sectors_to_write = bgdt_blocks * sectors_per_block;
	uint32_t bgdt_lba = partition_offset + block_group_descriptor_table_block * sectors_per_block;

	// Copy the block groups into the buffer
	buffer_t bg_buffer(sectors_to_write * 512);
	for (uint32_t i = 0; i < total_block_groups; ++i)
		bg_buffer.copy_from(block_groups[i], sizeof(block_group_descriptor_t));

	// Write the buffer to disk
	bg_buffer.set_offset(0);
	for (uint32_t i = 0; i < sectors_to_write; ++i)
		disk->write(bgdt_lba + i, &bg_buffer, 512);
}

/**
 * @brief Save the in memory superblock to the disk
 */
void Ext2Volume::write_back_superblock() {

	// Store superblock
	buffer_t buffer(1024);
	buffer.copy_from(&superblock, sizeof(superblock_t));
	buffer.set_offset(0);

	// Write to disk
	disk->write(partition_offset + 2, &buffer, 512);
	disk->write(partition_offset + 3, &buffer, 512);
}

/**
 * @brief How many blocks are needed to contain a set amount of bytes
 *
 * @param bytes Bytes needed
 * @return The blocks required
 */
uint32_t Ext2Volume::bytes_to_blocks(size_t bytes) const {
	return (bytes + block_size - 1) / block_size;
}

/**
 * @brief Allocates a new inode and sets the base metadata. Also allocates block 0 of the inode
 *
 * @param is_directory is the inode to be used for a directory
 * @return The new inode
 */
uint32_t Ext2Volume::create_inode(bool is_directory) {

	ext2_lock.lock();

	// Find the block group with enough free inodes
	block_group_descriptor_t *block_group = block_groups[0];
	uint32_t bg_index = 0;
	for (; bg_index < total_block_groups; block_group = block_groups[++bg_index])
		if (block_group->free_inodes >= 1)
			break;

	// Read bitmap
	buffer_t bitmap(block_size);
	read_block(block_group->block_inode_bitmap, &bitmap);

	// First group contains reserved inodes
	uint32_t inode_index = 0;
	if (bg_index == 0 && superblock.first_inode > 1)
		inode_index = superblock.first_inode - 1;

	// Find a free inode
	for (; inode_index < superblock.inodes_per_group; ++inode_index) {

		// Block is already used
		if ((bitmap.raw()[inode_index / 8] & (1u << (inode_index % 8))) != 0)
			continue;

		// Mark as used
		block_group->free_inodes--;
		superblock.unallocated_inodes--;
		bitmap.raw()[inode_index / 8] |= (uint8_t) (1u << (inode_index % 8));

		break;
	}

	// Convert into the 1-based inode index in the group
	inode_index += bg_index * superblock.inodes_per_group + 1;

	// Save the changed metadata
	write_block(block_group->block_inode_bitmap, &bitmap);
	write_back_block_groups();
	write_back_superblock();

	// Create the inode
	inode_t inode{};
	inode.creation_time = time_to_epoch(Clock::active_clock()->get_time());
	inode.last_modification_time = time_to_epoch(Clock::active_clock()->get_time());
	inode.block_pointers[0] = allocate_block();
	inode.hard_links = is_directory ? 2 : 1;
	inode.type = ((uint16_t) (is_directory ? InodeType::DIRECTORY : InodeType::FILE) >> 12) & 0xF;
	inode.permissions =
			(uint16_t) (is_directory ? InodePermissionsDefaults::DIRECTORY : InodePermissionsDefaults::FILE) & 0x0FFF;
	write_inode(inode_index, &inode);

	ext2_lock.unlock();
	return inode_index;
}

/**
 * @brief Mark an inode as free. Note: does NOT unallocated the inodes blocks, use free_group_blocks().
 * @see free_group_blocks
 *
 * @param inode The inode number to mark as free
 */
void Ext2Volume::free_inode(uint32_t inode) {

	// Find the block group containing the inode
	uint32_t bg_index = (inode - 1) / superblock.inodes_per_group;
	block_group_descriptor_t *block_group = block_groups[bg_index];

	// Read bitmap
	buffer_t bitmap(block_size);
	read_block(block_group->block_inode_bitmap, &bitmap);

	// First group contains reserved inodes
	uint32_t inode_index = (inode - 1) % superblock.inodes_per_group;
	if (bg_index == 0 && (inode_index < (superblock.first_inode - 1)))
		return;

	// Mark as used
	block_group->free_inodes++;
	superblock.unallocated_inodes++;
	bitmap.raw()[inode_index / 8] &= (uint8_t) ~(1u << (inode_index % 8));

	// Save the changed metadata
	write_block(block_group->block_inode_bitmap, &bitmap);
	write_back_block_groups();
	write_back_superblock();
}

/**
 * @brief Frees a group of blocks. Preferably with adjacent blocks apearing next to each other but not enforced.
 * @param blocks The blocks to free
 */
void Ext2Volume::free_blocks(const common::Vector<uint32_t> &blocks) {

	// No blocks to free
	if (blocks.empty())
		return;

	uint32_t start = blocks[0];
	uint32_t previous = start;
	uint32_t amount = 1;

	// Free each adjacent set of blocks
	for (auto &block: blocks) {

		// First is already accounted for
		if (block == start)
			continue;

		// Is this block adjacent
		if ((previous + 1) == block) {
			previous = block;
			amount += 1;
			continue;
		}

		// Adjacent set has ended
		uint32_t group = (start - superblock.starting_block) / superblock.blocks_per_group;
		free_group_blocks(group, amount, start);

		// Reset
		start = block;
		previous = start;
		amount = 1;
	}

	// Account for the last set of blocks in the loop
	uint32_t group = (start - superblock.starting_block) / superblock.blocks_per_group;
	free_group_blocks(group, amount, start);
}


InodeHandler::InodeHandler(Ext2Volume *volume, uint32_t inode_index)
: m_volume(volume),
  inode_number(inode_index),
  inode(m_volume->read_inode(inode_number))
{

	// Read the block pointers
	for (uint32_t direct_pointer = 0; direct_pointer < 12; ++direct_pointer)
		if (inode.block_pointers[direct_pointer])
			block_cache.push_back(inode.block_pointers[direct_pointer]);

	buffer_t buffer(m_volume->block_size);
	parse_indirect(1, inode.l1_indirect, &buffer);
	parse_indirect(2, inode.l2_indirect, &buffer);
	parse_indirect(3, inode.l3_indirect, &buffer);
}

/**
 * @brief Read the size upper and lower into a single size_t
 * @return The size of the inode data (not the inode itself)
 */
size_t InodeHandler::size() const {
	return ((size_t) inode.size_upper << 32) | (size_t) inode.size_lower;
}

/**
 * @brief Store the size of the inode data. (does not write to disk)
 * @param size The new size
 */
void InodeHandler::set_size(size_t size) {

	inode.size_lower = (uint32_t) (size & 0xFFFFFFFFULL);
	inode.size_upper = (uint32_t) (size >> 32);

}

/**
 * @brief Caches the indirect layers block pointers
 *
 * @param level Recursion level
 * @param block The block number to parse from
 * @param buffer Buffer to read into
 */
void InodeHandler::parse_indirect(uint32_t level, uint32_t block, buffer_t *buffer) {

	// Invalid
	if (block == 0)
		return;

	// Read the block
	m_volume->read_block(block, buffer);
	auto *pointers = (uint32_t *) (buffer->raw());

	// Parse the pointers
	for (size_t i = 0; i < m_volume->pointers_per_block; ++i) {
		uint32_t pointer = pointers[i];

		// Invaild
		if (pointer == 0)
			break;

		// Has indirect sub entries
		if (level > 1) {
			parse_indirect(level - 1, pointer, buffer);
			continue;
		}

		// Parse the entry
		block_cache.push_back(pointer);
	}
}

/**
 * @brief Writes the Cache to the indirect layer block pointers
 *
 * @param level Recursion level
 * @param block The block number to parse from
 * @param buffer Buffer to read into
 * @param index Current entry to write
 */
void InodeHandler::write_indirect(uint32_t level, uint32_t &block, size_t &index) {

	// Nothing left to write
	size_t remaining = block_cache.size() - index;
	if (remaining == 0 || index >= block_cache.size())
		return;

	// Level hasn't been set yet
	if (block == 0)
		block = m_volume->allocate_block();

	// Allocate a local buffer for this recursion level
	buffer_t buffer(m_volume->block_size);
	buffer.clear();
	auto *pointers = (uint32_t *) buffer.raw();

	// Write the pointers
	for (size_t i = 0; i < m_volume->pointers_per_block; ++i) {

		// Invalid
		if (index >= block_cache.size())
			break;

		// Has indirect
		if (level > 1) {
			write_indirect(level - 1, pointers[i], index);
			continue;
		}

		// Save the pointer
		pointers[i] = block_cache[index++];
	}

	m_volume->write_block(block, &buffer);
}

/**
 * @brief Saves the blocks to both the cached array and on disk inode
 *
 * @param blocks The blocks to save
 */
void InodeHandler::store_blocks(Vector<uint32_t> const &blocks) {

	Logger::DEBUG() << "STORING BLOCKS\n";

	// Store in cache
	for (auto block: blocks)
		block_cache.push_back(block);

	// Direct blocks
	for (uint32_t i = 0; i < 12; ++i)
		inode.block_pointers[i] = i < block_cache.size() ? block_cache[i] : 0;

	// No need to do any indirects
	if (block_cache.size() < 12)
		return;

	// Setup Recursive blocks
	size_t index = 12;

	// Write the blocks
	uint32_t indirect_blocks[3] = {inode.l1_indirect, inode.l2_indirect, inode.l3_indirect};
	for (int i = 0; i < 3; ++i) {

		// Have to use temp because of packed field
		uint32_t temp = indirect_blocks[i];
		write_indirect(i + 1, temp, index);
		indirect_blocks[i] = temp;
	}

	// Save the new blocks
	inode.l1_indirect = indirect_blocks[0];
	inode.l2_indirect = indirect_blocks[1];
	inode.l3_indirect = indirect_blocks[2];

	// NOTE: Blocks get allocated when writing indirects. This is then saved later in the write() function
}

/**
 * @brief Increase the size of the inode's storage capacity by allocating new blocks.
 *
 * @param amount The amount to grow to in bytes
 * @return
 */
size_t InodeHandler::grow(size_t amount, bool flush) {

	// Nothing to grow
	if (amount <= 0)
		return size();

	// Allocate new blocks
	auto blocks = m_volume->allocate_blocks(m_volume->bytes_to_blocks(amount));
	ASSERT(blocks[0] != 0, "Failed to allocate new blocks for file");

	// Save the changes
	store_blocks(blocks);
	set_size(size() + amount);
	if (flush)
		save();

	return size() + amount;
}

/**
 * @brief Writes the inode meta data to disk
 */
void InodeHandler::save() {

	m_volume->write_inode(inode_number, &inode);
}

/**
 * @brief Marks this inode's blocks as free and then the inode as free
 */
void InodeHandler::free() {

	m_volume->ext2_lock.lock();

	// Free the inode
	m_volume->free_blocks(block_cache);
	m_volume->free_inode(inode_number);

	m_volume->ext2_lock.unlock();
}

InodeHandler::~InodeHandler() = default;

Ext2File::Ext2File(Ext2Volume *volume, uint32_t inode, string const &name)
: m_volume(volume),
  m_inode(volume, inode)
{

	// Set up the base information
	m_name = name;
	m_size = m_inode.size();

}

/**
 * @brief Write data to the file (at the current seek position, updated to be += amount)
 *
 * @param data The byte buffer to write
 * @param amount The amount of data to write
 */
void Ext2File::write(buffer_t const *data, size_t amount) {

	// Nothing to write
	if (amount == 0)
		return;

	// Prepare for writing
	m_volume->ext2_lock.lock();
	const uint32_t block_size = m_volume->block_size;
	buffer_t buffer(block_size);

	// Expand the file
	if (m_offset + amount > m_size)
		m_size = m_inode.grow((m_offset + amount) - m_size, false);

	// Save the updated metadata
	m_inode.inode.last_modification_time = time_to_epoch(Clock::active_clock()->get_time());
	m_inode.save();

	// Convert bytes to blocks
	uint32_t block_start = m_offset / block_size;
	uint32_t block_offset = m_offset % block_size;

	// Write each block
	size_t current_block = block_start;
	size_t written = 0;
	while (written < amount) {

		// Read the block
		uint32_t block = m_inode.block_cache[current_block++];
		m_volume->read_block(block, &buffer);

		// Where in this block to start writing
		size_t buffer_start = (current_block - 1 == block_start) ? block_offset : 0;
		size_t writable = (amount - written < block_size - buffer_start) ? (amount - written) : (block_size -
																								 buffer_start);

		// Update the block
		buffer.copy_from(data, writable, buffer_start, written);
		m_volume->write_block(block, &buffer);
		written += writable;
	}

	// Clean up
	m_offset += amount;
	m_volume->ext2_lock.unlock();
}

/**
* @brief Read data from the file (at the current seek position, updated to be += amount)
*
* @param data The byte buffer to read into
* @param amount The amount of data to read
*/
void Ext2File::read(buffer_t *data, size_t amount) {

	// Nothing to read
	if (m_size == 0 || amount == 0)
		return;

	// Prepare for reading
	m_volume->ext2_lock.lock();
	const uint32_t block_size = m_volume->block_size;
	buffer_t buffer(block_size);

	// Force bounds
	if (m_offset + amount > m_size)
		amount = m_size - m_offset;

	// Convert bytes to blocks
	uint32_t block_start = m_offset / block_size;
	uint32_t block_offset = m_offset % block_size;

	// Read each block
	size_t current_block = block_start;
	size_t read = 0;
	while (read < amount) {

		// Read the block
		uint32_t block = m_inode.block_cache[current_block++];
		m_volume->read_block(block, &buffer);

		// Where in this block to start reading
		size_t buffer_start = (current_block - 1 == block_start) ? block_offset : 0;
		size_t readable = (amount - read < block_size - buffer_start) ? (amount - read) : (block_size - buffer_start);

		// Read the block
		buffer.copy_to(data, readable, buffer_start, read);
		read += readable;

	}

	// Clean up
	m_offset += amount;
	m_volume->ext2_lock.unlock();
}

/**
 * @brief Flush the file to the disk
 */
void Ext2File::flush() {
	File::flush();
}

Ext2File::~Ext2File() = default;

Ext2Directory::Ext2Directory(Ext2Volume *volume, uint32_t inode, const string &name)
: m_volume(volume),
  m_inode(m_volume, inode)
{
	m_name = name;
}

/**
 * @brief Store all entries from a buffer and convert to File or Directory objects
 */
void Ext2Directory::parse_block(buffer_t *buffer) {

	size_t offset = 0;
	while (offset < m_volume->block_size) {

		// Read the entry
		auto *entry = (directory_entry_t *) (buffer->raw() + offset);
		m_entries.push_back(*entry);

		// Not valid
		if (entry->inode == 0 || entry->name_length == 0)
			break;

		// Parse
		string filename(buffer->raw() + offset + sizeof(directory_entry_t), entry->name_length);
		m_entry_names.push_back(filename);
		uint32_t inode = entry->inode;

		// Create the object
		switch ((EntryType) entry->type) {

			case EntryType::FILE:
				m_files.push_back(new Ext2File(m_volume, inode, filename));
				break;

			case EntryType::DIRECTORY:
				m_subdirectories.push_back(new Ext2Directory(m_volume, inode, filename));
				break;

			default:
				Logger::WARNING() << "Unknown entry type: " << entry->type << "\n";

		}

		// Go to next
		offset += entry->size;
	}
}

/**
 * @brief Removes an entry reference from the directory
 *
 * @param name The name of the entry to remove
 * @param is_directory Is the entry expected to be a directory (otherwise assumed to be a file)
 * @param clear Should the inode be freed and the data blocks unallocated
 */
void Ext2Directory::remove_entry(string const &name, bool is_directory, bool clear) {

	// Find the entry
	uint32_t index = 0;
	directory_entry_t *entry = nullptr;
	for (; index < m_entries.size(); ++index)
		if (m_entry_names[index] == name) {
			entry = &m_entries[index];
			break;
		}

	// No entry found
	if (!entry || entry->type != (uint8_t) (is_directory ? EntryType::DIRECTORY : EntryType::FILE))
		return;

	// Clear the inode
	if (clear) {
		InodeHandler inode(m_volume, entry->inode);
		inode.free();
	}

	// Remove the reference from this directory
	m_entries.erase(entry);
	m_entry_names.erase(m_entry_names.begin() + index);
	write_entries();
}

/**
 * @brief Rename a directory entry and save it to disk
 *
 * @param old_name The current name of the entry
 * @param new_name The name to change it to
 * @param is_directory Search for entries with the type DIRECTORY (otherwise assume FILE)
 */
void Ext2Directory::rename_entry(string const &old_name, string const &new_name, bool is_directory) {

	// Change the name
	for (int i = 0; i < m_entry_names.size(); ++i)
		if (m_entry_names[i] == old_name &&
			m_entries[i].type == (uint8_t) (is_directory ? EntryType::DIRECTORY : EntryType::FILE))
			m_entry_names[i] = new_name;

	// Save the change
	write_entries();

}

/**
 * @brief Read the directory from the inode on the disk
 */
void Ext2Directory::read_from_disk() {

	m_volume->ext2_lock.lock();
	m_entries.clear();
	m_entry_names.clear();

	// Clear the old files & Directories
	for (auto &file: m_files)
		delete file;
	m_files.clear();

	for (auto &directory: m_subdirectories)
		delete directory;
	m_subdirectories.clear();

	// Read the direct blocks (cant use for( : ))
	buffer_t buffer(m_volume->block_size);
	for (int i = 0; i < m_inode.block_cache.size(); ++i) {
		uint32_t block_pointer = m_inode.block_cache[i];

		// Invalid block
		if (block_pointer == 0)
			break;

		// Parse the block
		m_volume->read_block(block_pointer, &buffer);
		parse_block(&buffer);
	}

	m_volume->ext2_lock.unlock();
}

void Ext2Directory::write_entries() {

	// Calculate the size needed to store the entries and the null entry
	size_t size_required = sizeof(directory_entry_t);
	for (int i = 0; i < m_entries.size(); ++i) {
		size_t size = sizeof(directory_entry_t) + m_entry_names[i].length() + 1;
		size += (size % 4) ? 4 - size % 4 : 0;
		size_required += size;
	}

	// Expand the directory
	size_t blocks_required = m_volume->bytes_to_blocks(size_required);
	if (blocks_required > m_inode.block_cache.size())
		m_inode.grow((blocks_required - m_inode.block_cache.size()) * m_volume->block_size, false);

	// Prepare for writing
	m_volume->ext2_lock.lock();
	const uint32_t block_size = m_volume->block_size;
	buffer_t buffer(block_size, false);
	buffer.clear();

	// Save the updated metadata
	m_inode.set_size(blocks_required * block_size);
	m_inode.inode.last_modification_time = time_to_epoch(Clock::active_clock()->get_time());
	m_inode.save();

	// Write each entry
	size_t current_block = 0;
	size_t buffer_offset = 0;
	for (int i = 0; i < m_entries.size(); ++i) {

		// Get the current entry
		directory_entry_t &entry = m_entries[i];
		char *name = m_entry_names[i].c_str();

		// Update the size
		entry.name_length = m_entry_names[i].length();
		entry.size = sizeof(directory_entry_t) + entry.name_length + 1;
		entry.size += (entry.size % 4) ? 4 - (entry.size % 4) : 0;

		// Entry needs to be stored in the next block
		if (entry.size + buffer_offset > block_size) {
			m_volume->write_block(m_inode.block_cache[current_block], &buffer);
			buffer.clear();
			current_block++;
			buffer_offset = 0;
		}

		// If it is the last entry it takes up the rest of the block
		if (i == m_entries.size() - 1)
			entry.size = block_size - buffer_offset;

		// Copy the entry and the name
		buffer.copy_from(&entry, sizeof(entry), buffer_offset);
		buffer.copy_from(name, entry.name_length + 1, buffer_offset + sizeof(entry));
		buffer_offset += entry.size;
	}

	// Save the last block
	m_volume->write_block(m_inode.block_cache[current_block], &buffer);

	// Clean up
	m_volume->ext2_lock.unlock();
}

directory_entry_t Ext2Directory::create_entry(const string &name, uint32_t inode, bool is_directory) {

	// Create the inode
	directory_entry_t entry{};
	entry.inode = inode ? inode : m_volume->create_inode(is_directory);
	entry.type = (uint32_t) (is_directory ? EntryType::DIRECTORY : EntryType::FILE);
	entry.name_length = name.length();
	entry.size = sizeof(entry) + entry.name_length;
	entry.size += entry.size % 4 ? 4 - entry.size % 4 : 0;

	// Save the inode
	m_entries.push_back(entry);
	m_entry_names.push_back(name);
	write_entries();

	return entry;
}

/**
 * @brief Create a new file in the directory
 *
 * @param name The name of the file to create
 * @return The new file object or null if it could not be created
 */
File *Ext2Directory::create_file(string const &name) {

	// Check if the file already exists
	for (auto &file: m_files)
		if (file->name() == name)
			return nullptr;

	// Create the file
	auto file = new Ext2File(m_volume, create_entry(name, 0, false).inode, name);
	m_files.push_back(file);
	return file;
}

/**
 * @brief Delete a file from this directory
 *
 * @param name The name of the file to delete
 */
void Ext2Directory::remove_file(string const &name) {
	remove_entry(name, false);
}

/**
 * @brief Renames the file from the old name to the new name if it exists
 *
 * @param old_name The current name of the file that is to be changed
 * @param new_name What the new name should be
 */
void Ext2Directory::rename_file(string const &old_name, string const &new_name) {
	rename_entry(old_name, new_name, false);
}

/**
 * @brief Create a new directory in the directory
 *
 * @param name The name of the directory to create
 * @return The new directory object or null if it could not be created
 */
Directory *Ext2Directory::create_subdirectory(string const &name) {

	// Check if the directory already exists
	for (auto &subdirectory: m_subdirectories)
		if (subdirectory->name() == name)
			return nullptr;

	// Store the directory
	auto directory = new Ext2Directory(m_volume, create_entry(name, 0, true).inode, name);
	m_subdirectories.push_back(directory);

	// Create self & parent references
	directory->create_entry(".", directory->m_inode.inode_number, true);
	directory->create_entry("..", m_inode.inode_number, true);

	return directory;
}

/**
 * @brief Remove a directory entry from the directory
 *
 * @param name The name of the entry to remove
 */
void Ext2Directory::remove_subdirectory(string const &name) {
	remove_entry(name, true);
}

/**
 * @brief Renames the directory from the old name to the new name if it exists
 *
 * @param old_name The current name of the directory that is to be changed
 * @param new_name What the new name should be
 */
void Ext2Directory::rename_subdirectory(string const &old_name, string const &new_name) {
	rename_entry(old_name, new_name, true);
}

Ext2Directory::~Ext2Directory() = default;

Ext2FileSystem::Ext2FileSystem(Disk *disk, uint32_t partition_offset)
		: m_volume(disk, partition_offset) {

	// Create the root directory
	m_root_directory = new Ext2Directory(&m_volume, 2, "/");
	m_root_directory->read_from_disk();

}

Ext2FileSystem::~Ext2FileSystem() {
	delete m_root_directory;
};