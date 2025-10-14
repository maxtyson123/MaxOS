//
// Created by 98max on 24/02/2025.
//

#include <common/spinlock.h>
#include <processes/scheduler.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::processes;
using namespace MaxOS::system;
using namespace MaxOS::hardwarecommunication;

Spinlock::Spinlock() = default;
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
	while (__atomic_test_and_set(&m_locked, __ATOMIC_ACQUIRE))
		asm("nop");
}

/**
 * @brief Release the spinlock
 */
void Spinlock::release() {
	__atomic_clear(&m_locked, __ATOMIC_RELEASE);
}

BlockingLock::BlockingLock() = default;
BlockingLock::~BlockingLock() = default;

bool BlockingLock::must_spin() {
	return GlobalScheduler::is_active();
}

void BlockingLock::lock() {
	acquire();
	m_locked = true;
}

void BlockingLock::unlock() {

	m_locked = false;
	release();
}

void BlockingLock::acquire() {

	// Try to get the lock
	for (int i = 0; (i < BLOCKING_FAST_TRY_LIMIT || must_spin()); ++i)
		if(!__atomic_test_and_set(&m_locked, __ATOMIC_ACQUIRE))
			return;

	// Add to the queue
	auto thread = GlobalScheduler::current_thread();
	thread->thread_state = ThreadState::WAITING;
	m_queue.push_back(thread->tid);

	thread->save_cpu_state();

	// Guard against being resumed here
	if(thread->thread_state == ThreadState::WAITING){

		// Yield to the next thread
		cpu_status_t* next = GlobalScheduler::core_scheduler()->schedule_next(&thread->execution_state);
		InterruptManager::ForceInterruptReturn(next);
	}


}

void BlockingLock::release() {

	// Next thread can be run
	auto tid = m_queue.pop_front();
	GlobalScheduler::get_thread(tid)->thread_state = ThreadState::READY;

	__atomic_clear(&m_locked, __ATOMIC_RELEASE);
}


