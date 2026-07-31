#ifndef ZB_META_CLUSTER_WRITE_HOOK_HPP_
#define ZB_META_CLUSTER_WRITE_HOOK_HPP_

#include "zbm_types.hpp"
#include "zbm_serialize.hpp"
#include "zbm_annotations.hpp"

namespace zbm
{
    namespace cluster_impl{
        consteval bool is_hook_function(std::meta::info f)
        {
            if (std::meta::has_identifier(f))
            {
                auto nm = std::meta::identifier_of(f);
                return nm.starts_with("hook_") && std::meta::is_function(f);
            }
            return false;
        }

        enum class issue_t
        {
            Ok,
            ReturnType,
            ArgCount,
            ArgEP
        };

        consteval issue_t is_hook_function_ok(std::meta::info f)
        {
            auto func_type = std::meta::type_of(f);
            auto return_type = std::meta::return_type_of(func_type);
            if (return_type != ^^void) return issue_t::ReturnType;
            auto params = std::meta::parameters_of(func_type);
            if (params.size() != 2)
                return issue_t::ArgCount;
            if (params[0] != std::meta::dealias(^^uint8_t))
                return issue_t::ArgEP;//must be EP
            return issue_t::Ok;
        }

        consteval std::meta::info find_hook_to_run(std::meta::info hook_handler, std::meta::info attribute_mem_refl)
        {
            auto ctx = std::meta::access_context::current();
            comp_str_t<255> expected_hook_name;
            expected_hook_name += "hook_";
            expected_hook_name += std::meta::identifier_of(attribute_mem_refl);

            for(auto m : std::meta::members_of(hook_handler, ctx))
            {
                if (std::meta::has_identifier(m) && (std::meta::identifier_of(m) == expected_hook_name.sv()))
                    return m;
            }
            return {};
        }

        struct hook_check_res_t
        {
            bool valid = true;
            static_assert_str_t message;
        };

        template<class T>
        consteval hook_check_res_t check_hook_type()
        {
            hook_check_res_t res;
            auto type_name = std::meta::display_string_of(^^T);
            auto ctx = std::meta::access_context::current();
            for(auto m : std::meta::members_of(^^T, ctx))
            {
                if (is_hook_function(m))
                {
                    if (auto r = is_hook_function_ok(m); r != issue_t::Ok)
                    {
                        res.valid = false;
                        if (res.message.size() != 0)
                            res.message += "; ";
                        auto func_type = std::meta::type_of(m);
                        switch(r)
                        {
                            case issue_t::Ok:
                                //shall not be possible to reach
                                break;
                            case issue_t::ReturnType:
                                res.message += type_name;
                                res.message += "::";
                                res.message += std::meta::identifier_of(m);
                                res.message += ": return type must be void, not ";
                                res.message += std::meta::display_string_of(std::meta::return_type_of(func_type));
                                break;
                            case issue_t::ArgCount:
                                res.message += type_name;
                                res.message += "::";
                                res.message += std::meta::identifier_of(m);
                                res.message += ": must have 2 arguments, but ";
                                res.message += std::meta::parameters_of(func_type).size();
                                res.message += " found";
                                break;
                            case issue_t::ArgEP:
                                res.message += type_name;
                                res.message += "::";
                                res.message += std::meta::identifier_of(m);
                                res.message += ": 1st argument must be uint8_t for ep, not ";
                                res.message += std::meta::display_string_of(std::meta::parameters_of(func_type)[0]);
                                break;
                        }
                    }
                }
            }
            return res;
        }

    }
}

#endif
