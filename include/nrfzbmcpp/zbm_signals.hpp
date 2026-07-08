#ifndef ZB_META_SIGNALS_HPP_
#define ZB_META_SIGNALS_HPP_

#include "zbm_buf.hpp"
#include <concepts>
#include <array>
#include <meta>

extern "C"{
#include <zigbee/zigbee_app_utils.h>
}

namespace zbm
{
    struct sig_handlers_t
    {
        unsigned signal;
        std::meta::info function_refl;
    };

    namespace details
    {
        consteval bool is_valid_signal(unsigned SigId)
        { 
            return (SigId >= ZB_ZDO_SIGNAL_DEFAULT_START && SigId <= ZB_DEBUG_SIGNAL_TCLK_READY);
        }

        struct signal_to_param_t
        {
            unsigned signal;
            std::meta::info param_type_ref;
        };

        /**********************************************************************/
        /* Signals that support specific parameters                           */
        /**********************************************************************/
        static constexpr std::array g_SignalToParams = {
             signal_to_param_t{ZB_ZDO_SIGNAL_DEVICE_ANNCE,              ^^zb_zdo_signal_device_annce_params_t}
            ,signal_to_param_t{ZB_ZDO_SIGNAL_LEAVE,                     ^^zb_zdo_signal_leave_params_t}
            ,signal_to_param_t{ZB_NWK_SIGNAL_DEVICE_ASSOCIATED,         ^^zb_nwk_signal_device_associated_params_t}
            ,signal_to_param_t{ZB_ZDO_SIGNAL_LEAVE_INDICATION,          ^^zb_zdo_signal_leave_indication_params_t}
            ,signal_to_param_t{ZB_COMMON_SIGNAL_CAN_SLEEP,              ^^zb_zdo_signal_can_sleep_params_t}
            ,signal_to_param_t{ZB_ZDO_SIGNAL_DEVICE_AUTHORIZED,         ^^zb_zdo_signal_device_authorized_params_t}
            ,signal_to_param_t{ZB_ZDO_SIGNAL_DEVICE_UPDATE,             ^^zb_zdo_signal_device_update_params_t}
            ,signal_to_param_t{ZB_NLME_STATUS_INDICATION,               ^^zb_nwk_command_status_t}
            ,signal_to_param_t{ZB_NWK_SIGNAL_PERMIT_JOIN_STATUS,        ^^zb_uint8_t}
            ,signal_to_param_t{ZB_ZDO_SIGNAL_DEVICE_INTERVIEW_FINISHED, ^^zb_zdo_signal_device_ready_for_interview_params_t}
            ,signal_to_param_t{ZB_ZDO_DEVICE_UNAVAILABLE,               ^^zb_zdo_device_unavailable_params_t}
        };

        template<std::meta::info callable_refl>
        consteval std::optional<std::meta::info> get_call_operator_overload()
        {
            for(auto m : std::meta::members_of(callable_refl, std::meta::access_context::current()))
            {
                auto t = std::meta::type_of(m);
                if (std::meta::is_function_type(t) && std::meta::identifier_of(m) == "operator()")
                    return t;
            }
            return std::nullopt;
        }

        struct function_type_t
        {
            std::meta::info functionType;
        };
        template<std::meta::info callable_refl>
        consteval function_type_t get_callable_type()
        {
            constexpr auto type_refl = std::meta::is_type(callable_refl) ? callable_refl : std::meta::type_of(callable_refl);
            if constexpr (std::meta::is_pointer_type(type_refl) || std::meta::is_function_type(type_refl))
            {
                constexpr auto no_ptr = std::meta::remove_pointer(type_refl);
                static_assert(std::meta::is_function_type(no_ptr), "A handler of pointer type shall only be a pointer to a function");
                return {no_ptr};
            }else
            {
                static_assert(std::meta::is_object_type(type_refl), "If not a function pointer, than it shall be a functor");
                constexpr auto func_call_t_refl = get_call_operator_overload<type_refl>();
                static_assert(func_call_t_refl, "Could not find fitting operator() overload");
                return {*func_call_t_refl};
            }
        }

        consteval std::optional<signal_to_param_t> find_signal_with_parameters(unsigned signal)
        {
            for(auto s : g_SignalToParams)
                if (s.signal == signal)
                    return s;
            return std::nullopt;
        }

        template<sig_handlers_t h>
        std::optional<zb_ret_t> invoke_signal(zb_ret_t status, zb_zdo_app_signal_hdr_t *pHdr)
        {
            static_assert(details::is_valid_signal(h.signal), "Don't know the signal");
            constexpr auto f_type_refl = details::get_callable_type<h.function_refl>().functionType;
            constexpr auto ret_type_refl = std::meta::return_type_of(f_type_refl);
            constexpr bool optional_default_handlling = ret_type_refl == ^^std::optional<zb_ret_t>;
            static_assert(ret_type_refl == ^^void || ret_type_refl == ^^std::optional<zb_ret_t>, "Handler may return nothing or boolean to indicate if the signal was completely processed");
            constexpr auto signal_param_type_relf = details::find_signal_with_parameters(h.signal);
            constexpr size_t max_allowed_params = 1 + (bool)signal_param_type_relf;
            constexpr auto params = std::define_static_array(std::meta::parameters_of(f_type_refl));
            static_assert(params.size() <= max_allowed_params, "Too many parameters");

            if constexpr (params.size() >= 1)
            {
                static_assert(params[0] == std::meta::dealias(^^zb_ret_t), "1st parameter must be zb_ret_t");
                if constexpr (params.size() == 2)
                {
                    static_assert(std::meta::remove_const(params[1]) == std::meta::add_pointer(signal_param_type_relf->param_type_ref), "2nd parameter must be signal specific type pointer");
                    auto *pTyped = reinterpret_cast<typename [:signal_param_type_relf->param_type_ref:]*>((uint8_t*)pHdr + sizeof(zb_zdo_app_signal_hdr_t));
                    if constexpr (optional_default_handlling)
                    {
                        if (auto r = [:h.function_refl:](status, pTyped))
                            return *r;
                    }else
                        [:h.function_refl:](status, pTyped);
                }else
                {
                    //1. arg case
                    if constexpr (optional_default_handlling)
                    {
                        if (auto r = [:h.function_refl:](status))
                            return *r;
                    }else
                        [:h.function_refl:](status);
                }
            }else
            {
                //no args
                if constexpr (optional_default_handlling)
                {
                    if (auto r = [:h.function_refl:]())
                        return *r;
                }else
                    [:h.function_refl:]();
            }

            return std::nullopt;
        }
    }

    template<sig_handlers_t... handlers>
    zb_ret_t tpl_signal_handler(zb_bufid_t bufid)
    {
        buf_ptr_t b{bufid};
        zb_zdo_app_signal_hdr_t *pHdr;
        auto signalId = zb_get_app_signal(bufid, &pHdr);
        zb_ret_t status = zb_buf_get_status(bufid);

        template for(constexpr auto h : {handlers...})
        {
            if (signalId == h.signal)
            {
                if (auto r = details::invoke_signal<h>(status, pHdr))
                    return *r;
                return zigbee_default_signal_handler(bufid);
            }
        }

        return zigbee_default_signal_handler(bufid);
    }
}

#endif
