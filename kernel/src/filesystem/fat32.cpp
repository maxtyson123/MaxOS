//
// Created by 98max on 1/01/2023.
//

#include <filesystem/fat32.h>
#include <memory/memoryIO.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::disk;
using namespace MaxOS::filesystem;
using namespace MaxOS::memory;

Fat32Volume::Fat32Volume(Disk* hd, uint32_t partition_offset)
: disk(hd)
{

  // Read the BIOS parameter block
  disk -> read(partition_offset, (uint8_t *)&bpb, sizeof(bpb32_t));

  // Parse the FAT info
  uint32_t total_data_sectors = bpb.total_sectors_32 - (bpb.reserved_sectors + (bpb.table_copies * bpb.table_size_32));
  fat_total_clusters = total_data_sectors / bpb.sectors_per_cluster;
  fat_lba = partition_offset + bpb.reserved_sectors;
  fat_copies = bpb.table_copies;
  fat_info_lba = partition_offset + bpb.fat_info;
  disk -> read(fat_info_lba, (uint8_t *)&fsinfo, sizeof(fs_info_t));

  data_lba = fat_lba + (bpb.table_copies * bpb.table_size_32);
  root_lba = data_lba + bpb.sectors_per_cluster * (bpb.root_cluster - 2);

  // Validate the fat information
  if (fsinfo.lead_signature != 0x41615252 || fsinfo.structure_signature != 0x61417272 || fsinfo.trail_signature != 0xAA550000)
  {
    Logger::ERROR() << "Invalid FAT32 filesystem information TODO: Handle this\n";
    return;
  }

}

Fat32Volume::~Fat32Volume() = default;


/**
 * @brief Take the cluster and gets the next cluster in the chain
 *
 * @param cluster The base cluster to start from
 * @return The next cluster in the chain
 */
lba_t Fat32Volume::next_cluster(lba_t cluster)
{

  // Get the location in the FAT table
  lba_t offset = cluster * sizeof(uint32_t);
  lba_t sector = fat_lba + (offset / bpb.bytes_per_sector);
  uint32_t entry_index  = offset % bpb.bytes_per_sector;

  // Read the FAT entry
  uint8_t fat[bpb.bytes_per_sector];
  disk -> read(sector, fat, bpb.bytes_per_sector);

  // Get the next cluster info (mask the upper 4 bits)
  auto entry = (uint32_t *)(&fat[entry_index]);
  return *entry & 0x0FFFFFFF;
}

/**
 * @brief Sets the next cluster in the chain (where the base cluster should point)
 *
 * @param cluster The base cluster to start from
 * @param next_cluster The next cluster in the chain
 * @return The next cluster in the chain
 */
uint32_t Fat32Volume::set_next_cluster(uint32_t cluster, uint32_t next_cluster)
{

  // TODO - when in userspace: For performance cache fat entirely, cache file data, cache cluster chains

  // Get the location in the FAT table
  lba_t offset = cluster * sizeof(uint32_t);

  for (int i = 0; i < fat_copies; ++i) {

    lba_t sector = (fat_lba + i * bpb.table_size_32) + (offset / bpb.bytes_per_sector);
    uint32_t entry_index  = offset % bpb.bytes_per_sector;

    // Read the FAT entry
    uint8_t fat[bpb.bytes_per_sector];
    disk -> read(sector, fat, bpb.bytes_per_sector);

    // Set the next cluster info (mask the upper 4 bits)
    auto entry = (uint32_t *)(&fat[entry_index]);
    *entry = next_cluster & 0x0FFFFFFF;
    disk -> write(sector, fat, bpb.bytes_per_sector);

  }

  return next_cluster;
}

/**
 * @brief Searches the fat table for a free cluster starting from the first free cluster in the fsinfo, will then wrap around
 *
 * @return The first free cluster in the FAT table
 */
uint32_t Fat32Volume::find_free_cluster()
{

  // Get the first free cluster
  for (uint32_t start = fsinfo.next_free_cluster; start < fat_total_clusters + 1; start++)
    if (next_cluster(start) == 0)
      return start;

  // Check any clusters before the first free cluster
  for (uint32_t start = 2; start < fsinfo.next_free_cluster; start++)
    if (next_cluster(start) == 0)
      return start;

  ASSERT(false, "No free clusters found in the FAT table");
  return 0;
}

