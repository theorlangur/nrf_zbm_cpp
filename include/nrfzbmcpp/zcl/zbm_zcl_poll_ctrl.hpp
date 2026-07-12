#ifndef ZBM_ZCL_POLL_CTRL_HPP_
#define ZBM_ZCL_POLL_CTRL_HPP_

#include "../zbm.hpp"

namespace zbm
{
    namespace zcl
    {
        namespace literals
        {
            constexpr uint32_t operator""_sec_to_qs(unsigned long long v) { return v * 4; }
            constexpr uint32_t operator""_min_to_qs(unsigned long long v) { return v * 4 * 60; }
            constexpr uint32_t operator""_h_to_qs(unsigned long long v) { return v * 4 * 60 * 60; }
        }

        constexpr uint32_t qs_to_s(uint32_t v) { return v / 4; }

        using namespace literals;
        static constexpr uint16_t kZB_ZCL_CLUSTER_ID_POLL_CTRL = 0x0020;
        struct 
            [[=cluster_a{.id = kZB_ZCL_CLUSTER_ID_POLL_CTRL}]]
            poll_ctrl_basic_t
        {
            [[=attribute_a{.id = 0, .a = access_t::RW}]]
            uint32_t check_in_interval = 1_h_to_qs;//1h
                                                   //
            [[=attribute_a{.id = 1}]]
            uint32_t long_poll_interval = 5_sec_to_qs;//5sec

            [[=attribute_a{.id = 2}]]
            uint16_t short_poll_interval = 2;//2quater-sec

            [[=attribute_a{.id = 3, .a = access_t::RW}]]
            uint16_t fast_poll_timeout = 10_sec_to_qs;//10sec

            [[=attribute_a{.id = 0xeffeU, .a = access_t::Internal, .type=type_t::Null}]]
            zb_zcl_poll_control_srv_cfg_data_t srv_cfg = 
            { ZB_ZCL_POLL_CTRL_INVALID_ADDR, ZB_ZCL_POLL_INVALID_EP, 0, 0 };
        };

        struct poll_ctrl_t: poll_ctrl_basic_t
        {
            [[=attribute_a{.id = 4}]]
            uint32_t check_in_interval_min = 0;

            [[=attribute_a{.id = 5}]]
            uint32_t long_poll_interval_min = 0;

            [[=attribute_a{.id = 6}]]
            uint16_t fast_poll_timeout_max = 0;
        };
    }
}
#endif
