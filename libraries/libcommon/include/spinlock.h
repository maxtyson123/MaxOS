/**
 * @file spinlock.h
 * @brief Defines Spinlock and BlockingLock classes for thread synchronization
 *
 * @date 24th February 2025
 * @author Max Tyson
 */

#ifndef MAXOS_COMMON_SPINLOCK_H
#define MAXOS_COMMON_SPINLOCK_H

#include <vector.h>


namespace MaxOS::common {

	/**
	 * @class Spinlock
	 * @brief Enables a resource to be used by only one instance at a time through locking and unlocking
	 */
	class Spinlock {

		private:
			bool m_locked = false;

		public:
			Spinlock();
			~Spinlock();

			void lock();
			void unlock();
			[[nodiscard]] bool is_locked() const;

			void acquire();
			void release();
	};


	/// How many attempts to acquire the lock should fail before queueing
	constexpr uint8_t BLOCKING_FAST_TRY_LIMIT = UINT8_MAX;

	/**
	 * @class BlockingLock
	 * @brief Enables a resource to be used by only one instance at a time through a combination of spinning and queuing. When waiting enqueued, thread will sleep.
	 *
	 * @note Repeated API that could be made a class that isn't because lock types shouldn't be interchangeable
	 * @see Spinlock
	 */
	class BlockingLock {

		private:
			bool m_locked = false;
			Vector<uint64_t> m_queue;

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


}


#endif // MAXOS_COMMON_SPINLOCK_H
