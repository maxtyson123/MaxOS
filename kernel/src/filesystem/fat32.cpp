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
  fat_total_clusters = bpb.total_sectors_32 - (bpb.reserved_sectors + (bpb.table_copies * bpb.table_size_32));
  fat_lba = partition_offset + bpb.reserved_sectors;
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
  uint32_t entry  = offset % bpb.bytes_per_sector;

  // Read the FAT entry
  uint8_t fat[bpb.bytes_per_sector];
  disk -> read(sector, fat, bpb.bytes_per_sector);

  // Get the next cluster info (mask the upper 4 bits)
  uint32_t next_cluster = *(uint32_t *)&fat[entry];
  return next_cluster & 0x0FFFFFFF;
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

  // Get the location in the FAT table
  lba_t offset = cluster * sizeof(uint32_t);
  lba_t sector = fat_lba + (offset / bpb.bytes_per_sector);
  uint32_t entry  = offset % bpb.bytes_per_sector;

  // Read the FAT entry
  uint8_t fat[bpb.bytes_per_sector];
  disk -> read(sector, fat, bpb.bytes_per_sector);

  // Set the next cluster info (mask the upper 4 bits)
  *(uint32_t *)&fat[entry] = next_cluster & 0x0FFFFFFF;
  disk -> write(sector, fat, bpb.bytes_per_sector);

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
  for (uint32_t start = 0; start < fsinfo.next_free_cluster; start++)
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
 * @brief Allocate a number of clusters in the FAT table
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


Fat32File::Fat32File(Fat32Volume* volume, uint32_t cluster, size_t size, const string& name)
: m_volume(volume),
  m_first_cluster(cluster)
{

  m_name = name;
  m_size = size;
  m_offset = 0;
}

Fat32File::~Fat32File() = default;

/**
 * @brief Write data to the file
 *
 * @param data The byte buffer to write
 * @param size The amount of data to write
 */
void Fat32File::write(const uint8_t* data, size_t size)
{
  File::write(data, size);
}

/**
 * @brief Read data from the file
 *
 * @param data The byte buffer to read into
 * @param size The amount of data to read
 */
