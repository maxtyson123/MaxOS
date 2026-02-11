/**
 * @file tar.cpp
 * @brief Implementation of an TAR filesystem driver
 *
 * @date 22nd January 2026
 * @author Max Tyson
 */

#include <format/tar.h>

using namespace FileServer;
using namespace LibFS;
using namespace FileServer::format;
using namespace MaxOS::common;

TARVolume::TARVolume(uint64_t* start)
{
    uint8_t* address = (uint8_t*)start;

    // Cache all the headers
    for (size_t i = 0; ; i++) {

        // Get the header
        auto header = (tar_header_t*)address;

        if (header->filename[0] == '\0')
            break;

        // Cache
        m_headers.push_back(header);

        // Move to the starting block for the next header
        size_t size = file_size(header);
        address += ((size / 512) + 1) * 512;

        // All blocks are 512 bytes, if the file size is not a multiple of this then it is padded with zeros, skip
        if (size % 512)
            address += 512;

    }

}

TARVolume::~TARVolume() = default;

Vector<tar_header_t *> TARVolume::headers() {
    return m_headers;
}

size_t TARVolume::file_size(const tar_header_t* header) {

    size_t size = 0;
    size_t count = 1;

    // Compute the size (encoded in base 8)
    for (size_t i = 11; i > 0; i--, count *= 8)
        size += (header->size[i - 1] - '0') * count;

    return size;
}

TARFile::TARFile(TARVolume *volume, tar_header_t* header)
: m_volume(volume),
  m_header(header)
{
    m_size = m_volume->file_size(header);
    m_name = Path::file_name(header->filename);
}

TARFile::~TARFile() = default;

void TARFile::read(buffer_t* data, size_t amount) {

    // End of file
    if (m_offset >= m_size)
        return;

    // Ensure bounds
    if (amount > m_size - m_offset)
        amount = m_size - m_offset;

    // Copy into the buffer
    uint8_t* file_data = (uint8_t*)m_header + 512;
    data->copy_from(file_data + m_offset, amount);

    m_offset += amount;


}

TARDirectory::TARDirectory(TARVolume *volume, const string &name)
: m_volume(volume)
{
    if (name == "/")
        m_name = name;
    else {
        m_parent = Path::parent_directory(name);
        m_name   = Path::file_name(name);           // Assume no trailing slash @todo enforce
    }
}

TARDirectory::~TARDirectory() = default;

void TARDirectory::read_from_disk() {

    //@todo this is called a lot, maybe dont? i think bc of cache

    m_files.clear();
    m_subdirectories.clear();

    const string this_path = m_parent.empty() ? m_name : m_parent + "/" + m_name;
    Map<string, bool> seen_dirs;

    for (const auto& header : m_volume->headers()) {
        const string& path = (string)"/" + header->filename;

        // Only care about storing files in this directory
        if (!Path::is_child_of(path, this_path))
            continue;

        // Remainder after this directory
        string rest = path.substring(this_path.length(), path.length() - this_path.length());

        // File
        if (!rest.contains('/')) {
            m_files.push_back(new TARFile(m_volume, header));
            continue;
        }

        // Make sure this subdirectory hasn't already been stored
        string subdir = rest.substring(0, rest.find('/'));
        auto found = seen_dirs.find(subdir);
        if (found != seen_dirs.end() && found->second)
            continue;

        // Store the subdir
        seen_dirs.insert(subdir, true);
        auto dir = new TARDirectory(m_volume, path);
        m_subdirectories.push_back(dir);

    }
}

TARFileSystem::TARFileSystem(void* address)
: m_volume((uint64_t*)address)
{
    m_root_directory = new TARDirectory(&m_volume, "/");
    m_root_directory->read_from_disk();

}

TARFileSystem::~TARFileSystem() = default;