#ifndef ZB_META_HANDLERS_HPP_
#define ZB_META_HANDLERS_HPP_

extern "C" {
#include <zboss_api.h>
}
#include "zbm_buf.hpp"
#include "zbm_str.hpp"
#include <meta>

namespace zbm
{
    using dev_callback_handler_t = void(*)(zb_zcl_device_callback_param_t *);
    using err_callback_handler_t = void(*)(int err);
    using set_attr_value_handler_t = void (*)(zb_zcl_set_attr_value_param_t *p, zb_zcl_device_callback_param_t *pDevCBParam);
    struct dev_cb_handlers_desc_t
    {
        dev_callback_handler_t default_handler = nullptr;
        err_callback_handler_t error_handler = nullptr;
    };

    struct cb_handler_t
    {
        enum zb_zcl_device_callback_id_e id;
        std::meta::info target;
        std::meta::info handler;
    };
    
    template<dev_cb_handlers_desc_t generic={}, cb_handler_t... handlers>
    void tpl_device_cb(zb_bufid_t bufid)
    {
        buf_view_ptr_t bv{bufid};
        auto *pDevParam = bv.param<zb_zcl_device_callback_param_t>();
        if (!pDevParam)
        {
            if (generic.error_handler)
                generic.error_handler(-1);
            return;
        }
        pDevParam->status = RET_OK;
        switch(pDevParam->device_cb_id)
        {
            case ZB_ZCL_SET_ATTR_VALUE_CB_ID:
            break;
        }

        if constexpr (generic.default_handler)
            generic.default_handler(pDevParam);
    }
}

#endif
