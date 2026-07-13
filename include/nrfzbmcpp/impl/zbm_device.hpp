#ifndef ZB_META_DEVICE_HPP_
#define ZB_META_DEVICE_HPP_

#include "zbm_ep.hpp"

namespace zbm
{
    template<std::meta::info dev_data_ref>
    struct device_factory_t
    {
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
            template for(constexpr auto ei : define_static_array(extract_ep_from_device(dev_data_ref)))
            {
                constexpr auto nsdm_ep_mem = std::meta::reflect_object([:dev_data_ref:].[:ei.ep:]);
                constexpr auto ep_create_inst_t = std::meta::substitute(^^ep_create_t, {std::meta::reflect_constant(ei.ep), std::meta::reflect_constant(nsdm_ep_mem)});
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
        using base_t = device_factory_t<dev_data_ref>::device_t;
        static constexpr auto ep_list = get_all_ep_ref<dev_data_ref>();
        constexpr device_full_t():
            device_full_t(std::make_index_sequence<ep_list.size()>{})
        {
        }

        zb_af_device_ctx_t* device_context() { return &ctx; }

        template<uint8_t EP>
        static consteval auto ep_field()
        {
            return refl::find_member_by_name(^^base_t, refl::name_with_hex<2>("ep_", EP));
        }

        template<uint8_t EP>
        constexpr auto& ep()
        {
            constexpr auto ep_field = refl::find_member_by_name(^^base_t, refl::name_with_hex<2>("ep_", EP));
            return this->[:ep_field:];
        }
    protected:
        template<size_t... I>
        constexpr device_full_t(std::index_sequence<I...>):
            base_t{
                ep_create_t<ep_list[I].mem_decl, ep_list[I].obj_ref>::value...
            },
            endpoints{
                &ep_create_t<ep_list[I].mem_decl, ep_list[I].obj_ref>::value.ep_data.ep...
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
