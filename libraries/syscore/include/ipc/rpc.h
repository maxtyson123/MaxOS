/**
 * @file rpc.h
 * @brief Defines Remote Procedure Call (RPC) structures and functions for inter-process communication
 *
 * @date 30th November 2025
 * @author Max Tyson
 *
 * @todo: C api
 */

#ifndef SYSCORE_IPC_RPC_H
#define SYSCORE_IPC_RPC_H

#include <cstdint>
#include <cstddef>
#include <common.h>
#include <ipc/messages.h>
#include <processes/process.h>

namespace syscore::ipc {

	/**
	 * @enum RPCMessageType
	 * @brief The type of RPC message being sent
	 */
	enum class RPCMessageType {
		CALL,
		REPLY,
		SIGNAL,
		ERROR
	};

	/**
	 * @enum RPCMEssageFlags
	 * @brief Flags for RPC messages
	 */
	enum class RPCMEssageFlags {
		NONE 				= 0,
		ONE_WAY 			= (1 << 0),
		URGENT  			= (1 << 1),
		DESCRIPTOR_AS_DATA	= (1 << 2),
	};

	/**
	 * @struct RPCHeader
	 * @brief An IPC message header for RPC communication metadata
	 *
	 * @typedef rpc_message_t
	 * @brief Alias for RPCHeader struct
	 */
	typedef struct RPCHeader {

		size_t source_pid;
		size_t flags;
		RPCMessageType type;

	} rpc_header_t;

	/**
	 * @enum ArgType
	 * @brief The type of argument being passed to an RPC function
	 */
	enum class ArgType {
		NONE,
		UINT32,
		UINT64,
		INT32,
		INT64,
		STRING,
		BLOB,
	};

	/**
	 * @struct ArgEntry
	 * @brief An entry representing an argument in an RPC call
	 *
	 * @typedef arg_entry_t
	 * @brief Alias for ArgEntry struct
	 */
	typedef struct ArgEntry {
		ArgType  type;
		uint32_t length;	/// Length of the argument data
		uint32_t offset;    /// Offset in the payload where the argument data starts
	} arg_entry_t;


	constexpr size_t MAX_ARGS = 16; 	///< The maximum number of arguments that can be passed to an RPC function
	constexpr size_t MAX_PAYLOAD_SIZE = 1024; ///< The maximum size of the payload for RPC arguments
	constexpr size_t MAX_SERIALIZED_SIZE = MAX_ARGS * sizeof(arg_entry_t) + MAX_PAYLOAD_SIZE; ///< The maximum size of the serialized ArgList

	class ArgList {

		private:
			uint8_t  m_payload[MAX_PAYLOAD_SIZE];
			size_t   m_payload_size = 0;

			arg_entry_t m_entries[MAX_ARGS];
			size_t   	m_entry_count = 0;

		public:
			ArgList();
			~ArgList();

			void clear();

			bool add_arg(ArgType type, const void* data, size_t length);
			bool get_arg(size_t index, ArgType& type, void* buffer, size_t buffer_size) const;
			size_t arg_count() const;

			void push_uint32(uint32_t value);
			void push_uint64(uint64_t value);
			void push_int32(int32_t value);
			void push_int64(int64_t value);
			void push_string(const char* value);
			void push_blob(const void* data, size_t length);

			uint32_t get_uint32(size_t index) const;
			uint64_t get_uint64(size_t index) const;
			int32_t  get_int32(size_t index) const;
			int64_t  get_int64(size_t index) const;
			const char* get_string(size_t index) const;
			const void* get_blob(size_t index) const;

			const uint8_t* payload() const;
			size_t payload_size() const;

			void append_args(const ArgList& other, size_t start_index = 0);

			size_t serialise(uint8_t* buffer, size_t size);
			void deserialise(const uint8_t* buffer, size_t size);
	};

	/**
	 * @typedef rpc_function_t
	 * @brief Type definition for RPC functions
	 *
	 * @param args The arguments passed to the function
	 * @param return_values The return values from the function
	 * @param return_values_count The number of return values
	 */
	typedef void (* rpc_function_t)(ArgList* args, ArgList* return_values);

	/**
	 * @struct FunctionEntry
	 * @brief An entry in the RPC function registry
	 *
	 * @typedef function_entry_t
	 * @brief Alias for FunctionEntry struct
	 */
	typedef struct FunctionEntry {

		const char* name;
		rpc_function_t function;

	} function_entry_t;


	constexpr int MAX_RPC_FUNCTIONS = 128;                                	///< The maximum number of functions that can be registered for RPC (TODO: add std::vector support)
	inline function_entry_t m_rpc_function_registry[MAX_RPC_FUNCTIONS];    	///< The registry of functions
	inline int m_rpc_functions = 0;                                        	///< The number of registered functions
	inline uint64_t m_response_endpoint = 0;                               	///< The response endpoint for RPC calls

	void register_function(const char* name, rpc_function_t function);
	function_entry_t find_function(const char* name);

	bool rpc_call(const char* server, const char* function, ArgList* args, ArgList* return_values, size_t flags);
	[[noreturn]] void rpc_server_loop(const char* server);


}

#endif //SYSCORE_IPC_RPC_H
