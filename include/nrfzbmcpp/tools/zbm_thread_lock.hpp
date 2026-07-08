#ifndef ZBM_TOOLS_THREAD_LOCK_HPP_
#define ZBM_TOOLS_THREAD_LOCK_HPP_

#include <utility>
#include <zephyr/spinlock.h>

namespace zbm{
    namespace thread{
        class spin_lock_t
        {
        public:
            auto lock() { return k_spin_lock(&m_Lock); }
            void unlock(k_spinlock_key_t k) { k_spin_unlock(&m_Lock, k); }

        private:
            k_spinlock m_Lock{};
        };

        struct dummy_lock_t
        {
            k_spinlock_key_t lock() { return {}; }
            void unlock(k_spinlock_key_t k) {}
        };

        class i_lockable_t
        {
        public:
            virtual ~i_lockable_t() = default; 
            virtual void lock() = 0;
            virtual void unlock() = 0;
        };

        class no_lock_t: public i_lockable_t
        {
        public:
            virtual void lock() override {}
            virtual void unlock() override {}
        };

        template<class L>
        struct lock_guard_t
        {
            lock_guard_t(L *pL):m_pLock(pL) { if (pL) m_Key = pL->lock(); }
            lock_guard_t(L &l):lock_guard_t(&l){}
            ~lock_guard_t() { if (m_pLock) m_pLock->unlock(m_Key); }
        private:
            L *m_pLock;
            k_spinlock_key_t m_Key;
        };
        template<class L>
        lock_guard_t(L *)->lock_guard_t<L>;
        template<class L>
        lock_guard_t(L &)->lock_guard_t<L>;

        template<class V, class L = spin_lock_t>
        struct sync_var_t
        {
            constexpr sync_var_t() = default;
            constexpr sync_var_t(V &&v):m_Var(std::move(v)) {}
            constexpr sync_var_t(const V &v):m_Var(v) {}

            V exchange(V newVal)
            {
                lock_guard_t lg{m_Lock};
                return std::exchange(m_Var, newVal);
            }

            V get() const
            {
                lock_guard_t lg{m_Lock};
                return m_Var;
            }

            sync_var_t& operator=(V newVal)
            {
                lock_guard_t lg{m_Lock};
                m_Var = newVal;
                return *this;
            }
        private:
            mutable L m_Lock;
            V m_Var;
        };
    }
}
#endif