/**
 * @brief Allocate a cluster in the FAT table
 *
 * @param cluster The base cluster to start from or 0 if this is a new chain
 * @return The next cluster in the chain
 */
uint32_t Fat32Volume::allocate_cluster(uint32_t cluster)
{

    // Allocate 1 cluster
    return allocate_cluster(cluster, 1);
}

/**
 * @brief Allocate a number of clusters in the FAT table, updates the fsinfo and the chain
 *
 * @param cluster The base cluster to start from or 0 if this is a new chain
 * @param amount The number of clusters to allocate
 * @return The next cluster in the chain
 */
uint32_t Fat32Volume::allocate_cluster(uint32_t cluster, size_t amount)
{

  // Make sure within bounds
  if (cluster > fat_total_clusters || cluster + amount > fat_total_clusters)
    return 0;

  // Go through allocating the clusters
  for (size_t i = 0; i < amount; i++)
  {
    uint32_t next_cluster = find_free_cluster();

    // Update the fsinfo
    fsinfo.next_free_cluster = next_cluster + 1;
    fsinfo.free_cluster_count -= 1;

    // If there is an existing chain it needs to be updated
    if (cluster != 0)
      set_next_cluster(cluster, next_cluster);

    cluster = next_cluster;
  }

  // Once all the updates are done flush the changes to the disk
  disk -> write(fat_info_lba, (uint8_t *)&fsinfo, sizeof(fs_info_t));

  // Finish the chin
  set_next_cluster(cluster, (uint32_t)ClusterState::END_OF_CHAIN);
  uint32_t next = next_cluster(cluster);
  return cluster;
}

/**
 * @brief Free a cluster in the FAT table
 *
 * @param cluster The base cluster to start from
 * @param full Weather the chain's length is 1 or not
 */
void Fat32Volume::free_cluster(lba_t cluster)
{

  // Free 1 cluster
  free_cluster(cluster, 1);

}

/**
 * @brief Free a number of clusters in the FAT table
 *
 * @param cluster The base cluster to start from
 * @param amount The number of clusters to free
 */
void Fat32Volume::free_cluster(uint32_t cluster, size_t amount)
{

  // Make sure within bounds
  if (cluster < 2 || cluster > fat_total_clusters || cluster + amount > fat_total_clusters)
    return;

  // Go through freeing the clusters
  for (size_t i = 0; i < amount; i++)
  {

    // Find the next cluster before it is removed from the chain
    uint32_t next_in_chain = next_cluster(cluster);

    // Update the fsinfo
    fsinfo.next_free_cluster = cluster;
    fsinfo.free_cluster_count += 1;

    // Update the chain
    set_next_cluster(cluster, (lba_t)ClusterState::FREE);
    cluster = next_in_chain;
  }

  // Save the fsinfo
  disk -> write(fat_info_lba, (uint8_t *)&fsinfo, sizeof(fs_info_t));

  // Mark the end of the chain
  set_next_cluster(cluster, (uint32_t)ClusterState::END_OF_CHAIN);
}


Fat32File::Fat32File(Fat32Volume* volume, Fat32Directory* parent, dir_entry_t* info, const string& name)
: m_volume(volume),
  m_parent_directory(parent),
  m_entry(info),
  m_first_cluster((info -> first_cluster_high << 16) | info -> first_cluster_low)
{

  m_name = name;
  m_size = info -> size;
  m_offset = 0;
}

Fat32File::~Fat32File() = default;

/**
 * @brief Write data to the file (at the current seek position, updated to be += amount)
 *
 * @param data The byte buffer to write
 * @param amount The amount of data to write
 */
