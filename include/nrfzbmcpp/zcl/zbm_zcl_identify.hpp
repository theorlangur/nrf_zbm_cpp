#ifndef ZBM_ZCL_IDENTIFY_HPP_
#define ZBM_ZCL_IDENTIFY_HPP_

#include "../zbm.hpp"

namespace zbm
{
    namespace zcl
    {
        struct 
            [[=cluster_a{.id = ZB_ZCL_CLUSTER_ID_IDENTIFY, .pre_init = ZB_ZCL_CLUSTER_ID_IDENTIFY_SERVER_ROLE_INIT}]]
            identify_t
        {
            [[=attribute_a{.id = ZB_ZCL_ATTR_IDENTIFY_IDENTIFY_TIME_ID, .a = access_t::RW}]]
            zb_uint16_t identify_time{};
        };

        struct 
            [[=cluster_a{.id = ZB_ZCL_CLUSTER_ID_IDENTIFY, .role=role_t::Client, .pre_init = ZB_ZCL_CLUSTER_ID_IDENTIFY_CLIENT_ROLE_INIT}]]
            identify_client_t
        {
            enum effect_id: uint8_t
            {
                Blink = 0x00, Breathe = 0x01, Okay=0x02, ChannelChange=0x0b, FinishEffect=0xfe, StopEffect=0xff
            };
            enum effect_variant: uint8_t
            {
                Default = 0x00
            };

            [[=cmd_out_a{{.id = ZB_ZCL_CMD_IDENTIFY_IDENTIFY_ID}}]]
            [[no_unique_address]]cmd_out_t<void(uint16_t)> identify;

            [[=cmd_out_a{{.id = ZB_ZCL_CMD_IDENTIFY_IDENTIFY_QUERY_ID}}]]
            [[no_unique_address]]cmd_out_t<void()> indentify_query;

            [[=cmd_out_a{{.id = ZB_ZCL_CMD_IDENTIFY_TRIGGER_EFFECT_ID}}]]
            [[no_unique_address]]cmd_out_t<void(effect_id, effect_variant)> trigger_effect;
        };
    }
}
#endif
