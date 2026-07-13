#ifndef ZBM_MISC_ZC_STATUS_HPP_
#define ZBM_MISC_ZC_STATUS_HPP_

#include "../zbm.hpp"

namespace zbm
{
    namespace misc_zc
    {
        static constexpr uint16_t kZB_ZCL_CLUSTER_ID_STATUS = 0xfc80;
        struct 
            [[=cluster_a{.id = kZB_ZCL_CLUSTER_ID_STATUS}]]
            status_t
        {
            static constexpr uint8_t kCMD1 = 1;
            static constexpr uint8_t kCMD2 = 2;
            static constexpr uint8_t kCMD3 = 3;

            [[=attribute_a{.id = 0, .a = access_t::RP}]]
            int16_t status1 = 0;

            [[=attribute_a{.id = 1, .a = access_t::RP}]]
            int16_t status2 = 0;

            [[=attribute_a{.id = 2, .a = access_t::RP}]]
            int16_t status3 = 0;

            /**********************************************************************/
            /* Commands                                                           */
            /**********************************************************************/
            [[=cmd_in_a{.id = kCMD1}]]
            void(*cmd1)();

            [[=cmd_in_a{.id = kCMD2}]]
            void(*cmd2)();

            [[=cmd_in_a{.id = kCMD3}]]
            void(*cmd3)();
        };
    }
}
#endif
