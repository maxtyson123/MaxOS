/**
 * @file path.h
 * @brief Provides utilities for handling file and directory paths
 *
 * @date 2nd September 2025
 * @author Max Tyson
 */

#ifndef MAXOS_FILESYSTEM_PATH_H
#define MAXOS_FILESYSTEM_PATH_H

#include <common/string.h>

namespace MaxOS {

	namespace filesystem {

		/**
		* @class Path
		* @brief Handles file & directory paths
		*/
		class Path
		{
		public:
			static bool   valid(string path);
			static bool   is_file(const string& path);

			static string file_name(string path);
			static string file_extension(string path);
			static string file_path(string path);

			static string top_directory(string path);
			static string parent_directory(string path);

			static string absolute_path(string path);
			static string join_path(string base, string extended);
		};


	}
}

#endif //MAXOS_FILESYSTEM_PATH_H
