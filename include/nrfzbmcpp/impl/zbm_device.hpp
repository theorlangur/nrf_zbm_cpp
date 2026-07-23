#ifndef ZB_META_DEVICE_HPP_
#define ZB_META_DEVICE_HPP_

#include "zbm_ep.hpp"

namespace zbm
{
    consteval std::vector<cluster_global_info_t> analyze_global_cluster_info(auto ep_info_list)
    {
        std::vector<cluster_global_info_t> res;
        auto find_entry = [&](uint16_t cluster_id, role_t role)
        {
            for(auto i = res.begin(); i != res.end(); ++i)
            {
                if (i->id == cluster_id && i->role == role)
                    return i;
            }
            return res.end();
        };
        for(auto ei : ep_info_list)
        {
            std::meta::info ep_type_refl = std::meta::type_of(ei.ep);
            auto cluster_list = extract_clusters_from_ep(ep_type_refl);
            for(auto ci : cluster_list)
            {
                auto in_cmds = extract_incoming_commands_from_cluster(ci.cluster);
                bool needs_add_handler = false;
                if (!in_cmds.empty())
                    needs_add_handler = true;
                if (!needs_add_handler)
                {
                    auto attrs = extract_attributes_from_cluster(ci.cluster);
                    for(auto ai : attrs)
                    {
                        if ((ai.annotation.a & access_t::Write) && ai.annotation.validator)
                        {
                            needs_add_handler = true;
                            break;
                        }
                    }
                }

                if (needs_add_handler)
                {
                    auto cluster_global_info_entry = find_entry(ci.annotation.id, ci.annotation.role);
                    if (cluster_global_info_entry == res.end())
                        res.emplace_back(ci.annotation.id, ci.annotation.role, 0);
                    else
                        ++cluster_global_info_entry->addHandlerDepth;
                }
            }
        }
        auto part_it = std::partition(res.begin(), res.end(), [](auto &i){ return i.addHandlerDepth > 0; });
        res.resize(std::distance(res.begin(), part_it));
        return res;
    }

    template<size_t N>
    consteval cluster_global_info_list_t<N> get_global_cluster_info_as_array(auto ep_info_list)
    {
        cluster_global_info_list_t<N> res;
        auto global_info_list = analyze_global_cluster_info(ep_info_list);
        for(size_t i = 0; i < global_info_list.size(); ++i)
            res[i] = global_info_list[i];
        return res;
    }

    template<std::meta::info dev_data_ref>
    struct device_factory_t
    {
        static constexpr auto ep_info_list = define_static_array(extract_ep_from_device(dev_data_ref));
        static constexpr auto global_info_list = define_static_array(analyze_global_cluster_info(ep_info_list));
        static constexpr auto global_info_list_array = get_global_cluster_info_as_array<global_info_list.size()>(ep_info_list); 

        /*
         * struct device_t
         * {
             ep_create_t<>::ep_type_t &ep_01;//1 EP ID
             ep_create_t<>::ep_type_t &ep_02;//2 EP ID
         * };
         * */
        struct device_t;
        consteval{
            std::vector<std::meta::info> mems;
            template for(constexpr auto ei : ep_info_list)
            {
                constexpr auto nsdm_ep_mem = std::meta::reflect_object([:dev_data_ref:].[:ei.ep:]);
                constexpr auto ep_create_inst_t = std::meta::substitute(^^ep_create_t, {std::meta::reflect_constant(ei.ep), std::meta::reflect_constant(nsdm_ep_mem), std::meta::reflect_constant(global_info_list_array)});
                using ep_end_type_t = typename [:ep_create_inst_t:]::ep_front_t;
                constexpr auto ep_ref = std::meta::add_lvalue_reference(^^ep_end_type_t);
                mems.push_back(std::meta::data_member_spec(ep_ref, std::meta::data_member_options{.name = refl::name_with_hex<2>("ep_", ei.annotation.ep)}));
            }
            std::meta::define_aggregate(^^device_t, mems);
        };
    };

    struct ep_refl_info
    {
        std::meta::info mem_decl;
        std::meta::info obj_ref;
    };

    template<std::meta::info dev_data_ref>
    consteval auto get_all_ep_ref()
    {
        std::vector<ep_refl_info> refs;
        template for(constexpr auto ei : define_static_array(extract_ep_from_device(dev_data_ref)))
            refs.emplace_back(ei.ep, std::meta::reflect_object([:dev_data_ref:].[:ei.ep:]));
        return define_static_array(refs);
    }

    template<std::meta::info dev_data_ref>
    struct device_full_t: device_factory_t<dev_data_ref>::device_t
    {
        using factory_t = device_factory_t<dev_data_ref>;
        using base_t = factory_t::device_t;
        static constexpr auto ep_list = get_all_ep_ref<dev_data_ref>();
        constexpr device_full_t():
            device_full_t(std::make_index_sequence<ep_list.size()>{})
        {
        }

        zb_af_device_ctx_t* device_context() { return &ctx; }

        void init()
        {
            template for(constexpr size_t i : std::ranges::views::indices(ep_list.size()))
            {
                constexpr auto ep_a = try_get_ep_annotation(ep_list[i].mem_decl);
                if constexpr (ep_a)
                    ep_create_t<ep_list[i].mem_decl, ep_list[i].obj_ref, factory_t::global_info_list_array>::value.init();
            }
        }

        template<uint8_t EP>
        constexpr auto& ep()
        {
            template for(constexpr size_t i : std::ranges::views::indices(ep_list.size()))
            {
                constexpr auto ep_a = try_get_ep_annotation(ep_list[i].mem_decl);
                if constexpr (ep_a)
                {
                    if constexpr (ep_a->ep == EP)
                        return ep_create_t<ep_list[i].mem_decl, ep_list[i].obj_ref, factory_t::global_info_list_array>::value;
                }
            }
        }
    protected:
        template<size_t... I>
        constexpr device_full_t(std::index_sequence<I...>):
            base_t{
                ep_create_t<ep_list[I].mem_decl, ep_list[I].obj_ref, factory_t::global_info_list_array>::value...
            },
            endpoints{
                &ep_create_t<ep_list[I].mem_decl, ep_list[I].obj_ref, factory_t::global_info_list_array>::value.ep_data.ep...
            },
            ctx{
                .ep_count = ep_list.size(),
                .ep_desc_list = endpoints
            }
        {}

        zb_af_endpoint_desc_t *endpoints[ep_list.size()];
        zb_af_device_ctx_t ctx;
    };
}

#endif
