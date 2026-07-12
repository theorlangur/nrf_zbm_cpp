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
    }
}
#endif
