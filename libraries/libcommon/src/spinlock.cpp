/**
 * @file spinlock.cpp
 * @brief Implements a Spinlock class for mutual exclusion in concurrent programming
 *
 * @date 24th February 2025
 * @author Max Tyson
 */

#include <libcommon/spinlock.h>
//#include <processes/scheduler.h>

using namespace MaxOS;
using namespace MaxOS::common;

Spinlock::Spinlock() = default;
Spinlock::~Spinlock() = default;

/**
 * @brief Lock the spinlock once it is available
 */
void Spinlock::lock() {
	acquire();
}

/**
 * @brief Unlock the spinlock
 */
void Spinlock::unlock() {

	release();
}

/**
 * @brief Check if the spinlock is locked
 *
 * @return True if the spinlock is locked, false otherwise
 */
bool Spinlock::is_locked() const {
	bool v;
	__atomic_load(&m_locked, &v, __ATOMIC_RELAXED);
	return v;
}


/**
 * @brief Acquire the spinlock: wait until the lock is available, spinning until that happens.
 */
void Spinlock::acquire() {
	while (__atomic_test_and_set(&m_locked, __ATOMIC_ACQUIRE))
		asm("pause");
}

/**
 * @brief Release the spinlock
 */
void Spinlock::release() {
	__atomic_clear(&m_locked, __ATOMIC_RELEASE);
}