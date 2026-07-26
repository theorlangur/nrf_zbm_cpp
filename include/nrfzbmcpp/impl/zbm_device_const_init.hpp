#ifndef ZB_META_DEVICE_CONST_INIT_HPP_
#define ZB_META_DEVICE_CONST_INIT_HPP_
#include "zbm_types.hpp"

namespace zbm
{
    template<class Key=void>
    struct device_init_tag_t{};

    consteval auto get_const_device_initializer(device_init_tag_t<void>);

    //this can be used at most once
    template<auto _Val, class Key=void>
    struct const_init_device
    {
        friend consteval auto get_const_device_initializer(device_init_tag_t<Key>)
        {
            return _Val;
        }

        constexpr operator decltype(_Val) ()
        {
            return _Val;
        }
    };

    //declaration trick. will be defined in const_init_device
    template<class Key=void>
    consteval auto get_const_device_initializer()
    {
        return get_const_device_initializer(device_init_tag_t<Key>{});
    }

    template<typename Key = void>
    consteval bool has_const_device_initializer() {
        //soft error, not hard error
        return requires { get_const_device_initializer(device_init_tag_t<Key>{}); };
    }
}
#endif
