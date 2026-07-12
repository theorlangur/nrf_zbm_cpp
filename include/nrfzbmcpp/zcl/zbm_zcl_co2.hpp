#ifndef ZBM_ZCL_CO2_HPP_
#define ZBM_ZCL_CO2_HPP_

#include "../zbm.hpp"
#include "zbm_zcl_measuring_cluster_template.hpp"

namespace zbm
{
    namespace zcl
    {
        static constexpr uint16_t kZB_ZCL_CLUSTER_ID_CO2 = 0x040D;

        using co2_basic_t = measure_tpl_basic_t<kZB_ZCL_CLUSTER_ID_CO2>;
        using co2_t       = measure_tpl_t<kZB_ZCL_CLUSTER_ID_CO2>;
        using co2_ext_t   = measure_tpl_ext_t<kZB_ZCL_CLUSTER_ID_CO2>;
    }
}
#endif