void Fat32File::write(const uint8_t* data, size_t amount)
{

  size_t buffer_space = m_volume->bpb.bytes_per_sector * m_volume->bpb.sectors_per_cluster;
  auto buffer = new uint8_t[buffer_space];

  uint64_t current_offset = 0;
  uint64_t bytes_written  = 0;
  uint32_t last = m_first_cluster;

  // Read the file
  for (uint32_t cluster = last; cluster != (uint32_t)ClusterState::END_OF_CHAIN; cluster = m_volume -> next_cluster(cluster)) {
    last = cluster;

    // No cluster to read from (blank file)
    if (cluster == 0)
        break;

    // Skip clusters before the offset
    if((current_offset + buffer_space) < m_offset){
      current_offset += buffer_space;
      continue;
    }

    // Read each sector in the cluster (prevent overwriting the data)
    memset(buffer, 0, buffer_space);
    lba_t lba = m_volume->data_lba + (cluster - 2) * m_volume->bpb.sectors_per_cluster;
    for (size_t sector = 0; sector < m_volume->bpb.sectors_per_cluster; sector++)
      m_volume->disk->read(lba + sector,buffer + sector * m_volume->bpb.bytes_per_sector,m_volume->bpb.bytes_per_sector);

    // If the offset is in the middle of the cluster
    size_t buffer_offset = 0;
    if(m_offset > current_offset)
      buffer_offset = m_offset - current_offset;

    // Calculate how many bytes are being copied (read from cluster at offset?
    // or read part of cluster?)
    size_t cluster_remaining_bytes = buffer_space - buffer_offset;
    size_t data_remaining_bytes    = amount - bytes_written ;
    size_t bytes_to_copy           = (cluster_remaining_bytes < data_remaining_bytes) ? cluster_remaining_bytes : data_remaining_bytes;
    bytes_to_copy                  = (bytes_to_copy > buffer_space) ? buffer_space : bytes_to_copy;

    // Update the data
    memcpy(buffer + buffer_offset, data + bytes_written , bytes_to_copy);
    bytes_written  += bytes_to_copy;
    current_offset += bytes_to_copy;

    // Write the data back to the disk
    for (size_t sector = 0; sector < m_volume->bpb.sectors_per_cluster; sector++)
      m_volume->disk->write(lba + sector,buffer + sector * m_volume->bpb.bytes_per_sector,m_volume->bpb.bytes_per_sector);
  }

  // Extend the file
  while(bytes_written < amount)
  {
    // Allocate a new cluster
    uint32_t new_cluster = m_volume -> allocate_cluster(last);
    if (new_cluster == 0)
      break;

    if(last == 0)
      m_first_cluster = new_cluster;

    // Update the data
    size_t bytes_to_copy = (amount - bytes_written) > buffer_space ? buffer_space : (amount - bytes_written);
    memcpy(buffer, data + bytes_written , bytes_to_copy);
    bytes_written  += bytes_to_copy;
    current_offset += bytes_to_copy;

    // Write the data back to the disk
    lba_t lba = m_volume->data_lba + (new_cluster - 2) * m_volume->bpb.sectors_per_cluster;
    for (size_t sector = 0; sector < m_volume->bpb.sectors_per_cluster; sector++)
      m_volume->disk->write(lba + sector,buffer + sector * m_volume->bpb.bytes_per_sector,m_volume->bpb.bytes_per_sector);

    // Go to the next cluster
    last = new_cluster;

  }

  // Update file size
  m_offset += bytes_written;
  if (m_offset > m_size)
    m_size = m_offset;

  // Update entry info
  m_entry -> size = m_size;
  m_entry -> first_cluster_high = (m_first_cluster >> 16) & 0xFFFF;
  m_entry -> first_cluster_low = m_first_cluster & 0xFFFF;
  // TODO: When implemented as a usermode driver save the time
  m_parent_directory -> save_entry_to_disk(m_entry);

  delete[] buffer;
}

/**
 * @brief Read data from the file (at the current seek position, updated to be += amount)
 *
 * @param data The byte buffer to read into
 * @param amount The amount of data to read
 */
