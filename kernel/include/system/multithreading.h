//
// Created by 98max on 15/11/2022.
//

#ifndef MAXOS_SYSTEM_MULTITHREADING_H
#define MAXOS_SYSTEM_MULTITHREADING_H

#include <system/multitasking.h>
#include <stdint.h>
#include <system/gdt.h>


namespace MaxOS{

  namespace system{

      /**
       * @class Thread
       * @brief A execution thread that can be scheduled by the ThreadManager
       */
      class Thread
      {
          friend class ThreadManager;
              private:
                  uint8_t m_stack[4096];
                  CPUState* m_cpu_state;
                  bool m_yield_status;
                  int m_tid;
              public:
                  Thread();
                  ~Thread();

                  void init(void entrypoint());
      };

      /**
       * @class ThreadManager
       * @brief Manages the scheduling of threads
       */
      class ThreadManager
      {
          private:
              // Vector for threads and stack
              common::Vector<Thread*> m_threads;

              uint32_t m_current_thread;
              system::GlobalDescriptorTable*m_gdt;

          public:
              ThreadManager();
              ThreadManager(system::GlobalDescriptorTable *gdt);
              ~ThreadManager();

              uint32_t create_thread(void entrypoint());
              CPUState*schedule(CPUState *cpu_state);

              bool terminate_thread(uint32_t tid);
      };
  }
}

#endif //MAXOS_SYSTEM_MULTITHREADING_H
