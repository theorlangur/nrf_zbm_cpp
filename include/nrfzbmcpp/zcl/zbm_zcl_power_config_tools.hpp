#ifndef ZBM_ZCL_POWER_CONFIG_TOOLS_HPP_
#define ZBM_ZCL_POWER_CONFIG_TOOLS_HPP_

#include "../zbm.hpp"
#include "zbm_zcl_power_config.hpp"
#include <zephyr/kernel.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/regulator.h>

namespace zbm
{
    namespace zcl
    {
        template<bool enabled>
        struct reg_raii_t
        {
            reg_raii_t(const struct device *pD):pDev(pD)
            { 
                if constexpr (enabled)
                {
                    if (pDev) regulator_enable(pDev); 
                }
            }
            ~reg_raii_t() 
            { 
                if constexpr (enabled)
                {
                    if (pDev) regulator_disable(pDev); 
                }
            }
        private:
            const struct device *pDev;
        };

        template<bool enabled>
        struct adc_raii_t
        {
            adc_raii_t(const struct adc_dt_spec *_adc_spec, const struct device *_gpio_port, int _pin):
                adc_spec(_adc_spec), gpio_port(_gpio_port), pin(_pin)
            {
                if constexpr(enabled)
                {
                    if (gpio_port)
                        adc_channel_setup_dt(adc_spec);
                }
            }

            ~adc_raii_t()
            {
                if constexpr(enabled)
                {
                    if (gpio_port)
                        gpio_pin_configure(gpio_port, pin, GPIO_DISCONNECTED);
                }
            }

            const struct adc_dt_spec *adc_spec;
            const struct device *gpio_port;
            int pin;
        };

        struct battery_cfg_t
        {
            int battery_adc_channel = 0;
            int32_t maxBatteryVoltage = 1600;//mV
            int32_t minBatteryVoltage = 900;//mV
            bool with_regulators = true;
            bool disconnect_adc = false;
        };

        template<battery_cfg_t cfg, class EP>
        struct battery_measurements_block_t
        {
            static const constexpr auto kAttrBattVoltage = ^^power_cfg_battery_info_t::batt_voltage;
            static const constexpr auto kAttrBattPercentage = ^^power_cfg_battery_info_t::batt_percentage_remaining;
            static const constexpr int32_t g_BatteryVoltageRange = cfg.maxBatteryVoltage - cfg.minBatteryVoltage;//mV
            inline static battery_measurements_block_t *g_Battery = nullptr;

            EP &zb_ep;
            const struct adc_dt_spec *adc_channels;
            const struct device *regulator;
            const struct device *gpio_port;
            int gpio_pin;

            void update()
            {
                reg_raii_t<cfg.with_regulators> batteryRegulator(regulator);
                adc_raii_t<cfg.disconnect_adc> adcReg(&adc_channels[cfg.battery_adc_channel], gpio_port, gpio_pin);
                uint16_t buf;
                struct adc_sequence sequence = {
                    .buffer = &buf,
                    /* buffer size in bytes, not number of samples */
                    .buffer_size = sizeof(buf),
                };
                (void)adc_sequence_init_dt(&adc_channels[cfg.battery_adc_channel], &sequence);

                int err = adc_read_dt(&adc_channels[cfg.battery_adc_channel], &sequence);
                if (err == 0)
                {
                    int32_t batteryVoltage = (int32_t)buf;
                    adc_raw_to_millivolts_dt(&adc_channels[cfg.battery_adc_channel],
                            &batteryVoltage);

                    printk("update_battery_state_zb: volt %d\r\n", batteryVoltage);
                    zb_ep.template set<kAttrBattVoltage>(uint8_t(batteryVoltage / 100));
                    zb_ep.template set<kAttrBattPercentage>(uint8_t((batteryVoltage - cfg.minBatteryVoltage) * 200 / g_BatteryVoltageRange));
                }
            }

            static void update_zb_callback(uint8_t dummy)
            {
                if (g_Battery)
                    g_Battery->update();
            }

            int setup()
            {
                if (g_Battery)//double init not allowed
                    return -1;
                g_Battery = this;

                if (!adc_is_ready_dt(&adc_channels[cfg.battery_adc_channel])) {
                    printk("ADC controller device %s not ready\n", adc_channels[cfg.battery_adc_channel].dev->name);
                    return -1;
                }

                int err = adc_channel_setup_dt(&adc_channels[cfg.battery_adc_channel]);
                if (err < 0) {
                    printk("Could not setup channel #%d (%d)\n", 0, err);
                    return err;
                }
                return 0;
            }
        };

        template<battery_cfg_t cfg, class EP>
        constexpr auto make_battery_measurements_block(EP &e, const struct adc_dt_spec *adc_channels, const struct device *reg = nullptr, const struct device *gpio_port = nullptr, int pin = 0)
        {
            return battery_measurements_block_t<cfg, EP>{e, adc_channels, reg, gpio_port, pin};
        }
    }
}
#endif
