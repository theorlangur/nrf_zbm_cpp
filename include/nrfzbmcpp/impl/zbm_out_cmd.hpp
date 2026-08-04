#ifndef ZB_META_OUT_CMD_HPP_
#define ZB_META_OUT_CMD_HPP_

#include "lib_object_pool.hpp"
#include "zbm_annotations.hpp"
#include <format>

namespace zbm
{
    static constexpr size_t kMaxAllowedArgumentSize = 100;

    template<class Func> requires std::is_function_v<Func>
    struct cmd_out_t
    {
        //std::meta::info -> parameter types
        static constexpr auto g_Params = []() consteval{
            auto params = std::meta::parameters_of(^^Func);
            return std::define_static_array(params);
        }();
        static constexpr size_t g_ParamsTotalSize = []() consteval -> size_t{
            size_t r = 0;
            for(auto p : g_Params)
                r += std::meta::size_of(p);
            return r;
        }();

        static_assert(g_Params.size() <= 10, "Too many arguments. Max 10 is supported");
    };

    struct short_addr_t
    {
        using addr_tag = void;
        uint16_t short_addr;
        uint8_t ep;
    };
    struct long_addr_t
    {
        using addr_tag = void;
        long_addr_t(zb_ieee_addr_t a, uint8_t e):
            ep(e)
        {
            memcpy(long_addr, a, sizeof(zb_ieee_addr_t));
        }
        zb_ieee_addr_t long_addr;
        uint8_t ep;
    };
    struct group_addr_t
    {
        using addr_tag = void;
        uint16_t group;
    };
    struct bind_id_addr_t
    {
        using addr_tag = void;
        uint8_t bind_table_id;
    };

    inline short_addr_t to_short(uint16_t _short, uint8_t ep) { return {_short, ep}; }
    inline long_addr_t to_long(zb_ieee_addr_t addr, uint8_t ep) { return {addr, ep}; }
    inline group_addr_t to_group(uint16_t group) { return {group}; }
    inline bind_id_addr_t to_bind_id(uint8_t id) { return {id}; }

    template<class C>
    concept is_zb_addr_type_c = requires{ typename C::addr_tag; };


    template<class... Args>
    zb_ret_t send_cmd_raw(zb_bufid_t b, zb_callback_t cb, cmd_out_a cmd_a, cluster_rt_a clust_a, const zb_addr_u &addr, addr_mode_t mode, uint8_t dst_ep, uint8_t src_ep, Args&&...args)
    {
        uint16_t manu_code = clust_a.manuf_code != ZB_ZCL_MANUF_CODE_INVALID ? clust_a.manuf_code : cmd_a.manuf_code;
        frame_ctl_t f{.f{
            .cluster_specific = true, 
                .manufacture_specific = manu_code != ZB_ZCL_MANUF_CODE_INVALID
                    , .direction = clust_a.role == role_t::Client ? frame_direction_t::ToServer : frame_direction_t::ToClient
                    , .disable_default_response = false
        }};
        ZB_ZCL_GET_SEQ_NUM();
        uint8_t* ptr = (uint8_t*)zb_zcl_start_command_header(b, f.u8, manu_code, cmd_a.id, nullptr);
        uint8_t* init = ptr;
        template for(constexpr size_t i : std::ranges::views::indices(sizeof...(Args)))
            ptr = *serialize_to(args...[i], ptr, kMaxAllowedArgumentSize - (ptr - init));
        return zb_zcl_finish_and_send_packet(b, ptr, &addr, (uint8_t)mode/*addr mode*/, dst_ep, src_ep, ZB_AF_HA_PROFILE_ID, clust_a.id, cb);
    }

    template<cmd_out_a cmd_a, cluster_rt_a clust_a, class... Args>
    zb_ret_t send_cmd_raw_cluster(zb_bufid_t b, zb_callback_t cb, const zb_addr_u &addr, addr_mode_t mode, uint8_t dst_ep, uint8_t src_ep, Args&&...args)
    {
        return send_cmd_raw<Args...>(b, cb, cmd_a, clust_a, addr, mode, dst_ep, src_ep, std::forward<Args>(args)...);
    }