void Fat32File::read(uint8_t* data, size_t amount)
{
  size_t buffer_space = m_volume->bpb.bytes_per_sector * m_volume->bpb.sectors_per_cluster;
  auto buffer = new uint8_t[buffer_space];

  uint64_t current_offset = 0;
  uint64_t bytes_read = 0;

  // Read the file
  for (uint32_t cluster = m_first_cluster; cluster != (uint32_t)ClusterState::END_OF_CHAIN; cluster = m_volume -> next_cluster(cluster)) {

    // Skip clusters before the offset
    if((current_offset + buffer_space) < m_offset){
      current_offset += buffer_space;
      continue;
    }

    // Read each sector in the cluster
    memset(buffer, 0, buffer_space);
    lba_t lba = m_volume->data_lba + (cluster - 2) * m_volume->bpb.sectors_per_cluster;
    for (size_t sector = 0; sector < m_volume->bpb.sectors_per_cluster; sector++)
      m_volume->disk->read(lba + sector,buffer + sector * m_volume->bpb.bytes_per_sector,m_volume->bpb.bytes_per_sector);

    // If the offset is in the middle of the cluster
    size_t buffer_offset = 0;
    if(m_offset > current_offset)
      buffer_offset = m_offset - current_offset;

    // Calculate how many bytes are being copied (read from cluster at offset?
    // or read part of cluster?)
    size_t cluster_remaining_bytes = buffer_space - buffer_offset;
    size_t data_remaining_bytes    = amount - bytes_read;
    size_t bytes_to_copy           = (cluster_remaining_bytes < data_remaining_bytes) ? cluster_remaining_bytes : data_remaining_bytes;
    bytes_to_copy                  = (bytes_to_copy > buffer_space) ? buffer_space : bytes_to_copy;

    // Read the data
    memcpy(data + bytes_read, buffer + buffer_offset, bytes_to_copy);
    bytes_read += bytes_to_copy;
    current_offset += buffer_space;

    // Dont read more than needed
    if(bytes_read >= amount)
      break;
  }

  m_offset += bytes_read;
  delete[] buffer;
}

/**
 * @brief Flush the file to the disk
 */
void Fat32File::flush()
{
  File::flush();
}

Fat32Directory::Fat32Directory(Fat32Volume* volume, uint32_t cluster, const string& name)
: m_volume(volume),
  m_first_cluster(cluster)
{

  m_name = name;

}

Fat32Directory::~Fat32Directory() = default;

/**
 * @brief Create a new entry in the directory
 *
 * @param name The name of the file or directory to create
 * @param is_directory True if the entry is a directory, false if it is a file
 * @return The cluster of the new entry
 */
dir_entry_t* Fat32Directory::create_entry(const string& name, bool is_directory)
{

  // Allocate a cluster for the new entry
  uint32_t cluster = m_volume -> allocate_cluster(0);
  if (cluster == 0)
    return nullptr;

  // Store the name
  Vector<long_file_name_entry_t> lfn_entries = to_long_filenames(name);
  char short_name[8];
  char short_extension[3];
  for (int i = 0; i < 8; i++)
    short_name[i] = (i < name.length()) ? name[i] : ' ';
  for (int i = 0; i < 3; i++)
    short_extension[i] = (8 + i < name.length()) ? name[8 + i] : ' ';

  // Create the directory entry
  dir_entry_t entry = {};
  memcpy(entry.name, short_name, sizeof(short_name));
  memcpy(entry.extension, short_extension, sizeof(short_extension));
  entry.attributes = is_directory ? (uint8_t)DirectoryEntryAttributes::DIRECTORY : (uint8_t)DirectoryEntryAttributes::ARCHIVE;
  entry.first_cluster_high = (cluster >> 16) & 0xFFFF;
  entry.first_cluster_low = cluster & 0xFFFF;

  // Find free space for the new entry and the name
  size_t entries_needed = 1 + lfn_entries.size();
  int entry_index = find_free_entries(entries_needed);
  if(entry_index == -1)
    entry_index = expand_directory(entries_needed);

  // Store the entries in the cache
  for (size_t i = 0; i < entries_needed; i++)
    m_entries[entry_index + i] = i == 0 ? entry : *(dir_entry_t *)&lfn_entries[i - 1];

  // Write the long file name entries
  for (size_t index = entry_index + lfn_entries.size() - 1; index >= entry_index; index--)
    update_entry_on_disk(index);

  // Creating the file is done
  if (!is_directory)
    return &m_entries[entry_index];

  // Create the "." in the directory
  dir_entry_t current_dir_entry = {};
  memcpy(current_dir_entry.name, ".", 1);
  current_dir_entry.attributes = 0x10;
  current_dir_entry.first_cluster_high = (cluster >> 16) & 0xFFFF;
  current_dir_entry.first_cluster_low = cluster & 0xFFFF;

  // Create the ".." in the directory
  dir_entry_t parent_dir_entry = {};
  memcpy(parent_dir_entry.name, "..", 2);
  parent_dir_entry.attributes = 0x10;
  parent_dir_entry.first_cluster_high = (m_first_cluster >> 16) & 0xFFFF;
  parent_dir_entry.first_cluster_low = m_first_cluster & 0xFFFF;

  // Write the entries to the disk
  uint32_t bytes_per_sector = m_volume -> bpb.bytes_per_sector;
  lba_t child_lba = m_volume -> data_lba + (cluster - 2) * bytes_per_sector;
  uint8_t buffer[bytes_per_sector];
  memset(buffer, 0, bytes_per_sector);
  memcpy(buffer, (uint8_t *)&current_dir_entry, sizeof(dir_entry_t));
  memcpy(buffer + sizeof(dir_entry_t), (uint8_t *)&parent_dir_entry, sizeof(dir_entry_t));
  m_volume -> disk -> write(child_lba, buffer, bytes_per_sector);

  // Directory created
  return &m_entries[entry_index];
}

