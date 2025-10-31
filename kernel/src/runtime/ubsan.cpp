//
// Created by 98max on 30/12/2024.
//

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

extern "C" void __ubsan_handle_type_mismatch(type_mismatch_info_t* info, uintptr_t ptr) {

	UBSanHandler::print_type_mismatch(info, ptr);
}

extern "C" void __ubsan_handle_type_mismatch_v1(type_mismatch_info_v1_t* info, uintptr_t ptr) {

	UBSanHandler::print_type_mismatch_v1(info, ptr);
}

extern "C" void __ubsan_handle_pointer_overflow(overflow_info_t* info) {

	UBSanHandler::handle(info->location, "Pointer overflow");

}

extern "C" void __ubsan_handle_sub_overflow(overflow_info_t* info) {

	UBSanHandler::handle(info->location, "Subtraction overflow");

}

extern "C" void __ubsan_handle_shift_out_of_bounds(shift_out_of_bounds_info_t* info) {

	UBSanHandler::handle(info->location, "Shift out of bounds");
}

extern "C" void __ubsan_handle_out_of_bounds(out_of_bounds_info_t* info) {

	UBSanHandler::handle(info->location, "Array out of bounds");

}

extern "C" void __ubsan_handle_add_overflow(overflow_info_t* info) {

	UBSanHandler::handle(info->location, "Addition overflow");

}

extern "C" void __ubsan_handle_divrem_overflow(overflow_info_t* info) {

	UBSanHandler::handle(info->location, "Division overflow");

}

extern "C" void __ubsan_handle_negate_overflow(overflow_info_t* info) {

	UBSanHandler::handle(info->location, "Negation overflow");

}

extern "C" void __ubsan_handle_builtin_unreachable(location_only_info_t* info) {

	UBSanHandler::handle(info->location, "Unreachable code");

}

extern "C" void __ubsan_handle_mul_overflow(overflow_info_t* info) {

	UBSanHandler::handle(info->location, "Multiplication overflow");

}

extern "C" void __ubsan_handle_load_invalid_value(invalid_value_info_t* info) {

	UBSanHandler::handle(info->location, "Load of invalid value");

}

extern "C" void __ubsan_handle_missing_return(location_only_info_t* info) {

	UBSanHandler::handle(info->location, "Missing return");
}

extern "C" void __ubsan_handle_vla_bound_not_positive(vla_bound_not_positive_info_t* info) {

	UBSanHandler::handle(info->location,  "VLA bound not positive");
}