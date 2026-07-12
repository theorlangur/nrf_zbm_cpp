#ifndef ZBM_MISC_ZC_AIR_Q_HPP_
#define ZBM_MISC_ZC_AIR_Q_HPP_

#include "../zbm.hpp"

namespace zbm
{
    namespace misc_zc
    {
        static constexpr uint16_t KZB_ZCL_CLUSTER_ID_AIR_Q = 0xfc08;
        struct 
            [[=cluster_a{.id = KZB_ZCL_CLUSTER_ID_AIR_Q}]]
            air_q_t
        {
            enum class AQI: uint8_t
            {
                Excellent = 1,
                Good,
                Moderate,
                Poor,
                Unhealthy
            };

            [[=attribute_a{.id = 0, .a = access_t::RP}]]
            float tvoc = 0;

            [[=attribute_a{.id = 1, .a = access_t::RP}]]
            AQI aqi = AQI::Excellent;
        };
    }
}
#endif
