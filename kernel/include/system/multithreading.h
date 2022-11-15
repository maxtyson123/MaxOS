//
// Created by 98max on 15/11/2022.
//

#ifndef MAXOS_SYSTEM_MULTITHREADING_H
#define MAXOS_SYSTEM_MULTITHREADING_H

#include <common/types.h>
#include <system/gdt.h>


namespace maxOS{

    struct CPUState_Thread
    {
        common::uint32_t eax;
        common::uint32_t ebx;
        common::uint32_t ecx;
        common::uint32_t edx;

        common::uint32_t esi;
        common::uint32_t edi;
        common::uint32_t ebp;

        /*
        common::uint32_t gs;
        common::uint32_t fs;
        common::uint32_t es;
        common::uint32_t ds;
        */
        common::uint32_t error;

        common::uint32_t eip;
        common::uint32_t cs;
        common::uint32_t eflags;
        common::uint32_t esp;
        common::uint32_t ss;
    }  __attribute__((packed));


    class Thread
    {
        friend class ThreadManager;
        private:
            common::uint8_t stack[4096];                // 4 KiB
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
            static common::uint8_t stack[256][5012];
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
