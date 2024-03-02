//
// Created by 98max on 2/11/2024.
//

#ifndef MAXOS_VIRTUAL_H
#define MAXOS_VIRTUAL_H

#include <stdint.h>
#include <stddef.h>
#include <memory/physical.h>

namespace MaxOS {
  namespace memory {


    class VirtualMemoryManager{

      private:
        uint64_t * m_pml4_root_address;
        PhysicalMemoryManager* m_physical_memory_manager;



      public:
        VirtualMemoryManager(PhysicalMemoryManager* physical_memory_manager);
        ~VirtualMemoryManager();

    };
  }
}


#endif // MAXOS_VIRTUAL_H