/**
 * @brief Remove an entry from the directory
 *
 * @param cluster The cluster of the entry to remove
 * @param name The name of the entry to remove
 */
void Fat32Directory::remove_entry(uint32_t cluster, const string& name)
{

  // Find the entry in the directory
  int entry = entry_index(cluster);
  if(entry == -1)
    return;

  // Find any long file name entries that belong to this entry
  size_t delete_entry_index = entry;
  while (delete_entry_index > 0 && m_entries[delete_entry_index - 1].attributes == (uint8_t)DirectoryEntryAttributes::LONG_NAME)
    delete_entry_index--;

  // Mark the entries as free
  for (size_t i = delete_entry_index; i < entry; i++)
    m_entries[i].name[0] = (uint8_t)DirectoryEntryType::FREE;

  // Update the entries on the disk
  for (size_t i = delete_entry_index; i <= entry; i++)
    update_entry_on_disk(i);

  // Count the number of clusters in the chain
  size_t cluster_count = 0;
  for (uint32_t next_cluster = cluster; next_cluster < (lba_t)ClusterState::BAD; next_cluster = m_volume -> next_cluster(next_cluster))
    cluster_count++;

  // Free all the clusters in the chain
  m_volume -> free_cluster(cluster, cluster_count);
}

/**
 * @brief Read all of the directory entries for each cluster in this directory
 */
void Fat32Directory::read_all_entries() {
  m_entries.clear();

  size_t buffer_space = m_volume->bpb.bytes_per_sector * m_volume->bpb.sectors_per_cluster;
  auto buffer = new uint8_t[buffer_space];

  // Read the directory
  for (uint32_t cluster = m_first_cluster; cluster != (uint32_t)ClusterState::END_OF_CHAIN; cluster = m_volume -> next_cluster(cluster)) {

    // Cache info to prevent re-traversing the chain
    m_last_cluster = cluster;
    m_current_cluster_length++;

    // Read each sector in the cluster
    memset(buffer, 0, buffer_space);
    lba_t lba = m_volume->data_lba + (cluster - 2) * m_volume->bpb.sectors_per_cluster;
    for (size_t sector = 0; sector < m_volume->bpb.sectors_per_cluster; sector++)
      m_volume->disk->read(lba + sector,buffer + sector * m_volume->bpb.bytes_per_sector,m_volume->bpb.bytes_per_sector);

    // Parse the directory entries (each entry is 32 bytes)
    for (size_t entry_offset = 0; entry_offset < buffer_space; entry_offset += 32) {

      // Store the entry
      auto entry = (dir_entry_t*)&buffer[entry_offset];
      m_entries.push_back(*entry);

      // Check if the entry is the end of the directory
      if (entry -> name[0] == (uint8_t)DirectoryEntryType::LAST)
        return;
    }

  }

  delete[] buffer;
}


