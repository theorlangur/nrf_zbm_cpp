#ifndef ZB_META_HANDLERS_HPP_
#define ZB_META_HANDLERS_HPP_

extern "C" {
#include <zboss_api.h>
}
#include "zbm_buf.hpp"
#include "zbm_str.hpp"
#include <meta>

namespace zbm
{
    using dev_callback_handler_t = void(*)(zb_zcl_device_callback_param_t *);
    using err_callback_handler_t = void(*)(int err);
    using set_attr_value_handler_t = void (*)(zb_zcl_set_attr_value_param_t *p, zb_zcl_device_callback_param_t *pDevCBParam);
    struct dev_cb_handlers_desc_t
    {
        dev_callback_handler_t default_handler = nullptr;
        err_callback_handler_t error_handler = nullptr;
    };

    struct cb_handler_t
    {
        enum zb_zcl_device_callback_id_e id;
        std::meta::info ep;
        std::meta::info target;
        std::meta::info handler;
    };

    template<auto handler>
    consteval cb_handler_t on_set_attribute_value(std::meta::info ep, std::meta::info target)
    {
        return cb_handler_t{
            .id = ZB_ZCL_SET_ATTR_VALUE_CB_ID, 
            .ep = ep, 
            .target = target, 
            .handler = std::meta::reflect_constant(handler)
        };
    };

    template<auto &inst, auto handler_method>
    consteval cb_handler_t on_set_attribute_value(std::meta::info ep, std::meta::info target)
    {
        return cb_handler_t{
            .id = ZB_ZCL_SET_ATTR_VALUE_CB_ID, 
            .ep = ep, 
            .target = target, 
            .handler = ^^refl::call_method_1arg<inst, handler_method>
        };
    };

    struct cb_generic_filter_t
    {
        static constexpr uint16_t kCLUSTER_ANY = 0xffff;
        static constexpr uint16_t kATTRIBUTE_ANY = 0xffff;

        uint16_t cluster = kCLUSTER_ANY;
        uint16_t attribute = kATTRIBUTE_ANY;
    };

    struct cb_generic_handler_t
    {
        using cb_t = void(*)(zb_zcl_device_callback_param_t*);
        cb_t handler = nullptr;
    };

    namespace details
    {
        struct cb_id_range
        {
            unsigned min;
            unsigned max;
        };
        consteval std::optional<cb_id_range> get_cb_id_range(std::initializer_list<cb_handler_t> h)
        {
            if (h.size() == 0)
                return std::nullopt;

            cb_id_range res{.min = h.begin()->id, .max = h.begin()->id};
            for(auto _h : h)
            {
                if (_h.id < res.min) res.min = _h.id;
                if (_h.id > res.max) res.max = _h.id;
            }
            return res;
        }

        consteval auto filter_handlers_by_cb_id(unsigned cb_id, std::span<const cb_handler_t> handlers)
        {
            std::vector<cb_handler_t> filtered;
            for(auto h : handlers)
            {
                if (h.id == cb_id)
                    filtered.push_back(h);
            }
            return filtered;
        }

        template<size_t N, class T>
        consteval auto span_to_array(std::span<T> s)
        {
            std::array<std::remove_const_t<T>, N> ar{};
            size_t i = 0;
            for(auto h : s)
            {
                ar[i] = h;
                ++i;
            }
            return ar;
        }

        consteval std::optional<std::meta::info> cb_id_to_param_type(unsigned cb_id)
        {
            struct info_t
            {
                unsigned id;
                std::meta::info type_refl;
            };
            std::array info {
                info_t{.id = ZB_ZCL_SET_ATTR_VALUE_CB_ID, .type_refl = ^^zb_zcl_set_attr_value_param_t},
            };

            for(auto const& i : info)
                if (i.id == cb_id)
                    return i.type_refl;
            return std::nullopt;
        }

        consteval std::optional<std::meta::info> find_cb_param(std::meta::info param_type)
        {
            for(auto m : std::meta::nonstatic_data_members_of(std::meta::type_of(^^zb_zcl_device_callback_param_t::cb_param), std::meta::access_context::current()))
            {
                if (std::meta::dealias(std::meta::type_of(m)) == std::meta::dealias(param_type))
                    return m;
            }
            return std::nullopt;
        }

        enum class handler_type_e
        {
            NoArgs,
            DevCallback,
            DevCallbackAndTypedParam,
            SetAttrValueTyped,
            SetAttrValue1ArgTyped,
            More,
        };

