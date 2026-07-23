#ifndef ZB_META_CLUSTER_HPP_
#define ZB_META_CLUSTER_HPP_

#include "zbm_types.hpp"
#include "zbm_annotations.hpp"

namespace zbm
{
    struct additional_cluster_handlers_base_t
    {
        uint8_t ep=0xff;
    };

    constexpr static  uint8_t kNeedsValueChecker = 0x01;
    constexpr static  uint8_t kNeedsCmdHandler = 0x02;
    constexpr static  uint8_t kNeedsBoth = kNeedsValueChecker | kNeedsCmdHandler;

    template<uint8_t kNeeds>
    struct additional_cluster_handlers_t
    {
        static_assert((kNeeds & kNeedsBoth) && !(kNeeds & ~kNeedsBoth), "Invalid configuration of the additional cluster handlers");
    };

    template<>
    struct additional_cluster_handlers_t<kNeedsBoth>: additional_cluster_handlers_base_t
    {
        zb_zcl_cluster_check_value_t checker;
        zb_zcl_cluster_handler_t cmd_handler;
    };

    template<>
    struct additional_cluster_handlers_t<kNeedsValueChecker>: additional_cluster_handlers_base_t
    {
        zb_zcl_cluster_check_value_t checker;
    };

    template<>
    struct additional_cluster_handlers_t<kNeedsCmdHandler>: additional_cluster_handlers_base_t
    {
        zb_zcl_cluster_handler_t cmd_handler;
    };

    template<uint16_t clusterId, uint8_t kNeeds, size_t kMaxEntries>
    struct reserved_array_additional_cluster_handlers_t
    {
        using storage_t = additional_cluster_handlers_t<kNeeds>;
        storage_t slots[kMaxEntries] = {};

        storage_t* add()
        {
            for(auto &s : slots)
                if (s.ep == 0xff)
                    return &s;
            return nullptr;
        }

        storage_t* find(uint8_t ep)
        {
            for(auto &s : slots)
            {
                if (s.ep == ep)
                    return &s;
                else if (s.ep == 0xff)
                    return nullptr;
            }
            return nullptr;
        }
    };

    template<uint16_t clusterId, uint8_t kNeeds, size_t N>
    constinit static inline reserved_array_additional_cluster_handlers_t<clusterId, kNeeds, N> g_AdditionalClusterHandlers;

    using global_error_handler_t = void(*)(zb_ret_t r);
    constinit inline global_error_handler_t g_GlobalErrorHandler = nullptr;

    template<std::meta::info cluster_ref/*reflection of the cluster field reference (reference to an actual object)*/>
    struct cluster_t
    {
        using cluster_data_type_t = [: []() consteval{ return std::meta::remove_cvref(std::meta::type_of(cluster_ref)); }() :];
        static constexpr cluster_a g_ClusterA = *get_cluster_annotation(^^cluster_data_type_t);
        static constexpr auto cluster_data_ref_refl = cluster_ref;/*reflection of the cluster field reference (reference to an actual object)*/
        static constexpr auto attributes_info = std::define_static_array(extract_attributes_from_cluster(cluster_ref));
        static constexpr size_t N = attributes_info.size();
        static constexpr auto cmd_in_info = std::define_static_array(extract_incoming_commands_from_cluster(cluster_ref));
        static constexpr size_t N_cmd_in = cmd_in_info.size();
        static constexpr auto cmd_out_info = std::define_static_array(extract_sending_commands_from_cluster(cluster_ref));
        static constexpr size_t N_cmd_out = cmd_out_info.size();


        consteval static  size_t attributes_want_check()
        {
            size_t count = 0;
            for(auto ai : attributes_info)
                if ((ai.annotation.a & access_t::Write) && ai.annotation.validator)
                    ++count;
            return count;
        }

        consteval static uint8_t get_handling_needs()
        {
            return (N_cmd_in >= 0) * kNeedsCmdHandler + (attributes_want_check() > 0) * kNeedsValueChecker;
        }

        consteval static bool has_attribute(std::meta::info user_attr_mem)
        {
            for(auto a : attributes_info)
                if (a.attribute == user_attr_mem)
                    return true;
            return false;
        }

