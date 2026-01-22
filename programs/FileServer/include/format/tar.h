/**
 * @file tar.h
 * @brief Defines structures and enums for the Tape Archive format and exposes it as Filesystem, Directory and File classes.
 *
 * @date 22nd January 2026
 * @author Max Tyson
 *
 * @todo Either warn read only or make it writeable by updating the archive
 */

#ifndef FILESERVER_FORMAT_TAR_H
#define FILESERVER_FORMAT_TAR_H

#include <filesystem.h>

namespace FileServer::format {

    /**
     * @struct TARHeader
     * @brief Header that prepends a file in a TAR archive
     *
     * @typedef tar_header_t
     * @brief Alais for TARHeader struct
     */
    typedef struct TARHeader
    {
        char filename[100];
        char mode[8];
        char user_id[8];
        char group_id[8];
        char size[12];
        char modified_time[12];
        char check_sum[8];
        char typeflag[1];
    } tar_header_t;

    class TARVolume {

        private:
            void* m_start = nullptr;
            size_t m_size = 0;

            MaxOS::common::Vector<tar_header_t*> m_headers;

        public:
            TARVolume(uint64_t* start);
            ~TARVolume();

            MaxOS::common::Vector<tar_header_t*> headers();
            size_t file_size(tar_header_t* header);

    };

    /**
     * @class TARFile
     * @brief Handles the file operations on the ext2 filesystem
     */
    class TARFile final : public File {
        private:
            TARVolume* m_volume;
            tar_header_t* m_header;

        public:
            TARFile(TARVolume* volume, tar_header_t* header);
            ~TARFile() final;

            void read(MaxOS::common::buffer_t* data, size_t amount) final;
    };

    /**
     * @class TARDirectory
     * @brief Handles the directory operations on the TAR filesystem wrapper
     */
    class TARDirectory final : public Directory {

        private:
            TARVolume* m_volume;
            string m_parent;

        public:
            TARDirectory(TARVolume* volume, const string& path);
            ~TARDirectory() final;

            void read_from_disk() final;
    };

    /**
     * @class TARFileSystem
     * @brief Wraps a TAR archive as a filesystem
     */
    class TARFileSystem final : public FileSystem {

        private:
            TARVolume m_volume;

        public:
            TARFileSystem(void* address, size_t size);
            ~TARFileSystem() final;
    };



}

#endif //FILESERVER_FORMAT_TAR_H