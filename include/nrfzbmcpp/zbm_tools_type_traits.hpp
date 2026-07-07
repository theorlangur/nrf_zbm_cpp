#ifndef ZB_META_TYPE_TRAITS_HPP_
#define ZB_META_TYPE_TRAITS_HPP_

#include <cstring>
#include <type_traits>
#if __has_include(<expected>)
#include <expected>
#endif

namespace zbm
{
    namespace tools
    {
        template<class T>
        using deref_t = std::remove_cvref_t<decltype(*std::declval<T>())>;

        template<class T>
        concept simple_destructible_t = std::is_trivially_destructible_v<T>;// || requires { typename T::can_relocate; };

#if __has_include(<expected>)
        template<class C>
        struct is_expected_type
        {
            static constexpr const bool value = false;
        };

        template<class V, class E>
        struct is_expected_type<std::expected<V,E>>
        {
            static constexpr const bool value = true;
        };

        template<class C>
        constexpr bool is_expected_type_v = is_expected_type<std::remove_cvref_t<C>>::value;
#endif

        template<size_t N>
        struct min_size_type_t { using type_t = size_t; };
        template<size_t N> requires (N <= 255)
        struct min_size_type_t<N> { using type_t = uint8_t; };
        template<size_t N> requires (N > 255 && N <= 65535)
        struct min_size_type_t<N> { using type_t = uint16_t; };

        template<size_t N>
        struct min_bit_size_type_t { using type_t = size_t; };
        template<size_t N> requires (N <= 7)
        struct min_bit_size_type_t<N> { using type_t = uint8_t; };
        template<size_t N> requires (N > 7 && N <= 15)
        struct min_bit_size_type_t<N> { using type_t = uint16_t; };
    }
}

#endif
