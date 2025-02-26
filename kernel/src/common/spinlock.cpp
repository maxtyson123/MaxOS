//
// Created by 98max on 24/02/2025.
//

#include <common/spinlock.h>

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
      while (__sync_lock_test_and_set(&m_locked, 1)) {
            while (m_locked) {
                // spin
                asm("hlt");
            }
      }
}

/**
 * @brief Release the spinlock
 */
void Spinlock::release() {
    __sync_lock_release(&m_locked);
}
