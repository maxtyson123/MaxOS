//
// Created by 98max on 30/12/2024.
//

#ifndef MAXOS_RUNTIME_UBSAN_H
#define MAXOS_RUNTIME_UBSAN_H

#include <stdint.h>

namespace MaxOS {
    namespace runtime {

        #define ubsan_aligned(value, alignment) !(value & (alignment - 1))

        typedef struct source_location {
          const char *file;
          uint32_t line;
          uint32_t column;
        } source_location_t;

        typedef struct type_descriptor {
          uint16_t kind;
          uint16_t info;
          char name[];
        } type_descriptor_t;

        typedef struct type_mismatch_info {
          source_location_t location;
          type_descriptor_t* type;
          uintptr_t alignment;
          uint8_t type_check_kind;
        } type_mismatch_info_t;

        typedef struct type_mismatch_info_v1 {
          source_location_t location;
          type_descriptor_t* type;
          unsigned char log_alignment;
          unsigned char type_check_kind;
        } type_mismatch_info_v1_t;

        typedef struct overflow_info {
          source_location_t location;
          type_descriptor_t* base_type;
        } overflow_info_t;

        typedef struct shift_out_of_bounds_info {
          source_location_t location;
          type_descriptor_t* left_type;
          type_descriptor_t* right_type;
        } shift_out_of_bounds_info_t;

        typedef struct out_of_bounds_info {
          source_location_t location;
          type_descriptor_t* array_type;
          type_descriptor_t* index_type;
        } out_of_bounds_info_t;

        typedef struct location_only_info {
          source_location_t location;
        } location_only_info_t;

        typedef struct invalid_value_info {
          source_location_t location;
          type_descriptor_t* type;
        } invalid_value_info_t;

        const char *Type_Check_Kinds[] = {
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


        typedef struct vla_bound_not_positive_info{
                source_location_t location;
                type_descriptor_t* type;
        } vla_bound_not_positive_info_t;

        /**
         * @class UBSanHandler
         * @brief Handles undefined behaviour sanitizer
         */
        class UBSanHandler {

          public:
              UBSanHandler();
              ~UBSanHandler();

              static void handle(source_location_t location);

              static void print_type_mismatch(type_mismatch_info_t* info, uintptr_t ptr);
              static void print_type_mismatch_v1(type_mismatch_info_v1_t* info, uintptr_t ptr);

        };

    }

}


#endif // MAXOS_RUNTIME_UBSAN_H
