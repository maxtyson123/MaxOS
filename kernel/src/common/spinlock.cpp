//
// Created by 98max on 24/02/2025.
//

#include <common/spinlock.h>
#include <processes/scheduler.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::processes;

Spinlock::Spinlock(bool should_yield)
: m_should_yield(should_yield) {

}

Spinlock::~Spinlock() = default;

/**
 * @brief Lock the spinlock once it is available
 */
void Spinlock::lock() {

    acquire();
    m_locked = true;
}

/**
 * @brief Unlock the spinlock
 */
void Spinlock::unlock() {

    m_locked = false;
    release();

}

/**
 * @brief Check if the spinlock is locked
 *
 * @return True if the spinlock is locked, false otherwise
 */
bool Spinlock::is_locked() const {
    return m_locked;
}


/**
 * @brief Acquire the spinlock: wait until the lock is available, yielding if desired until that happens.
 */
void Spinlock::acquire() {
  while (__atomic_test_and_set(&m_locked, __ATOMIC_ACQUIRE)) {

      // Wait for the lock to be available
      if(m_should_yield)
        Scheduler::system_scheduler()->yield();

  }
}

/**
 * @brief Release the spinlock
 */
void Spinlock::release() {
  __atomic_clear(&m_locked, __ATOMIC_RELEASE);
}