void Fat32File::read(uint8_t* data, size_t size)
{
  File::read(data, size);
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
lba_t Fat32Directory::create_entry(const string& name, bool is_directory)
{

  //TODO: Write to all the FAT copies

  // Allocate a cluster for the new entry
  uint32_t cluster = m_volume -> allocate_cluster(0);
  if (cluster == 0)
    return 0;

  // Store the short name and extension
  char short_name[8];
  char short_extension[3];
  for (int i = 0; i < 8; i++)
    short_name[i] = (i < name.length()) ? name[i] : ' ';
  for (int i = 0; i < 3; i++)
    short_extension[i] = (8 + i < name.length()) ? name[8 + i] : ' ';

  // Information for the new long file name entry
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

  // Create the directory entry
  dir_entry_t entry;
  memcpy(entry.name, short_name, sizeof(short_name));
  memcpy(entry.extension, short_extension, sizeof(short_extension));
  entry.attributes = is_directory ? 0x10 : 0x20;
  entry.first_cluster_high = (cluster >> 16) & 0xFFFF;
  entry.first_cluster_low = cluster & 0xFFFF;

  // Find free space for the new entry and the name
  size_t entries_needed = 1 + lfn_entries.size();
  size_t entry_index = 0;
  for (; entry_index < m_entries.size(); entry_index++)
  {
    // Check if there are enough free entries in a row
    bool found = true;
    for (size_t j = 0; j < entries_needed; j++)
      if (m_entries[entry_index + j].name[0] != 0x00 && m_entries[entry_index + j].name[0] != 0xE5)
        found = false;

    if (found)
      break;
  }

  // Store the entries in the cache
  for (size_t i = 0; i < entries_needed; i++)
  {
    if (i == 0)
      m_entries[entry_index + i] = entry;
    else
      m_entries[entry_index + i] = *(dir_entry_t *)&lfn_entries[i - 1];
  }

  // Get where to write the entry
  lba_t lba = m_volume -> data_lba + (m_first_cluster - 2) * m_volume -> bpb.sectors_per_cluster;
  uint32_t offset = entry_index * sizeof(dir_entry_t);

  // Write the long file name entries
  for (auto& lfn_entry : lfn_entries)
  {
    // Write the entry to the disk
    m_volume -> disk -> write(lba + offset, (uint8_t *)&lfn_entry, sizeof(long_file_name_entry_t));
    offset += sizeof(long_file_name_entry_t);
  }

  // Write the directory entry
  m_volume -> disk -> write(lba + offset, (uint8_t *)&entry, sizeof(dir_entry_t));

  // Creating the file is done
  if (!is_directory)
    return cluster;

  // Create the "." in the directory
  dir_entry_t current_dir_entry;
  memcpy(current_dir_entry.name, ".", 1);
  current_dir_entry.attributes = 0x10;
  current_dir_entry.first_cluster_high = (cluster >> 16) & 0xFFFF;
  current_dir_entry.first_cluster_low = cluster & 0xFFFF;

  // Create the ".." in the directory
  dir_entry_t parent_dir_entry;
  memcpy(parent_dir_entry.name, "..", 2);
  parent_dir_entry.attributes = 0x10;
  parent_dir_entry.first_cluster_high = (m_first_cluster >> 16) & 0xFFFF;
  parent_dir_entry.first_cluster_low = m_first_cluster & 0xFFFF;

  // Write the entries to the disk
  lba_t child_lba = m_volume -> data_lba + (cluster - 2) * m_volume -> bpb.sectors_per_cluster;
  m_volume -> disk -> write(child_lba, (uint8_t *)&current_dir_entry, sizeof(dir_entry_t));
  m_volume -> disk -> write(child_lba + sizeof(dir_entry_t), (uint8_t *)&parent_dir_entry, sizeof(dir_entry_t));

  // Directory created
  return cluster;
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
  size_t entry_index = 0;
  for (; entry_index < m_entries.size(); entry_index++)
  {
    auto& entry = m_entries[entry_index];

    // End of directory means no more entries
    if (entry.name[0] == (uint8_t)DirectoryEntryType::LAST)
      return;

    // Skip deleted entries
    if (entry.name[0] == (uint8_t)DirectoryEntryType::DELETED)
      continue;

    // Check if the entry is the one to remove
    uint32_t start_cluster = (entry.first_cluster_high << 16) | entry.first_cluster_low;
    if (start_cluster == cluster)
      break;
  }

  // Make sure the entry is valid
  if (entry_index >= m_entries.size())
    return;

  // Find any long file name entries that belong to this entry
  size_t delete_entry_index = entry_index;
  while (delete_entry_index > 0 && (m_entries[delete_entry_index - 1].attributes & 0x0F) == 0x0F)
    delete_entry_index--;

  // Mark the entries as deleted
  for (size_t i = delete_entry_index; i < entry_index; i++)
    m_entries[i].name[0] = (uint8_t)DirectoryEntryType::DELETED;

  // Update the entries on the disk
  lba_t first_directory_entry_lba = m_volume -> data_lba + (m_first_cluster - 2) * m_volume -> bpb.sectors_per_cluster;
  for (size_t i = delete_entry_index; i <= entry_index; i++)
  {
    uint32_t offset = (i * sizeof(dir_entry_t)) / m_volume -> bpb.bytes_per_sector;
    m_volume -> disk -> write(first_directory_entry_lba + offset, (uint8_t *)&m_entries[i], sizeof(dir_entry_t));
  }

  // Count the number of clusters in the chain
  size_t cluster_count = 0;
  for (uint32_t next_cluster = cluster; next_cluster < (lba_t)ClusterState::BAD; next_cluster = m_volume -> next_cluster(next_cluster))
    cluster_count++;

  // Free the clusters in the chain (more performant than calling a single
  // free_cluser(cluster) as fs is updated at the end)
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
    if (entry.name[0] == (uint8_t)DirectoryEntryType::DELETED
    || entry.attributes == (uint8_t)DirectoryEntryAttributes::FREE
    || entry.attributes == (uint8_t)DirectoryEntryAttributes::VOLUME_ID)
      continue;

    if (entry.attributes == (uint8_t)DirectoryEntryAttributes::LONG_NAME)
    {

      // Extract the long name from each part (in reverse order)
      auto long_name_entry = (long_file_name_entry_t*)&entry;
      string current_long_name = "";
      for (int i = 0; i < 13; i++) {

        // Get the character (in utf8 encoding)
        char c;
        if (i < 5)
          c = long_name_entry -> name1[i] & 0xFF;
        else if (i < 11)
          c = long_name_entry -> name2[i - 5] & 0xFF;
        else
          c = long_name_entry -> name3[i - 11] & 0xFF;

        // Padding / invalid or end of string
        if (c == (char)0xFF || c == '\0')
          break;

        // Add to the start as the entries are stored in reverse
        current_long_name += string(c);
      }

      // Entry parsed (prepend name)
      long_name = current_long_name + long_name;
      continue;

    }

    bool is_directory = entry.attributes == (uint8_t)DirectoryEntryAttributes::DIRECTORY;

    // Get the name of the entry
    string name = long_name;
    if(long_name == ""){
      name = string(entry.name, 8);
      if(!is_directory){
        name = name.strip();
        name += ".";
        name += string(entry.extension, 3);
      }
    }

    long_name = "";

    // Get the starting cluster
    uint32_t start_cluster = (entry.first_cluster_high << 16) | entry.first_cluster_low;

    // Store the file or directory
    if (is_directory)
      m_subdirectories.push_back(new Fat32Directory(m_volume, start_cluster, name.strip()));
    else
      m_files.push_back(new Fat32File(m_volume, start_cluster, entry.size, name));

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
  auto file = new Fat32File(m_volume, create_entry(name, false), 0, name);
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
  auto directory = new Fat32Directory(m_volume, create_entry(name, true), name);
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