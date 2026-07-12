#ifndef ZBM_ZCL_REL_HUMIDITY_HPP_
#define ZBM_ZCL_REL_HUMIDITY_HPP_

#include "../zbm.hpp"
#include "zbm_zcl_measuring_cluster_template.hpp"

namespace zbm
{
    namespace zcl
    {
        static constexpr uint16_t kZB_ZCL_CLUSTER_ID_REL_HUMIDITY = 0x0405;

        using rel_humid_basic_t = measure_tpl_basic_t<kZB_ZCL_CLUSTER_ID_REL_HUMIDITY>;
        using rel_humid_t       = measure_tpl_t<kZB_ZCL_CLUSTER_ID_REL_HUMIDITY>;
        using rel_humid_ext_t   = measure_tpl_ext_t<kZB_ZCL_CLUSTER_ID_REL_HUMIDITY>;

        inline uint16_t FromRelH(float v) { return uint16_t(v * 100); }
        inline float ToRelH(uint16_t v) { return float(v) / 100.f; }
    }
}
#endif