        template<unsigned cb_id>
        consteval std::optional<handler_type_e> infer_handler_type(std::meta::info h, std::meta::info cb_param_type)
        {
            auto f_type = refl::get_callable_type(h);
            if (!f_type) return std::nullopt;
            auto params = std::meta::parameters_of(*f_type);
            if (params.size() == 0)
                return handler_type_e::NoArgs;

            if (params.size() >= 1)
            {
                if constexpr (cb_id != ZB_ZCL_SET_ATTR_VALUE_CB_ID)
                {
                    if (params[0] != ^^zb_zcl_device_callback_param_t*)
                        return std::nullopt;

                    if (params.size() == 1)
                        return handler_type_e::DevCallback;
                }else
                {
                    if (params[0] == ^^zb_zcl_device_callback_param_t*)
                    {
                        if (params.size() == 1)
                            return handler_type_e::DevCallback;
                    }else if (params.size() == 1)
                        return handler_type_e::SetAttrValue1ArgTyped;
                    else
                        return std::nullopt;
                }

                if constexpr (cb_id != ZB_ZCL_SET_ATTR_VALUE_CB_ID)
                {
                    if (!std::meta::is_type(cb_param_type) || params[1] != std::meta::add_pointer(cb_param_type))
                        return std::nullopt;

                    if (params.size() == 2)
                        return handler_type_e::DevCallbackAndTypedParam;
                }else
                {
                    if (!std::meta::is_type(cb_param_type))
                        return std::nullopt;

                    if (params.size() == 2)
                    {
                        if (params[1] != std::meta::add_pointer(cb_param_type))
                            return handler_type_e::SetAttrValueTyped;
                        else
                            return handler_type_e::DevCallbackAndTypedParam;
                    }
                }

            }
            return handler_type_e::More;
        }

        template<handler_type_e ht, std::meta::info h, class T>
        void call_handler_for_set_attr_value(zb_zcl_device_callback_param_t *pDev, zb_zcl_set_attr_value_param_t *pSetAttributeValue, T const& a)
        {
            if constexpr (ht == handler_type_e::More)
                [:h:](pDev, pSetAttributeValue, a);
            else if constexpr (ht == handler_type_e::SetAttrValueTyped)
                [:h:](pDev, a);
            else if constexpr (ht == handler_type_e::SetAttrValue1ArgTyped)
                [:h:](a);
        }

