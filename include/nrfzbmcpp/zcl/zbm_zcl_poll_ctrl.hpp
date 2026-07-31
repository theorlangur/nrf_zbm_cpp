#ifndef ZBM_ZCL_POLL_CTRL_HPP_
#define ZBM_ZCL_POLL_CTRL_HPP_

#include "zbm_zcl_poll_ctrl_impl.hpp"

namespace zbm
{
    namespace zcl
    {
        namespace literals
        {
            constexpr uint32_t operator""_sec_to_qs(unsigned long long v) { return v * 4; }
            constexpr uint32_t operator""_min_to_qs(unsigned long long v) { return v * 4 * 60; }
            constexpr uint32_t operator""_h_to_qs(unsigned long long v) { return v * 4 * 60 * 60; }
        }

        constexpr uint32_t qs_to_s(uint32_t v) { return v / 4; }

        using namespace literals;

        /**********************************************************************/
        /* Cluster definitions                                                */
        /**********************************************************************/

        //TODO: make a non-legacy version, not depending on ZB_ZCL_CLUSTER_ID_POLL_CONTROL_SERVER_ROLE_INIT
        //with all necessary commands received/generated
        static constexpr uint16_t kZB_ZCL_CLUSTER_ID_POLL_CTRL = 0x0020;
        struct 
            [[=cluster_a{.id = kZB_ZCL_CLUSTER_ID_POLL_CTRL, .pre_init=ZB_ZCL_CLUSTER_ID_POLL_CONTROL_SERVER_ROLE_INIT}]]
            poll_ctrl_basic_t
        {
            [[=attribute_a{.id = ZB_ZCL_ATTR_POLL_CONTROL_CHECKIN_INTERVAL_ID, .a = access_t::RW}]]
            uint32_t check_in_interval = 1_h_to_qs;//1h
                                                   //
            [[=attribute_a{.id = ZB_ZCL_ATTR_POLL_CONTROL_LONG_POLL_INTERVAL_ID}]]
            uint32_t long_poll_interval = 5_sec_to_qs;//5sec

            [[=attribute_a{.id = ZB_ZCL_ATTR_POLL_CONTROL_SHORT_POLL_INTERVAL_ID}]]
            uint16_t short_poll_interval = 2;//2quater-sec

            [[=attribute_a{.id = ZB_ZCL_ATTR_POLL_CONTROL_FAST_POLL_TIMEOUT_ID, .a = access_t::RW}]]
            uint16_t fast_poll_timeout = 10_sec_to_qs;//10sec

            [[=attribute_a{.id = ZB_ZCL_ATTR_POLL_CONTROL_ADDR_DATA_ID, .a = access_t::Internal, .type=type_t::Null}]]
            zb_zcl_poll_control_srv_cfg_data_t srv_cfg = 
            { ZB_ZCL_POLL_CTRL_INVALID_ADDR, ZB_ZCL_POLL_INVALID_EP, 0, 0 };
        };

        struct poll_ctrl_t: poll_ctrl_basic_t
        {
            [[=attribute_a{.id = ZB_ZCL_ATTR_POLL_CONTROL_MIN_CHECKIN_INTERVAL_ID}]]
            uint32_t check_in_interval_min = 0;

            [[=attribute_a{.id = ZB_ZCL_ATTR_POLL_CONTROL_LONG_POLL_MIN_INTERVAL_ID}]]
            uint32_t long_poll_interval_min = 0;

            [[=attribute_a{.id = ZB_ZCL_ATTR_POLL_CONTROL_FAST_POLL_MAX_TIMEOUT_ID}]]
            uint16_t fast_poll_timeout_max = 0;
        };

        //TODO: add a client counterpart


