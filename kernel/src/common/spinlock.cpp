//
// Created by 98max on 24/02/2025.
//

#include <common/spinlock.h>
#include <common/kprint.h>

using namespace MaxOS;
using namespace MaxOS::common;

Spinlock::Spinlock(){

}

Spinlock::~Spinlock() {

}

/**
 * @brief Lock the spinlock
 */
void Spinlock::lock() {

    // Wait for the lock to be available
    acquire();

    // Set the lock to be locked
    m_locked = true;


}

/**
 * @brief Unlock the spinlock
 */
void Spinlock::unlock() {

    // Set the lock to be unlocked
    m_locked = false;

    // Release the lock
    release();

}

/**
 * @brief Check if the spinlock is locked
 * @return True if the spinlock is locked, false otherwise
 */
bool Spinlock::is_locked() {
    return m_locked;
}


/**
 * @brief Acquire the spinlock
 */
void Spinlock::acquire() {
      while (__atomic_test_and_set(&m_locked, __ATOMIC_ACQUIRE)) {
        // Wait for the lock to be available
        asm("nop");
      }
}

/**
 * @brief Release the spinlock
 */
void Spinlock::release() {
  __atomic_clear(&m_locked, __ATOMIC_RELEASE);
}
