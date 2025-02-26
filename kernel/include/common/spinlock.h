//
// Created by 98max on 24/02/2025.
//

#ifndef MAXOS_COMMON_SPINLOCK_H
#define MAXOS_COMMON_SPINLOCK_H

namespace MaxOS{

  namespace common{

    /**
     * @class Spinlock
     * @brief A simple spinlock implementation
     */
    class Spinlock
    {
      private:
        bool m_locked;

      public:
        Spinlock();
        ~Spinlock();

        void lock();
        void unlock();
        bool is_locked();

        void acquire();
        void release();
    };

  }

}

#endif // MAXOS_COMMON_SPINLOCK_H
