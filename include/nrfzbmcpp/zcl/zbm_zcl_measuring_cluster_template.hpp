#ifndef ZBM_ZCL_MEASURING_CLUSTER_TEMPLATE_HPP_
#define ZBM_ZCL_MEASURING_CLUSTER_TEMPLATE_HPP_

#include "../zbm.hpp"

namespace zbm
{
    namespace zcl
    {
        template<uint16_t ClusterID>
        struct 
            [[=cluster_a{.id = ClusterID}]]
            measure_tpl_basic_t
        {
            [[=attribute_a{.id = 0, .a = access_t::RP}]]
            zb_uint16_t measured_value{};
        };

        template<uint16_t ClusterID>
        struct measure_tpl_t: measure_tpl_basic_t<ClusterID>
        {
            [[=attribute_a{.id = 1}]]
            uint16_t min_measured_value;

            [[=attribute_a{.id = 2}]]
            uint16_t max_measured_value;
        };

        template<uint16_t ClusterID>
        struct measure_tpl_ext_t: measure_tpl_t<ClusterID>
        {
            [[=attribute_a{.id = 3}]]
            uint16_t tolerance;
        };
    }
}
#endif
