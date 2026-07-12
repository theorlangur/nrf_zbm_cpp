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
    }
}
#endif