/**
 * @brief Writes an updated directory entry to the disk
 *
 * @param entry The entry to write
 */
void Fat32Directory::save_entry_to_disk(DirectoryEntry* entry) {

  int index = 0;
  for (auto & m_entry : m_entries){
    if (&m_entry == entry)
      break;
    index++;
  }

  update_entry_on_disk(index);
}

void Fat32Directory::update_entry_on_disk(int index) {

  // Get the entry
  auto entry = m_entries[index];

  // Determine sector offset and in-sector byte offset
  uint32_t bytes_per_sector = m_volume->bpb.bytes_per_sector;
  uint32_t entry_offset = index * sizeof(dir_entry_t);
  uint32_t sector_offset = entry_offset / bytes_per_sector;
  uint32_t in_sector_offset = entry_offset % bytes_per_sector;

  // Find which cluster has the entry
  uint32_t cluster = m_first_cluster;
  for (uint32_t offset_remaining = entry_offset; offset_remaining >= bytes_per_sector; offset_remaining -= bytes_per_sector)
    cluster = m_volume -> next_cluster(cluster);

  // Read the full sector into a buffer
  lba_t base_lba = m_volume->data_lba + (cluster - 2) * m_volume->bpb.sectors_per_cluster;
  uint8_t sector_buffer[bytes_per_sector];
  m_volume->disk->read(base_lba + sector_offset, sector_buffer, bytes_per_sector);

  // Update the entry in the buffer
  memcpy(sector_buffer + in_sector_offset, &entry, sizeof(dir_entry_t));
  m_volume->disk->write(base_lba + sector_offset, sector_buffer, bytes_per_sector);
}

/**
 * @brief Find cluster's directory entry index in the store entries
 *
 * @param cluster The cluster
 * @return The index or -1 if not found
 */
int Fat32Directory::entry_index(lba_t cluster) {

  int entry_index = 0;
  for (; entry_index < m_entries.size(); entry_index++)
  {
    auto& entry = m_entries[entry_index];

    // End of directory means no more entries
    if (entry.name[0] == (uint8_t)DirectoryEntryType::LAST)
      return -1;

    // Skip free entries
    if (entry.name[0] == (uint8_t)DirectoryEntryType::FREE)
      continue;

    // Check if the entry is the one
    uint32_t start_cluster = (entry.first_cluster_high << 16) | entry.first_cluster_low;
    if (start_cluster == cluster)
      break;
  }

  // Make sure the entry is valid
  if (entry_index >= m_entries.size())
    return -1;

  return entry_index;

}

/**
 * @brief Find a series of free entries in a row
 *
 * @param amount The amount of adjacent entries to find
 *
 * @return The index of the first free entry or -1 if cant find that many free entries
 */
int Fat32Directory::find_free_entries(size_t amount) {

  for (int entry_index = 0; entry_index < m_entries.size(); entry_index++)
  {
    // Check if there are enough free entries in a row
    bool found = true;
    for (size_t j = 0; j < amount; j++)
      if (m_entries[entry_index + j].name[0] != (char)DirectoryEntryType::FREE)
        found = false;

    if (found)
      return entry_index;
  }

  return -1;
}


/**
 * @brief Expand the directory to fit 'amount - free' more entries.
 *
 * @note Caller must write data to the entries as only the updated LAST entry is
 * saved to disk. Theoretically there wont be an issue if caller doesn't as the
 * old LAST will still be in the same space.
 *
 * @param amount How many free entries are needed
 * @return The index of the first free entry in the chain
 */
