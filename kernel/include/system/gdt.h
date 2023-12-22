//
// Created by 98max on 10/3/2022.
//

#ifndef MAX_OS_SYSTEM_GDT_H
#define MAX_OS_SYSTEM_GDT_H

#include <stdint.h>
#include <system/multiboot.h>

namespace maxOS {
    namespace system {

        /**
         * @class SegmentDescriptor
         * @brief Stores data for the segment descriptors in the GDT
         */
        class SegmentDescriptor {
            private:
              uint16_t m_limit_lo;
              uint16_t m_base_lo;
              uint8_t m_base_hi;
              uint8_t m_type;
              uint8_t m_flags_limit_hi;
              uint8_t m_base_vhi;

            public:
              SegmentDescriptor(uint32_t base, uint32_t limit, uint8_t type);

              uint32_t base();
              uint32_t limit();

        } __attribute__((packed));


        /**
         * @class GlobalDescriptorTable
         * @brief Sets up the GDT in the CPU
         */
        class GlobalDescriptorTable {

          private:
              SegmentDescriptor m_null_segment_selector;
              SegmentDescriptor m_unused_segment_selector;
              SegmentDescriptor m_code_segment_selector;
              SegmentDescriptor m_data_segment_selector;
              SegmentDescriptor m_task_state_segment_selector;

          public:

              GlobalDescriptorTable(const multiboot_info& multiboot_header);
              ~GlobalDescriptorTable();

              uint16_t code_segment_selector();
              uint16_t data_segment_selector();
              uint16_t task_state_segment_selector();
          };
    }
}

#endif //MAX_OS_SYSTEM_GDT_H
