#ifndef ZBM_STR_HPP
#define ZBM_STR_HPP

#include "zbm_types.hpp"
#include <cstring>
#include <string_view>
#include <span>

namespace zbm
{
    struct str_view_t
    {
        const char *pStr;

        //operator void*() { return pStr; }
        uint8_t size() const { return pStr[0]; }
        std::string_view sv() const { return {pStr + 1, pStr[0]}; }
        operator std::string_view() const { return sv(); }

        std::optional<uint8_t*> serialize_to(uint8_t *pDst, size_t len) const
        {
            uint8_t serialize_size = size() + 1;
            if (len >= serialize_size)
            {
                std::memcpy(pDst, pStr, serialize_size);
                pDst += serialize_size;
                return pDst;
            }
            return std::nullopt;
        }

        std::optional<const uint8_t*> serialize_from(const uint8_t *pSrc, size_t len)
        {
            if (len < 1) return std::nullopt;
            if (pSrc[0] >= len) return std::nullopt;
            pStr = (const char*)pSrc;
            uint8_t serialize_size = pSrc[0] + 1;
            pSrc += serialize_size;
            return pSrc;
        }
    };
    static_assert(serializable_c<str_view_t>);

    struct octet_view_t
    {
        const uint8_t *pData;

        uint8_t size() const { return pData[0]; }
        std::span<const uint8_t> data() const { return {pData + 1, pData[0]}; }
        operator std::span<const uint8_t>() const { return data(); }

        std::optional<uint8_t*> serialize_to(uint8_t *pDst, size_t len) const
        {
            uint8_t serialize_size = size() + 1;
            if (len >= serialize_size)
            {
                std::memcpy(pDst, pData, serialize_size);
                pDst += serialize_size;
                return pDst;
            }
            return std::nullopt;
        }

        std::optional<const uint8_t*> serialize_from(const uint8_t *pSrc, size_t len)
        {
            if (len < 1) return std::nullopt;
            if (pSrc[0] >= len) return std::nullopt;
            pData = pSrc;
            uint8_t serialize_size = pSrc[0] + 1;
            pSrc += serialize_size;
            return pSrc;
        }
    };
    static_assert(serializable_c<octet_view_t>);

    struct str_ref_t
    {
        char sz;

        uint8_t size() const { return sz; }
        std::string_view sv() const { return {&sz + 1, sz}; }
    };

    template<size_t N>
    struct str_buf_t: public str_ref_t
    {
        char data[N];

        std::optional<uint8_t*> serialize_to(uint8_t *pDst, size_t len) const
        {
            uint8_t serialize_size = size() + 1;
            if (len >= serialize_size)
            {
                std::memcpy(pDst, this, serialize_size);
                pDst += serialize_size;
                return pDst;
            }
            return std::nullopt;
        }

        std::optional<const uint8_t*> serialize_from(const uint8_t *pSrc, size_t len)
        {
            if (len < 1) return std::nullopt;
            if (pSrc[0] >= len || pSrc[0] > N) return std::nullopt;
            sz = pSrc[0];
            std::memcpy(data, pSrc + 1, sz);
            pSrc += sz + 1;
            return pSrc;
        }
    };

    struct octet_ref_t
    {
        uint8_t sz;

        uint8_t size() const { return sz; }
        std::span<const uint8_t> sv() const { return {&sz + 1, sz}; }
    };

    template<size_t N>
    struct octet_buf_t: public octet_ref_t
    {
        uint8_t data[N];

        std::optional<uint8_t*> serialize_to(uint8_t *pDst, size_t len) const
        {
            uint8_t serialize_size = size() + 1;
            if (len >= serialize_size)
            {
                std::memcpy(pDst, this, serialize_size);
                pDst += serialize_size;
                return pDst;
            }
            return std::nullopt;
        }

        std::optional<const uint8_t*> serialize_from(const uint8_t *pSrc, size_t len)
        {
            if (len < 1) return std::nullopt;
            if (pSrc[0] >= len || pSrc[0] > N) return std::nullopt;
            sz = pSrc[0];
            std::memcpy(data, pSrc + 1, sz);
            pSrc += sz + 1;
            return pSrc;
        }
    };

