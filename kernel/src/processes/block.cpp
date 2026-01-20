//
// Created by Max Tyson on 19/01/2026.
//

#include <processes/block.h>
#include <processes/scheduler.h>

using namespace MaxOS;
using namespace MaxOS::processes;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::system;

BlockingLock::BlockingLock() = default;
BlockingLock::~BlockingLock() = default;

bool BlockingLock::must_spin() {
    return GlobalScheduler::is_active();
}

/**
 * @brief Lock the spinlock once it is available, sleeping until other processes are done with it
 */
void BlockingLock::lock() {
    acquire();
    m_locked = true;
}

/**
 * @brief Unlock the spinlock
 */
void BlockingLock::unlock() {

    m_locked = false;
    release();
}

/**
 * @brief Check if the spinlock is locked
 *
 * @return True if the spinlock is locked, false otherwise
 */
bool BlockingLock::is_locked() const {
    return m_locked;
}

/**
 * @brief Acquire the spinlock, spin until the lock is available and sleeping the thread until marked as available
 *
 * @todo Move the yielding logic into process/thread code so that it can be reused
 * @todo circular dependancy resolution
 */
void BlockingLock::acquire() {

    // Try to get the lock
    for (int i = 0; (i < BLOCKING_FAST_TRY_LIMIT || must_spin()); ++i)
        if(!__atomic_test_and_set(&m_locked, __ATOMIC_ACQUIRE))
            return;

    // Add to the queue
    auto thread = GlobalScheduler::current_thread();
    thread->thread_state = ThreadState::WAITING;
    m_queue.push_back(thread->tid);

    // Scheduler will awake when the process that holds this lock marks this thread as READY
    thread->yield();
}

/**
 * @brief Mark as unlocked, wake the next enqueued thread
 */
void BlockingLock::release() {

    // Next thread can be run
    if(!m_queue.empty())
    	GlobalScheduler::get_thread(m_queue.pop_front())->thread_state = ThreadState::READY;

    __atomic_clear(&m_locked, __ATOMIC_RELEASE);
}