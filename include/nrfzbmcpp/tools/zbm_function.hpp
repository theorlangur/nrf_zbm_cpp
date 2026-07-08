#ifndef ZBM_TOOLS_GENERIC_FUNCTION_HPP_
#define ZBM_TOOLS_GENERIC_FUNCTION_HPP_

#include <memory>
#include <utility>

namespace zbm{
    namespace tools{
        namespace function{
            template<class Sig>
            struct v_table_t;

            template<class R, class... Args>
            struct v_table_t<R(Args...)>
            {
                using Sig = R(*)(Args...);
                using Invoke = R(*)(Args...,void*);

                using Destr = void(*)(void*);
                using Copy = void(*)(void *pDst, const void *pSrc);
                using Move = void(*)(void *pDst, void *pSrc);

                const Destr m_Dtr;
                const Copy m_Copy;
                const Move m_Move;
            };

            template<class R, class... Args>
            R InvokeFunction(Args... args, void *pF)
            {
                using S = v_table_t<R(Args...)>::Sig;
                S *pS = (S*)pF;
                return (*pS)(std::forward<Args>(args)...);
            }

            template<class F, class R, class... Args>
            R InvokeFunctor(Args... args, void *pF)
            {
                F *pS = (F*)pF;
                return (*pS)(std::forward<Args>(args)...);
            }

            template<class R, class... Args>
            const v_table_t<R(Args...)> v_PlainFunc = {
                /*dtr*/ [](void *pF){},
                /*copy*/[](void *_pDst, const void *_pSrc){
                    using S = v_table_t<R(Args...)>::Sig;
                    S *pDst = (S*)_pDst;
                    const S *pSrc = (const S*)_pSrc;
                    *pDst = *pSrc;
                },
                /*move*/[](void *_pDst, void *_pSrc){
                    using S = v_table_t<R(Args...)>::Sig;
                    S *pDst = (S*)_pDst;
                    S *pSrc = (S*)_pSrc;
                    *pDst = *pSrc;
                    *pSrc = nullptr;
                },
            };

            template<class F, class R, class... Args>
            const v_table_t<R(Args...)> v_Functor = {
                /*dtr*/ [](void *pF){ std::destroy_at((F*)pF); },
                /*copy*/[](void *_pDst, const void *_pSrc){
                    F *pDst = (F*)_pDst;
                    const F *pSrc = (const F*)_pSrc;
                    pDst = new (pDst) F(*pSrc);
                },
                /*move*/[](void *_pDst, void *_pSrc){
                    F *pDst = (F*)_pDst;
                    F *pSrc = (F*)_pSrc;
                    pDst = new (pDst) F(std::move(*pSrc));
                },
            };

            template<class F, class R, class... Args>
            const v_table_t<R(Args...)> v_FunctorCopyOnly = {
                /*dtr*/ [](void *pF){ std::destroy_at((F*)pF); },
                /*copy*/[](void *_pDst, const void *_pSrc){
                    F *pDst = (F*)_pDst;
                    const F *pSrc = (const F*)_pSrc;
                    pDst = new (pDst) F(*pSrc);
                },
                /*move*/nullptr
            };

            template<class F, class R, class... Args>
            const v_table_t<R(Args...)> v_FunctorMoveOnly = {
                /*dtr*/ [](void *pF){ std::destroy_at((F*)pF); },
                /*copy*/nullptr,
                /*move*/[](void *_pDst, void *_pSrc){
                    F *pDst = (F*)_pDst;
                    F *pSrc = (F*)_pSrc;
                    pDst = new (pDst) F(std::move(*pSrc));
                },
            };

            template<class F, class R, class... Args>
            const v_table_t<R(Args...)> v_FunctorImmovable = {
                /*dtr*/ [](void *pF){ std::destroy_at((F*)pF); },
                /*copy*/nullptr,
                /*move*/nullptr
            };
        }

        template<size_t Sz, class Sig>
        class fixed_function_t;

        template<size_t Sz, class R, class...Args>
        class fixed_function_t<Sz, R(Args...)>
        {
            using v_table_type = function::v_table_t<R(Args...)>;
            using invoke_t = typename v_table_type::Invoke;

            template<class F>
            static const v_table_type* GetVTableFor()
            {
                constexpr bool copy_ok = std::is_copy_constructible_v<F>;
                constexpr bool move_ok = std::is_move_constructible_v<F>;
                if constexpr (copy_ok && move_ok)
                    return &function::v_Functor<F, R, Args...>;
                else if constexpr (copy_ok)
                    return &function::v_FunctorCopyOnly<F, R, Args...>;
                else if constexpr (move_ok)
                    return &function::v_FunctorMoveOnly<F, R, Args...>;
                else
                    return &function::v_FunctorImmovable<F, R, Args...>;
            }

            public:
            constexpr fixed_function_t() = default;

            constexpr fixed_function_t(R(*pF)(Args...)):
                m_pTable(&function::v_PlainFunc<R,Args...>)
                ,m_pInvoker(&function::InvokeFunction<R, Args...>)
            {
                static_assert(sizeof(pF) <= Sz);
                *((typename v_table_type::Sig*)m_Storage) = pF;
            }

            template<class F>
            constexpr fixed_function_t(F &&f):
                m_pTable(GetVTableFor<F>())
                ,m_pInvoker(&function::InvokeFunctor<F, R, Args...>)
            {
                static_assert(sizeof(F) <= Sz);
                static_assert(std::is_move_constructible_v<F>);
                m_pTable->m_Move(m_Storage, &f);
            }

            fixed_function_t& operator=(const fixed_function_t &src)
            {
                m_pTable = src.m_pTable;
                m_pInvoker = src.m_pInvoker;
                m_pTable->m_Copy(m_Storage, src.m_Storage);
                return *this;
            }

            fixed_function_t& operator=(fixed_function_t &&src)
            {
                m_pTable = src.m_pTable;
                m_pInvoker = src.m_pInvoker;
                m_pTable->m_Move(m_Storage, src.m_Storage);
                return *this;
            }

            operator bool() const { return m_pTable != nullptr; }

            R operator()(Args... args)
            {
                return m_pInvoker(std::forward<Args>(args)..., m_Storage);
            }

        private:
            alignas(16) std::byte m_Storage[Sz]{};
            const v_table_type *m_pTable = nullptr;
            invoke_t m_pInvoker = nullptr;
        };

        template<class Sig>
        using generic_callback_t = fixed_function_t<48, Sig>;
    }
}

#endif