        struct 
            [[=cluster_a{.id = kZB_ZCL_CLUSTER_ID_POLL_CTRL, .write_attr_hook_meta=^^poll_ctrl_impl::write_attribute_hook_handlers}]]
            poll_ctrl_basic_new_t
        {
            [[=attribute_a{.id = ZB_ZCL_ATTR_POLL_CONTROL_CHECKIN_INTERVAL_ID, .a = access_t::RW}]]
            uint32_t check_in_interval = 1_h_to_qs;//1h
                                                   //
            [[=attribute_a{.id = ZB_ZCL_ATTR_POLL_CONTROL_LONG_POLL_INTERVAL_ID}]]
            uint32_t long_poll_interval = 5_sec_to_qs;//5sec

            [[=attribute_a{.id = ZB_ZCL_ATTR_POLL_CONTROL_SHORT_POLL_INTERVAL_ID}]]
            uint16_t short_poll_interval = 2;//2quater-sec

            [[=attribute_a{.id = ZB_ZCL_ATTR_POLL_CONTROL_FAST_POLL_TIMEOUT_ID, .a = access_t::RW}]]
            uint16_t fast_poll_timeout = 10_sec_to_qs;//10sec

            //[[=attribute_a{.id = ZB_ZCL_ATTR_POLL_CONTROL_ADDR_DATA_ID, .a = access_t::Internal, .type=type_t::Null}]]
            //zb_zcl_poll_control_srv_cfg_data_t srv_cfg = 
            //{ ZB_ZCL_POLL_CTRL_INVALID_ADDR, ZB_ZCL_POLL_INVALID_EP, 0, 0 };
        };

        struct poll_ctrl_new_t: poll_ctrl_basic_new_t
        {
            [[=attribute_a{.id = ZB_ZCL_ATTR_POLL_CONTROL_MIN_CHECKIN_INTERVAL_ID}]]
            uint32_t check_in_interval_min = 0;

            [[=attribute_a{.id = ZB_ZCL_ATTR_POLL_CONTROL_LONG_POLL_MIN_INTERVAL_ID}]]
            uint32_t long_poll_interval_min = 0;

            [[=attribute_a{.id = ZB_ZCL_ATTR_POLL_CONTROL_FAST_POLL_MAX_TIMEOUT_ID}]]
            uint16_t fast_poll_timeout_max = 0;
        };

        /**********************************************************************/
        /* Cluster configuration helpers                                      */
        /**********************************************************************/

        struct poll_ctrl_cfg_t
        {
            uint8_t ep;
            zb_callback_t callback_on_check_in;
            bool sleepy_end_device;
            zb_time_t long_poll_at_start = 2 * 1000;//2s
            zb_time_t start_awake_time = 30 * 1000;//30s to configure/communicate
        };

        template<poll_ctrl_cfg_t cfg>
        void configure_poll_control(poll_ctrl_basic_t &poll_ctrl_cluster)
        {
            zb_zcl_poll_control_start(0, cfg.ep);
            zb_zcl_poll_controll_register_cb(cfg.callback_on_check_in);

            if constexpr (cfg.sleepy_end_device)
            {
                if constexpr (cfg.start_awake_time)
                {
                    static alarm_ext_16_t g_EnterLowPowerLongPollMode;
                    //we start with 2-sec long poll for the first 30 seconds
                    zb_zdo_pim_set_long_poll_interval(cfg.long_poll_at_start);
                    g_EnterLowPowerLongPollMode.Setup([&poll_ctrl_cluster]{
                            if (poll_ctrl_cluster.long_poll_interval != 0xffffffff)
                            {
                                //printk("on_zigbee_start: long poll set to power save %d ms\r\n", (poll_ctrl_cluster.long_poll_interval * 1000 / 4));
                                zb_zdo_pim_set_long_poll_interval(poll_ctrl_cluster.long_poll_interval * 1000 / 4);
                            }
                        }, cfg.start_awake_time);
                }else
                {
                    if (poll_ctrl_cluster.long_poll_interval != 0xffffffff)
                    {
                        //printk("on_zigbee_start: long poll set to power save %d ms\r\n", (poll_ctrl_cluster.long_poll_interval * 1000 / 4));
                        zb_zdo_pim_set_long_poll_interval(poll_ctrl_cluster.long_poll_interval * 1000 / 4);
                    }
                }
            }
            else
            {
                //printk("on_zigbee_start: long poll set to non-power save\r\n");
                zb_zdo_pim_set_long_poll_interval(cfg.long_poll_at_start);
            }
        }
    }
}
#endif
