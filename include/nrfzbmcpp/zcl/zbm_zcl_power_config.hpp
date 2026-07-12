#ifndef ZBM_ZCL_POWER_CONFIG_HPP_
#define ZBM_ZCL_POWER_CONFIG_HPP_

#include "../zbm.hpp"

namespace zbm
{
    namespace zcl
    {
        static constexpr uint16_t kZB_ZCL_CLUSTER_ID_POWER_CFG = 0x0001;

        struct 
            [[=cluster_a{.id = kZB_ZCL_CLUSTER_ID_POWER_CFG}]]
            power_cfg_mains_info_t
        {
            [[=attribute_a{.id = 0}]]
            uint16_t mains_voltage{};

            [[=attribute_a{.id = 1}]]
            uint8_t mains_freq{};
        };

        struct mains_alarm_mask_t
        {
            static constexpr type_t type_id() { return type_t::Map8; }

            uint8_t voltage_low  : 1 = 0;
            uint8_t voltage_high : 1 = 0;
            uint8_t supply_lost  : 1 = 0;
        };

        struct power_cfg_mains_settings_t: power_cfg_mains_info_t
        {
            [[=attribute_a{.id = 0x10, .a = access_t::RW}]]
            mains_alarm_mask_t mains_alarm_mask{};

            [[=attribute_a{.id = 0x11, .a = access_t::RW}]]
            uint16_t mains_voltage_min_threshold{};

            [[=attribute_a{.id = 0x12, .a = access_t::RW}]]
            uint16_t mains_voltage_max_threshold{};

            [[=attribute_a{.id = 0x13, .a = access_t::RW}]]
            uint16_t mains_voltage_dwell_trip_point{};
        };

        struct 
            [[=cluster_a{.id = kZB_ZCL_CLUSTER_ID_POWER_CFG}]]
            power_cfg_battery_info_t
        {
            [[=attribute_a{.id = 0x20, .a = access_t::RP}]]
            uint8_t batt_voltage{};

            [[=attribute_a{.id = 0x21, .a = access_t::RP}]]
            uint8_t batt_percentage_remaining{};
        };


        enum class battery_size_t: uint8_t
        {
            NoBattery = 0,
            BuiltIn = 1,
            Other = 2,
            AA = 3,
            AAA = 4,
            C = 5,
            D = 6,
            CR2 = 7,
            CR123A= 8,
            Unkown = 0xff
        };

        struct battery_alarm_mask_t
        {
            static constexpr type_t type_id() { return type_t::Map8; }

            uint8_t low    : 1;
            uint8_t alarm1 : 1;
            uint8_t alarm2 : 1;
            uint8_t alarm3 : 1;
        };

        struct battery_alarm_state_t
        {
            static constexpr type_t type_id() { return type_t::Map32; }

            uint32_t low_src1    : 1;
            uint32_t alarm1_src1 : 1;
            uint32_t alarm2_src1 : 1;
            uint32_t alarm3_src1 : 1;
            uint32_t unused1     : 6;
            uint32_t low_src2    : 1;
            uint32_t alarm1_src2 : 1;
            uint32_t alarm2_src2 : 1;
            uint32_t alarm3_src2 : 1;
            uint32_t unused2     : 6;
            uint32_t low_src3    : 1;
            uint32_t alarm1_src3 : 1;
            uint32_t alarm2_src3 : 1;
            uint32_t alarm3_src3 : 1;
            uint32_t unused3     : 6;
            uint32_t mains_lost  : 1;
        };

        struct power_cfg_battery_settings_t: power_cfg_battery_info_t
        {
            [[=attribute_a{.id = 0x30, .a = access_t::RW}]]
            str_t<16> batt_manufacture;

            [[=attribute_a{.id = 0x31, .a = access_t::RW}]]
            battery_size_t batt_size{};

            [[=attribute_a{.id = 0x32, .a = access_t::RW}]]
            uint16_t batt_AHr_rating{};

            [[=attribute_a{.id = 0x33, .a = access_t::RW}]]
            uint8_t batt_quantity{};

            [[=attribute_a{.id = 0x34, .a = access_t::RW}]]
            uint8_t batt_rated_voltage{};

            [[=attribute_a{.id = 0x35, .a = access_t::RW}]]
            battery_alarm_mask_t batt_alarm_mask{};

            [[=attribute_a{.id = 0x36, .a = access_t::RW}]]
            uint8_t batt_voltage_min_threshold{};

            [[=attribute_a{.id = 0x37, .a = access_t::RW}]]
            uint8_t batt_voltage_threshold1{};

            [[=attribute_a{.id = 0x38, .a = access_t::RW}]]
            uint8_t batt_voltage_threshold2{};

            [[=attribute_a{.id = 0x39, .a = access_t::RW}]]
            uint8_t batt_voltage_threshold3{};

            [[=attribute_a{.id = 0x3a, .a = access_t::RW}]]
            uint8_t batt_percentage_min_threshold{};

            [[=attribute_a{.id = 0x3b, .a = access_t::RW}]]
            uint8_t batt_percentage_threshold1{};

            [[=attribute_a{.id = 0x3c, .a = access_t::RW}]]
            uint8_t batt_percentage_threshold2{};

            [[=attribute_a{.id = 0x3d, .a = access_t::RW}]]
            uint8_t batt_percentage_threshold3{};

            [[=attribute_a{.id = 0x3e, .a = access_t::RW}]]
            battery_alarm_state_t batt_alarm_state{};
        };
    }
}
#endif
