#ifndef ZB_META_EP_HPP_
#define ZB_META_EP_HPP_

#include "zbm_alarm.hpp"
#include "zbm_types.hpp"
#include "zbm_buf.hpp"
#include "zbm_annotations.hpp"
#include "zbm_cluster.hpp"
#include "zbm_out_cmd.hpp"


namespace zbm
{
    template<size_t ServerCount, size_t ClientCount>
    struct ZB_PACKED_PRE simple_desc_t: zb_af_simple_desc_1_1_t
    {
        zb_uint16_t app_cluster_list_ext[(ServerCount + ClientCount) - 2];
    } ZB_PACKED_STRUCT;

    template<size_t ServerCount, size_t ClientCount> requires ((ServerCount + ClientCount) < 2)
    struct ZB_PACKED_PRE simple_desc_t<ServerCount, ClientCount>: zb_af_simple_desc_1_1_t
    {
    } ZB_PACKED_STRUCT;


    struct short_addr_t
    {
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

    template<std::meta::info local_clusters_r>
    struct meta_ctr_param_t{};

    template<ep_base_cfg_t cfg, uint8_t ep_id>
    struct ep_base_t
    {
        using SimpleDesc = simple_desc_t<cfg.server_clusters, cfg.client_clusters>;

        consteval ep_base_t(ep_base_t const&) = delete;
        consteval ep_base_t(ep_base_t &&) = delete;
        template<std::meta::info local_clusters_r>
        consteval ep_base_t(ep_a epa, auto clusters, meta_ctr_param_t<local_clusters_r> dummy):
            simple_desc{ 
                /*base struct*/{
                    .endpoint = epa.ep,
                        .app_profile_id = ZB_AF_HA_PROFILE_ID, 
                        .app_device_id = epa.dev_id,
                        .app_device_version = epa.dev_ver,
                        .reserved = 0,
                        .app_input_cluster_count = (uint8_t)cfg.server_clusters,
                        .app_output_cluster_count = (uint8_t)cfg.client_clusters,
                        .app_cluster_list = {}
                } 
            },
            rep_ctx{},
            cvc_alarm_ctx{},
            clusters_descriptions{},
            ep{
                .ep_id = epa.ep,
                .profile_id = ZB_AF_HA_PROFILE_ID,
                .device_handler = nullptr,
                .identify_handler = nullptr,
                .reserved_size = 0,
                .reserved_ptr = nullptr,
                .cluster_count = uint8_t(cfg.server_clusters + cfg.client_clusters),
                .cluster_desc_list = clusters_descriptions,
                .simple_desc = &simple_desc,
                .rep_info_count = (uint8_t)cfg.reporting_attributes,
                .reporting_info = rep_ctx,
                .cvc_alarm_count = (uint8_t)cfg.cvc_attributes,
                .cvc_alarm_info = cvc_alarm_ctx
            }
        {
            static constexpr auto local_cluster_r = std::meta::remove_cvref(std::meta::type_of(local_clusters_r));
            static constexpr auto local_cluster_mems = std::define_static_array(std::meta::nonstatic_data_members_of(local_cluster_r, std::meta::access_context::current()));

            for(size_t i = 0, n = clusters.size(); i < n; ++i)
            {
                auto const& ca = clusters[i];
                if (i < 2)
                    simple_desc.app_cluster_list[i] = ca.annotation.id;
                else
                {
                    if constexpr (local_cluster_mems.size() > 2)
                        simple_desc.app_cluster_list_ext[i - 2] = ca.annotation.id;
                }
            }

            int i = 0;
            template for(constexpr auto m : local_cluster_mems)
            {
                auto const& ca = clusters[i];
                static constexpr auto cluster_refl = std::meta::reflect_object([:local_clusters_r:].[:m:]);//cluster_t &
                clusters_descriptions[i] = zb_zcl_cluster_desc_t{
                    .cluster_id = ca.annotation.id,
                    .attr_count = std::size([:local_clusters_r:].[:m:].attributes),
                    .attr_desc_list = [:local_clusters_r:].[:m:].attributes,
                    .role_mask = (zb_uint8_t)ca.annotation.role,
                    .manuf_code = ca.annotation.manuf_code,
                    .cluster_init = &generic_cluster_init<cluster_refl, ep_id>
                };
                ++i;
            }
        }

        alignas(4) SimpleDesc simple_desc;
        alignas(4) zb_zcl_reporting_info_t rep_ctx[cfg.reporting_attributes];
        alignas(4) zb_zcl_cvc_alarm_variables_t cvc_alarm_ctx[cfg.cvc_attributes];
        alignas(4) zb_zcl_cluster_desc_t clusters_descriptions[cfg.server_clusters + cfg.client_clusters];
        alignas(4) zb_af_endpoint_desc_t ep;
    };

