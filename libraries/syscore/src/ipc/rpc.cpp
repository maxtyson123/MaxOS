/**
 * @file rpc.cpp
 * @brief Implements Remote Procedure Call (RPC) structures and functions for inter-process communication
 *
 * @date 30th November 2025
 * @author Max Tyson
 *
 * @todo may need to optimise for less syscalls per RPC round trip
 */

#include <ipc/rpc.h>

using namespace syscore;
using namespace syscore::ipc;

ArgList::ArgList() = default;
ArgList::~ArgList() = default;

/**
 * @brief Resets the arg payload and entries
 */
void ArgList::clear() {

	m_payload_size = 0;
	m_entry_count = 0;

}

/**
 * @brief Adds an argument to the argument list
 *
 * @param type The type of argument
 * @param data The argument data
 * @param length The length of the argument data
 * @return
 */
bool ArgList::add_arg(ArgType type, void const* data, size_t length) {

	// Not enough space to add more args
	if (m_entry_count >= MAX_ARGS || m_payload_size + length > sizeof(m_payload))
		return false;

	// Create the entry
	m_entries[m_entry_count].type = type;
	m_entries[m_entry_count].offset = m_payload_size;
	m_entries[m_entry_count].length = length;

	// Copy the data into the payload
	memcpy(&m_payload[m_payload_size], data, length);
	m_payload_size += length;
	m_entry_count++;

	return true;
}

/**
 * @brief Gets an argument from the argument list
 *
 * @param index The index of the argument to get
 * @param type The type of the argument
 * @param buffer The buffer to store the argument data
 * @param buffer_size The size of the buffer
 * @param out_length The length of the argument data
 * @return True if the argument was retrieved successfully, false otherwise
 */
bool ArgList::get_arg(size_t index, ArgType& type, void* buffer, size_t buffer_size) const {

	// Index out of bounds
	if (index >= m_entry_count)
		return false;

	// Get the entry
	const arg_entry_t& entry = m_entries[index];
	type = entry.type;

	// Make sure buffer is large enough and the type matches
	if (buffer_size < entry.length || entry.type != type)
		return false;

	// Copy the data into the buffer
	memcpy(buffer, &m_payload[entry.offset], entry.length);
	return true;
}

/**
 * @brief Gets the number of arguments in the argument list
 *
 * @return The number of arguments
 */
size_t ArgList::arg_count() const {

	return m_entry_count;
}

/**
 * @brief Adds a uint32_t argument to the end of the argument list
 *
 * @param value The uint32_t value to add
 */
void ArgList::push_uint32(uint32_t value) {

	add_arg(ArgType::UINT32, &value, sizeof(uint32_t));

}

/**
 * @brief Adds a uint64_t argument to the end of the argument list
 *
 * @param value The uint64_t value to add
 */
void ArgList::push_uint64(uint64_t value) {

	add_arg(ArgType::UINT64, &value, sizeof(uint64_t));

}

/**
 * @brief Adds an int32_t argument to the end of the argument list
 *
 * @param value The int32_t value to add
 */
void ArgList::push_int32(int32_t value) {

	add_arg(ArgType::INT32, &value, sizeof(int32_t));

}

/**
 * @brief Adds an int64_t argument to the end of the argument list
 *
 * @param value The int64_t value to add
 */
void ArgList::push_int64(int64_t value) {

	add_arg(ArgType::INT64, &value, sizeof(int64_t));

}

/**
 * @brief Adds a string argument to the end of the argument list
 *
 * @param value The string value to add
 */
void ArgList::push_string(const char* value) {

	size_t length = strlen(value) + 1; // Include null terminator
	add_arg(ArgType::STRING, value, length);

}

/**
 * @brief Adds a blob argument to the end of the argument list
 *
 * @param data The blob data to add
 * @param length The length of the blob data
 */
void ArgList::push_blob(const void* data, size_t length) {

	add_arg(ArgType::BLOB, data, length);

}

/**
 * @brief Gets a uint32_t argument from the argument list
 *
 * @param index The index of the argument to get
 * @return The uint32_t value
 */
uint32_t ArgList::get_uint32(size_t index) const {

	// Get the argument
	uint32_t value = 0;
	ArgType type;
	get_arg(index, type, &value, sizeof(uint32_t));
	return value;

}