    template<cmd_out_a cmd_a, cluster_rt_a clust_a, class... Args>
    zb_ret_t send_cmd_raw_cluster_to(zb_bufid_t b, zb_callback_t cb, uint8_t src_ep, Args&&...args)
    {
        return send_cmd_raw_cluster<cmd_a, clust_a, Args...>(b, cb, {.addr_short = 0}, addr_mode_t::NoAddr_NoEP, 0, src_ep, std::forward<Args>(args)...);
    }

    template<cmd_out_a cmd_a, cluster_rt_a clust_a, class... Args>
    zb_ret_t send_cmd_raw_cluster_to_short(short_addr_t a, zb_bufid_t b, zb_callback_t cb, uint8_t src_ep, Args&&...args)
    {
        return send_cmd_raw_cluster<cmd_a, clust_a, Args...>(b, cb, {.addr_short = a.short_addr}, addr_mode_t::Dst16EP, a.ep, src_ep, std::forward<Args>(args)...);
    }

    template<cmd_out_a cmd_a, cluster_rt_a clust_a, class... Args>
    zb_ret_t send_cmd_raw_cluster_to_long(long_addr_t a, zb_bufid_t b, zb_callback_t cb, uint8_t src_ep, Args&&...args)
    {
        zb_addr_u addr;
        std::memcpy(addr.addr_long, a.long_addr, sizeof(a.long_addr));
        return send_cmd_raw_cluster<cmd_a, clust_a, Args...>(b, cb, addr, addr_mode_t::Dst64EP, a.ep, src_ep, std::forward<Args>(args)...);
    }

    template<cmd_out_a cmd_a, cluster_rt_a clust_a, class... Args>
    zb_ret_t send_cmd_raw_cluster_to_group(group_addr_t a, zb_bufid_t b, zb_callback_t cb, uint8_t src_ep, Args&&...args)
    {
        return send_cmd_raw_cluster<cmd_a, clust_a, Args...>(b, cb, {.addr_short = a.group}, addr_mode_t::Group_NoEP, 0, src_ep, std::forward<Args>(args)...);
    }

    template<cmd_out_a cmd_a, cluster_rt_a clust_a, class... Args>
    zb_ret_t send_cmd_raw_cluster_to_bind_id(bind_id_addr_t a, zb_bufid_t b, zb_callback_t cb, uint8_t src_ep, Args&&...args)
    {
        return send_cmd_raw_cluster<cmd_a, clust_a, Args...>(b, cb, {.addr_short = 0}, addr_mode_t::EPAsBindTableId, a.bind_table_id, src_ep, std::forward<Args>(args)...);
    }

    template<std::meta::info cmd_mem>
    struct send_cmd_mem
    {
        using cmd_t = typename [:std::meta::type_of(cmd_mem):];
        static constexpr auto cmd_a = *get_sending_command_annotation(cmd_mem);
        static constexpr cluster_rt_a cluster_a = cluster_rt_a{*get_parent_cluster_annotation(cmd_mem)};
        static constexpr auto substitue_args = []() consteval{
            std::vector<std::meta::info> args;
            args.reserve(cmd_t::g_Params.size() + 2);
            args.push_back(std::meta::reflect_constant(cmd_a));
            args.push_back(std::meta::reflect_constant(cluster_a));
            args.insert(args.end(), cmd_t::g_Params.begin(), cmd_t::g_Params.end());
            return std::define_static_array(args);
        }();

        static constexpr auto to = []() consteval { constexpr auto f = std::meta::substitute(^^send_cmd_raw_cluster_to, substitue_args); return &[:f:]; }();
        static constexpr auto to_short = []() consteval { constexpr auto f = std::meta::substitute(^^send_cmd_raw_cluster_to_short, substitue_args); return &[:f:]; }();
        static constexpr auto to_long = []() consteval { constexpr auto f = std::meta::substitute(^^send_cmd_raw_cluster_to_long, substitue_args); return &[:f:]; }();
        static constexpr auto to_group = []() consteval { constexpr auto f = std::meta::substitute(^^send_cmd_raw_cluster_to_group, substitue_args); return &[:f:]; }();
        static constexpr auto to_bind_id = []() consteval { constexpr auto f = std::meta::substitute(^^send_cmd_raw_cluster_to_bind_id, substitue_args); return &[:f:]; }();
    };
}

#endif
