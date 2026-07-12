#ifndef ZBM_ZCL_SCENES_HPP_
#define ZBM_ZCL_SCENES_HPP_

#include "../zbm.hpp"

namespace zbm
{
    namespace zcl
    {
        struct 
            [[=cluster_a{.id = ZB_ZCL_CLUSTER_ID_SCENES, .pre_init = ZB_ZCL_CLUSTER_ID_SCENES_SERVER_ROLE_INIT}]]
            scenes_t
        {
            [[=attribute_a{.id = ZB_ZCL_ATTR_SCENES_SCENE_COUNT_ID}]]
            zb_uint8_t  scene_count{};
            [[=attribute_a{.id = ZB_ZCL_ATTR_SCENES_CURRENT_SCENE_ID}]]
            zb_uint8_t  current_scene{};
            [[=attribute_a{.id = ZB_ZCL_ATTR_SCENES_SCENE_VALID_ID, .type=type_t::Bool}]]
            zb_uint8_t  scene_valid{};
            [[=attribute_a{.id = ZB_ZCL_ATTR_SCENES_NAME_SUPPORT_ID, .type=type_t::Map8}]]
            zb_uint8_t  name_support{};
            [[=attribute_a{.id = ZB_ZCL_ATTR_SCENES_CURRENT_GROUP_ID}]]
            zb_uint16_t current_group{};
        };
    }
}
#endif