        consteval cluster_t():
            attributes{
                {
                    .id = ZB_ZCL_ATTR_GLOBAL_CLUSTER_REVISION_ID, 
                    .type = (zb_uint8_t)type_t::Invalid, 
                    .access = (zb_uint8_t)access_t::Read, 
                    .manuf_code = ZB_ZCL_NON_MANUFACTURER_SPECIFIC, 
                    .data_p = &rev
                }
            },
            rev(g_ClusterA.rev)
            {

                /**********************************************************************/
                /* attributes                                                         */
                /**********************************************************************/
                size_t i = 1;
                template for(constexpr auto a : attributes_info)
                {
                    attributes[i++] = zb_zcl_attr_t{
                        .id = a.annotation.id, 
                            .type = (zb_uint8_t)a.annotation.type, 
                            .access = (zb_uint8_t)a.annotation.a, 
                            .manuf_code = ZB_ZCL_NON_MANUFACTURER_SPECIFIC, 
                            .data_p = &[:cluster_ref:].[:a.attribute:]
                    };
                }
                attributes[N + 1] = g_LastAttribute;

                /**********************************************************************/
                /* incoming commands                                                  */
                /**********************************************************************/
                i = 0;
                template for(constexpr auto a : cmd_in_info)
                    received_commands[i++] = a.annotation.id; 

                /**********************************************************************/
                /* outgoing commands                                                  */
                /**********************************************************************/
                i = 0;
                template for(constexpr auto a : cmd_out_info)
                    generated_commands[i++] = a.annotation.id; 
            }

        alignas(4) zb_zcl_attr_t attributes[N + 2];
        zb_uint16_t rev;

        zb_uint8_t received_commands[N_cmd_in];
        zb_uint8_t generated_commands[N_cmd_out];

        zb_discover_cmd_list_t cmd_list =
        {
          zb_uint8_t(N_cmd_in), received_commands,
          zb_uint8_t(N_cmd_out), generated_commands
        };
    };

    template<std::meta::info ep_ref>
    struct cluster_list_factory_t
    {
        /*
         * struct cluster_list_t
         * {
         *   cluster_t cluster_server_abcd;//0xabcd - cluster ID
         *   cluster_t cluster_client_0012;//0x0012 - cluster ID
         *   ...
         *   cluster_t cluster_8321;
         * };
         * */
        struct cluster_list_t;
        consteval{
            std::vector<std::meta::info> mems;
            std::string server_prefix = "cluster_server_";
            std::string client_prefix = "cluster_client_";
            template for(constexpr auto ci : define_static_array(extract_clusters_from_ep(ep_ref)))
            {
                static_assert(verify_member_in_cluster<zbm::attribute_a>(std::meta::type_of(ci.cluster)) == std::meta::info{}, "Found duplicate attribute in cluster definition");
                static_assert(verify_member_in_cluster<zbm::cmd_in_a>(std::meta::type_of(ci.cluster)) == std::meta::info{}, "Found duplicate incomming command in cluster definition");
                static_assert(verify_member_in_cluster<zbm::cmd_out_a>(std::meta::type_of(ci.cluster)) == std::meta::info{}, "Found duplicate outgoing command in cluster definition");
                auto c_ref = std::meta::substitute(^^cluster_t, {std::meta::reflect_constant(std::meta::reflect_object([:ep_ref:].[:ci.cluster:]))});
                const auto &field_prefix = ci.annotation.role == role_t::Server ? server_prefix : client_prefix;
                mems.push_back(std::meta::data_member_spec(
                                c_ref, 
                                std::meta::data_member_options{.name = refl::name_with_hex<4>(field_prefix, ci.annotation.id)}
                            )
                        );
            }
            std::meta::define_aggregate(^^cluster_list_t, mems);
        };
    };

    struct cmd_to_arg_t
    {
        const uint8_t *pData;
        size_t dataLeft;
        bool error = false;
        template<class A> requires ((alignof(A) == 1) && !serializable_c<A>)
        const A& extract()
        {
            const A *p = (const A*)pData;
            pData += sizeof(A);
            if (dataLeft < sizeof(A))
            {
                error = true;
                dataLeft = 0;
            }
            else
                dataLeft -= sizeof(A);
            return *p;
        }

        template<class A> requires ((alignof(A) > 1) && !serializable_c<A>)
        A extract()
        {
            static_assert(sizeof(A) <= 4, "type_t is too big");
            const A *p = (const A*)pData;
            if (dataLeft < sizeof(A))
            {
                error = true;
                dataLeft = 0;
                return {};
            }
            else
                dataLeft -= sizeof(A);
            pData += sizeof(A);
            A ret;
            memcpy(&ret, p, sizeof(A));
            return ret;
        }

        template<serializable_c A>
        A extract()
        {
            A ret;
            if (auto res = ret.serialize_from(pData, dataLeft); !res)
            {
                error = true;
                dataLeft = 0;
                return {};
            }else
            {
                size_t sz = *res - pData;
                dataLeft -= sz;
                pData = *res;
            }
            return ret;
        }
    };

