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
                uint8_t raw = 0;
                struct{
                    uint8_t PIR             : 1;
                    uint8_t Ultrasonic      : 1;
                    uint8_t PhysicalContact : 1;
                    uint8_t Unused          : 5;
                }bits;
            };

            [[=attribute_a{.id = 0, .a = access_t::RP, .type = type_t::Map8}]]
            uint8_t occupancy = 0;

            [[=attribute_a{.id = 1}]]
            sensor_type_t sensor_type = {};

            [[=attribute_a{.id = 2, .type = type_t::Map8}]]
            type_bitmap_u type_bitmap;
        };

        /**********************************************************************/
        /* Configuration blocks (NOT CLUSTERS)                                */
        /**********************************************************************/
        struct occupancy_cfg_pir_t
        {
            [[=attribute_a{.id = 0x10, .a = access_t::RW}]]
            uint16_t PIROccupiedToUnoccupiedDelay = 0;

            [[=attribute_a{.id = 0x11, .a = access_t::RW}]]
            uint16_t PIRUnoccupiedToOccupiedDelay = 0;

            [[=attribute_a{.id = 0x12, .a = access_t::RW}]]
            uint8_t PIRUnoccupiedToOccupiedThreshold = 1;
        };

        struct occupancy_cfg_ultrasonic_t
        {
            [[=attribute_a{.id = 0x20, .a = access_t::RW}]]
            uint16_t UltrasonicOccupiedToUnoccupiedDelay = 0;

            [[=attribute_a{.id = 0x21, .a = access_t::RW}]]
            uint16_t UltrasonicUnoccupiedToOccupiedDelay = 0;

            [[=attribute_a{.id = 0x22, .a = access_t::RW}]]
            uint8_t  UltrasonicUnoccupiedToOccupiedThreshold = 1;
        };

        struct occupancy_cfg_physical_contact_t
        {
            [[=attribute_a{.id = 0x30, .a = access_t::RW}]]
            uint16_t PhysicalContactOccupiedToUnoccupiedDelay = 0;

            [[=attribute_a{.id = 0x31, .a = access_t::RW}]]
            uint16_t PhysicalContactUnoccupiedToOccupiedDelay = 0;

            [[=attribute_a{.id = 0x32, .a = access_t::RW}]]
            uint8_t  PhysicalContactUnoccupiedToOccupiedThreshold = 1;
        };

        namespace occupancy_details{
            template<class Cfg>
            constexpr uint8_t get_occupancy_type_bitmask()
            {
                if constexpr (std::is_same_v<Cfg, occupancy_cfg_pir_t>)
                    return 0b001;
                else if constexpr (std::is_same_v<Cfg, occupancy_cfg_ultrasonic_t>)
                    return 0b010;
                else if constexpr (std::is_same_v<Cfg, occupancy_cfg_physical_contact_t>)
                    return 0b100;
            }

            constexpr occupancy_t::sensor_type_t get_occupancy_bitmask_to_type(uint8_t bmp)
            {
                switch(bmp)
                {
                    case 0b001://PIR
                        return occupancy_t::sensor_type_t::PIR;
                    case 0b010://Ultrasonic
                        return occupancy_t::sensor_type_t::Ultrasonic;
                    case 0b011://PIR + Ultrasonic
                        return occupancy_t::sensor_type_t::PIRandUltrasonic;
                    case 0b100://PhysicalContact
                        return occupancy_t::sensor_type_t::PhysicalContact;
                    case 0b101://PIR + PhysicalContact
                        return occupancy_t::sensor_type_t::PIR;
                    case 0b110://PIR + Ultrasonic
                        return occupancy_t::sensor_type_t::Ultrasonic;
                    case 0b111://PIR + Ultrasonic + PhysicalContact
                        return occupancy_t::sensor_type_t::PIRandUltrasonic;
                    default:
                        return occupancy_t::sensor_type_t::PIR;
                }
            }
        }


        /**********************************************************************/
        /* Generic template to configure occupancy attributes                 */
        /**********************************************************************/
        template<class... Cfg>
        struct occupancy_tpl_t: occupancy_t, Cfg...
        {
            static constexpr uint8_t kTypeBitMask = (occupancy_details::get_occupancy_type_bitmask<Cfg>() | ...);
            constexpr occupancy_tpl_t():
                //auto-initializing the correct type of the sensor by default based on the configuration combination used
                occupancy_t{.sensor_type = occupancy_details::get_occupancy_bitmask_to_type(kTypeBitMask), .type_bitmap={.raw = kTypeBitMask}}
            {}
        };

        /**********************************************************************/
        /* Several predefined types                                           */
        /**********************************************************************/
        using occupancy_pir_t                = occupancy_tpl_t<occupancy_cfg_pir_t>;
        using occupancy_ultrasonic_t         = occupancy_tpl_t<occupancy_cfg_ultrasonic_t>;
        using occupancy_physical_contact_t   = occupancy_tpl_t<occupancy_cfg_physical_contact_t>;
        using occupancy_pir_and_ultrasonic_t = occupancy_tpl_t<occupancy_cfg_pir_t, occupancy_cfg_ultrasonic_t>;
    }
}
#endif
