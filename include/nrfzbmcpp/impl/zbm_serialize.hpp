#ifndef ZB_META_SERIALIZE_HPP_
#define ZB_META_SERIALIZE_HPP_
#include "zbm_types.hpp"

namespace zbm
{
    template<class T>
    concept serializable_c = requires(T t, T const ct, uint8_t *pDst, const uint8_t *pSrc, size_t len){
        { ct.serialize_to(pDst, len) } -> std::same_as<std::optional<uint8_t*>>;
        { t.serialize_from(pSrc, len) } -> std::same_as<std::optional<const uint8_t*>>;
    };

    template<class T>
    concept serializable_with_limit_c = serializable_c<T> && requires()
    {
        { T::serialize_limit() } -> std::same_as<size_t>;
    };

    template<class T>
    concept validatable_c = requires { T::validate_value((uint8_t*)nullptr); };


    template<class T>
    concept cmd_arg_c = serializable_with_limit_c<T> || std::is_arithmetic_v<T> || std::is_enum_v<T>;


    template<cmd_arg_c T>
    std::optional<const uint8_t*> serialize_from(T &dst, const uint8_t *pSrc, size_t limit)
    {
        if constexpr (serializable_with_limit_c<T>)
            return dst.serialize_from(pSrc, limit);
        else
        {
            //raw
            if (sizeof(T) > limit) return std::nullopt;
            memcpy(&dst, pSrc, sizeof(T));
            return pSrc + sizeof(T);
        }
    }

    template<cmd_arg_c T>
    std::optional<uint8_t*> serialize_to(T const& src, uint8_t *pDst, size_t limit)
    {
        if constexpr (serializable_with_limit_c<T>)
            return src.serialize_to(pDst, limit);
        else
        {
            if (sizeof(T) > limit) return std::nullopt;
            memcpy(pDst, &src, sizeof(T));
            return pDst + sizeof(T);
        }
    }

    template<cmd_arg_c T>
    constexpr size_t serialize_limit()
    {
        if constexpr (serializable_with_limit_c<T>)
            return T::serialize_limit();
        else
            return sizeof(T);
    }

    template<cmd_arg_c... T>
    constexpr size_t total_serialize_limit()
    {
        return (serialize_limit<T>() + ... + 0);
    }
}
#endif
