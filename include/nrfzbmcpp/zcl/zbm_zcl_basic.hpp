#ifndef ZBM_ZCL_BASIC_HPP_
#define ZBM_ZCL_BASIC_HPP_

#include "../zbm.hpp"

namespace zbm
{
    namespace zcl
    {
        struct 
            [[=cluster_a{.id = ZB_ZCL_CLUSTER_ID_BASIC, .pre_init = ZB_ZCL_CLUSTER_ID_BASIC_SERVER_ROLE_INIT}]]
            basic_min_t
        {
            enum PowerSource: zb_uint8_t
            {
                Unknown               = 0x00,
                Mains1Phase           = 0x01,
                Mains3Phase           = 0x02,
                Battery               = 0x03,
                DC                    = 0x04,
                EmergencyConst        = 0x05,
                EmergencySwitch       = 0x06,
                BackupUnknown         = 0x80,
                BackupMains1Phase     = 0x81,
                BackupMains3Phase     = 0x82,
                BackupBattery         = 0x83,
                BackupDC              = 0x84,
                BackupEmergencyConst  = 0x85,
                BackupEmergencySwitch = 0x86,
            };
            [[=attribute_a{.id = ZB_ZCL_ATTR_BASIC_ZCL_VERSION_ID}]]
            zb_uint8_t zcl_version{};
            [[=attribute_a{.id = ZB_ZCL_ATTR_BASIC_POWER_SOURCE_ID}]]
            PowerSource power_source{PowerSource::Unknown};
        };

        struct basic_names_t: basic_min_t
        {
            [[=attribute_a{.id = ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID}]]
            str_t<33> manufacturer{};
            [[=attribute_a{.id = ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID}]]
            str_t<33> model{};
        };

        struct basic_ext_t: basic_names_t
        {
            [[=attribute_a{.id = ZB_ZCL_ATTR_BASIC_APPLICATION_VERSION_ID}]]
            zb_uint8_t app_version{};
            [[=attribute_a{.id = ZB_ZCL_ATTR_BASIC_STACK_VERSION_ID}]]
            zb_uint8_t stack_version{};
            [[=attribute_a{.id = ZB_ZCL_ATTR_BASIC_HW_VERSION_ID}]]
            zb_uint8_t hw_version{};
            [[=attribute_a{.id = ZB_ZCL_ATTR_BASIC_DATE_CODE_ID}]]
            str_t<17>  date_code{};
            [[=attribute_a{.id = ZB_ZCL_ATTR_BASIC_LOCATION_DESCRIPTION_ID}]]
            str_t<17>  location_id{};
            [[=attribute_a{.id = ZB_ZCL_ATTR_BASIC_PHYSICAL_ENVIRONMENT_ID}]]
            zb_uint8_t ph_env{};
            [[=attribute_a{.id = ZB_ZCL_ATTR_BASIC_SW_BUILD_ID}]]
            str_t<17>  sw_ver{};
        };
    }
}
#endif
