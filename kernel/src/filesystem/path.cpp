/**
 * @file path.cpp
 * @brief Implementation of a Path class for handling filesystem paths
 *
 * @date 2nd September 2025
 * @author Max Tyson
 */

#include <filesystem/path.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::filesystem;

/**
 * @brief Check if a path is valid
 *
 * @param path The path to check
 * @return True if the path is valid, false otherwise
 */
bool Path::valid(string path) {

	// Must not be empty
	if (path.length() == 0)
		return false;

	// Must start with a /
	if (path[0] != '/')
		return false;

	// Valid
	return true;

}

/**
 * @brief Check if a path is a file
 *
 * @param path The path to check
 * @return True if the path is a file, false otherwise
 */
bool Path::is_file(const string& path) {

	return file_name(path).length() > 0 && file_extension(path).length() > 0;
}


/**
 * @brief Get the file name component of a path if it exists or an empty string
 *
 * @param path The path to get the file name from
 * @return The file name or the original path if it does not exist
 */
string Path::file_name(string path) {

	// Find the last /
	int last_slash = -1;
	for (size_t i = 0; i < path.length(); i++)
		if (path[i] == '/')
			last_slash = i;

	// Make sure there was a slash to split
	if (last_slash == -1)
		return path;

	// Get the file name
	string file_name = path.substring(last_slash + 1, path.length() - last_slash - 1);
	return file_name;
}

/**
 * @brief Try to get the file extension from a path (assumes split by ".")
 *
 * @param path The path to get the file extension from
 * @return The file extension or the original path if it does not exist
 */
string Path::file_extension(string path) {

	// Find the last .
	int last_dot = -1;
	for (size_t i = 0; i < path.length(); i++)
		if (path[i] == '.')
			last_dot = i;

	// Make sure there was a dot to split
	if (last_dot == -1)
		return "";

	// Get the file extension (what is after the ".")
	string file_extension = path.substring(last_dot + 1, path.length() - last_dot - 1);
	return file_extension;

}

/**
 * @brief Finds the path to the directory the file is in
 *
 * @param path The path to get the file path from
 * @return The file path or the original path if it does not exist
 */
string Path::file_path(string path) {

	// Try to find the last /
	int last_slash = -1;
	for (size_t i = 0; i < path.length(); i++)
		if (path[i] == '/')
			last_slash = i;

	// Make sure there was a slash to split
	if (last_slash == -1)
		return path;

	// Get the file path
	string file_path = path.substring(0, last_slash);
	return file_path;

}

/**
 * @brief Get the top directory of a path
 *
 * @param path The path to get the top directory from
 * @return The top directory or the original path if it does not exist
 */
string Path::top_directory(string path) {

	// Find the first /
	int first_slash = -1;
	for (size_t i = 0; i < path.length(); i++)
		if (path[i] == '/')
			first_slash = i;

	// Make sure there was a slash to split
	if (first_slash == -1)
		return path;

	// Get the top directory
	string top_directory = path.substring(0, first_slash);
	return top_directory;
}

/**
 * @brief Get the parent directory of the path
 *
 * @param path The path to either the file or the directory
 * @return
 */
string Path::parent_directory(string path) {

	// Find the last /
	int last_slash = -1;
	for (size_t i = 0; i < path.length(); i++)
		if (path[i] == '/')
			last_slash = i;

	// If no slash or only root, return empty string
	if (last_slash <= 0)
		return "/";

	// Return substring up to last slash
	return path.substring(0, last_slash);
}

/**
 * @brief Calculates path with out any "../" or "./"
 *
 * @param path The path
 * @return The new path, direct from root
 */
string Path::absolute_path(string path) {

	// Split the path into components
	auto components = path.split("/");
	common::Vector<string> absolute_components;
	for (auto &component: components) {

		// Skip empty or self references
		if (component == "" || component == ".")
			continue;

		// Handle parent directory references
		if (component == "..") {
			if (!absolute_components.empty())
				absolute_components.pop_back();
		} else
			absolute_components.push_back(component);
	}

	// Join the components back into a path
	string absolute_path = "/";
	for(const auto& component : absolute_components)
		absolute_path += component + "/";

	// Remove trailing slash if file
	if(Path::is_file(absolute_path) && absolute_path.length() > 1)
		absolute_path = absolute_path.substring(0, absolute_path.length() - 1);

	return absolute_path;
}

/**
 * @brief Joins two paths together
 *
 * @param base The base path
 * @param extended What to add to the base path (if its from root, "/", then it will just return this)
 * @return The joint path
 */
string Path::join_path(string base, string extended) {

	// The new path is from root
	if(extended[0] == '/')
		return extended;

	return base + extended;
}
