#ifndef ZBM_ZCL_GROUPS_HPP_
#define ZBM_ZCL_GROUPS_HPP_

#include "../zbm.hpp"

namespace zbm
{
    namespace zcl
    {
        struct 
            [[=cluster_a{.id = ZB_ZCL_CLUSTER_ID_GROUPS, .pre_init = ZB_ZCL_CLUSTER_ID_GROUPS_SERVER_ROLE_INIT}]]
            groups_t
        {
            [[=attribute_a{.id = ZB_ZCL_ATTR_GROUPS_NAME_SUPPORT_ID, .type=type_t::Map8}]]
            zb_uint8_t name_support;
        };
    }
}
#endif
