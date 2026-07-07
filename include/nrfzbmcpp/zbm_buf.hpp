#ifndef ZBM_BUF_HPP_
#define ZBM_BUF_HPP_

#include "zbm_types.hpp"

namespace zbm
{
    struct buf_ptr_base_t
    {
        zb_bufid_t release()
        {
            zb_bufid_t r = bufid; 
            bufid = 0;
            return r;
        }

        zb_uint_t size() const { return bufid ? zb_buf_len(bufid) : 0; }

        void* raw() const { return bufid ? zb_buf_begin(bufid) : nullptr; }

        template<class T>
        T* as() const { return reinterpret_cast<T*>(raw()); }

        template<class T>
        T* param() const { return bufid ? reinterpret_cast<T*>(zb_buf_get_tail_func(bufid, sizeof(T))) : nullptr; }

        zb_bufid_t bufid = 0;
    };

    struct buf_ptr_t: buf_ptr_base_t
    {
        ~buf_ptr_t() { reset(); }

        void reset(zb_bufid_t b = 0)
        {
            if (bufid) zb_buf_free(bufid);
            bufid = b;
        }
    };

    struct buf_view_ptr_t: buf_ptr_base_t
    {
    };

}
#endif