    struct cmd_handling_result_t
    {
        zb_ret_t status = RET_OK;
        bool processed = true;
    };

    template<std::meta::info cmd_t_refl>
    consteval std::optional<std::meta::info> get_fitting_call_operator_overload()
    {
        for(auto m : std::meta::members_of(cmd_t_refl, std::meta::access_context::current()))
        {
            auto t = std::meta::type_of(m);
            if (std::meta::is_function_type(t))
            {
                if (std::meta::return_type_of(t) == ^^cmd_handling_result_t)
                    return t;
            }
        }
        return std::nullopt;
    }

    struct cmd_handler_info_t
    {
        std::meta::info functionType;
    };
    template<std::meta::info cmd_t_refl>
    consteval cmd_handler_info_t analyze_cmd_handler()
    {
        if constexpr (std::meta::is_pointer_type(cmd_t_refl))
        {
            constexpr auto no_ptr = std::meta::remove_pointer(cmd_t_refl);
            static_assert(std::meta::is_function_type(no_ptr), "A handler of pointer type shall only be a pointer to a function");
            static_assert(std::meta::return_type_of(no_ptr) == ^^cmd_handling_result_t, "Command handling must return cmd_handling_result_t");
            return {no_ptr};
        }else
        {
            static_assert(std::meta::is_object_type(cmd_t_refl), "If not a function pointer, than it shall be a functor");
            constexpr auto func_call_t_refl = get_fitting_call_operator_overload<cmd_t_refl>();
            static_assert(func_call_t_refl, "Could not find fitting operator() overload");
            return {*func_call_t_refl};
        }
    }

    template<std::meta::info cluster_data_ref, cmd_in_with_annotation ci>
    cmd_handling_result_t call_cmd(zb_zcl_parsed_hdr_t *cmd_info, std::span<uint8_t> raw_data)
    {
        constexpr auto cmd_t_refl = std::meta::type_of(ci.cmd);//type of cmd how it's defined in the user data structure being reflected
        if constexpr (std::convertible_to<typename [:cmd_t_refl:], bool>)//can it be checked for validity?
            if (!([:cluster_data_ref:].[:ci.cmd:])) return {RET_OK, false};//if so - we shall check it

        constexpr cmd_handler_info_t handler_info = analyze_cmd_handler<cmd_t_refl>();
        static constexpr auto params = std::define_static_array(std::meta::parameters_of(handler_info.functionType));

        return [&]<size_t... I>(std::index_sequence<I...>)//need to generate actual arguments
        {
            cmd_to_arg_t args(raw_data.data(), raw_data.size(), false);
            //additional layer of calling is needed to be able to check for error during extracting arguments in 'args'
            return [&]<class... Args>(Args&&... extracted_args)->cmd_handling_result_t
            {
                if (args.error)
                    return {RET_ILLEGAL_REQUEST, true};
                else
                    return ([:cluster_data_ref:].[:ci.cmd:])(std::forward<Args>(extracted_args)...);
            }(args.extract<typename [:params[I]:]>()...);
        }(std::make_index_sequence<params.size()>());
    }

    template<std::meta::info cluster_ref, uint8_t ep, uint8_t addHandlingDepth>
    inline zb_bool_t on_cluster_cmd_handling(zb_uint8_t param)
    {
        using cluster_desc_t = [:std::meta::remove_cvref(std::meta::type_of(cluster_ref)):];
        constexpr auto i = cluster_desc_t::g_ClusterA;

        if ( ZB_ZCL_GENERAL_GET_CMD_LISTS_PARAM == param )
        {
            ZCL_CTX().zb_zcl_cluster_cmd_list = &[:cluster_ref:].cmd_list;
            return ZB_TRUE;
        }

        zb_zcl_parsed_hdr_t *cmd_info = ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t);
        cmd_handling_result_t r;
        if (cmd_info->addr_data.common_data.dst_endpoint != ep)
        {
            ZB_ASSERT(i.id == cmd_info->cluster_id);
            if constexpr (addHandlingDepth > 0)
            {
                auto *pSlot = g_AdditionalClusterHandlers<i.id, cluster_desc_t::get_handling_needs(), addHandlingDepth>.find(cmd_info->addr_data.common_data.dst_endpoint);
                if (!pSlot)
                {
                    r.status = RET_NOT_FOUND;
                    r.processed = true;
                }else
                    return pSlot->cmd_handler(param);
            }else
            {
                //something is terribly wrong
                ZB_ASSERT(addHandlingDepth > 0);
                r.status = RET_NOT_FOUND;
                r.processed = true;
            }
        }else
        {
            using cluster_t = [:std::meta::remove_cvref(std::meta::type_of(cluster_ref)):];
            template for (constexpr auto cmdInfo : cluster_t::cmd_in_info)
            {
                if (cmdInfo.annotation.id == cmd_info->cmd_id)
                {
                    //call that command
                    r = call_cmd<cluster_t::cluster_data_ref_refl, cmdInfo>(cmd_info, std::span<uint8_t>{(uint8_t*)zb_buf_begin(param), zb_buf_len(param)});
                    break;
                }
            }
        }

