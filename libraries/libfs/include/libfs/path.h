/**
 * @file path.h
 * @brief Provides utilities for handling file and directory paths
 *
 * @date 2nd September 2025
 * @author Max Tyson
 */

#ifndef LIBFS_PATH_H
#define LIBFS_PATH_H

#include <libcommon/string.h>


namespace LibFS {

	/**
	* @class Path
	* @brief Handles file & directory paths
	*/
	class Path {
		public:
			static bool valid(const MaxOS::string& path);
			static bool is_file(const MaxOS::string& path);

			static MaxOS::string file_name(const MaxOS::string& path);
			static MaxOS::string file_extension(const MaxOS::string& path);
			static MaxOS::string file_path(const MaxOS::string& path);
			static bool is_child_of(const MaxOS::string& path, const MaxOS::string& parent);

			static MaxOS::string top_directory(const MaxOS::string& path);
			static MaxOS::string parent_directory(const MaxOS::string& path);

			static MaxOS::string absolute_path(const MaxOS::string& path);
			static MaxOS::string join_path(const MaxOS::string& base, const MaxOS::string& extended);
	};


}


#endif //LIBFS_PATH_H