    consteval ep_a get_ep_annotations(std::meta::info ep_ref)
    {
        auto ep_annotations = std::meta::annotations_of_with_type(ep_ref, ^^zbm::ep_a);
        return std::meta::extract<zbm::ep_a>(ep_annotations[0]);
    } 

    template<std::meta::info ep_mem_decl, std::meta::info epm>
    struct ep_factory_t
    {
        /*
         * struct ep_t
         * {
         *   ep_base_t<cfg, ep_a> ep_data;
         * };
         * */
        struct ep_t;//it's important this is a first declaration
        static constexpr ep_a g_Annotation = get_ep_annotations(ep_mem_decl);//source of constexpr template-capable ep_id
        consteval
        {
            constexpr std::meta::info ep_type = std::meta::remove_cvref(std::meta::type_of(epm));
            static_assert(verify_clusters_in_ep(ep_type) == std::meta::info{}, "Duplicate cluster found");
            auto mems = std::meta::nonstatic_data_members_of(ep_type, std::meta::access_context::current());
            std::vector<std::meta::info> cluster_types;
            for(auto m : mems)
                cluster_types.push_back(std::meta::type_of(m));
            if (!cluster_types.empty())
            {
                ep_base_cfg_t cfg = analyze_clusters(cluster_types);
                auto ep_data_type = std::meta::substitute(^^ep_base_t, {std::meta::reflect_constant(cfg), std::meta::reflect_constant(g_Annotation.ep)});
                std::meta::define_aggregate(^^ep_t, {
                        std::meta::data_member_spec(ep_data_type, std::meta::data_member_options{"ep_data"})
                        });
            }
        };
    };

    consteval std::meta::info get_ep_type_from_factory(std::meta::info ep_mem_decl, std::meta::info ep_ref)
    {
        auto ep_fact_inst = std::meta::substitute(^^ep_factory_t, {std::meta::reflect_constant(ep_mem_decl), std::meta::reflect_constant(ep_ref)});
        return std::define_static_array(std::meta::members_of(ep_fact_inst, std::meta::access_context::current()))[0];//struct ep_t;
    } 


    template<std::meta::info ep_mem_decl, std::meta::info ep_ref> requires (!std::meta::annotations_of_with_type(ep_mem_decl, ^^zbm::ep_a).empty())
    struct ep_create_t
    {
        using ep_type_t = [:get_ep_type_from_factory(ep_mem_decl, ep_ref):];
        static constexpr auto epa = get_ep_annotations(ep_mem_decl);
        static constexpr auto cluster_list = define_static_array(extract_clusters_from_ep(ep_ref));
        static constexpr auto cluster_refs = []() consteval{
            static_assert(!zbm::extract_clusters_from_ep(ep_ref).empty(), "No valid clusters found!");
            std::vector<std::meta::info> refs;
            template for(constexpr auto ci : cluster_list)
                refs.push_back(std::meta::reflect_object([:ep_ref:].[:ci.cluster:]));
            return define_static_array(refs);
        }();

        struct ep_front_t: ep_type_t
        {

            /**********************************************************************/
            /* Various types and declarations and statics                         */
            /**********************************************************************/
            static constexpr size_t kCmdQueueSize = epa.cmd_queue_depth;
            using cmd_id_t = int;
            using cmd_send_status_cb_t = void(*)(cmd_id_t, zb_zcl_command_send_status_t *);
            using send_request_func_t = bool (*)(uint16_t argsPoolIdx);
            using cancel_func_t = void (*)(uint16_t argsPoolIdx);
            inline static cmd_id_t g_cmd_num = 0;

            static constexpr size_t kMaxAllowedArgumentSize = 100;
            //static_assert(kCmdMaxArgsSize <= kMaxAllowedArgumentSize, "Too much data for command arguments");

            struct issued_cmd_t
            {
                cmd_send_status_cb_t cb = nullptr;
                zb_bufid_t buf = ZB_BUF_INVALID;
                cmd_id_t cmd_id;
            };
            inline static pre_alloc_zb_bufs_out<kCmdQueueSize> g_PreAllocBufs;
            inline static std::array<issued_cmd_t, kCmdQueueSize> g_IssuedCmds;

            static issued_cmd_t* find_free_issued_cmd_entry()
            {
                for(auto &cmd : g_IssuedCmds)
                {
                    if (cmd.buf == ZB_BUF_INVALID)
                        return &cmd;
                }
                return nullptr;
            }