        auto const& [status, processed] = r;

        if(processed)
        {
            if( cmd_info->disable_default_response && status == RET_OK)
            {
                zb_buf_free(param);
            }
            else if (status == RET_NOT_IMPLEMENTED)
            {
                ZB_ZCL_PROCESS_COMMAND_FINISH(param, cmd_info, ZB_ZCL_STATUS_UNSUP_CMD);
            }
            else if (status != RET_BUSY)
            {
                ZB_ZCL_PROCESS_COMMAND_FINISH(param, cmd_info, status==RET_OK ? ZB_ZCL_STATUS_SUCCESS : ZB_ZCL_STATUS_INVALID_FIELD);
            }
        }

        return processed;
    }

    template<std::meta::info cluster_ref, uint8_t ep, uint8_t addHandlingDepth>
    inline zb_ret_t on_cluster_check_value(zb_uint16_t attr_id, zb_uint8_t endpoint, zb_uint8_t *value)
    {
        using cluster_desc_t = [:std::meta::remove_cvref(std::meta::type_of(cluster_ref)):];
        constexpr auto i = cluster_desc_t::g_ClusterA;
        if (ep != endpoint)
        {
            if constexpr (addHandlingDepth > 0)
            {
                auto *pSlot = g_AdditionalClusterHandlers<i.id, cluster_desc_t::get_handling_needs(), addHandlingDepth>.find(endpoint);
                if (!pSlot)
                    return RET_BUSY;
                else
                    return pSlot->checker(attr_id, endpoint, value);
            }else
            {
                ZB_ASSERT(false);
                return RET_BUSY;
            }
        }

        template for (constexpr auto ai : cluster_desc_t::attributes_info)
        {
            if constexpr ((ai.annotation.a & access_t::Write) && ai.annotation.validator)
            {
                if (ai.annotation.id == attr_id)
                {
                    //weird that I had to spell it out by declaring a local variable...
                    auto validator = ai.annotation.validator;
                    return validator(value);
                }
            }
        }
        return RET_OK;
    }

    template<std::meta::info cluster_r, uint8_t ep, uint8_t addHandlingDepth>
    void generic_cluster_init()
    {
        using cluster_desc_t = [:std::meta::remove_cvref(std::meta::type_of(cluster_r)):];
        constexpr auto i = cluster_desc_t::g_ClusterA;
        if constexpr (i.pre_init)
            i.pre_init();

        zb_zcl_cluster_check_value_t check_val = nullptr;
        zb_zcl_cluster_write_attr_hook_t write_hook = nullptr;
        zb_zcl_cluster_handler_t cmd_handler = nullptr;
        if constexpr (cluster_desc_t::N_cmd_in >= 0)
            cmd_handler = &on_cluster_cmd_handling<cluster_r, ep, addHandlingDepth>;

        constexpr size_t attributes_want_check = cluster_desc_t::attributes_want_check();

        if constexpr (attributes_want_check > 0)
            check_val = &on_cluster_check_value<cluster_r, ep, addHandlingDepth>;

        if (check_val || write_hook || cmd_handler)
        {
            zb_ret_t ret = zb_zcl_add_cluster_handlers(i.id, (uint8_t)i.role
                    , check_val /*cluster_check_value*/
                    , write_hook /*cluster_write_attr_hook*/
                    , cmd_handler /*cluster_handler*/
                    );
            if (ret == RET_ALREADY_EXISTS)
            {
                if constexpr (addHandlingDepth > 0)
                {
                    auto *pSlot = g_AdditionalClusterHandlers<i.id, cluster_desc_t::get_handling_needs(), addHandlingDepth>.add();
                    if (pSlot)
                    {
                        pSlot->ep = ep;
                        pSlot->checker = check_val;
                        pSlot->cmd_handler = cmd_handler;
                    }
                    else if (g_GlobalErrorHandler)
                    {
                        g_GlobalErrorHandler(RET_NO_MEMORY);
                    }
                }else
                {
                    if (g_GlobalErrorHandler)
                    {
                        g_GlobalErrorHandler(RET_NO_MEMORY);
                    }
                }
            }
        }
    }
}

#endif
