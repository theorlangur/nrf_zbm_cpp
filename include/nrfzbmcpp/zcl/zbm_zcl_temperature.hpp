#ifndef ZBM_ZCL_TEMPERATURE_HPP_
#define ZBM_ZCL_TEMPERATURE_HPP_

#include "../zbm.hpp"
#include "zbm_zcl_measuring_cluster_template.hpp"

namespace zbm
{
    namespace zcl
    {
        static constexpr uint16_t kZB_ZCL_CLUSTER_ID_TEMP = 0x0402;

        using temp_basic_t = measure_tpl_basic_t<kZB_ZCL_CLUSTER_ID_TEMP>;
        using temp_t       = measure_tpl_t<kZB_ZCL_CLUSTER_ID_TEMP>;
        using temp_ext_t   = measure_tpl_ext_t<kZB_ZCL_CLUSTER_ID_TEMP>;

        inline int16_t FromC(float c) { return int16_t(c * 100); }
        inline float ToC(int16_t centiC) { return float(centiC) / 100.f; }
    }
}
#endif
