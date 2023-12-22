//
// Created by 98max on 15/11/2022.
//

#ifndef MAXOS_SYSTEM_MULTITHREADING_H
#define MAXOS_SYSTEM_MULTITHREADING_H

#include <system/multitasking.h>
#include <stdint.h>
#include <system/gdt.h>


namespace maxOS{

  namespace system{

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

                  void init(system::GlobalDescriptorTable *gdt, void entrypoint());
      };


      class ThreadManager
      {
          private:
              // Vector for threads and stack
              common::Vector<Thread*> m_threads;

              int m_current_thread;
              system::GlobalDescriptorTable*m_gdt;

          public:
              ThreadManager();
              ThreadManager(system::GlobalDescriptorTable *gdt);
              ~ThreadManager();

              int create_thread(void entrypoint());
              CPUState*schedule(CPUState *cpu_state);

              bool terminate_thread(int tid);
      };
  }
}

#endif //MAXOS_SYSTEM_MULTITHREADING_H
