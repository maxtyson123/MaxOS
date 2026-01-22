/**
 * @file vfsresource.h
 * @brief Defines VFSResource classes for wrapping File and Directory objects as Resources in the Virtual File System,
 * allowing file system operations to be performed through the Resource interface.
 *
 * @date 1st September 2025
 * @author Max Tyson
 */

#ifndef FILESERVER_VFSRESOURCE_H
#define FILESERVER_VFSRESOURCE_H

#include <vfs.h>
#include <buffer.h>
#include <libfs/include/file.h>
#include <libfs/include/directory.h>
#include <libkpi/include/resource.h>

namespace FileServer {

	typedef MaxOS::KPI::Resource Resource;

	/**
	 * @class FileResource
	 * @brief A wrapper for a File which exposes File operations as Resource operations
	 *
	 * @see File
	 * @see Resource
	 */
	class FileResource final : public Resource{

		public:
			FileResource(const string& name, size_t flags, MaxOS::KPI::ResourceType type);
			~FileResource() final;

			File* file; ///< The file that this resource handles & exposes

			int read(void* buffer, size_t size, size_t flags) final;
			int write(const void* buffer, size_t size, size_t flags) final;

	};

	/**
	 * @class DirectoryResource
     * @brief A wrapper for a Directory which exposes Directory operations as Resource operations
	 */
	class DirectoryResource final : public Resource{

		private:

			void write_entries(const void* buffer, size_t size) const;
			[[nodiscard]] size_t entries_size() const;

		public:

			DirectoryResource(const string& name, size_t flags, MaxOS::KPI::ResourceType type);
			~DirectoryResource() final;

			Directory* directory; ///< The directory that this resource handles & exposes

			int read(void* buffer, size_t size, size_t flags) final;
			int write(const void* buffer, size_t size, size_t flags) final;

	};

	/**
	 * @class VFSResourceServer
	 * @brief A resource server for both Files & Directories
	 */
	class VFSResourceServer : public MaxOS::KPI::ResourceServer{

		private:
			VirtualFileSystem* m_vfs;

			MaxOS::common::Map<uint64_t, string> m_working_directories;

			Resource* open_as_resource(const string& name, Directory* directory);
			Resource* open_as_resource(const string& name, File* file);

		public:

			explicit VFSResourceServer(VirtualFileSystem* vfs);
			~VFSResourceServer();

			Resource* 	get_resource(const string& name) final;
			Resource* 	create_resource(const string& name, size_t flags) final;

			void change_working_dir(uint64_t pid, string dir);
			string process_relative_path(uint64_t pid, const string& relative_path);

	};

}

#endif //FILESERVER_VFSRESOURCE_H