            static void on_send_cmd_timeout2(zb_bufid_t buf)
            {
                for(auto &cmd : g_IssuedCmds)
                {
                    if (cmd.buf == buf)
                    {
                        //g_PreAllocBufs.deallocate(cmd.buf);
                        cmd.buf = ZB_BUF_INVALID;
                        cmd.cb(cmd.cmd_id, nullptr);
                        return;
                    }
                }
                ZB_ASSERT(false);
            }

            static void on_send_cmd_cb2(zb_uint8_t buf)
            {
                for(auto &cmd : g_IssuedCmds)
                {
                    if (cmd.buf == buf)
                    {
                        zb_schedule_alarm_cancel(on_send_cmd_cb2, buf, nullptr);
                        zb_zcl_command_send_status_t *cmd_send_status = buf ? ZB_BUF_GET_PARAM(buf, zb_zcl_command_send_status_t) : nullptr;
                        g_PreAllocBufs.deallocate(cmd.buf);//cmd_send_status memory is still valid
                        cmd.buf = ZB_BUF_INVALID;
                        cmd.cb(cmd.cmd_id, cmd_send_status);
                        return;
                    }
                }
                g_PreAllocBufs.deallocate(buf);
            }

            /**********************************************************************/
            /* Attribute Set                                                      */
            /**********************************************************************/
            template<std::meta::info attribute_refl, bool checked, class A>
            zb_zcl_status_t set_raw(A &&arg)
            {
                static constexpr auto user_cluster_ref = std::meta::parent_of(attribute_refl);
                static constexpr auto attribute_a = get_attribute_annotation(attribute_refl);
                using attribute_type_t = typename [:std::meta::type_of(attribute_refl):];

                static_assert(attribute_a.type != type_t::Invalid, "Undefined or incorrectly defined attribute!");
                //static_assert(attribute_a.a & access_t::Write, "Attribute is not writable!");
                static_assert(std::is_convertible_v<A, attribute_type_t>, "Cannot convert to attribute type");

                static constexpr auto local_cluster_r = ^^typename cluster_list_factory_t<ep_ref>::cluster_list_t;
                static constexpr auto local_cluster_mems = std::define_static_array(refl::nsdms_with_parents(local_cluster_r));
                static constexpr std::optional<std::meta::info> zbm_cluster_refl_opt = []()consteval -> std::optional<std::meta::info>
                {
                    template for(constexpr auto m : local_cluster_mems)
                    {
                        constexpr auto zbm_cluster_refl = std::meta::type_of(m);
                        using zbm_cluster_t = typename [:zbm_cluster_refl:];
                        constexpr auto zbm_cluster_type = std::meta::type_of(zbm_cluster_t::cluster_data_ref_refl);
                        if constexpr ((zbm_cluster_type == user_cluster_ref) || std::meta::is_base_of_type(user_cluster_ref, zbm_cluster_type))
                        {
                            static_assert(zbm_cluster_t::has_attribute(attribute_refl), "Attribute unknown to the framework");
                            return zbm_cluster_refl;
                        }
                    }
                    return std::nullopt;
                }();
                static_assert(zbm_cluster_refl_opt, "Attribute not found!");

                static constexpr auto zbm_cluster_refl = *zbm_cluster_refl_opt;
                using zbm_cluster_t = typename [:zbm_cluster_refl:];

                //ZBOSS API call
                return zb_zcl_set_attr_val(epa.ep, zbm_cluster_t::g_ClusterA.id, (zb_uint8_t)zbm_cluster_t::g_ClusterA.role, attribute_a.id, (zb_uint8_t*)&arg, checked);
            }

            template<std::meta::info attribute_refl, class A>
            zb_zcl_status_t set(A &&arg) { return set_raw<attribute_refl, false, A>(std::forward<A>(arg)); }
            template<std::meta::info attribute_refl, class A>
            zb_zcl_status_t set_checked(A &&arg) { return set_raw<attribute_refl, true, A>(std::forward<A>(arg)); }

            /**********************************************************************/
            /* Sending commands                                                   */
            /**********************************************************************/

            struct send_cmd_config_t
            {
                cmd_send_status_cb_t cb = nullptr;
                uint32_t timeout_ms = kCmdTimeoutDefault;
            };

            template<std::meta::info cmd_out_ref, send_cmd_config_t cfg={}, class... Args> requires (!is_zb_addr_type_c<Args> && ...)
            std::optional<cmd_id_t> send_cmd(Args&&...args)
            {
                return send_cmd_impl<cmd_out_ref, cfg>({.addr_short = 0}, addr_mode_t::NoAddr_NoEP, 0, std::forward<Args>(args)...);
            }

