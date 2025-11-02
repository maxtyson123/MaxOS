/**
 * @file ubsan.cpp
 * @brief Implementation of the Undefined Behavior Sanitizer (UBSan) handler
 *
 * @date 30th December 2024
 * @author Max Tyson
 */

#include <runtime/ubsan.h>
#include <common/logger.h>

using namespace MaxOS;
using namespace MaxOS::runtime;


UBSanHandler::UBSanHandler() = default;

UBSanHandler::~UBSanHandler() = default;

/**
 * @brief Handles the UBSan error (currently only printing the location and panicking)
 * @param location The location of the error
 * @param msg The message that informs about the error
 */
void UBSanHandler::handle(source_location_t location, const char* msg) {

	// Print the location
	ASSERT(false, "UBSAN ERROR AT %s:%d:%d - %s", location.file, location.line, location.column, msg);
}

/**
 * @brief Prints the type mismatch error for UBSan
 *
 * @param info The type mismatch info
 * @param ptr The pointer to the object
 */
void UBSanHandler::print_type_mismatch(type_mismatch_info_t* info, uintptr_t ptr) {

	// Print the error
	string msg;
	if (info->alignment != 0 && ubsan_aligned(ptr, info->alignment))
		msg = "misaligned memory access";
	else
		msg = (StringBuilder)TYPE_CHECK_KINDS[info->type_check_kind] << " address 0x" << ptr << " with insufficient space for an object of type " << info->type->name;

	// Print the location
	handle(info->location, msg.c_str());
}

/**
 * @brief Prints the type mismatch error for UBSan v1
 *
 * @param info The type mismatch info
 * @param ptr The pointer to the object
 */
void UBSanHandler::print_type_mismatch_v1(type_mismatch_info_v1_t* info, uintptr_t ptr) {

	// Print the error
	string msg;
	if (info->log_alignment != 0 && ubsan_aligned(ptr, (1 << (info->log_alignment))))
		msg = "misaligned memory access";
	else
		msg= StringBuilder() << TYPE_CHECK_KINDS[info->type_check_kind] << " address 0x" << ptr << " with insufficient space for an object of type " << info->type->name;

	// Print the location
	handle(info->location, msg.c_str());
}

/**
 * @brief Triggered when memory is accessed with an unexpected type
 *
 * @param info The mismatch between the two types
 * @param ptr The pointer to where the type mismatch occurred
 */
extern "C" void __ubsan_handle_type_mismatch(type_mismatch_info_t* info, uintptr_t ptr) {

	UBSanHandler::print_type_mismatch(info, ptr);
}

/**
 * @brief Triggered when memory is accessed with an unexpected type (UBSan v1 - includes more metadata)
 *
 * @param info The mismatch between the two types
 * @param ptr The pointer to where the type mismatch occurred
 */
extern "C" void __ubsan_handle_type_mismatch_v1(type_mismatch_info_v1_t* info, uintptr_t ptr) {

	UBSanHandler::print_type_mismatch_v1(info, ptr);
}

/**
 * @brief Triggered when adding/subbing to a memory address is outside the expected bounds
 *
 * @param info The overflow information
 */
extern "C" void __ubsan_handle_pointer_overflow(overflow_info_t* info) {

	UBSanHandler::handle(info->location, "Pointer overflow");

}

/**
 * @brief Triggered when subtracting below the minimum value for the type
 *
 * @param info The location and type of the overflow
 */
extern "C" void __ubsan_handle_sub_overflow(overflow_info_t* info) {

	UBSanHandler::handle(info->location, "Subtraction overflow");

}

/**
 * @brief Triggered when shifting beyond the capacity of the type
 *
 * @param info The shift out of bounds information
 */
extern "C" void __ubsan_handle_shift_out_of_bounds(shift_out_of_bounds_info_t* info) {

	UBSanHandler::handle(info->location, "Shift out of bounds");
}

/**
 * @brief Triggered when accessing an array at an index larger than its size (or negative)
 *
 * @param info The location and type of the out of bounds access
 */
extern "C" void __ubsan_handle_out_of_bounds(out_of_bounds_info_t* info) {

	UBSanHandler::handle(info->location, "Array out of bounds");

}

/**
 * @brief Triggered when adding beyond the maximum value for the type
 *
 * @param info The location and type of the overflow
 */
extern "C" void __ubsan_handle_add_overflow(overflow_info_t* info) {

	UBSanHandler::handle(info->location, "Addition overflow");

}

/**
 * @brief Triggered when dividing or taking the remainder results in an overflow (eg dividing INT_MIN by -1)
 *
 * @param info The location and type of the overflow
 */
extern "C" void __ubsan_handle_divrem_overflow(overflow_info_t* info) {

	UBSanHandler::handle(info->location, "Division overflow");

}

/**
 * @brief Triggered when negating (taking -x) when there is no corresponding positive/negative value
 *
 * @param info The location and type of the overflow
 */
extern "C" void __ubsan_handle_negate_overflow(overflow_info_t* info) {

	UBSanHandler::handle(info->location, "Negation overflow");

}

/**
 * @brief Triggered when reaching code that should be unreachable
 *
 * @param info The location of the unreachable code
 */
extern "C" void __ubsan_handle_builtin_unreachable(location_only_info_t* info) {

	UBSanHandler::handle(info->location, "Unreachable code");

}

/**
 * @brief Triggered when multiplying beyond the maximum value for the type
 *
 * @param info The location and type of the overflow
 */
extern "C" void __ubsan_handle_mul_overflow(overflow_info_t* info) {

	UBSanHandler::handle(info->location, "Multiplication overflow");

}

/**
 * @brief Triggered when loading a value that is invalid for its type (eg corrupted or uninitialised)
 *
 * @param info The location and type of the invalid value
 */
extern "C" void __ubsan_handle_load_invalid_value(invalid_value_info_t* info) {

	UBSanHandler::handle(info->location, "Load of invalid value");

}

/**
 * @brief Triggered when a function that is expected to return a value does not
 *
 * @param info The location of the missing return
 */
extern "C" void __ubsan_handle_missing_return(location_only_info_t* info) {

	UBSanHandler::handle(info->location, "Missing return");
}

/**
 * @brief Triggered when a variable length array (VLA) is declared with a non-positive bound
 *
 * @param info The location and type of the VLA bound
 */
extern "C" void __ubsan_handle_vla_bound_not_positive(vla_bound_not_positive_info_t* info) {

	UBSanHandler::handle(info->location,  "VLA bound not positive");
}