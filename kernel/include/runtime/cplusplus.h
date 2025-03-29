//
// Created by 98max on 30/03/2025.
//

#ifndef MAXOS_RUNTIME_CPLUSPLUS_H
#define MAXOS_RUNTIME_CPLUSPLUS_H

#include <common/kprint.h>

namespace MaxOS {
    namespace runtime {


      // Thanks: https://wiki.osdev.org/C%2B%2B

      //Define what a constructor is
      typedef void (*constructor)();

      typedef unsigned uarch_t;

      extern "C" {

      struct atexit_func_entry_t
      {
        /*
	* Each member is at least 4 bytes large. Such that each entry is 12bytes.
	* 128 * 12 = 1.5KB exact.
	**/
        void (*destructor_func)(void *);
        void *obj_ptr;
        void *dso_handle;
      };

      int __cxa_atexit(void (*f)(void *), void *objptr, void *dso);
      void __cxa_finalize(void *f);

      }
    }
}

#endif // MAXOS_RUNTIME_CPLUSPLUS_H
