//
// Created by 98max on 9/1/2025.
//

#ifndef MAXOS_FILESYSTEM_VFSRESOURCE_H
#define MAXOS_FILESYSTEM_VFSRESOURCE_H

#include <processes/resource.h>
#include <filesystem/vfs.h>
#include <common/buffer.h>
#include <processes/scheduler.h>
#include <syscore/include/filesystem/file.h>
#include <syscore/include/filesystem/directory.h>

namespace MaxOS {

	namespace filesystem {

		/**
		 * @class FileResource
		 * @brief A wrapper for a File which exposes File operations as Resource operations
		 *
		 * @see File
		 * @see Resource
		 * @todo Should this free the memory of the file when the resource is done??
		 */
		class FileResource final : public processes::Resource{

			public:
				FileResource(const string& name, size_t flags, processes::resource_type_t type);
				~FileResource() final;

				File* file; ///< The file that this resource handles & exposes

				int read(void* buffer, size_t size, size_t flags) final;
				int write(const void* buffer, size_t size, size_t flags) final;

		};

		/**
		 * @class DirectoryResource
         * @brief A wrapper for a Directory which exposes Directory operations as Resource operations
		 */
		class DirectoryResource final : public processes::Resource{

			private:

				void write_entries(const void* buffer, size_t size) const;
				[[nodiscard]] size_t entries_size() const;

			public:

				DirectoryResource(const string& name, size_t flags, processes::resource_type_t type);
				~DirectoryResource() final;

				Directory* directory; ///< The directory that this resource handles & exposes

				int read(void* buffer, size_t size, size_t flags) final;
				int write(const void* buffer, size_t size, size_t flags) final;

		};

		/**
		 * @class VFSResourceRegistry
		 * @brief A resource registry for both Files & Directories
		 */
		class VFSResourceRegistry : processes::BaseResourceRegistry{

			private:
				VirtualFileSystem* m_vfs;

				processes::Resource* open_as_resource(const string& name, Directory* directory);
				processes::Resource* open_as_resource(const string& name, File* file);

			public:

				explicit VFSResourceRegistry(VirtualFileSystem* vfs);
				~VFSResourceRegistry();

				processes::Resource* 	get_resource(const string& name) final;
				processes::Resource* 	create_resource(const string& name, size_t flags) final;

		};

	}
}

#endif //MAXOS_FILESYSTEM_VFSRESOURCE_H