/**
 * @brief Gets a uint64_t argument from the argument list
 *
 * @param index The index of the argument to get
 * @return The uint64_t value
 */
uint64_t ArgList::get_uint64(size_t index) const {

	// Get the argument
	uint64_t value = 0;
	ArgType type;
	get_arg(index, type, &value, sizeof(uint64_t));
	return value;

}

/**
 * @brief Gets an int32_t argument from the argument list
 *
 * @param index The index of the argument to get
 * @return The int32_t value
 */
int32_t ArgList::get_int32(size_t index) const {

	// Get the argument
	int32_t value = 0;
	ArgType type;
	get_arg(index, type, &value, sizeof(int32_t));
	return value;

}

/**
 * @brief Gets an int64_t argument from the argument list
 *
 * @param index The index of the argument to get
 * @return The int64_t value
 */
int64_t ArgList::get_int64(size_t index) const {

	// Get the argument
	int64_t value = 0;
	ArgType type;
	get_arg(index, type, &value, sizeof(int64_t));
	return value;

}

/**
 * @brief Gets a string argument from the argument list
 *
 * @param index The index of the argument to get
 * @return The string value
 *
 */
const char* ArgList::get_string(size_t index) const {

	// Get the argument
	const arg_entry_t& entry = m_entries[index];
	return (const char*)(m_payload + entry.offset);

}

/**
 * @brief Gets a blob argument from the argument list
 *
 * @param index The index of the argument to get
 * @param out_length The length of the blob data
 * @return The blob data
 */
const void* ArgList::get_blob(size_t index) const {

	// Get the argument
	const arg_entry_t& entry = m_entries[index];
	return (void*)(m_payload + entry.offset);

}

/**
 * @brief Gets the payload of the argument list
 *
 * @return The payload data
 */
const uint8_t* ArgList::payload() const {

	return m_payload;
}

/**
 * @brief Gets the size of the payload of the argument list
 *
 * @return The payload size
 */
size_t ArgList::payload_size() const {

	return m_payload_size;
}

/**
 * @brief Appends arguments from another ArgList to this one
 *
 * @param other The other ArgList to append from
 * @param start_index Where in the other ArgList to begin appending from (defaults to 0)
 */
void ArgList::append_args(ArgList const& other,  size_t start_index) {

	// Append each argument from the other ArgList
	for (size_t i = start_index; i < other.m_entry_count; ++i)
		add_arg(other.m_entries[i].type, &other.m_payload[other.m_entries[i].offset], other.m_entries[i].length);

}

/**
 * @brief Converts the argument list into a buffer containing the type and data of each argument in the format [arg count][arg entries][payload]
 *
 * @param buffer The buffer to write the serialized data to
 * @param size The size of the buffer
 * @return The size of the serialized data
 *
 * @todo validation
 */
size_t ArgList::serialise(uint8_t* buffer, size_t size) {

	size_t offset = 0;
	size_t space_needed = sizeof(m_entry_count) + m_entry_count * sizeof(arg_entry_t) + payload_size();

	// Check bounds
	if(space_needed > size)
		return 0;

	// Arg count
	memcpy((void*)(buffer + offset), &m_entry_count, sizeof(m_entry_count));
	offset += sizeof(m_entry_count);

	// Arg entries
	memcpy((void*)(buffer + offset), &m_entries, m_entry_count * sizeof(arg_entry_t));
	offset += m_entry_count * sizeof(arg_entry_t);

	// Payload data
	memcpy((void*)(buffer + offset), &m_payload, m_payload_size);
	offset += m_payload_size;

	return offset;
}

/**
 * @brief Loads a buffer of serialised ArgList args into this list
 *
 * @param buffer The buffer to read the serialized data from
 * @param size The size of the buffer
 * @return The size of the now deserialized data
 */
