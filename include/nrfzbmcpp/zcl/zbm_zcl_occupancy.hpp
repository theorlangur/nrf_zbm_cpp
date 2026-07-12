#ifndef ZBM_ZCL_OCCUPANCY_SENSING_HPP_
#define ZBM_ZCL_OCCUPANCY_SENSING_HPP_

#include "../zbm.hpp"

namespace zbm
{
    namespace zcl
    {
        static constexpr uint16_t kZB_ZCL_CLUSTER_ID_OCCUPANCY_SENSING = 0x0406;

        /**********************************************************************/
        /* Basic attributes                                                   */
        /**********************************************************************/
        struct 
            [[=cluster_a{.id = kZB_ZCL_CLUSTER_ID_OCCUPANCY_SENSING}]]
            occupancy_t
        {
            enum sensor_type_t: uint8_t { PIR = 0, Ultrasonic = 1, PIRandUltrasonic = 2, PhysicalContact = 3 };
            union type_bitmap_u
            {
                uint8_t type_bitmap_raw = 0;
                struct{
                    uint8_t PIR             : 1;
                    uint8_t Ultrasonic      : 1;
                    uint8_t PhysicalContact : 1;
                    uint8_t Unused          : 5;
                }type_bitmap;
            };

            [[=attribute_a{.id = 0, .a = access_t::RP, .type = type_t::Map8}]]
            uint8_t occupancy = 0;

            [[=attribute_a{.id = 1}]]
            sensor_type_t sensor_type = {};

            [[=attribute_a{.id = 3, .type = type_t::Map8}]]
            type_bitmap_u type_bitmap;
        };
    }
}
#endif
