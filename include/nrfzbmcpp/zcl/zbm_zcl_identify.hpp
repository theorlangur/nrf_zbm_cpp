#ifndef ZBM_ZCL_IDENTIFY_HPP_
#define ZBM_ZCL_IDENTIFY_HPP_

#include "../zbm.hpp"

namespace zbm
{
    namespace zcl
    {
        struct identify_common_t
        {
            enum effect_id: uint8_t
            {
                Blink = 0x00, Breathe = 0x01, Okay=0x02, ChannelChange=0x0b, FinishEffect=0xfe, StopEffect=0xff
            };
            enum effect_variant: uint8_t
            {
                Default = 0x00
            };
        };

        struct 
            [[=cluster_a{.id = ZB_ZCL_CLUSTER_ID_IDENTIFY}]]
            identify_t: identify_common_t
        {
            //hook on write to start/stop identify process
            [[=attribute_a{.id = ZB_ZCL_ATTR_IDENTIFY_IDENTIFY_TIME_ID, .a = access_t::RW}]]
            zb_uint16_t identify_time{};

            [[=cmd_out_a{{ZB_ZCL_CMD_IDENTIFY_IDENTIFY_QUERY_RSP_ID}}]]
            [[no_unique_address]]cmd_out_t<void(uint16_t timeout)> identify_resp;

            [[=cmd_in_a{ZB_ZCL_CMD_IDENTIFY_IDENTIFY_ID}]]
            cmd_handling_result_t(*on_identify)(uint16_t ident_time) = {};

            [[=cmd_in_a{ZB_ZCL_CMD_IDENTIFY_IDENTIFY_QUERY_ID}]]
            cmd_handling_result_t(*on_identify_query)() = {};

            [[=cmd_in_a{ZB_ZCL_CMD_IDENTIFY_TRIGGER_EFFECT_ID}]]
            cmd_handling_result_t(*on_trigger_effect)(effect_id, effect_variant) = {};
        };

        struct 
            [[=cluster_a{.id = ZB_ZCL_CLUSTER_ID_IDENTIFY, .role=role_t::Client}]]
            identify_client_t: identify_common_t
        {
            [[=cmd_in_a{ZB_ZCL_CMD_IDENTIFY_IDENTIFY_QUERY_RSP_ID}]]
            cmd_handling_result_t(*on_identify_resp)(uint16_t timeout) = {};

            [[=cmd_out_a{{.id = ZB_ZCL_CMD_IDENTIFY_IDENTIFY_ID}}]]
            [[no_unique_address]]cmd_out_t<void(uint16_t ident_time)> identify;

            [[=cmd_out_a{{.id = ZB_ZCL_CMD_IDENTIFY_IDENTIFY_QUERY_ID}}]]
            [[no_unique_address]]cmd_out_t<void()> indentify_query;

            [[=cmd_out_a{{.id = ZB_ZCL_CMD_IDENTIFY_TRIGGER_EFFECT_ID}}]]
            [[no_unique_address]]cmd_out_t<void(effect_id, effect_variant)> trigger_effect;
        };
    }
}
#endif
