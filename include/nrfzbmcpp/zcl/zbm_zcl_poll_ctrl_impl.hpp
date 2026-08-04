#ifndef ZBM_ZCL_POLL_CTRL_IMPL_HPP_
#define ZBM_ZCL_POLL_CTRL_IMPL_HPP_

#include "../zbm.hpp"

namespace zbm
{
    namespace zcl
    {
        struct poll_ctrl_basic_new_t;
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

            struct handler
            {
                poll_ctrl_basic_new_t &cluster;
                uint8_t cmdBuf = 0;
                uint8_t ep = 0;

                void init(uint8_t ep);

                void start();
                void stop();

            private:
                void send_check_in(uint8_t buf);
                void on_bind_check(uint8_t param);
                void on_check_in_sent(uint8_t param);
                void on_no_response(uint8_t param);


                static cmd_handling_result_t on_check_in_response(bool start_fast_poll, uint16_t fast_poll_timeout);
                static cmd_handling_result_t on_fast_poll_stop();
                static cmd_handling_result_t on_set_long_poll(uint32_t new_long_poll);
                static cmd_handling_result_t on_set_short_poll(uint16_t new_short_poll);

                template<void (handler::*)(uint8_t)>
                friend void callback(uint8_t);

                friend struct zbm::zcl::poll_ctrl_basic_new_t;
            };
        }
    }
}
#endif
