//
// Created by 98max on 24/02/2025.
//

#ifndef MAXOS_COMMON_SPINLOCK_H
#define MAXOS_COMMON_SPINLOCK_H

namespace MaxOS{

  namespace common{

    /**
     * @class Spinlock
     * @brief Enables a resource to be used by only one instance at a time through locking and unlocking
     */
    class Spinlock
    {
      private:
        bool m_locked = false;
        bool m_should_yield;

      public:
        Spinlock(bool should_yield = false);
        ~Spinlock();

        void lock();
        void unlock();
        bool is_locked() const;

        void acquire();
        void release();
    };


  }

}

#endif // MAXOS_COMMON_SPINLOCK_H
