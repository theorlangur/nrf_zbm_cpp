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

    struct cb_generic_handler_t
    {
        static constexpr uint8_t kEP_ANY = 0xff;
        static constexpr uint16_t kCLUSTER_ANY = 0xffff;
        static constexpr uint16_t kATTRIBUTE_ANY = 0xffff;
        using cb_t = void(*)(zb_zcl_device_callback_param_t*);

        uint8_t ep = kEP_ANY;
        uint16_t cluster = kCLUSTER_ANY;
        uint16_t attribute = kATTRIBUTE_ANY;
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

        template<auto handlers>
        void generic_cb_handler(zb_zcl_device_callback_param_t*)
        {
        }

        template<cb_id_range range, auto handlers>
        consteval auto organize_cb_into_lookup()
        {
            constexpr size_t table_size = range.max - range.min + 1;
            std::array<cb_generic_handler_t::cb_t, table_size> mid_storage;

            template for(constexpr auto I : std::ranges::views::iota(unsigned(0), range.max - range.min + 1))
            {
                static constexpr auto filtered_handlers = std::define_static_array(filter_handlers_by_cb_id(I + range.min, std::span{handlers.begin(), handlers.end()}));
                static constexpr auto _f_a = span_to_array<filtered_handlers.size()>(filtered_handlers);
                mid_storage[I] = &generic_cb_handler<_f_a>;
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