void ArgList::deserialise(const uint8_t* buffer, size_t size) {

	// Reset
	clear();
	size_t offset = 0;

	// Arg count
	memcpy(&m_entry_count, (void*)(buffer + offset), sizeof(m_entry_count));
	offset += sizeof(m_entry_count);

	// Arg entries
	memcpy(&m_entries, (void*)(buffer + offset), m_entry_count * sizeof(arg_entry_t));
	offset += m_entry_count * sizeof(arg_entry_t);

	// Payload data
	m_payload_size = size - (sizeof(m_entry_count) + m_entry_count * sizeof(arg_entry_t));
	memcpy(&m_payload, (void*)(buffer + offset), m_payload_size);
	offset += m_payload_size;
}


/**
 * @brief Registers an RPC function under a specific name
 *
 * @param name The name of the function
 * @param function The function to register
 */
void syscore::ipc::register_function(const char* name, rpc_function_t function) {

	// Check enough space in the registry
	if (m_rpc_functions >= MAX_RPC_FUNCTIONS)
		return;

	// Create the function entry
	function_entry_t function_entry;
	function_entry.name = name;
	function_entry.function = function;

	// Add the function to the registry
	m_rpc_function_registry[m_rpc_functions++] = function_entry;
}

/**
 * @brief Finds a registered RPC function by name
 *
 * @param name The name of the function to find
 * @return The function entry or nullptr if not found
 */
function_entry_t syscore::ipc::find_function(const char* name) {

	for (int i = 0; i < m_rpc_functions; ++i) {
		if (strcmp(m_rpc_function_registry[i].name, name) == 0)
			return m_rpc_function_registry[i];

	}

	return {nullptr, nullptr};

}

/**
 * @brief Calls a remote procedure on a server
 *
 * @param server The name of the server
 * @param function The name of the function to call
 * @param args The arguments to pass to the function
 * @param return_values The return values from the function
 * @return True if the call was successful, false otherwise
 *
 * @todo: not multi-thread safe for m_response_endpoint
 * @todo: unique endpoint name
 * @todo: header
 */
bool syscore::ipc::rpc_call(const char* server, const char* function, ArgList* args, ArgList* return_values) {

	// Open the server
	uint64_t endpoint = open_endpoint(server);
	if (endpoint == 0)
		return false;

	// Set up a response endpoint if not already done
	if (m_response_endpoint == 0) {
		m_response_endpoint = create_endpoint("response");
		if (m_response_endpoint == 0)
			return false;
	}

	// Add the header to the message
	rpc_header_t header;
	header.source_pid = 0;
	header.flags = 0;
	header.type = 0;

	// Create a message: name, response endpoint, args
	ArgList message;
	message.push_blob(&header, sizeof(RPCHeader));
	message.push_string(function);
	message.push_string("response");
	message.append_args(*args);

	// Send the message
	uint8_t buffer[MAX_SERIALIZED_SIZE];
	size_t size = message.serialise(buffer, sizeof(buffer));
	send_message(endpoint, (void*)buffer, size);

	// Read the response
	size = read_message(m_response_endpoint, buffer, sizeof(buffer));
	return_values -> deserialise(buffer, size);

	return true;
}

/**
 * @brief Starts the RPC server loop to delegate incoming RPC calls to registered functions.
 *
 * @param server The name of the server
 *
 * @note Will yield the thread when no messages are available.
 */
[[noreturn]] void syscore::ipc::rpc_server_loop(const char* server) {

	// Create the endpoint
	uint64_t endpoint = create_endpoint(server);
	if (endpoint == 0)
		return;

	// Server loop
	while (true) {

		// Read the message
		ArgList message;
		uint8_t buffer[MAX_SERIALIZED_SIZE];
		size_t size = read_message(endpoint, buffer, sizeof(buffer));
		message.deserialise(buffer, size);

		// Load the message header
		auto header = (rpc_header_t*)message.get_blob(0);

		// Get the function and response endpoint
		function_entry_t function_entry = find_function(message.get_string(1));
		uint64_t response_endpoint 		= open_endpoint(message.get_string(2));

		// Validate request
		if (response_endpoint == 0 || function_entry.function == nullptr)
			continue;

		// Extract the arguments
		ArgList args;
		args.append_args(message, 3);

		// Delegate to the function
		ArgList return_values;
		function_entry.function(&args, &return_values);
		size = return_values.serialise(buffer, sizeof(buffer));
		send_message(response_endpoint, (void*)buffer, size);
	}
}