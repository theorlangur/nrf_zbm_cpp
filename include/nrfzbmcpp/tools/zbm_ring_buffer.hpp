#ifndef ZB_META_RING_BUFFER_HPP_
#define ZB_META_RING_BUFFER_HPP_

#include <cstddef>
#include <optional>
#include "zbm_misc_helpers.hpp"
#include "zbm_type_traits.hpp"

namespace zbm
{
    namespace tools
    {
        template<class T> requires std::is_integral_v<T>
        constexpr int HighestBitSet(T N)
        {
            int r = -1;
            while(N) 
            {
                N >>= 1;
                ++r;
            }
            return r;
        }

        template<class T, size_t N>
        struct ring_buffer_t
        {
            using size_type_t = min_size_type_t<N>::type_t;
            constexpr static int kHighestBit = HighestBitSet(size_type_t(N));
            constexpr static size_type_t kBufSize = kHighestBit >= 0 ? size_type_t(1) << (kHighestBit + 1) : 0;
            constexpr static size_type_t kMask = kBufSize - 1;

            template<class... Args> requires (N > 0)
            std::optional<size_type_t> push(Args&&...args)
            {
                auto newTail = (m_Tail + 1) & kMask;
                if (newTail == m_Head) return std::nullopt;
                m_Tail = newTail;
                new (&(m_Buf[m_Tail].item)) T{std::forward<Args>(args)...};
                return size_type_t(((kBufSize + m_Tail) - m_Head) & kMask);
            }

            std::optional<T> pop() requires (N > 0)
            {
                if (m_Tail == m_Head) return std::nullopt;
                scope_exit_t cleanup = [&]{
                    if constexpr (!std::is_trivially_destructible_v<T>)
                        m_Buf[m_Head].item.~T();
                };
                m_Head = (m_Head + 1) & kMask;
                return m_Buf[m_Head].item;
            }

            bool drop() requires (N > 0)
            {
                if (m_Tail == m_Head) return false;

                m_Head = (m_Head + 1) & kMask;
                if constexpr (!std::is_trivially_destructible_v<T>)
                    m_Buf[m_Head].item.~T();
                return true;
            }

            T* peek() requires (N > 0)
            {
                if (m_Tail == m_Head) return nullptr;
                return &m_Buf[(m_Head + 1) & kMask].item;
            }

            size_t size() const requires (N > 0)
            {
                if (m_Tail == m_Head) return 0;
                return (m_Tail + kBufSize - m_Head) & kMask;
            }

            struct iterator_t
            {
                ring_buffer_t &r;
                size_type_t i;
                void operator++()
                {
                    i = (i + 1) & kMask;
                }
                bool operator!=(const iterator_t &it) const { return i != it.i; }
                T* operator*()
                {
                    return &(r.m_Buf[(i + 1) & kMask].item);
                }
            };

            auto begin() { return iterator_t{*this, m_Head}; }
            auto end() { return iterator_t{*this, m_Tail}; }

            private:
            union raw_t
            {
                T item;
            };
            [[no_unique_address]]raw_t m_Buf[kBufSize];
            size_type_t m_Head = 0;
            size_type_t m_Tail = 0;
        };
    }
}

#endif
