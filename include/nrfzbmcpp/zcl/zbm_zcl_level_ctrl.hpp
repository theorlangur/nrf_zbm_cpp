#ifndef ZBM_ZCL_LEVEL_CTRL_HPP_
#define ZBM_ZCL_LEVEL_CTRL_HPP_

#include "../zbm.hpp"

namespace zbm
{
    namespace zcl
    {
        struct 
            [[=cluster_a{.id = ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL, .pre_init = ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL_SERVER_ROLE_INIT}]]
            level_ctrl_t
        {
            [[=attribute_a{.id = ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID, .a = access_t::RPS}]]
            zb_uint8_t  current_level{};

            [[=attribute_a{.id = ZB_ZCL_ATTR_LEVEL_CONTROL_REMAINING_TIME_ID}]]
            zb_uint16_t remaining_time{};
        };

        struct 
            [[=cluster_a{.id = ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL, .role = role_t::Client, .pre_init = ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL_CLIENT_ROLE_INIT}]]
            level_ctrl_client_t
        {
            enum fade_mode_e: uint8_t{ Up = 0, Down = 1};
            [[=cmd_out_a{{.id = ZB_ZCL_CMD_LEVEL_CONTROL_MOVE_TO_LEVEL}}]]
            [[no_unique_address]]cmd_out_t<void(uint8_t, uint16_t, uint8_t, uint8_t)> move_to_level;

            [[=cmd_out_a{{.id = ZB_ZCL_CMD_LEVEL_CONTROL_MOVE}}]]
            [[no_unique_address]]cmd_out_t<void(fade_mode_e, uint8_t, uint8_t, uint8_t)> move;

            [[=cmd_out_a{{.id = ZB_ZCL_CMD_LEVEL_CONTROL_STEP}}]]
            [[no_unique_address]]cmd_out_t<void(fade_mode_e, uint8_t, uint16_t, uint8_t, uint8_t)> step;

            [[=cmd_out_a{{.id = ZB_ZCL_CMD_LEVEL_CONTROL_STOP}}]]
            [[no_unique_address]]cmd_out_t<void(uint8_t, uint8_t)> stop;

            [[=cmd_out_a{{.id = ZB_ZCL_CMD_LEVEL_CONTROL_MOVE_TO_LEVEL_WITH_ON_OFF}}]]
            [[no_unique_address]]cmd_out_t<void(uint8_t, uint16_t, uint8_t, uint8_t)> move_to_level_with_on_off;

            [[=cmd_out_a{{.id = ZB_ZCL_CMD_LEVEL_CONTROL_MOVE_WITH_ON_OFF}}]]
            [[no_unique_address]]cmd_out_t<void(fade_mode_e, uint8_t, uint8_t, uint8_t)> move_with_on_off;

            [[=cmd_out_a{{.id = ZB_ZCL_CMD_LEVEL_CONTROL_STEP_WITH_ON_OFF}}]]
            [[no_unique_address]]cmd_out_t<void(fade_mode_e, uint8_t, uint16_t, uint8_t, uint8_t)> step_with_on_off;

            [[=cmd_out_a{{.id = ZB_ZCL_CMD_LEVEL_CONTROL_STOP_WITH_ON_OFF}}]]
            [[no_unique_address]]cmd_out_t<void(uint8_t, uint8_t)> stop_with_on_off;

            [[=cmd_out_a{{.id = ZB_ZCL_CMD_LEVEL_CONTROL_MOVE_TO_CLOSEST_FREQUENCY}}]]
            [[no_unique_address]]cmd_out_t<void(uint16_t)> move_to_closest_freq;
        };
    }
}
#endif