int Fat32Directory::expand_directory(size_t amount) {

  // Remove the old end of directory marker
  int free_start = m_entries.size() - 1;
  ASSERT(m_entries[free_start].name[0] == (uint8_t)DirectoryEntryType::LAST, "Last entry is not marked");
  m_entries[free_start].name[0] = (uint8_t)DirectoryEntryType::FREE;

  // Count how many free entries there is before the end
  for (int i = free_start; i >= 0; --i) {
    if(m_entries[i].name[0] == (char)DirectoryEntryType::FREE)
      free_start = i;
    else
      break;
  }

  // Calculate how many entries are need to be created (ie was there enough free entries already)
  uint32_t found          = m_entries.size() - free_start;
  uint32_t needed_entries = amount + 1;
  uint32_t additional_entries = 0;
  if(needed_entries > found)
    additional_entries = needed_entries - found;

  // Find the length of the current cluster chain
  uint32_t total_entries = m_entries.size() + additional_entries;
  uint32_t total_clusters = (total_entries * sizeof(dir_entry_t)) / (m_volume->bpb.bytes_per_sector * m_volume->bpb.sectors_per_cluster);

  // Expand the cluster chain if needed
  if(total_clusters > m_current_cluster_length){
    m_volume->allocate_cluster(m_last_cluster, total_clusters - m_current_cluster_length);
    m_current_cluster_length = total_clusters;
  }

  // Expand the directory to fit the remaining entries
  for (int i = 0; i < additional_entries; ++i) {
    dir_entry_t free = {};
    free.name[0] = (uint8_t)DirectoryEntryType::FREE;
    m_entries.push_back(free);
  }

  // Write the updated end of entries
  m_entries[m_entries.size() - 1].name[0] = (uint8_t)DirectoryEntryType::LAST;
  update_entry_on_disk(m_entries.size() - 1);

  return free_start;
}

/**
 * @brief Converts a string into a series of long file name entries (in reverse
 * order, correct directory entry order)
 *
 * @param name The name
 * @return A vector of longfile names
 */
Vector<long_file_name_entry_t> Fat32Directory::to_long_filenames(string name) {

  size_t lfn_count = (name.length() + 12) / 13;
  Vector<long_file_name_entry_t> lfn_entries;

  // Create the long file name entries (in reverse order)
  for (int i = lfn_count - 1; i >= 0; i--)
  {
    // Create the long file name entry
    long_file_name_entry_t lfn_entry;
    lfn_entry.order = i + 1;
    lfn_entry.attributes = 0x0F;
    lfn_entry.type = 0;
    lfn_entry.checksum = 0;

    // If it is the last entry, set the last bit
    if (i == lfn_entries.size() - 1)
      lfn_entry.order |= 0x40;

    // Set the name
    for (int j = 0; j < 13; j++)
    {

      // Get the character info (0xFFFF if out of bounds)
      size_t char_index = i * 13 + j;
      char c = (char_index < name.length()) ? name[char_index] : 0xFFFF;

      // Set the character in the entry
      if (j < 5)
        lfn_entry.name1[j] = c;
      else if (j < 11)
        lfn_entry.name2[j - 5] = c;
      else
        lfn_entry.name3[j - 11] = c;
    }
    lfn_entries.push_back(lfn_entry);
  }

  return lfn_entries;
}

/**
 * @brief Loads the characters from the entry and correctly appends to the whole name
 *
 * @param entry The entry to parse
 * @param current The currently parsed string
 *
 * @return The parsed entry prepended to the current string
 */
string Fat32Directory::parse_long_filename(long_file_name_entry_t* entry, const string& current) {

  // Extract the long name from each part (in reverse order)
  string current_long_name = "";
  for (int i = 0; i < 13; i++) {

    // Get the character (in utf8 encoding)
    char c;
    if (i < 5)
      c = entry -> name1[i] & 0xFF;
    else if (i < 11)
      c = entry -> name2[i - 5] & 0xFF;
    else
      c = entry -> name3[i - 11] & 0xFF;

    // Padding / invalid or end of string
    if (c == (char)0xFF || c == '\0')
      break;

    // Add to the start as the entries are stored in reverse
    current_long_name += string(c);
  }

  // Entry parsed (prepend name)
  return current_long_name + current;
}

