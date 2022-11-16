//
// Created by 98max on 15/11/2022.
//
#include <multithreading.h>

#define nullptr 0

using namespace maxOS;
using namespace maxOS::common;
void printf(char *str);

int ThreadManager::numThreads = 0;
int ThreadManager::currentThread = -1;
Thread *ThreadManager::Threads[256] = {nullptr};
GlobalDescriptorTable *ThreadManager::gdt;
common::uint8_t ThreadManager::stack[256][5012];

void Thread::init(GlobalDescriptorTable *gdt, void entrypoint())
{
    cpustate = (CPUState_Thread *)(stack + 4096 - sizeof(CPUState_Thread));

    cpustate->eax = 0;
    cpustate->ebx = 0;
    cpustate->ecx = 0;
    cpustate->edx = 0;

    cpustate->esi = 0;
    cpustate->edi = 0;
    cpustate->ebp = 0;

    cpustate->eip = (uint32_t)entrypoint;
    cpustate->eflags = 0x202;
    yieldStatus = false;
}

Thread::~Thread()
{
}

ThreadManager::ThreadManager()
{
}

ThreadManager::ThreadManager(GlobalDescriptorTable *gdt)
{
    this->gdt = gdt;
}

ThreadManager::~ThreadManager()
{
}

/**
 * @brief Add a thread to an empty place in the array
 *
 * @param Thread thread to add
 * @return true if succesfully added
 * @return false if error
 */
int ThreadManager::CreateThread(void entrypoint())
{

    if (numThreads >= 256)                                                  // if there are more than 256 threads, return false
        return false;

    int i = 0;
    while (i <= 256)                                                        // find an empty place in the array
    {
        if (Threads[i] == nullptr)                                          // if the place is empty
        {
            Thread *th = (Thread *)(stack[i] + (5012 - sizeof(Thread)));    // init space for thread
            th->init(gdt, entrypoint);                                      // init thread
            th->tid = i;                                                    // set thread id
            Threads[numThreads] = th;                                       // add thread to array
            Threads[numThreads]->cpustate->cs = gdt->CodeSegmentSelector(); // set code segment
            numThreads++;                                                   // increment number of threads
            return th->tid;                                                 // return thread id
        }

    i++;                                                                    // increment i
    }

    return -1;                                                              // if no empty place was found, return -1
}

/**
 * @brief Schedules the next thread to be executed by checking its yieldsStatus.
 *
 * @param cpustate state
 * @return CPUState_Thread* thread to be executed state
 */
CPUState_Thread *ThreadManager::Schedule(CPUState_Thread* cpustate)
{
    if (numThreads <= 0)                                                    // if there are no threads, return cpustate
        return cpustate;

    if (currentThread >= 0 && Threads[currentThread] != nullptr)            // if there is a current thread
        Threads[currentThread++]->cpustate = cpustate;

    int i = currentThread;                                                  // set i to currentThread
    while (i < 256)
    {
        if (i >= 0 && Threads[i] != nullptr)                                // if there is a thread in the array
        {

            if (Threads[i]->yieldStatus)                                    // if the thread is yielded
                Threads[i]->yieldStatus = false;                            // set yieldStatus to false
            else
            {
                currentThread = i;                                          // set currentThread to i
                return Threads[i] -> cpustate;                                // return the state of the thread
            }
        }

        i++;
        if (i >= 256)                                                       // if i is greater than 256
            i = 0;
    }

    return cpustate;
}

/**
 * @brief Terminates a thread by removing it from an array by making its pointer nullptr
 *
 * @param tid thread id to terminate
 * @return true if sucessfully terminated
 * @return false if error
 */
bool ThreadManager::TerminateThread(int tid)
{
    // if tid is out of bounds, return false
    if (tid < 0 || tid >= 256)
        return false;

    // if there is no thread in the array, return false
    if (Threads[tid] == nullptr)
        return false;

    // Set the pointer to nullptr
    Threads[tid] = nullptr;
    numThreads--;
    return true;
}

/**
 * @brief Joins a thread by waiting to finish
 *
 * @param other thread to join
 * @return true if succesfully joined
 * @return false if error
 */
bool ThreadManager::JoinThreads(int other)
{
    //check if thread is already terminated or null
    if (Threads[other] == nullptr)
        return false;
    if (Threads[other]->yieldStatus)
        return false;

    while (true)
    {
        if (Threads[other] == nullptr)
            break;
    }

    return true;
}

/**
 * @brief Makes yield status true of the current thread
 *
 */
void ThreadManager::YieldThreads(int tid)
{
    Threads[tid]->yieldStatus = true;
}

/**
 * @breif Checks if a thread is terminated
 * @param tid thread id to check
 * @return false if thread is terminated
 */
bool ThreadManager::CheckThreads(int tid) {

    return Threads[tid] == nullptr;

}