    template<size_t N>
    struct str_t
    {
        char name[N];

        template<size_t M, size_t...idx>
        constexpr str_t(std::index_sequence<idx...>, const char (&n)[M]):
            name{ M-1, n[idx]... }
        {
        }

        template<size_t M>
        constexpr str_t(const char (&n)[M]):
            str_t(std::make_index_sequence<M-1>(), n)
        {
        }

        constexpr str_t():
            name{0}
        {
        }

        size_t capacity() const { return N - 1; }
        size_t size() const { return *name; }
        std::string_view sv() const { return {name + 1, size()}; }
        str_view_t zsv() const { return {name}; }
        str_ref_t& zsv_ref() { return *(str_ref_t*)name; }

        template<size_t M>
        str_t<N>& operator=(const char (&n)[M])
        {
            static_assert(M <= N, "String literal is too big");
            name[0] = M - 1;
            std::memcpy(name + 1, n, M - 1);
            return *this;
        }

        std::optional<uint8_t*> serialize_to(uint8_t *pDst, size_t len) const
        {
            uint8_t serialize_size = size() + 1;
            if (len >= serialize_size)
            {
                std::memcpy(pDst, this, serialize_size);
                pDst += serialize_size;
                return pDst;
            }
            return std::nullopt;
        }

        std::optional<const uint8_t*> serialize_from(const uint8_t *pSrc, size_t len)
        {
            if (len < 1) return std::nullopt;
            if (pSrc[0] >= len || pSrc[0] > (N - 1)) return std::nullopt;
            uint8_t sz = pSrc[0];
            std::memcpy(name, pSrc, sz + 1);
            pSrc += sz + 1;
            return pSrc;
        }

        static constexpr type_t type_id() { return type_t::CharStr; }
        static zb_ret_t validate_value(uint8_t *value) { return *value < N ? RET_OK : RET_OUT_OF_RANGE; }
    };

    template<char... c>
    constexpr str_t<sizeof...(c) + 1> operator ""_zstr()
    {
        constexpr uint8_t N = sizeof...(c) + 1;
        static_assert(N < 255, "String too long");
        return str_t<N>{.name={N - 1, c...}};
    }

    template<size_t N>
    struct bin_t
    {
        uint8_t data[N];

        template<class T, size_t M, size_t...idx>
        constexpr bin_t(std::index_sequence<idx...>, std::array<T, M> const &n):
            data{ M-1, n[idx]... }
        {
        }

        template<class T, size_t M>
        constexpr bin_t(std::array<T, M> const& n):
            bin_t(std::make_index_sequence<M-1>(), n)
        {
        }

        constexpr bin_t():
            data{0}
        {
        }

        size_t size() const { return N - 1; }
        std::span<const uint8_t> sv() const { return {data + 1, N - 1}; }

        template<size_t M>
        bin_t<N>& operator=(std::array<uint8_t, M> const& n)
        {
            static_assert(M <= N, "String literal is too big");
            data[0] = M - 1;
            std::memcpy(data + 1, &n, M - 1);
            return *this;
        }

        std::optional<uint8_t*> serialize_to(uint8_t *pDst, size_t len) const
        {
            uint8_t serialize_size = size() + 1;
            if (len >= serialize_size)
            {
                std::memcpy(pDst, this, serialize_size);
                pDst += serialize_size;
                return pDst;
            }
            return std::nullopt;
        }

        std::optional<const uint8_t*> serialize_from(const uint8_t *pSrc, size_t len)
        {
            if (len < 1) return std::nullopt;
            if (pSrc[0] >= len || pSrc[0] > (N - 1)) return std::nullopt;
            uint8_t sz = pSrc[0];
            std::memcpy(data, pSrc, sz + 1);
            pSrc += sz + 1;
            return pSrc;
        }

        static constexpr type_t type_id() { return type_t::OctetStr; }
        static zb_ret_t validate_value(uint8_t *value) { return *value < N ? RET_OK : RET_OUT_OF_RANGE; }
    };

    template<class T, size_t N> requires (std::is_trivially_copyable_v<T>
            && std::is_trivially_constructible_v<T>)
    struct [[gnu::packed]] bin_typed_array_t
    {
        uint8_t len_bytes;
        T data[N];

