#ifndef ZBM_ZCL_POLL_CTRL_IMPL_HPP_
#define ZBM_ZCL_POLL_CTRL_IMPL_HPP_

#include "../zbm.hpp"

namespace zbm
{
    namespace zcl
    {
        namespace poll_ctrl_impl{
            struct write_attribute_hook_handlers
            {
                static void hook_check_in_interval(uint8_t ep, uint32_t new_val);
                static void hook_fast_poll_timeout(uint8_t ep, uint16_t new_val);
            };

            //template<std::meta::info cluster_ref, uint8_t ep, uint8_t addHandlingDepth>
            //void poll_control_write_attr_hook_server(zb_uint8_t endpoint, zb_uint16_t attr_id, zb_uint8_t *new_value, zb_uint16_t manuf_code)
            //{
            //}
        }
    }
}
#endif
