#include <nrfzbmcpp/zcl/zbm_zcl_poll_ctrl.hpp>

namespace zbm
{
    namespace zcl
    {
        namespace poll_ctrl_impl{
            void write_attribute_hook_handlers::hook_check_in_interval(uint8_t ep, uint32_t new_val)
            {
                zb_time_t new_interval;
                new_interval = ZB_QUARTERECONDS_TO_BEACON_INTERVAL(new_val);
                zb_uint8_t canceled_param = zb_zcl_poll_control_stop();

                if (new_val != ZB_ZCL_POLL_CONTROL_CHECKIN_INTERVAL_NO_CHECK_IN_VALUE)
                {
                    /* re-use buffer specified by canceled_param */
                    //zb_zcl_poll_control_start_internal(canceled_param, new_val);
                }
                else
                {
                    /* free buffer - it is not re-used */
                    if (canceled_param)
                        zb_buf_free(canceled_param);
                }
            }

            void write_attribute_hook_handlers::hook_fast_poll_timeout(uint8_t ep, uint16_t new_val)
            {
            }


            void handler::do_thing()
            {
            }

            void handler::init()
            {
            }
        }
    }
}
