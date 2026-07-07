#ifndef ZB_META_MISC_HELPERS_HPP_
#define ZB_META_MISC_HELPERS_HPP_

#include <chrono>
#include "zbm_tools_formatter.hpp"

namespace zbm
{
    namespace tools
    {
        using duration_ms_t = std::chrono::duration<int, std::milli>;
        inline static constexpr const duration_ms_t kForever = duration_ms_t(-1);

        template<class CB>
        struct scope_exit_t
        {
            scope_exit_t(CB &&cb):m_CB(std::move(cb)){}
            ~scope_exit_t(){ m_CB(); }
            private:
            CB m_CB;
        };

        template<class... O>
        struct overloaded:O...
        {
            using O::operator()...;
        };
        template<class... O>
        overloaded(O... o)->overloaded<O...>;

#if defined(NDEBUG) && !defined(FORCE_FMT)
#define FMT_PRINT(fmt,...) {}
#define FMT_PRINTLN(fmt,...) {}
#else
#ifndef PRINTF_FUNC
#define PRINTF_FUNC(...) printf(__VA_ARGS__)
#endif
#define FMT_PRINT(fmt,...) { char buf[256]; tools::format_to_silent(tools::BufferFormatter(buf), fmt __VA_OPT__(,) __VA_ARGS__); PRINTF_FUNC("%s", buf); }
#define FMT_PRINTLN(fmt,...) { char buf[256]; tools::format_to_silent(tools::BufferFormatter(buf), fmt "\n" __VA_OPT__(,) __VA_ARGS__); PRINTF_FUNC("%s", buf); }
#endif
    }
}

#endif