        template<unsigned cb_id, auto handlers>
        void generic_cb_handler(zb_zcl_device_callback_param_t *pDev)
        {
            constexpr auto info = cb_id_to_param_type(cb_id);
            constexpr std::meta::info cb_param_type = info ? *info : std::meta::info{};
            template for (constexpr auto h : handlers)
            {
                constexpr auto ep_a = try_get_ep_annotation(h.ep);
                if (!ep_a || ep_a->ep == pDev->endpoint)
                {
                    static constexpr auto attr_a = try_get_attribute_annotation(h.target);
                    static constexpr auto cluster_a = get_cluster_annotation(h.target);
                    if constexpr (cb_id == ZB_ZCL_SET_ATTR_VALUE_CB_ID)
                    {
                        static_assert(h.target == std::meta::info{} || attr_a || cluster_a, "Cannot use this as a target");
                        constexpr cb_generic_filter_t filter = []()consteval ->cb_generic_filter_t
                        {
                            if constexpr (attr_a)
                            {
                                constexpr auto cluster_a = get_cluster_annotation(std::meta::parent_of(h.target));
                                static_assert(cluster_a, "Cannot find cluster by attribute");
                                return {.cluster = cluster_a->id, .attribute = attr_a->id};
                            }else if constexpr (cluster_a)
                                return {.cluster = cluster_a->id};
                            else
                                return {};
                        }();

                        zb_zcl_set_attr_value_param_t *pSetAttributeValue = &pDev->cb_param.set_attr_value_param;
                        if ((filter.attribute != cb_generic_filter_t::kATTRIBUTE_ANY && filter.attribute != pSetAttributeValue->attr_id)
                            || (filter.cluster != cb_generic_filter_t::kCLUSTER_ANY && filter.cluster != pSetAttributeValue->cluster_id)
                           )
                        {
                            continue;
                        }
                    }
                    constexpr std::optional<handler_type_e> ht = infer_handler_type<cb_id>(h.handler, cb_param_type);
                    static_assert(ht, "Don't know how to call handler");
                    if constexpr (*ht == handler_type_e::NoArgs)
                        [:h.handler:]();
                    else if constexpr (*ht == handler_type_e::DevCallback)
                        [:h.handler:](pDev);
                    else if constexpr (*ht == handler_type_e::DevCallbackAndTypedParam)
                    {
                        constexpr auto cb_param_mem_opt = find_cb_param(cb_param_type);
                        static_assert(cb_param_mem_opt, "Cannot find a member in the cb_param union for the type");
                        [:h.handler:](pDev, &pDev->cb_param.[:*cb_param_mem_opt:]);
                    }
                    else if constexpr (*ht == handler_type_e::More || *ht == handler_type_e::SetAttrValueTyped || *ht == handler_type_e::SetAttrValue1ArgTyped)
                    {
                        static_assert(cb_id == ZB_ZCL_SET_ATTR_VALUE_CB_ID, "Typed handlers are supported only for ZB_ZCL_SET_ATTR_VALUE_CB_ID");
                        static_assert(attr_a, "This handler format requires attribute as a target");
                        zb_zcl_set_attr_value_param_t *pSetAttributeValue = &pDev->cb_param.set_attr_value_param;
                        constexpr auto attr_type = std::meta::type_of(h.target);

                        if constexpr ((attr_a->type == type_t::OctetStr) 
                                || (attr_a->type == type_t::CharStr)
                                || (attr_a->type == type_t::LongOctetStr)
                                || (attr_a->type == type_t::Array)
                                || (attr_a->type == type_t::Custom32Array)
                                || (attr_a->type == type_t::Sec128Key)
                            )
                        {
                            static_assert((attr_a->type == type_t::OctetStr) || (attr_a->type == type_t::CharStr), "Only char/octet str are supported atm");
                            if constexpr (attr_a->type == type_t::OctetStr)
                            {
                                if constexpr (std::meta::has_template_arguments(attr_type) && std::meta::template_of(attr_type) == ^^bin_typed_t)
                                {
                                    constexpr auto act_type_refl = std::meta::template_arguments_of(attr_type)[0];
                                    bin_view_t<typename [:act_type_refl:]> ov{(uint8_t*)pSetAttributeValue->values.data_variable.p_data};
                                    call_handler_for_set_attr_value<*ht, h.handler>(pDev, pSetAttributeValue, ov);
                                }
                                else
                                {
                                    octet_view_t ov{(uint8_t*)pSetAttributeValue->values.data_variable.p_data};
                                    call_handler_for_set_attr_value<*ht, h.handler>(pDev, pSetAttributeValue, ov);
                                }
                            }else if constexpr (attr_a->type == type_t::CharStr)
                            {
                                str_view_t sv{(char*)pSetAttributeValue->values.data_variable.p_data};
                                call_handler_for_set_attr_value<*ht, h.handler>(pDev, pSetAttributeValue, sv);
                            }
                        }else
                            call_handler_for_set_attr_value<*ht, h.handler>(pDev, pSetAttributeValue, *(typename[:attr_type:]*)&pSetAttributeValue->values);
                    }
                }
            }
        }

        template<cb_id_range range, auto handlers>
        consteval auto organize_cb_into_lookup()
        {
            constexpr size_t table_size = range.max - range.min + 1;
            std::array<cb_generic_handler_t::cb_t, table_size> mid_storage;

            template for(constexpr auto I : std::ranges::views::iota(unsigned(0), range.max - range.min + 1))
            {
                static constexpr auto filtered_handlers = std::define_static_array(filter_handlers_by_cb_id(I + range.min, std::span{handlers.begin(), handlers.end()}));
                static constexpr auto filtered_handlers_array = span_to_array<filtered_handlers.size()>(filtered_handlers);
                mid_storage[I] = &generic_cb_handler<unsigned(I + range.min), filtered_handlers_array>;
            }

            return mid_storage;
        }
    }
    
    template<dev_cb_handlers_desc_t generic={}, cb_handler_t... handlers>
    void tpl_device_cb(zb_bufid_t bufid)
    {
        buf_view_ptr_t bv{bufid};
        auto *pDevParam = bv.param<zb_zcl_device_callback_param_t>();
        if (!pDevParam)
        {
            if (generic.error_handler)
                generic.error_handler(-1);
            return;
        }
        pDevParam->status = RET_OK;
        static constexpr auto rng_opt = details::get_cb_id_range(std::initializer_list{handlers...});

        if constexpr (rng_opt)
        {
            static constexpr auto rng = *rng_opt;
            static constexpr auto cb_id_lookup = details::organize_cb_into_lookup<rng, std::array{handlers...}>();
            if (pDevParam->device_cb_id >= rng.min && pDevParam->device_cb_id <= rng.max)
            {
                cb_id_lookup[pDevParam->device_cb_id](pDevParam);
            }
        }

        if constexpr (generic.default_handler)
            generic.default_handler(pDevParam);
    }
}

#endif
