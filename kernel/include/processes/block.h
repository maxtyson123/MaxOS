/**
 * @file block.h
 * @brief Defines BlockingLock class for thread synchronization
 *
 * @date 19th January 2026
 * @author Max Tyson
 */

#ifndef MAXOS_PROCESSES_BLOCK_H
#define MAXOS_PROCESSES_BLOCK_H

#include <vector.h>

namespace MaxOS::processes {

    /// How many attempts to acquire the lock should fail before queueing
    constexpr uint8_t BLOCKING_FAST_TRY_LIMIT = UINT8_MAX;

    /**
     * @class BlockingLock
     * @brief Enables a resource to be used by only one instance at a time through a combination of spinning and queuing. When waiting enqueued, thread will sleep.
     *
     * @note Repeated API that could be made a class that isn't because lock types shouldn't be interchangeable
     * @see Spinlock
     *
     * @todo move to libcommon
     */
    class BlockingLock {

        private:
            bool m_locked = false;
            common::Vector<uint64_t> m_queue;

            static bool must_spin();

        public:
            BlockingLock();
            ~BlockingLock();

            void lock();
            void unlock();
            [[nodiscard]] bool is_locked() const;

            void acquire();
            void release();

    };

};

#endif //MAXOS_PROCESSES_BLOCK_H