            template<std::meta::info cmd_out_refl, send_cmd_config_t cfg, class... Args>
            std::optional<cmd_id_t> send_cmd(long_addr_t a, Args&&...args)
            {
                zb_addr_u addr;
                std::memcpy(addr.addr_long, a.long_addr, sizeof(a.long_addr));
                return send_cmd_impl<cmd_out_refl, cfg>(addr, addr_mode_t::Dst64EP, a.ep, std::forward<Args>(args)...);
            }

            template<std::meta::info cmd_out_refl, send_cmd_config_t cfg, class... Args>
            std::optional<cmd_id_t> send_cmd(group_addr_t a, Args&&...args)
            {
                return send_cmd_impl<cmd_out_refl, cfg>({.addr_short = a.group}, addr_mode_t::Group_NoEP, 0, std::forward<Args>(args)...);
            }

            template<std::meta::info cmd_out_refl, send_cmd_config_t cfg, class... Args>
            std::optional<cmd_id_t> send_cmd(bind_id_addr_t a, Args&&...args)
            {
                return send_cmd_impl<cmd_out_refl, cfg>({.addr_short = 0}, addr_mode_t::EPAsBindTableId, a.bind_table_id, std::forward<Args>(args)...);
            }

            void init()
            {
                g_PreAllocBufs.init();
            }

            private:

            template<std::meta::info cmd_out_refl, send_cmd_config_t cfg={}, class... Args>
            std::optional<cmd_id_t> send_cmd_impl(zb_addr_u addr, addr_mode_t mode, uint8_t dst_ep, Args&&...args)
            {
                zb_bufid_t b = g_PreAllocBufs.allocate();
                if (b == ZB_BUF_INVALID)
                    return std::nullopt;
                issued_cmd_t *pIssued = find_free_issued_cmd_entry();
                ZB_ASSERT(pIssued);//size of issued array and pre-allocated are the same
                                   //so it must be valid
                                   

                static constexpr auto cmd_a = *get_sending_command_annotation(cmd_out_refl);
                static constexpr auto cluster_a = *get_parent_cluster_annotation(cmd_out_refl);

                constexpr auto kTimeout = cfg.timeout_ms == kCmdTimeoutDefault ? cmd_a.timeout_ms : cfg.timeout_ms;

                constexpr uint16_t manu_code = cluster_a.manuf_code != ZB_ZCL_MANUF_CODE_INVALID ? cluster_a.manuf_code : cmd_a.manuf_code;
                frame_ctl_t f{.f{
                    .cluster_specific = true, 
                        .manufacture_specific = manu_code != ZB_ZCL_MANUF_CODE_INVALID
                            , .direction = cluster_a.role == role_t::Client ? frame_direction_t::ToServer : frame_direction_t::ToClient
                            , .disable_default_response = false
                }};
                ZB_ZCL_GET_SEQ_NUM();
                uint8_t* ptr = (uint8_t*)zb_zcl_start_command_header(b, f.u8, manu_code, cmd_a.id, nullptr);
                uint8_t* init = ptr;
                template for(constexpr size_t i : std::ranges::views::indices(sizeof...(Args)))
                    ptr = *serialize_to(args...[i], ptr, kMaxAllowedArgumentSize - (ptr - init));
                zb_ret_t ret = zb_zcl_finish_and_send_packet(b, ptr, &addr, (uint8_t)mode/*addr mode*/, dst_ep, epa.ep, ZB_AF_HA_PROFILE_ID, cluster_a.id, on_send_cmd_cb2);
                if (RET_OK != ret)
                {
                    g_PreAllocBufs.deallocate(b);
                    return std::nullopt;
                }

                if constexpr (cfg.cb)
                {
                    pIssued->buf = b;
                    pIssued->cb = cfg.cb;
                    pIssued->cmd_id = g_cmd_num;
                    if (zb_schedule_app_alarm(on_send_cmd_timeout2, b, ZB_MILLISECONDS_TO_BEACON_INTERVAL(kTimeout)) != RET_OK)
                    {
                        pIssued->buf = ZB_BUF_INVALID;
                        g_PreAllocBufs.deallocate(b);
                        return std::nullopt;
                    }
                }

                return g_cmd_num++;
            }
        };

        //actual place where ZBOSS structures for attributes, commands for each cluster are stored
        constinit static inline cluster_list_factory_t<ep_ref>::cluster_list_t clusters{};
        //actual place where ZBOSS structures for endpoints with cluster descriptions are stored
        constinit static inline ep_front_t value{
            {.ep_data{epa, cluster_list, meta_ctr_param_t<^^clusters>{}}}
        };
    };
}

#endif
