#ifndef ZBM_ZCL_ON_OFF_HPP_
#define ZBM_ZCL_ON_OFF_HPP_

#include "../zbm.hpp"

namespace zbm
{
    namespace zcl
    {
        struct 
            [[=cluster_a{.id = ZB_ZCL_CLUSTER_ID_ON_OFF, .pre_init = ZB_ZCL_CLUSTER_ID_ON_OFF_SERVER_ROLE_INIT}]]
            on_off_server_t
        {
            [[=attribute_a{.id = ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID, .a = access_t::RPS, .type=type_t::Bool}]]
            zb_uint8_t on_off;
        };

        struct 
            [[=cluster_a{.id = ZB_ZCL_CLUSTER_ID_ON_OFF, .role = role_t::Client, .pre_init = ZB_ZCL_CLUSTER_ID_ON_OFF_CLIENT_ROLE_INIT}]]
            on_off_client_t
        {
            [[=cmd_out_a{{.id = ZB_ZCL_CMD_ON_OFF_OFF_ID}}]]
            [[no_unique_address]]cmd_out_t<void()> off;
            [[=cmd_out_a{{.id = ZB_ZCL_CMD_ON_OFF_ON_ID}}]]
            [[no_unique_address]]cmd_out_t<void()> on;
            [[=cmd_out_a{{.id = ZB_ZCL_CMD_ON_OFF_ON_WITH_TIMED_OFF_ID}}]]
            [[no_unique_address]]cmd_out_t<void(uint8_t, uint16_t, uint16_t)> on_with_timed_off;
        };
    }
}

#endif
