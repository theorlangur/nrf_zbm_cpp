#include <nrfzbmcpp/zcl/zbm_zcl_poll_ctrl.hpp>

namespace zbm
{
    namespace zcl
    {
        namespace poll_ctrl_impl{
            zb_time_t kCheckInNoResponseIntervalMS = 7680;
            zb_time_t kCheckInNoResponseInterval = ZB_MILLISECONDS_TO_BEACON_INTERVAL(kCheckInNoResponseIntervalMS);
            constinit static handler *g_pHandler = nullptr;

            void handler::hook_check_in_interval(uint8_t ep, uint32_t new_val)
            {
                zb_time_t new_interval;
                new_interval = ZB_QUARTERECONDS_TO_BEACON_INTERVAL(new_val);
                g_pHandler->stop();

                if (new_val != ZB_ZCL_POLL_CONTROL_CHECKIN_INTERVAL_NO_CHECK_IN_VALUE)
                {
                    /* re-use buffer specified by canceled_param */
                    //zb_zcl_poll_control_start_internal(canceled_param, new_val);
                    //ZB_SCHEDULE_APP_CALLBACK(zb_zcl_poll_control_save_nvram, 0);
                    g_pHandler->start();
                    (void)zb_nvram_write_dataset(ZB_NVRAM_HA_POLL_CONTROL_DATA);
                }
            }

            void handler::hook_fast_poll_timeout(uint8_t ep, uint16_t new_val)
            {
                (void)zb_nvram_write_dataset(ZB_NVRAM_HA_POLL_CONTROL_DATA);
            }

            zb_ret_t handler::validate_check_in_interval(zb_uint8_t *value)
            {
                uint32_t v;
                memcpy(&v, value, sizeof(v));
                if (v > ZB_ZCL_POLL_CONTROL_CHECKIN_INTERVAL_MAX_VALUE)
                    return RET_ERROR;
                if (g_pHandler->type_size == sizeof(poll_ctrl_auto_t))
                {
                    poll_ctrl_auto_t *pFull = static_cast<poll_ctrl_auto_t *>(&g_pHandler->cluster);
                    if (v < pFull->check_in_interval_min)
                        return RET_ERROR;
                }
                return RET_OK;
            }

            zb_ret_t handler::validate_fast_poll_timeout(zb_uint8_t *value)
            {
                uint16_t v;
                memcpy(&v, value, sizeof(v));
                if (v < ZB_ZCL_POLL_CONTROL_FAST_POLL_TIMEOUT_MIN_VALUE)
                    return RET_ERROR;
                if (g_pHandler->type_size == sizeof(poll_ctrl_auto_t))
                {
                    poll_ctrl_auto_t *pFull = static_cast<poll_ctrl_auto_t *>(&g_pHandler->cluster);
                    if (v > pFull->fast_poll_timeout_max)
                        return RET_ERROR;
                }
                return RET_OK;
            }

            /**********************************************************************/
            /* handler                                                            */
            /**********************************************************************/


            consteval size_t get_param_total_size(std::meta::info m)
            {
                auto params = std::meta::parameters_of(std::meta::type_of(m));
                return std::ranges::fold_left(params, 0, [](size_t s, std::meta::info param){return s + std::meta::size_of(param);});
            }

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
                    send_cmd_mem<^^poll_ctrl_basic_auto_t::check_in>::to(param, callback_to<&handler::on_check_in_sent>, ep);
                    ZB_SCHEDULE_APP_ALARM(callback_to<&handler::on_no_response>, ep, kCheckInNoResponseInterval);

                    zb_zdo_pim_set_fast_poll_timeout(kCheckInNoResponseIntervalMS);
                    zb_zdo_pim_start_fast_poll(0);
                }else
                    zb_buf_free(param);

                //schedule next check-in either way
                ZB_SCHEDULE_APP_ALARM(callback_to<&handler::send_check_in>, 0, ZB_QUARTERECONDS_TO_BEACON_INTERVAL(cluster.check_in_interval));
            }

            template<std::meta::info method>
            void handler::on_cmd_handler_validated(uint8_t param, bool ok)
            {
                constexpr size_t params_size = get_param_total_size(method);
                uint8_t* pRawParams = (uint8_t*)zb_buf_begin(param);
                zb_zcl_parsed_hdr_t *cmd_info = (zb_zcl_parsed_hdr_t *)(pRawParams + params_size);
                std::span<uint8_t> raw_data{pRawParams, zb_buf_len(param)};
                static_assert(alignof(zb_zcl_parsed_hdr_t) == 1, "Unexpected alignment requirement of zb_zcl_parsed_hdr_t");
                if (!ok)
                {
                    //finish command with 'failed'
                    ZB_ZCL_PROCESS_COMMAND_FINISH(param, cmd_info, ZB_ZCL_STATUS_ACTION_DENIED);
                }else
                {
                    auto [status, processed] = call_handler_method<method, std::meta::type_of(method)>(*g_pHandler, raw_data);
                    if(cmd_info->disable_default_response && status == RET_OK)
                    {
                        zb_buf_free(param);
                    }else
                    {
                        ZB_ZCL_PROCESS_COMMAND_FINISH(param, cmd_info, status==RET_OK ? ZB_ZCL_STATUS_SUCCESS : ZB_ZCL_STATUS_INVALID_FIELD);
                    }
                }
            }

            template<std::meta::info method>
            void handler::check_bind_for_cmd_handler(uint8_t param)
            {
                constexpr size_t params_size = get_param_total_size(method);
                uint8_t* pRaw = (uint8_t*)zb_buf_begin(param);
                pRaw += params_size;
                zb_zcl_parsed_hdr_t *cmd_info = ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t);
                memcpy(pRaw, cmd_info, sizeof(zb_zcl_parsed_hdr_t));//saving zb_zcl_parsed_hdr_t right after expected parameter data
                //re-using param for bind-check
                check_bind<&handler::on_cmd_handler_validated<method>>(param);
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

            cmd_handling_result_t handler::on_check_in_response(cmd_raw_t raw)
            {
                if (g_pHandler) 
                {
                    check_bind_for_cmd_handler<^^handler::on_check_in_response_cb>(raw.param);
                    return {.status = RET_BUSY};
                }
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

            cmd_handling_result_t handler::on_fast_poll_stop(cmd_raw_t raw)
            {
                if (g_pHandler) 
                {
                    check_bind_for_cmd_handler<^^handler::on_fast_poll_stop_cb>(raw.param);
                    return {.status = RET_BUSY};
                    //return g_pHandler->on_fast_poll_stop_cb();
                }
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
                (void)zb_nvram_write_dataset(ZB_NVRAM_HA_POLL_CONTROL_DATA);
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
                (void)zb_nvram_write_dataset(ZB_NVRAM_HA_POLL_CONTROL_DATA);
                return {};
            }
        }
    }
}