void Fat32Directory::read_from_disk() {

  for(auto& file : m_files)
    delete file;
  m_files.clear();

  for(auto& directory : m_subdirectories)
    delete directory;
  m_subdirectories.clear();

  // Load the entries from the disk into memory
  read_all_entries();

  // Parse the entries
  string long_name = "";
  for(auto& entry : m_entries){

    // Skip free entries and volume labels
    if (entry.name[0]   == (uint8_t)DirectoryEntryType::FREE
    || entry.attributes == (uint8_t)DirectoryEntryAttributes::FREE
    || entry.attributes == (uint8_t)DirectoryEntryAttributes::VOLUME_ID)
      continue;

    // Extract the long name
    if (entry.attributes == (uint8_t)DirectoryEntryAttributes::LONG_NAME) {
      long_name = parse_long_filename((long_file_name_entry_t*)&entry, long_name);
      continue;
    }

    bool is_directory = entry.attributes == (uint8_t)DirectoryEntryAttributes::DIRECTORY;

    // Get the name of the entry
    string name = long_name;
    if(long_name == ""){
      name = string(entry.name, 8);

      // Add the extension
      if(!is_directory)
        name = name.strip() + "." + string(entry.extension, 3);
    }

    long_name = "";

    // Get the starting cluster
    uint32_t start_cluster = (entry.first_cluster_high << 16) | entry.first_cluster_low;

    // Store the file or directory
    if (is_directory)
      m_subdirectories.push_back(new Fat32Directory(m_volume, start_cluster, name.strip()));
    else
      m_files.push_back(new Fat32File(m_volume, this, &entry, name));

  }
}

/**
 * @brief Create a new file in the directory
 *
 * @param name The name of the file to create
 * @return The new file object or null if it could not be created
 */
File* Fat32Directory::create_file(const string& name)
{

  // Check if the file already exists
  for (auto & file : m_files)
    if (file -> name() == name)
      return nullptr;

  // Check if the name is too long
  if (name.length() > MAX_NAME_LENGTH)
    return nullptr;

  // Create the file
  auto file = new Fat32File(m_volume, this, create_entry(name, false), name);
  m_files.push_back(file);
  return file;
}


void Fat32Directory::remove_file(const string& name)
{
  // Find the file if it exists
  for (auto& file : m_files)
    if (file -> name() == name)
    {

      // Remove the file from the directory
      m_files.erase(file);
      remove_entry(((Fat32File*)file) -> first_cluster(), name);

      // Delete the file reference
      delete file;
      return;
    }
}

/**
 * @brief Create a new directory in the directory
 *
 * @param name The name of the directory to create
 * @return The new directory object or null if it could not be created
 */
Directory* Fat32Directory::create_subdirectory(const string& name)
{

  // Check if the directory already exists
  for (auto & subdirectory : m_subdirectories)
    if (subdirectory -> name() == name)
      return nullptr;

  // Check if the name is too long
  if (name.length() > MAX_NAME_LENGTH)
    return nullptr;

  // Create the directory
  auto entry = create_entry(name, true);
  uint32_t cluster = ((entry -> first_cluster_high << 16) | entry -> first_cluster_low);

  // Store the directory
  auto directory = new Fat32Directory(m_volume, cluster, name);
  m_subdirectories.push_back(directory);
  return directory;

}

/**
 * @brief Remove a directory entry from the directory
 *
 * @param name The name of the entry to remove
 */
void Fat32Directory::remove_subdirectory(const string& name)
{
  // Find the directory if it exists
  for (auto& subdirectory : m_subdirectories) {
    if (subdirectory->name() != name)
      continue;

    // Remove all the files in the directory
    for (auto &file : subdirectory->files())
      subdirectory->remove_file(file->name());

    // Remove all the subdirectories in the directory
    for (auto &subdirectory : subdirectory->subdirectories())
      subdirectory->remove_subdirectory(subdirectory->name());

    // Remove the entry
    m_subdirectories.erase(subdirectory);
    remove_entry(((Fat32Directory *)subdirectory)->first_cluster(), name);

    // Delete the directory
    delete subdirectory;
    return;
  }
}

Fat32FileSystem::Fat32FileSystem(Disk* disk, uint32_t partition_offset)
: m_volume(disk, partition_offset)
{

  // Create the root directory
  m_root_directory = new Fat32Directory(&m_volume, m_volume.bpb.root_cluster, "/");
  m_root_directory -> read_from_disk();

}

Fat32FileSystem::~Fat32FileSystem() = default;