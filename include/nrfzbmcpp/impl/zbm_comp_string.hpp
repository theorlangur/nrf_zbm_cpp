#ifndef ZB_META_COMP_STRING_HPP_
#define ZB_META_COMP_STRING_HPP_

#include <array>
#include <string_view>

namespace zbm
{
    template<size_t N>
    struct comp_str_t
    {
        std::array<char, N> d{};
        size_t len = 0;

        constexpr const char* data() const { return d.data(); };
        constexpr size_t size() const { return len; };

        constexpr std::string_view sv() const
        {
            return {data(), len};
        }

        constexpr comp_str_t& operator=(const char *pStr)
        {
            len = 0;
            while(*pStr)
                d[len++] = *pStr++;
            d[len] = 0;
            return *this;
        }

        constexpr comp_str_t& operator+=(const char *pStr)
        {
            while(*pStr)
                d[len++] = *pStr++;
            d[len] = 0;
            return *this;
        }

        constexpr comp_str_t& operator+=(std::string_view sv)
        {
            for(size_t i = 0, n = sv.size(); i < n; ++i)
                d[len++] = sv[i];
            d[len] = 0;
            return *this;
        }

        constexpr comp_str_t& operator+=(size_t i)
        {
            char num[64]{0};
            size_t l = 63;
            do
            {
                num[--l] = (i % 10) + '0';
                i /= 10;
            }while(i);
            return operator+=(num + l);
        }
    };

    using static_assert_str_t = comp_str_t<1024>;
}

#endif
