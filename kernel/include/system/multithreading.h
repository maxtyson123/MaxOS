//
// Created by 98max on 15/11/2022.
//

#ifndef MAXOS_SYSTEM_MULTITHREADING_H
#define MAXOS_SYSTEM_MULTITHREADING_H

#include <stdint.h>
#include <system/gdt.h>


namespace maxOS{

    struct CPUState_Thread
    {
        uint32_t eax;
        uint32_t ebx;
        uint32_t ecx;
        uint32_t edx;

        uint32_t esi;
        uint32_t edi;
        uint32_t ebp;

        /*
        uint32_t gs;
        uint32_t fs;
        uint32_t es;
        uint32_t ds;
        */
        uint32_t error;

        uint32_t eip;
        uint32_t cs;
        uint32_t eflags;
        uint32_t esp;
        uint32_t ss;
    }  __attribute__((packed));


    class Thread
    {
        friend class ThreadManager;
        private:
            uint8_t stack[4096];                // 4 KiB
            CPUState_Thread* cpustate;
            bool yieldStatus;                           // if true, Thread will be yielded
            int tid;                                    // thread id
        public:
            Thread(system::GlobalDescriptorTable *gdt, void entrypoint());
            Thread(void entrypoint());
            void init(system::GlobalDescriptorTable *gdt, void entrypoint());
            ~Thread();
    };


    class ThreadManager
    {
        private:
            static uint8_t stack[256][5012];
            static Thread* Threads[256];
            static int numThreads;
            static int currentThread;
            static system::GlobalDescriptorTable *gdt;
        public:
            ThreadManager();
            ThreadManager(system::GlobalDescriptorTable *gdt);
            ~ThreadManager();
            int CreateThread(void entrypoint());
            CPUState_Thread* Schedule(CPUState_Thread* cpustate);
            bool TerminateThread(int tid);
            bool JoinThreads(int other);
            bool CheckThreads(int tid);
            void YieldThreads(int tid);
    };

}

#endif //MAXOS_SYSTEM_MULTITHREADING_H
