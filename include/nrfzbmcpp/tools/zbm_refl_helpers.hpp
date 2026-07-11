#ifndef ZB_META_REFL_HELPERS_HPP_
#define ZB_META_REFL_HELPERS_HPP_

#include <meta>

namespace zbm
{
    namespace refl{
        template<size_t width = 1>
        consteval std::string name_with_hex(std::string prefix, unsigned val)
        {
            char suffix[width + 1] = {};
            for(size_t i = 0; i < width; ++i, val >>= 4)
            {
                uint8_t d = val & 0xf;
                if (d <= 9)
                    suffix[width - i - 1] = '0' + d;
                else
                    suffix[width - i - 1] = 'a' + (d - 10);
            }
            return prefix + suffix;
        }

        consteval std::meta::info find_member_by_name(std::meta::info struct_type, std::string_view name)
        {
            auto mems = std::meta::nonstatic_data_members_of(struct_type, std::meta::access_context::current());
            for(auto m : mems)
                if (std::meta::identifier_of(m) == name)
                    return m;
            return {};
        }

        consteval std::optional<std::meta::info> get_call_operator_overload(std::meta::info callable_refl)
        {
            for(auto m : std::meta::members_of(callable_refl, std::meta::access_context::current()))
            {
                if (std::meta::is_operator_function(m) && std::meta::operator_of(m) == std::meta::operators::op_parentheses)
                    return std::meta::type_of(m);
            }
            return std::nullopt;
        }

        consteval std::optional<std::meta::info> get_callable_type(std::meta::info callable_refl)
        {
            auto type_refl = std::meta::is_type(callable_refl) ? callable_refl : std::meta::type_of(callable_refl);
            if (std::meta::is_pointer_type(type_refl) || std::meta::is_function_type(type_refl))
            {
                auto no_ptr = std::meta::remove_pointer(type_refl);
                if (std::meta::is_function_type(no_ptr))
                    return no_ptr;
            }else if (std::meta::is_object_type(type_refl))
            {
                return get_call_operator_overload(type_refl);
            }
            return std::nullopt;
        }
    }
}

#endif
