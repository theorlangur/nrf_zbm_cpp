#ifndef ZB_META_OUT_CMD_HPP_
#define ZB_META_OUT_CMD_HPP_

#include "lib_object_pool.hpp"
#include "zbm_annotations.hpp"
#include <format>

namespace zbm
{

    template<class Func> requires std::is_function_v<Func>
    struct cmd_out_t
    {
        //std::meta::info -> parameter types
        static constexpr auto g_Params = []() consteval{
            auto params = std::meta::parameters_of(^^Func);
            return std::define_static_array(params);
        }();
        static constexpr size_t g_ParamsTotalSize = []() consteval -> size_t{
            size_t r = 0;
            for(auto p : g_Params)
                r += std::meta::size_of(p);
            return r;
        }();

        static_assert(g_Params.size() <= 10, "Too many arguments. Max 10 is supported");
    };
}

#endif
