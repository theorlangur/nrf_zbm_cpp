#include <nrfzbmcpp/zcl/zbm_zcl_poll_ctrl.hpp>

namespace zbm
{
    namespace zcl
    {
        namespace poll_ctrl_impl{
            zb_time_t kCheckInNoResponseIntervalMS = 7680;
            zb_time_t kCheckInNoResponseInterval = ZB_MILLISECONDS_TO_BEACON_INTERVAL(kCheckInNoResponseIntervalMS);
            constinit static poll_ctrl_basic_new_t *g_pCluster = nullptr;
            constinit static uint8_t g_CmdBuf = 0;
            constinit static uint8_t g_EP = 0;

            void write_attribute_hook_handlers::hook_check_in_interval(uint8_t ep, uint32_t new_val)
            {
                zb_time_t new_interval;
                new_interval = ZB_QUARTERECONDS_TO_BEACON_INTERVAL(new_val);
                zb_uint8_t canceled_param = zb_zcl_poll_control_stop();

                if (new_val != ZB_ZCL_POLL_CONTROL_CHECKIN_INTERVAL_NO_CHECK_IN_VALUE)
                {
                    /* re-use buffer specified by canceled_param */
                    //zb_zcl_poll_control_start_internal(canceled_param, new_val);
                    ZB_SCHEDULE_CALLBACK(zb_zcl_poll_control_save_nvram, 0);
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

            void handler::init(uint8_t ep)
            {
                g_pCluster = &cluster;
                g_EP = ep;
            }

            void start()
            {
                ZB_SCHEDULE_ALARM(send_check_in, 0, ZB_QUARTERECONDS_TO_BEACON_INTERVAL(g_pCluster->check_in_interval));
            }

            void stop()
            {
                zb_uint8_t canceled_param = 0;
                ZB_SCHEDULE_ALARM_CANCEL_AND_GET_BUF(
                        send_check_in, ZB_ALARM_ANY_PARAM, &canceled_param);
                if (canceled_param != 0)
                {
                    zb_buf_free(g_CmdBuf);
                    g_CmdBuf = 0;
                }
            }

            void handler::send_check_in(uint8_t buf)
            {
                if (buf) g_CmdBuf = buf;
                if (buf == 0)
                {
                    zb_buf_get_out_delayed(send_check_in);
                    return;
                }

                zb_aps_check_binding_req_t *check_binding_req = nullptr;
                check_binding_req = ZB_BUF_GET_PARAM(g_CmdBuf, zb_aps_check_binding_req_t);
                ZB_BZERO(check_binding_req, sizeof(*check_binding_req));

                check_binding_req->src_endpoint = ZB_ZCL_BROADCAST_ENDPOINT;
                check_binding_req->cluster_id = ZB_ZCL_CLUSTER_ID_POLL_CONTROL;
                check_binding_req->response_cb = on_bind_check;
                zb_zdo_check_binding_request(g_CmdBuf);
            }

            void handler::on_bind_check(uint8_t param)
            {
                zb_aps_check_binding_resp_t *check_binding_resp = NULL;
                check_binding_resp = ZB_BUF_GET_PARAM(param, zb_aps_check_binding_resp_t);

                if (check_binding_resp->exists)
                {
                    zb_uint16_t dst_addr = 0;
                    zb_uint8_t dst_ep = 0;
                    zb_uint8_t addr_mode = ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT;

                    ZB_ZCL_POLL_CONTROL_SEND_CHECK_IN_REQ(
                      param,
                      dst_addr, addr_mode, dst_ep,
                      g_EP, ZB_AF_HA_PROFILE_ID,
                      on_check_in_sent);

                    ZB_SCHEDULE_ALARM(on_no_response, g_EP, kCheckInNoResponseInterval);

                    zb_zdo_pim_set_fast_poll_timeout(kCheckInNoResponseIntervalMS);
                    zb_zdo_pim_start_fast_poll(0);
                }else
                {
                    zb_buf_free(g_CmdBuf);
                    g_CmdBuf = 0;
                }

                ZB_SCHEDULE_ALARM(send_check_in, 0, ZB_QUARTERECONDS_TO_BEACON_INTERVAL(g_pCluster->check_in_interval));
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
                    ZB_SCHEDULE_ALARM_CANCEL(on_no_response, g_EP);

                    zb_zdo_pim_start_turbo_poll_packets(0);
                    ZB_SCHEDULE_ALARM(on_no_response, g_EP, kCheckInNoResponseInterval);
                }
            }
        }
    }
}
