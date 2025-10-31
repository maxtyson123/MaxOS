/**
 * @file ubsan.h
 * @brief Defines structures and a handler for Undefined Behavior Sanitizer (UBSan) runtime errors
 *
 * @date 30th December 2024
 * @author Max Tyson
 */

#ifndef MAXOS_RUNTIME_UBSAN_H
#define MAXOS_RUNTIME_UBSAN_H

#include <stdint.h>

namespace MaxOS {
	namespace runtime {

		#define ubsan_aligned(value, alignment) !(value & (alignment - 1))

		/**
		 * @struct SourceLocation
		 * @brief The location in the source code where the undefined behaviour occurred
		 */
		typedef struct SourceLocation {
			const char* file;       ///< The name of the source file
			uint32_t line;          ///< The line number in the source file
			uint32_t column;        ///< The column number in the source file
		} source_location_t;

		/**
		 * @struct TypeDescriptor
		 * @brief Describes a type in the program
		 */
		typedef struct TypeDescriptor {
			uint16_t kind;  ///< The kind of type (e.g., struct, class, union)
			uint16_t info;  ///< Additional type information
			char name[];    ///< The name of the type
		} type_descriptor_t;

		/**
		 * @struct TypeMismatchInfo
		 * @brief Information about a type mismatch error
		 */
		typedef struct TypeMismatchInfo {
			source_location_t location; ///< The location of the type mismatch
			type_descriptor_t* type;    ///< The type that was expected
			uintptr_t alignment;        ///< The required alignment
			uint8_t type_check_kind;    ///< The kind of type check that failed
		} type_mismatch_info_t;

		/**
		 * @struct TypeMismatchInfoV1
		 * @brief Information about a type mismatch error (version 1)
		 */
		typedef struct TypeMismatchInfoV1 {
			source_location_t location;         ///< The location of the type mismatch
			type_descriptor_t* type;            ///< The type that was expected
			unsigned char log_alignment;        ///< The log2 of the required alignment
			unsigned char type_check_kind;      ///< The kind of type check that failed
		} type_mismatch_info_v1_t;

		/**
		 * @struct OverflowInfo
		 * @brief Information about an overflow error
		 */
		typedef struct OverflowInfo {
			source_location_t location; 	///< The location of the overflow
			type_descriptor_t* base_type;   ///< The base type of the operation
		} overflow_info_t;

		/**
		 * @struct ShiftOutOfBoundsInfo
		 * @brief Information about a shift out of bounds error
		 */
		typedef struct ShiftOutOfBoundsInfo {
			source_location_t location;     ///< The location of the shift out of bounds
			type_descriptor_t* left_type;   ///< The type of the left operand
			type_descriptor_t* right_type;  ///< The type of the right operand
		} shift_out_of_bounds_info_t;

		/**
		 * @struct OutOfBoundsInfo
		 * @brief Information about an out of bounds error
		 */
		typedef struct OutOfBoundsInfo {
			source_location_t location;     ///< The location of the out of bounds access
			type_descriptor_t* array_type;  ///< The type of the array
			type_descriptor_t* index_type;  ///< The type of the index
		} out_of_bounds_info_t;

		/**
		 * @struct LocationOnlyInfo
		 * @brief Information that an error that only has a location
		 */
		typedef struct LocationOnlyInfo {
			source_location_t location;     ///< The location of the error
		} location_only_info_t;

		/**
		 * @struct InvaildValueInfo
		 * @brief Information about an invalid value error
		 */
		typedef struct InvaildValueInfo {
			source_location_t location;     ///< The location of the invalid value
			type_descriptor_t* type;        ///< The type of the invalid value
		} invalid_value_info_t;

		/**
		 * @struct VLABoundNotPositiveInfo
		 * @brief Information about a VLA bound not positive error
		 */
		typedef struct VLABoundNotPositiveInfo {
			source_location_t location;     ///< The location of the VLA bound not positive
			type_descriptor_t* type;        ///< The type of the VLA bound
		} vla_bound_not_positive_info_t;

		/// List of type check errors
		const char* TYPE_CHECK_KINDS[] = {
				"load of",
				"store to",
				"reference binding to",
				"member access within",
				"member call on",
				"constructor call on",
				"downcast of",
				"downcast of",
				"upcast of",
				"cast to virtual base of",
		};

		/**
		 * @class UBSanHandler
		 * @brief Handles undefined behaviour sanitizer runtime errors
		 */
		class UBSanHandler {

			public:
				UBSanHandler();
				~UBSanHandler();

				static void handle(source_location_t location, const char* msg);

				static void print_type_mismatch(type_mismatch_info_t* info, uintptr_t ptr);
				static void print_type_mismatch_v1(type_mismatch_info_v1_t* info, uintptr_t ptr);
		};
	}
}


#endif // MAXOS_RUNTIME_UBSAN_H
