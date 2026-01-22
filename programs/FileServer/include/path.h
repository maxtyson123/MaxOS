/**
 * @file path.h
 * @brief Provides utilities for handling file and directory paths
 *
 * @date 2nd September 2025
 * @author Max Tyson
 */

#ifndef FILESERVER_PATH_H
#define FILESERVER_PATH_H

#include <string.h>


namespace FileServer {

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

			static MaxOS::string top_directory(const MaxOS::string& path);
			static MaxOS::string parent_directory(const MaxOS::string& path);

			static MaxOS::string absolute_path(const MaxOS::string& path);
			static MaxOS::string join_path(const MaxOS::string& base, const MaxOS::string& extended);
	};


}


#endif //FILESERVER_PATH_H
