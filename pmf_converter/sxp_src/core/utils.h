//============================================================================
// Spin-X Platform
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================

#ifndef PFC_CORE_UTILS_H
#define PFC_CORE_UTILS_H
//----------------------------------------------------------------------------


//============================================================================
// interface
//============================================================================
// external
#include "meta.h"
#include "cstr.h"
namespace pfc
{
// new
// data swapping
template<typename T> PFC_INLINE void swap(T&, T&);
template<typename T> PFC_INLINE void swap(T*, T*, usize_t count_);
template<typename T> PFC_INLINE T swap_bytes(T);
template<typename T> PFC_INLINE void swap_bytes(T*, usize_t count_);
// array construction/destruction
template<typename T> PFC_INLINE void default_construct(T*, usize_t count_);
template<typename T> PFC_INLINE void copy_construct(T*, const T*, usize_t count_);
template<typename T> PFC_INLINE void copy_construct(T*, const T&, usize_t count_);
template<typename T> PFC_INLINE void move_construct(T*, T*, usize_t count_);
template<typename T> PFC_INLINE void destruct(T*, usize_t count_);
template<typename T> PFC_INLINE void reverse_default_construct(T*, usize_t count_);
template<typename T> PFC_INLINE void reverse_copy_construct(T*, const T*, usize_t count_);
template<typename T> PFC_INLINE void reverse_copy_construct(T*, const T&, usize_t count_);
template<typename T> PFC_INLINE void reverse_move_construct(T*, T*, usize_t count_);
template<typename T> PFC_INLINE void reverse_destruct(T*, usize_t count_);
// tokenization
unsigned tokenize_command_line(char *cmd_line_, const char **tokens_, unsigned max_tokens_);
// pair
template<typename, typename> struct pair;
template<typename T, typename U> PFC_INLINE pair<T, U> make_pair(const T&, const U&);
template<typename T, typename U> PFC_INLINE bool operator==(const pair<T, U>&, const pair<T, U>&);
template<typename T, typename U> PFC_INLINE bool operator==(const pair<T, U>&, const T&);
template<typename T, typename U> PFC_INLINE bool operator!=(const pair<T, U>&, const pair<T, U>&);
template<typename T, typename U> PFC_INLINE bool operator!=(const pair<T, U>&, const T&);
// functor
template<typename FS> class functor;
template<class T, class U, typename R> PFC_INLINE functor<R()> make_functor(T&, R(*)(U&));
#define PFC_MAKE_MEM_FUNCTOR(functor_t__, obj__, class__, mem_func__) functor_t__(obj__, functor_t__::call_mem_func<class__, &class__::mem_func__>)
#define PFC_MAKE_CMEM_FUNCTOR(functor_t__, obj__, class__, mem_func__) functor_t__(obj__, functor_t__::call_cmem_func<class__, &class__::mem_func__>)
#define PFC_MAKE_VMEM_FUNCTOR(functor_t__, obj__, class__, mem_func__) functor_t__(obj__, functor_t__::call_vmem_func<class__, &class__::mem_func__>)
#define PFC_MAKE_CVMEM_FUNCTOR(functor_t__, obj__, class__, mem_func__) functor_t__(obj__, functor_t__::call_cvmem_func<class__, &class__::mem_func__>)
//----------------------------------------------------------------------------


//============================================================================
// pair
//============================================================================
template<typename T, typename U>
struct pair
{ PFC_MONO(pair) {PFC_VAR2(first, second);}
  // construction
  PFC_INLINE pair();
  PFC_INLINE pair(const T&, const U&);
  //--------------------------------------------------------------------------

  T first;
  U second;
};
PFC_SET_TYPE_TRAIT_PARTIAL2(typename T, typename U, pair<T, U>, is_type_pod_move, is_type_pod_move<T>::res && is_type_pod_move<U>::res);
//----------------------------------------------------------------------------


//============================================================================
// functor
//============================================================================
// functor takes a function signature as a template argument, e.g.
//   int trunc(float f_)  {return int(f_);}
//   functor<int(float)> f(trunc);
// for member functions use cv-qualifier matching call_*mem_func function, e.g.
//   struct foo {int trunc(float f_) const {return int(f_);}};
//   foo obj;
//   functor<int(float)> f(obj, functor<int(float)>::call_cmem_func<foo, &foo::trunc>);
// and to reduce some redundancy there's helper macro:
//   auto f=PFC_MAKE_CMEM_FUNCTOR(functor<int(float)>, obj, foo, trunc);
// note: function signatures up to 10 arguments are supported
template<typename R>
class functor<R()>
{
public:
  // construction and execution
  PFC_INLINE functor();
  PFC_INLINE functor(R(*)());
  template<class T, class U> PFC_INLINE functor(T&, R(*)(U&));
  inline R operator()() const;
  PFC_INLINE operator bool() const;
  PFC_INLINE void clear();
  //--------------------------------------------------------------------------

  // member function call functions
  template<class T, R(T::*mem_fun)()> static R call_mem_func(T&);
  template<class T, R(T::*mem_fun)() const> static R call_cmem_func(const T&);
  template<class T, R(T::*mem_fun)() volatile> static R call_vmem_func(volatile T&);
  template<class T, R(T::*mem_fun)() const volatile> static R call_cvmem_func(const volatile T&);
  //--------------------------------------------------------------------------

private:
  void *m_this;
  void *m_func;
};
//----------------------------------------------------------------------------

//============================================================================
#include "utils.inl"
} // namespace pfc
#endif
