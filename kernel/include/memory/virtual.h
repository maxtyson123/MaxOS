//
// Created by 98max on 2/11/2024.
//

#ifndef MAXOS_VIRTUAL_H
#define MAXOS_VIRTUAL_H

#include <stdint.h>
#include <stddef.h>

namespace MaxOS {
  namespace memory {

    // Useful for readability
    typedef void virtual_address_t;
    typedef void physical_address_t;

    class VirtualMemoryManager{

      private:
        uint64_t * m_pml4_root_address;

      public:
        VirtualMemoryManager();
        ~VirtualMemoryManager();

    };
  }
}


#endif // MAXOS_VIRTUAL_H
