#ifndef ZBM_ZCL_POLL_CTRL_IMPL_HPP_
#define ZBM_ZCL_POLL_CTRL_IMPL_HPP_

#include "../zbm.hpp"

namespace zbm
{
    namespace zcl
    {
        struct poll_ctrl_basic_auto_t;
        namespace poll_ctrl_impl{
            struct handler
            {
                poll_ctrl_basic_auto_t &cluster;
                uint8_t type_size;
                uint8_t ep = 0;

                template<class T> requires std::is_base_of_v<poll_ctrl_basic_auto_t, T>
                constexpr handler(T &c): cluster(c), type_size(sizeof(T))
                {
                    static_assert(sizeof(T) <= 255, "Unexpectedly big size of the poll control cluster type");
                }

                void init(uint8_t ep);

                void start();
                void stop();

                static void hook_check_in_interval(uint8_t ep, uint32_t new_val);
                static void hook_fast_poll_timeout(uint8_t ep, uint16_t new_val);
            private:
                void send_check_in(uint8_t buf);
                void send_check_in_validated(uint8_t param, bool ok);
                void on_check_in_sent(uint8_t param);
                void on_no_response(uint8_t param);

                cmd_handling_result_t on_check_in_response_cb(bool start_fast_poll, uint16_t fast_poll_timeout);
                cmd_handling_result_t on_fast_poll_stop_cb();
                cmd_handling_result_t on_set_long_poll_cb(uint32_t new_long_poll);
                cmd_handling_result_t on_set_short_poll_cb(uint16_t new_short_poll);


                static cmd_handling_result_t on_check_in_response(cmd_raw_t raw);
                static cmd_handling_result_t on_fast_poll_stop(cmd_raw_t raw);
                static cmd_handling_result_t on_set_long_poll(uint32_t new_long_poll);
                static cmd_handling_result_t on_set_short_poll(uint16_t new_short_poll);


                template<std::meta::info method>
                void on_cmd_handler_validated(uint8_t param, bool ok);

                template<std::meta::info method>
                static void check_bind_for_cmd_handler(uint8_t param);

                template<void (handler::*)(uint8_t)>
                friend void callback(uint8_t);

                friend struct zbm::zcl::poll_ctrl_basic_auto_t;

                template<std::meta::info method, std::meta::info call_type, class T, class... FixedArgs>
                friend cmd_handling_result_t zbm::call_handler_method(T &o, std::span<uint8_t> raw_data, FixedArgs... fixedArgs);
            };
        }
    }
}
#endif
