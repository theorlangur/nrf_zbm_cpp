#ifndef ZB_META_CLUSTER_IMPL_HPP_
#define ZB_META_CLUSTER_IMPL_HPP_

#include "zbm_types.hpp"
#include "zbm_serialize.hpp"
#include "zbm_annotations.hpp"

namespace zbm
{
    namespace cluster_impl{
        enum class hook_issue_t
        {
            Ok,
            ReturnType,
            ArgCount,
            ArgEP
        };

        consteval bool is_hook_function(std::meta::info f)
        {
            if (std::meta::has_identifier(f))
            {
                auto nm = std::meta::identifier_of(f);
                return nm.starts_with("hook_") && std::meta::is_function(f);
            }
            return false;
        }

        consteval hook_issue_t is_hook_function_ok(std::meta::info f)
        {
            auto func_type = std::meta::type_of(f);
            auto return_type = std::meta::return_type_of(func_type);
            if (return_type != ^^void) return hook_issue_t::ReturnType;
            auto params = std::meta::parameters_of(func_type);
            if (params.size() != 2)
                return hook_issue_t::ArgCount;
            if (params[0] != std::meta::dealias(^^uint8_t))
                return hook_issue_t::ArgEP;//must be EP
            return hook_issue_t::Ok;
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
            int hooks = 0;
        };

        template<class T>
        consteval hook_check_res_t check_hook_type()
        {
            hook_check_res_t res;
            auto type_name = std::meta::display_string_of(^^T);
            auto ctx = std::meta::access_context::current();
            if (!std::meta::is_class_type(^^T))
            {
                res.message += type_name;
                res.message += " must be a struct/class type!";
                res.valid = false;
                return res;
            }
            for(auto m : std::meta::members_of(^^T, ctx))
            {
                if (is_hook_function(m))
                {
                    if (auto r = is_hook_function_ok(m); r != hook_issue_t::Ok)
                    {
                        res.valid = false;
                        if (res.message.size() != 0)
                            res.message += "; ";
                        auto func_type = std::meta::type_of(m);
                        switch(r)
                        {
                            case hook_issue_t::Ok:
                                //shall not be possible to reach
                                break;
                            case hook_issue_t::ReturnType:
                                res.message += type_name;
                                res.message += "::";
                                res.message += std::meta::identifier_of(m);
                                res.message += ": return type must be void, not ";
                                res.message += std::meta::display_string_of(std::meta::return_type_of(func_type));
                                break;
                            case hook_issue_t::ArgCount:
                                res.message += type_name;
                                res.message += "::";
                                res.message += std::meta::identifier_of(m);
                                res.message += ": must have 2 arguments, but ";
                                res.message += std::meta::parameters_of(func_type).size();
                                res.message += " found";
                                break;
                            case hook_issue_t::ArgEP:
                                res.message += type_name;
                                res.message += "::";
                                res.message += std::meta::identifier_of(m);
                                res.message += ": 1st argument must be uint8_t for ep, not ";
                                res.message += std::meta::display_string_of(std::meta::parameters_of(func_type)[0]);
                                break;
                        }
                    }else
                    {
                        ++res.hooks;
                    }
                }
            }
            return res;
        }

        template<std::meta::info handler>
        consteval hook_check_res_t check_handler_for_hook_type()
        {
            if constexpr (handler != std::meta::info{})
                return check_hook_type<typename [:handler:]>();
            else
                return {};
        }


        /**********************************************************************/
        /* attribute validation                                               */
        /**********************************************************************/

        enum class validate_issue_t
        {
            Ok,
            ReturnType,
            ArgCount,
            ArgDataPtr
        };

        consteval bool is_validate_function(std::meta::info f)
        {
            if (std::meta::has_identifier(f))
            {
                auto nm = std::meta::identifier_of(f);
                return nm.starts_with("validate_") && std::meta::is_function(f);
            }
            return false;
        }

        consteval validate_issue_t is_validate_function_ok(std::meta::info f)
        {
            auto func_type = std::meta::type_of(f);
            auto return_type = std::meta::return_type_of(func_type);
            if (std::meta::dealias(return_type) != std::meta::dealias(^^zb_ret_t)) return validate_issue_t::ReturnType;
            auto params = std::meta::parameters_of(func_type);
            if (params.size() != 1)
                return validate_issue_t::ArgCount;
            if (params[0] != std::meta::dealias(^^zb_uint8_t *))
                return validate_issue_t::ArgDataPtr;
            return validate_issue_t::Ok;
        }

        consteval std::meta::info find_validate_to_run(std::meta::info validate_handler, std::meta::info attribute_mem_refl)
        {
            if (validate_handler == std::meta::info{})
                return {};
            auto ctx = std::meta::access_context::current();
            comp_str_t<255> expected_validate_name;
            expected_validate_name += "validate_";
            expected_validate_name += std::meta::identifier_of(attribute_mem_refl);

            for(auto m : std::meta::members_of(validate_handler, ctx))
            {
                if (std::meta::has_identifier(m) && (std::meta::identifier_of(m) == expected_validate_name.sv()))
                    return m;
            }
            return {};
        }


        struct validate_check_res_t
        {
            bool valid = true;
            static_assert_str_t message;
            int validators = 0;
        };

        template<class T>
        consteval validate_check_res_t check_validator_type()
        {
            validate_check_res_t res;
            auto type_name = std::meta::display_string_of(^^T);
            auto ctx = std::meta::access_context::current();
            if (!std::meta::is_class_type(^^T))
            {
                res.message += type_name;
                res.message += " must be a struct/class type!";
                res.valid = false;
                return res;
            }
            for(auto m : std::meta::members_of(^^T, ctx))
            {
                if (is_validate_function(m))
                {
                    if (auto r = is_validate_function_ok(m); r != validate_issue_t::Ok)
                    {
                        res.valid = false;
                        if (res.message.size() != 0)
                            res.message += "; ";
                        auto func_type = std::meta::type_of(m);
                        switch(r)
                        {
                            case validate_issue_t::Ok:
                                //shall not be possible to reach
                                break;
                            case validate_issue_t::ReturnType:
                                res.message += type_name;
                                res.message += "::";
                                res.message += std::meta::identifier_of(m);
                                res.message += ": return type must be void, not ";
                                res.message += std::meta::display_string_of(std::meta::return_type_of(func_type));
                                break;
                            case validate_issue_t::ArgCount:
                                res.message += type_name;
                                res.message += "::";
                                res.message += std::meta::identifier_of(m);
                                res.message += ": must have 1 argument, but ";
                                res.message += std::meta::parameters_of(func_type).size();
                                res.message += " found";
                                break;
                            case validate_issue_t::ArgDataPtr:
                                res.message += type_name;
                                res.message += "::";
                                res.message += std::meta::identifier_of(m);
                                res.message += ": 1st argument must be uint8_t*, not ";
                                res.message += std::meta::display_string_of(std::meta::parameters_of(func_type)[0]);
                                break;
                        }
                    }else
                    {
                        ++res.validators;
                    }
                }
            }
            return res;
        }

        template<std::meta::info handler>
        consteval validate_check_res_t check_handler_for_validator_type()
        {
            if constexpr (handler != std::meta::info{})
                return check_validator_type<typename [:handler:]>();
            else
                return {};
        }


    }
}

#endif
