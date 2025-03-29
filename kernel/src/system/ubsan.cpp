//
// Created by 98max on 30/12/2024.
//

#include <system/ubsan.h>
#include <common/kprint.h>

using namespace MaxOS;
using namespace MaxOS::system;


UBSanHandler::UBSanHandler() {

}

UBSanHandler::~UBSanHandler() {

}

/**
 * @brief Handles the UBSan error (currently only printing the location and panicking)
 * @param location The location of the error
 */
void UBSanHandler::handle(source_location_t location) {

  // Print the location
  ASSERT(false, "UBSAN ERROR AT %s:%d:%d\n", location.file, location.line, location.column);


}

/**
 * @brief Prints the type mismatch error for UBSan
 *
 * @param info The type mismatch info
 * @param ptr The pointer to the object
 */
void UBSanHandler::print_type_mismatch(type_mismatch_info_t *info, uintptr_t ptr) {

  // Print the error
  _kprintf("UBSan: ");
  if(info -> alignment != 0 && ubsan_aligned(ptr, info -> alignment))
    _kprintf("%h misaligned memory access\n");
  else
    _kprintf("%h %s address 0x%x with insufficient space for an object of type %s\n", Type_Check_Kinds[info -> type_check_kind], ptr, info -> type -> name);

  // Print the location
  handle(info -> location);
}

/**
 * @brief Prints the type mismatch error for UBSan v1
 *
 * @param info The type mismatch info
 * @param ptr The pointer to the object
 */
void UBSanHandler::print_type_mismatch_v1(type_mismatch_info_v1_t *info, uintptr_t ptr) {

  // Print the error
  _kprintf("UBSan: ");
  if(info -> log_alignment != 0 && ubsan_aligned(ptr, (1 << (info -> log_alignment))))
    _kprintf("%h misaligned memory access\n");
  else
    _kprintf("%h %s address 0x%x with insufficient space for an object of type %s\n", Type_Check_Kinds[info -> type_check_kind], ptr, info -> type -> name);

  // Print the location
  handle(info -> location);


}


extern "C" void __ubsan_handle_type_mismatch(type_mismatch_info_t *info, uintptr_t ptr) {
  UBSanHandler::print_type_mismatch(info, ptr);
  UBSanHandler::handle(info -> location);
}

extern "C" void __ubsan_handle_type_mismatch_v1(type_mismatch_info_v1_t *info, uintptr_t ptr) {
  UBSanHandler::print_type_mismatch_v1(info, ptr);
  UBSanHandler::handle(info -> location);
}

extern "C" void __ubsan_handle_pointer_overflow(overflow_info_t *info) {
  _kprintf("UBSan: Pointer overflow\n");
  UBSanHandler::handle(info -> location);

}

extern "C" void __ubsan_handle_sub_overflow(overflow_info_t *info) {
  _kprintf("UBSan: Subtraction overflow\n");
  UBSanHandler::handle(info -> location);

}

extern "C" void __ubsan_handle_shift_out_of_bounds(shift_out_of_bounds_info_t *info) {
  _kprintf("UBSan: Shift out of bounds\n");
  UBSanHandler::handle(info -> location);
}

extern "C" void __ubsan_handle_out_of_bounds(out_of_bounds_info_t *info) {
  _kprintf("UBSan: Array out of bounds\n");
  UBSanHandler::handle(info -> location);

}

extern "C" void __ubsan_handle_add_overflow(overflow_info_t* info) {
  _kprintf("UBSan: Addition overflow\n");
  UBSanHandler::handle(info -> location);

}

extern "C" void __ubsan_handle_divrem_overflow(overflow_info_t* info) {
  _kprintf("UBSan: Division overflow\n");
  UBSanHandler::handle(info -> location);

}

extern "C" void __ubsan_handle_negate_overflow(overflow_info_t* info) {
  _kprintf("UBSan: Negation overflow\n");
  UBSanHandler::handle(info -> location);

}

extern "C" void __ubsan_handle_builtin_unreachable(location_only_info_t* info) {
  _kprintf("UBSan: Unreachable code\n");
  UBSanHandler::handle(info -> location);

}

extern "C" void __ubsan_handle_mul_overflow(overflow_info_t* info) {
  _kprintf("UBSan: Multiplication overflow\n");
  UBSanHandler::handle(info -> location);

}

extern "C" void __ubsan_handle_load_invalid_value(invalid_value_info_t* info) {
  _kprintf("UBSan: Load of invalid value\n");
  UBSanHandler::handle(info -> location);

}

extern "C" void __ubsan_handle_missing_return(location_only_info_t* info) {
  _kprintf("UBSan: Missing return\n");
  UBSanHandler::handle(info -> location);
}