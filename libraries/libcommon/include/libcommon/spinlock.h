/**
 * @file spinlock.h
 * @brief Defines Spinlock class for thread synchronization
 *
 * @date 24th February 2025
 * @author Max Tyson
 */

#ifndef MAXOS_COMMON_SPINLOCK_H
#define MAXOS_COMMON_SPINLOCK_H

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





}


#endif // MAXOS_COMMON_SPINLOCK_H
