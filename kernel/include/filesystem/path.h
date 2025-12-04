/**
 * @file path.h
 * @brief Provides utilities for handling file and directory paths
 *
 * @date 2nd September 2025
 * @author Max Tyson
 */

#ifndef MAXOS_FILESYSTEM_PATH_H
#define MAXOS_FILESYSTEM_PATH_H

#include <string.h>


namespace MaxOS::filesystem {

	/**
	* @class Path
	* @brief Handles file & directory paths
	*/
	class Path {
		public:
			static bool valid(const string& path);
			static bool is_file(const string& path);

			static string file_name(const string& path);
			static string file_extension(const string& path);
			static string file_path(const string& path);

			static string top_directory(const string& path);
			static string parent_directory(const string& path);

			static string absolute_path(const string& path);
			static string join_path(const string& base, const string& extended);
	};


}


#endif //MAXOS_FILESYSTEM_PATH_H