        template<size_t M, size_t...idx>
        constexpr bin_typed_array_t(std::index_sequence<idx...>, std::array<T, M> const &n):
            len_bytes{sizeof(T) * M},
            data{ n[idx]... }
        {
            static_assert(sizeof(T) * M <= 255);
        }

        template<size_t M>
        constexpr bin_typed_array_t(std::array<T, M> const& n):
            bin_typed_array_t(std::make_index_sequence<M>(), n)
        {
        }

        constexpr bin_typed_array_t():
            len_bytes{0}
            , data{}
        {
        }

        operator void*() { return this; }
        bool valid() const { return (len_bytes % sizeof(T)) == 0; }
        size_t size() const { return len_bytes / sizeof(T); }
        size_t raw_size() const { return len_bytes; }
        static constexpr size_t max_size() { return N; }
        static constexpr size_t size_bytes() { return N * sizeof(T); }
        std::span<const T> sv() const { return {data, N}; }

        template<class Me>
        auto& operator[](this Me const& t, size_t i) { return t.data[i]; }

        std::optional<uint8_t*> serialize_to(uint8_t *pDst, size_t len) const
        {
            uint8_t serialize_size = raw_size() + 1;
            if (valid() && len >= serialize_size)
            {
                std::memcpy(pDst, this, serialize_size);
                pDst += serialize_size;
                return pDst;
            }
            return std::nullopt;
        }

        std::optional<const uint8_t*> serialize_from(const uint8_t *pSrc, size_t len)
        {
            if (len < 1) return std::nullopt;
            if (pSrc[0] >= len || pSrc[0] > size_bytes()) return std::nullopt;
            if ((pSrc[0] % sizeof(T)) != 0) return std::nullopt;
            len_bytes = pSrc[0];
            std::memcpy(data, pSrc + 1, len_bytes);
            pSrc += len_bytes + 1;
            return pSrc;
        }

        static constexpr type_t type_id() { return type_t::OctetStr; }
        static zb_ret_t validate_value(uint8_t *value) 
        {
            if (*value > size_bytes())
                return RET_OUT_OF_RANGE;
            if (*value % sizeof(T) != 0)
                return RET_INVALID_PARAMETER;
            return RET_OK;
        }
    };

    template<class T> requires (std::is_trivially_copyable_v<T>
            && sizeof(T) <= 255)
    struct [[gnu::packed]] bin_typed_t
    {
        uint8_t len_bytes;
        T data;

        constexpr bin_typed_t(T d = {}):
            len_bytes{sizeof(T)}
            , data{d}
        {
        }

        T* operator->() { return &data; }
        operator T&() { return data; }
        operator const T&() const { return data; }

        std::optional<uint8_t*> serialize_to(uint8_t *pDst, size_t len) const
        {
            uint8_t serialize_size = sizeof(T) + 1;
            if (len >= serialize_size)
            {
                std::memcpy(pDst, this, serialize_size);
                pDst += serialize_size;
                return pDst;
            }
            return std::nullopt;
        }

        std::optional<const uint8_t*> serialize_from(const uint8_t *pSrc, size_t len)
        {
            if (len < 1) return std::nullopt;
            if (pSrc[0] >= len || pSrc[0] > sizeof(T)) return std::nullopt;
            if (pSrc[0] != sizeof(T)) return std::nullopt;
            len_bytes = pSrc[0];
            std::memcpy(&data, pSrc + 1, len_bytes);
            pSrc += len_bytes + 1;
            return pSrc;
        }

        static constexpr type_t type_id() { return type_t::OctetStr; }
        static zb_ret_t validate_value(uint8_t *value) 
        {
            if (*value != sizeof(T))
                return RET_INVALID_PARAMETER;

            if constexpr (validatable_c<T>)
                return T::validate_value(value + 1);
            else
                return RET_OK;
        }
    };

    template<class T>
    struct bin_view_t
    {
        const uint8_t *pData;

        static constexpr type_t type_id() { return type_t::OctetStr; }

        T* operator->() { return (T*)(pData + 1); }
        operator T&() { return *(T*)(pData + 1); }
        operator const T&() const { return *(T*)(pData + 1); }
    };
}
#endif
