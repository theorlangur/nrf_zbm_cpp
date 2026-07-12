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

            [[=cmd_in_a{ZB_ZCL_CMD_ON_OFF_OFF_ID}]]
            cmd_handling_result_t(*on_cmd_off)();

            [[=cmd_in_a{ZB_ZCL_CMD_ON_OFF_ON_ID}]]
            cmd_handling_result_t(*on_cmd_on)();

            [[=cmd_in_a{ZB_ZCL_CMD_ON_OFF_TOGGLE_ID}]]
            cmd_handling_result_t(*on_cmd_toggle)();
        };

        struct on_off_server_ext_t: on_off_server_t
        {
            [[=cmd_in_a{ZB_ZCL_CMD_ON_OFF_OFF_WITH_EFFECT_ID}]]
            cmd_handling_result_t(*on_cmd_off_with_effect)(uint8_t effect_id, uint8_t effect_variant);

            [[=cmd_in_a{ZB_ZCL_CMD_ON_OFF_ON_WITH_RECALL_GLOBAL_SCENE_ID}]]
            cmd_handling_result_t(*on_cmd_on_with_recall_global_scene)();

            [[=cmd_in_a{ZB_ZCL_CMD_ON_OFF_ON_WITH_TIMED_OFF_ID}]]
            cmd_handling_result_t(*on_cmd_on_with_timed_off)(uint8_t on_off_ctrl, uint16_t on_time, uint16_t off_wait_time);
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
