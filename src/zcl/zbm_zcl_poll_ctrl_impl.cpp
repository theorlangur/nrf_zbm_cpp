#include <nrfzbmcpp/zcl/zbm_zcl_poll_ctrl.hpp>

namespace zbm
{
    namespace zcl
    {
        namespace poll_ctrl_impl{
            zb_time_t kCheckInNoResponseIntervalMS = 7680;
            zb_time_t kCheckInNoResponseInterval = ZB_MILLISECONDS_TO_BEACON_INTERVAL(kCheckInNoResponseIntervalMS);
            constinit static handler *g_pHandler = nullptr;

            void write_attribute_hook_handlers::hook_check_in_interval(uint8_t ep, uint32_t new_val)
            {
                zb_time_t new_interval;
                new_interval = ZB_QUARTERECONDS_TO_BEACON_INTERVAL(new_val);
                zb_uint8_t canceled_param = zb_zcl_poll_control_stop();

                if (new_val != ZB_ZCL_POLL_CONTROL_CHECKIN_INTERVAL_NO_CHECK_IN_VALUE)
                {
                    /* re-use buffer specified by canceled_param */
                    //zb_zcl_poll_control_start_internal(canceled_param, new_val);
                    ZB_SCHEDULE_APP_CALLBACK(zb_zcl_poll_control_save_nvram, 0);
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

            /**********************************************************************/
            /* handler                                                            */
            /**********************************************************************/

            template<void (handler::*MemberMethod)(uint8_t)>
            void callback(uint8_t b) { return (g_pHandler->*MemberMethod)(b); }

            template<auto MemberMethod>
            constexpr zb_callback_t callback_to = callback<MemberMethod>;


            template<auto CB>
            concept bind_complete_handler_c = requires(uint8_t param, bool exists) { (g_pHandler->*CB)(param, exists); };

            template<auto CB>
            void generic_check_bind_handler(uint8_t param)
            {
                zb_aps_check_binding_resp_t *check_binding_resp = NULL;
                check_binding_resp = ZB_BUF_GET_PARAM(param, zb_aps_check_binding_resp_t);
                static_assert(bind_complete_handler_c<CB>, "Unsupported callback for the result of bind checking");
                if constexpr (bind_complete_handler_c<CB>)
                {
                    (g_pHandler->*CB)(param, check_binding_resp->exists);
                }
            }

            template<auto cb>
            void check_bind(uint8_t buf)
            {
                zb_aps_check_binding_req_t *check_binding_req = nullptr;
                check_binding_req = ZB_BUF_GET_PARAM(buf, zb_aps_check_binding_req_t);
                ZB_BZERO(check_binding_req, sizeof(*check_binding_req));

                check_binding_req->src_endpoint = ZB_ZCL_BROADCAST_ENDPOINT;
                check_binding_req->cluster_id = ZB_ZCL_CLUSTER_ID_POLL_CONTROL;
                check_binding_req->response_cb = generic_check_bind_handler<cb>;
                zb_zdo_check_binding_request(buf);
            }

            void handler::init(uint8_t _ep)
            {
                g_pHandler = this;
                ep = _ep;
            }

            void handler::start()
            {
                //ZB_SCHEDULE_APP_ALARM(callback_to<&handler::send_check_in>, 0, ZB_QUARTERECONDS_TO_BEACON_INTERVAL(g_pHandler->cluster.check_in_interval));
                send_check_in(0);
            }

            void handler::stop()
            {
                uint8_t no_resp_param, check_in_param;
                zb_schedule_alarm_cancel(callback_to<&handler::on_no_response>, ZB_ALARM_ANY_PARAM, &no_resp_param);
                zb_schedule_alarm_cancel(callback_to<&handler::send_check_in>, ZB_ALARM_ANY_PARAM, &check_in_param);
                if (no_resp_param != 0)
                    zb_buf_free(no_resp_param);
                if (check_in_param != 0)
                    zb_buf_free(check_in_param);
            }

            void handler::send_check_in(uint8_t buf)
            {
                if (buf == 0)
                {
                    zb_buf_get_out_delayed(callback_to<&handler::send_check_in>);
                    return;
                }

                //It makes sense sending check-in requests only when there's someone interested in them (bound to our cluster)
                check_bind<&handler::send_check_in_validated>(buf);
            }

            void handler::send_check_in_validated(uint8_t param, bool ok)
            {
                if (ok)
                {
                    send_cmd_mem<^^poll_ctrl_basic_new_t::check_in>::to(param, callback_to<&handler::on_check_in_sent>, ep);
                    ZB_SCHEDULE_APP_ALARM(callback_to<&handler::on_no_response>, ep, kCheckInNoResponseInterval);

                    zb_zdo_pim_set_fast_poll_timeout(kCheckInNoResponseIntervalMS);
                    zb_zdo_pim_start_fast_poll(0);
                }else
                    zb_buf_free(param);

                //schedule next check-in either way
                ZB_SCHEDULE_APP_ALARM(callback_to<&handler::send_check_in>, 0, ZB_QUARTERECONDS_TO_BEACON_INTERVAL(cluster.check_in_interval));
            }

            void handler::on_no_response(uint8_t param)
            {
                //stop fast poll
                zb_zdo_pim_stop_fast_poll(0);
            }

            void handler::on_check_in_sent(uint8_t param)
            {
                zb_zcl_command_send_status_t *cmd_send_status = ZB_BUF_GET_PARAM(param, zb_zcl_command_send_status_t);
                if (cmd_send_status->status == 0)
                {
                    ZB_SCHEDULE_APP_ALARM_CANCEL(callback_to<&handler::on_no_response>, ep);

                    //re-arm no-response timer
                    //not sure why ref implementation was switching to 'turbo poll'
                    //zb_zdo_pim_start_turbo_poll_packets(0);
                    ZB_SCHEDULE_APP_ALARM(callback_to<&handler::on_no_response>, ep, kCheckInNoResponseInterval);
                }
                zb_buf_free(param);
            }

            cmd_handling_result_t handler::on_check_in_response(bool start_fast_poll, uint16_t fast_poll_timeout)
            {
                if (g_pHandler) return g_pHandler->on_check_in_response_cb(start_fast_poll, fast_poll_timeout);
                return {.status = RET_ERROR};
            }

            cmd_handling_result_t handler::on_check_in_response_cb(bool start_fast_poll, uint16_t fast_poll_timeout)
            {
                if (start_fast_poll)
                {
                    if (fast_poll_timeout == 0) fast_poll_timeout = cluster.fast_poll_timeout;
                    zb_zdo_pim_set_fast_poll_timeout(fast_poll_timeout);
                    zb_zdo_pim_start_fast_poll(0);
                }
                return {};
            }

            cmd_handling_result_t handler::on_fast_poll_stop()
            {
                if (g_pHandler) return g_pHandler->on_fast_poll_stop_cb();
                return {.status = RET_ERROR};
            }

            cmd_handling_result_t handler::on_fast_poll_stop_cb()
            {
                //zb_zdo_pim_stop_fast_poll_extended_req(param, fast_poll_stop_handler_send_default_response);
                zb_zdo_pim_stop_fast_poll(0);
                return {};
            }

            cmd_handling_result_t handler::on_set_long_poll(uint32_t new_long_poll)
            {
                if (g_pHandler) return g_pHandler->on_set_long_poll_cb(new_long_poll);
                return {.status = RET_ERROR};
            }

            cmd_handling_result_t handler::on_set_long_poll_cb(uint32_t new_long_poll)
            {
                if (new_long_poll > ZB_ZCL_POLL_CONTROL_LONG_POLL_INTERVAL_MAX_VALUE || new_long_poll < ZB_ZCL_POLL_CONTROL_LONG_POLL_INTERVAL_MIN_VALUE)
                    return {.status = RET_INVALID_PARAMETER_1};
                cluster.long_poll_interval = new_long_poll;
                zb_zdo_pim_set_long_poll_interval(ZB_QUARTERECONDS_TO_MSEC(new_long_poll));
                //optionally save nvram
                //ZB_SCHEDULE_CALLBACK(zb_zcl_poll_control_save_nvram, 0);
                return {};
            }

            cmd_handling_result_t handler::on_set_short_poll(uint16_t new_short_poll)
            {
                if (g_pHandler) return g_pHandler->on_set_short_poll_cb(new_short_poll);
                return {.status = RET_ERROR};
            }

            cmd_handling_result_t handler::on_set_short_poll_cb(uint16_t new_short_poll)
            {
                if (g_pHandler)
                cluster.short_poll_interval = new_short_poll;
                zb_zdo_pim_set_fast_poll_interval(ZB_QUARTERECONDS_TO_MSEC(new_short_poll));
                //optionally save nvram
                //ZB_SCHEDULE_CALLBACK(zb_zcl_poll_control_save_nvram, 0);
                return {};
            }
        }
    }
}
