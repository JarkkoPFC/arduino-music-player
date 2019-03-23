//============================================================================
// Spin-X Platform
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================


//============================================================================
// swap
//============================================================================
namespace priv
{
  template<typename T>
  static PFC_INLINE void swap_impl(T &v0_, T &v1_, meta_case<0> is_type_non_class_)
  {
    // swap values
    T t=v0_;
    v0_=v1_;
    v1_=t;
  }
  //----

  template<typename T>
  static PFC_INLINE void swap_impl(T &v0_, T &v1_, meta_case<1> is_type_pod_move_)
  {
    // swap values
    char tmp[sizeof(T)];
    mem_copy(&tmp, &v0_, sizeof(T));
    mem_copy(&v0_, &v1_, sizeof(T));
    mem_copy(&v1_, &tmp, sizeof(T));
  }
  //----

  template<typename T>
  static PFC_INLINE void swap_impl(T &v0_, T &v1_, meta_case<2> default_)
  {
    // swap values
    T v=v0_;
    v0_.~T();
    PFC_PNEW(&v0_)T(v1_);
    v1_.~T();
    PFC_PNEW(&v1_)T(v);
  }
} // namespace priv
//----

template<typename T>
PFC_INLINE void swap(T &v0_, T &v1_)
{
  priv::swap_impl(v0_, v1_, meta_case<!is_type_class<T>::res?0:is_type_pod_move<T>::res?1:2>());
}
//----

template<typename T>
PFC_INLINE void swap(T *s0_, T *s1_, usize_t count_)
{
  // swap values between arrays
  if(count_)
  {
    T *end=s0_+count_;
    do
    {
      priv::swap_impl(*s0_++, *s1_++, meta_case<!is_type_class<T>::res?0:is_type_pod_move<T>::res?1:2>());
    } while(s0_<end);
  }
}
//----------------------------------------------------------------------------


//============================================================================
// swap_bytes
//============================================================================
namespace priv
{
  static PFC_INLINE uint8 swap_bytes(const void *p_, meta_int<1>)
  {
    return *reinterpret_cast<const uint8*>(p_);
  }
  //----

  static PFC_INLINE uint16 swap_bytes(const void *p_, meta_int<2>)
  {
#ifdef PFC_INTRINSIC_BSWAP16
    uint16 v;
    PFC_INTRINSIC_BSWAP16(v, p_);
    return v;
#else
    uint16 v=*reinterpret_cast<const uint16*>(p_);
    return v<<8|v>>8;
#endif
  }
  //----

  static PFC_INLINE uint32 swap_bytes(const void *p_, meta_int<4>)
  {
#ifdef PFC_INTRINSIC_BSWAP32
    uint32 v;
    PFC_INTRINSIC_BSWAP32(v, p_);
    return v;
#else
    uint32 v=*reinterpret_cast<const uint32*>(p_);
    return (v<<24)|(v<<8&0xff0000)|(v>>8&0xff00)|(v>>24);
#endif
  }
  //----

  static PFC_INLINE uint64 swap_bytes(const void *p_, meta_int<8>)
  {
#ifdef PFC_INTRINSIC_BSWAP64
    uint64 v;
    PFC_INTRINSIC_BSWAP64(v, p_);
    return v;
#else
    uint64 v=*reinterpret_cast<const uint64*>(p_);
    return  (v<<56)|(v>>56)|(v<<40&PFC_CONST_UINT64(0xff000000000000))
           |(v>>40&0xff00)|(v<<24&PFC_CONST_UINT64(0xff0000000000))
           |(v>>24&0xff0000)|(v<<8&PFC_CONST_UINT64(0xff00000000))
           |(v>>8&0xff000000);
#endif
  }
  //----

  static PFC_INLINE uint128 swap_bytes(const void *p_, meta_int<16>)
  {
    uint128 v;
#ifdef PFC_INTRINSIC_BSWAP64
    PFC_INTRINSIC_BSWAP64(v.lo, &reinterpret_cast<const uint128*>(p_)->hi);
    PFC_INTRINSIC_BSWAP64(v.hi, &reinterpret_cast<const uint128*>(p_)->lo);
#else
    uint64 vt=reinterpret_cast<const uint128*>(p_)->hi;
    v.lo= (vt<<56)|(vt>>56)|(vt<<40&PFC_CONST_UINT64(0xff000000000000))
         |(vt>>40&0xff00)|(vt<<24&PFC_CONST_UINT64(0xff0000000000))
         |(vt>>24&0xff0000)|(vt<<8&PFC_CONST_UINT64(0xff00000000))
         |(vt>>8&0xff000000);
    vt=reinterpret_cast<const uint128*>(p_)->lo;
    v.hi= (vt<<56)|(vt>>56)|(vt<<40&PFC_CONST_UINT64(0xff000000000000))
         |(vt>>40&0xff00)|(vt<<24&PFC_CONST_UINT64(0xff0000000000))
         |(vt>>24&0xff0000)|(vt<<8&PFC_CONST_UINT64(0xff00000000))
         |(vt>>8&0xff000000);
#endif
    return v;
  }
  //----

  static PFC_INLINE void swap_bytes(void *p_, usize_t count_, meta_int<1>)
  {
    return;
  }
  //----

  static PFC_INLINE void swap_bytes(void *p_, usize_t count_, meta_int<2>)
  {
    // swap bytes for an array of values
    uint16 *p=(uint16*)p_, *end=p+count_;
    if(count_)
      do
      {
#ifdef PFC_INTRINSIC_BSWAP16
        PFC_INTRINSIC_BSWAP16(*p, p);
#else
        uint16 v=*p;
        *p=v<<8 | v>>8;
#endif
        ++p;
      } while(p!=end);
  }
  //----

  static PFC_INLINE void swap_bytes(void *p_, usize_t count_, meta_int<4>)
  {
    // swap bytes for an array of values
    uint32 *p=(uint32*)p_, *end=p+count_;
    if(count_)
      do
      {
#ifdef PFC_INTRINSIC_BSWAP32
        PFC_INTRINSIC_BSWAP32(*p, p);
#else
        uint32 v=*p;
        *p=(v<<24) | (v<<8&0xff0000) | (v>>8&0xff00) | (v>>24);
#endif
        ++p;
      } while(p!=end);
  }
  //----

  static PFC_INLINE void swap_bytes(void *p_, usize_t count_, meta_int<8>)
  {
    // swap bytes for an array of values
    uint64 *p=(uint64*)p_, *end=p+count_;
    if(count_)
      do
      {
#ifdef PFC_INTRINSIC_BSWAP64
        PFC_INTRINSIC_BSWAP64(*p, p);
#else
        uint64 v=*p;
        *p= (v<<56)|(v>>56)|(v<<40&PFC_CONST_UINT64(0xff000000000000))
           |(v>>40&0xff00)|(v<<24&PFC_CONST_UINT64(0xff0000000000))
           |(v>>24&0xff0000)|(v<<8&PFC_CONST_UINT64(0xff00000000))
           |(v>>8&0xff000000);
#endif
        ++p;
      } while(p!=end);
  }
  //----

  static PFC_INLINE void swap_bytes(void *p_, usize_t count_, meta_int<16>)
  {
    // swap bytes for an array of values
    uint128 *p=(uint128*)p_, *end=p+count_;
    if(count_)
      do
      {
#ifdef PFC_INTRINSIC_BSWAP64
        uint64 lo;
        PFC_INTRINSIC_BSWAP64(lo, &p->lo);
        PFC_INTRINSIC_BSWAP64(p->lo, &p->hi);
        p->hi=lo;
#else
        uint64 lo=p->lo;
        lo= (lo<<56)|(lo>>56)|(lo<<40&PFC_CONST_UINT64(0xff000000000000))
           |(lo>>40&0xff00)|(lo<<24&PFC_CONST_UINT64(0xff0000000000))
           |(lo>>24&0xff0000)|(lo<<8&PFC_CONST_UINT64(0xff00000000))
           |(lo>>8&0xff000000);
        uint64 hi=p->hi;
        p->hi=lo;
        hi= (hi<<56)|(hi>>56)|(hi<<40&PFC_CONST_UINT64(0xff000000000000))
           |(hi>>40&0xff00)|(hi<<24&PFC_CONST_UINT64(0xff0000000000))
           |(hi>>24&0xff0000)|(hi<<8&PFC_CONST_UINT64(0xff00000000))
           |(hi>>8&0xff000000);
        p->lo=hi;
#endif
        ++p;
      } while(p!=end);
  }
} // namespace priv
//----------------------------------------------------------------------------

template<typename T>
PFC_INLINE T swap_bytes(T v_)
{
  PFC_CTF_ASSERT_MSG(!is_type_class<T>::res, unable_to_swap_bytes_of_a_class_type);
  return raw_cast<T>(priv::swap_bytes(&v_, meta_int<sizeof(T)>()));
}
//----

template<typename T>
PFC_INLINE void swap_bytes(T *a_, usize_t count_)
{
  PFC_CTF_ASSERT_MSG(!is_type_class<T>::res, unable_to_swap_bytes_of_a_class_type);
  priv::swap_bytes(a_, count_, meta_int<sizeof(T)>());
}
//----------------------------------------------------------------------------


//============================================================================
// array construction/destruction
//============================================================================
namespace priv
{
  //==========================================================================
  // default_construct
  //==========================================================================
  template<typename T>
  PFC_INLINE void default_construct(T *dst_, usize_t count_, meta_bool<true> is_type_pod_def_ctor_)
  {
    mem_zero(dst_, sizeof(T)*count_);
  }
  //----

  template<typename T>
  PFC_INLINE void default_construct(T *dst_, usize_t count_, meta_bool<false> is_type_pod_def_ctor_)
  {
    // default construct an array of values
    if(count_)
    {
      eh_array_dtor<T> v(dst_, dst_);
      T *end=dst_+count_;
      do
      {
        PFC_PNEW(v.dst)T;
      } while(++v.dst!=end);
      v.begin=0;
    }
  }
  //--------------------------------------------------------------------------

  //==========================================================================
  // copy_construct
  //==========================================================================
  template<typename T>
  PFC_INLINE void copy_construct(T *dst_, const T *src_, usize_t count_, meta_bool<true> is_type_pod_copy_)
  {
    mem_copy(dst_, src_, sizeof(T)*count_);
  }
  //----

  template<typename T>
  PFC_INLINE void copy_construct(T *dst_, const T *src_, usize_t count_, meta_bool<false> is_type_pod_copy_)
  {
    // copy construct an array of values from an array
    if(count_)
    {
      eh_array_dtor<T> v(dst_, dst_);
      T *end=dst_+count_;
      do
      {
        PFC_PNEW(v.dst)T(*src_++);
      } while(++v.dst!=end);
      v.begin=0;
    }
  }
  //----

  template<typename T>
  PFC_INLINE void copy_construct(T *dst_, const T &v_, usize_t count_, meta_bool<true> is_type_pod_copy_)
  {
    // copy construct an array of values from an value
    if(count_)
    {
      T *end=dst_+count_;
      do
      {
        PFC_PNEW(dst_)T(v_);
      } while(++dst_!=end);
    }
  }
  //----

  template<typename T>
  PFC_INLINE void copy_construct(T *dst_, const T &v_, usize_t count_, meta_bool<false> is_type_pod_copy_)
  {
    // copy construct an array of values from an value
    if(count_)
    {
      eh_array_dtor<T> v(dst_, dst_);
      T *end=dst_+count_;
      do
      {
        PFC_PNEW(v.dst)T(v_);
      } while(++v.dst!=end);
      v.begin=0;
    }
  }
  //--------------------------------------------------------------------------

  //==========================================================================
  // move_construct
  //==========================================================================
  template<typename T>
  PFC_INLINE void move_construct(T *dst_, T *src_, usize_t count_, meta_bool<true> is_type_pod_move_)
  {
    mem_move(dst_, src_, sizeof(T)*count_);
  }
  //----

  template<typename T>
  PFC_INLINE void move_construct(T *dst_, T *src_, usize_t count_, meta_bool<false> is_type_pod_move_)
  {
    // move construct an array of values
    if(count_)
    {
      if(dst_<src_)
      {
        // move items starting from the begin of the array
        T *end=src_+count_;
        do
        {
          PFC_PNEW(dst_++)T(*src_);
          src_->~T();
        } while(++src_!=end);
      }
      else
      {
        // move items starting from the end of the array
        T *end=src_;
        dst_+=count_;
        src_+=count_;
        do
        {
          PFC_PNEW(--dst_)T(*--src_);
          src_->~T();
        } while(src_!=end);
      }
    }
  }
  //--------------------------------------------------------------------------

  //==========================================================================
  // destruct
  //==========================================================================
  PFC_INLINE void destruct(void*, usize_t, meta_bool<true> is_type_pod_dtor_)
  {
  }
  //----

  template<typename T>
  PFC_INLINE void destruct(T *dst_, usize_t count_, meta_bool<false> is_type_pod_dtor_)
  {
    // destruct an array of values
    if(count_)
    {
      T *end=dst_+count_;
      do
      {
        dst_->~T();
      } while(++dst_!=end);
    }
  }
  //--------------------------------------------------------------------------

  //==========================================================================
  // reverse_default_construct
  //==========================================================================
  template<typename T>
  PFC_INLINE void reverse_default_construct(T *dst_, usize_t count_, meta_bool<true> is_type_pod_def_ctor_)
  {
    mem_zero(dst_, sizeof(T)*count_);
  }
  //----

  template<typename T>
  PFC_INLINE void reverse_default_construct(T *dst_, usize_t count_, meta_bool<false> is_type_pod_def_ctor_)
  {
    // default construct an array of values in reverse order
    if(count_)
    {
      dst_+=count_-1;
      eh_array_dtor<T> v(dst_, dst_);
      T *end=dst_-count_;
      do
      {
        PFC_PNEW(v.dst)T;
      } while(--v.dst!=end);
      v.begin=0;
    }
  }
  //--------------------------------------------------------------------------

  //==========================================================================
  // reverse_copy_construct
  //==========================================================================
  template<typename T>
  PFC_INLINE void reverse_copy_construct(T *dst_, const T *src_, usize_t count_, meta_bool<true> is_type_pod_copy_)
  {
    // copy construct an array of values from an array in reverse order
    if(count_)
    {
      dst_+=count_-1;
      T *end=dst_-count_;
      do
      {
        PFC_PNEW(dst_)T(*src_++);
      } while(--dst_!=end);
    }
  }
  //----

  template<typename T>
  PFC_INLINE void reverse_copy_construct(T *dst_, const T *src_, usize_t count_, meta_bool<false> is_type_pod_copy_)
  {
    // copy construct an array of values from an array in reverse order
    if(count_)
    {
      dst_+=count_-1;
      eh_array_dtor<T> v(dst_, dst_);
      T *end=dst_-count_;
      do
      {
        PFC_PNEW(v.dst)T(*src_++);
      } while(--v.dst!=end);
      v.begin=0;
    }
  }
  //----

  template<typename T>
  PFC_INLINE void reverse_copy_construct(T *dst_, const T &v_, usize_t count_, meta_bool<true> is_type_pod_copy_)
  {
    // copy construct an array of value from an value in reverse order
    if(count_)
    {
      dst_+=count_-1;
      T *end=dst_-count_;
      do
      {
        PFC_PNEW(dst_)T(v_);
      } while(--dst_!=end);
    }
  }
  //----

  template<typename T>
  PFC_INLINE void reverse_copy_construct(T *dst_, const T &v_, usize_t count_, meta_bool<false> is_type_pod_copy_)
  {
    // copy construct an array of value from an value in reverse order
    if(count_)
    {
      dst_+=count_-1;
      eh_array_dtor<T> v(dst_, dst_);
      T *end=dst_-count_;
      do
      {
        PFC_PNEW(v.dst)T(v_);
      } while(--v.dst!=end);
      v.begin=0;
    }
  }
  //--------------------------------------------------------------------------

  //==========================================================================
  // reverse_move_construct
  //==========================================================================
  template<typename T>
  PFC_INLINE void reverse_move_construct(T *dst_, T *src_, usize_t count_, meta_bool<true> is_type_pod_move_)
  {
    // move construct an array of values in reverse order
    if(count_)
    {
      dst_+=count_;
      T *end=src_+count_;
      do
      {
        PFC_PNEW(--dst_)T(*src_);
      } while(++src_!=end);
    }
  }
  //----

  template<typename T>
  PFC_INLINE void reverse_move_construct(T *dst_, T *src_, usize_t count_, meta_bool<false> is_type_pod_move_)
  {
    // move construct an array of values in reverse order
    if(count_)
    {
      dst_+=count_;
      T *end=src_+count_;
      do
      {
        PFC_PNEW(--dst_)T(*src_);
        src_->~T();
      } while(++src_!=end);
    }
  }
  //--------------------------------------------------------------------------

  //==========================================================================
  // reverse_destruct
  //==========================================================================
  PFC_INLINE void reverse_destruct(void*, usize_t, meta_bool<true> is_type_pod_dtor_)
  {
  }
  //----

  template<typename T>
  PFC_INLINE void reverse_destruct(T *dst_, usize_t count_, meta_bool<false> is_type_pod_dtor_)
  {
    // destruct an array of values in reverse order
    if(count_)
    {
      T *end=dst_;
      dst_+=count_;
      do
      {
        (--dst_)->~T();
      } while(dst_!=end);
    }
  }
  //--------------------------------------------------------------------------
} // namespace priv
//----------------------------------------------------------------------------

template<typename T>
PFC_INLINE void default_construct(T *dst_, usize_t count_)
{
  // default construct an array
  priv::default_construct(dst_, count_, meta_bool<is_type_pod_def_ctor<T>::res>());
}
//----

template<typename T>
PFC_INLINE void copy_construct(T *dst_, const T *src_, usize_t count_)
{
  // copy construct an array from an array
  PFC_ASSERT_PEDANTIC_MSG(dst_+count_<=src_ || src_+count_<=dst_, ("Memory regions may not overlap\r\n"));
  priv::copy_construct(dst_, src_, count_, meta_bool<is_type_pod_copy<T>::res>());
}
//----

template<typename T>
PFC_INLINE void copy_construct(T *dst_, const T &v_, usize_t count_)
{
  // copy construct an array from a value
  PFC_ASSERT_PEDANTIC_MSG(dst_+count_<=&v_ || dst_>&v_, ("Target array overwrites the source value\r\n"));
  if(count_)
    priv::copy_construct(dst_, v_, count_, meta_bool<is_type_pod_copy<T>::res>());
}
//----

template<typename T>
PFC_INLINE void move_construct(T *dst_, T *src_, usize_t count_)
{
  // move construct an array
  priv::move_construct(dst_, src_, count_, meta_bool<is_type_pod_move<T>::res>());
}
//----

template<typename T>
PFC_INLINE void destruct(T *dst_, usize_t count_)
{
  // destruct an array
  priv::destruct(dst_, count_, meta_bool<is_type_pod_dtor<T>::res>());
}
//----------------------------------------------------------------------------


template<typename T>
PFC_INLINE void reverse_default_construct(T *dst_, usize_t count_)
{
  // default construct an array in reverse order
  priv::reverse_default_construct(dst_, count_, meta_bool<is_type_pod_def_ctor<T>::res>());
}
//----

template<typename T>
PFC_INLINE void reverse_copy_construct(T *dst_, const T *src_, usize_t count_)
{
  // copy construct an array from an array in reverse order
  PFC_ASSERT_PEDANTIC_MSG(dst_+count_<=src_ || src_+count_<=dst_, ("Memory regions may not overlap\r\n"));
  priv::reverse_copy_construct(dst_, src_, count_, meta_bool<is_type_pod_copy<T>::res>());
}
//----

template<typename T>
PFC_INLINE void reverse_copy_construct(T *dst_, const T &v_, usize_t count_)
{
  // copy construct an array from a value in reverse order
  PFC_ASSERT_PEDANTIC_MSG(dst_+count_<=&v_ || dst_>&v_, ("Target array overwrites the source value\r\n"));
  if(count_)
    priv::reverse_copy_construct(dst_, v_, count_, meta_bool<is_type_pod_copy<T>::res>());
}
//----

template<typename T>
PFC_INLINE void reverse_move_construct(T *dst_, T *src_, usize_t count_)
{
  // move construct an array in reverse order
  PFC_ASSERT_PEDANTIC_MSG(dst_+count_<=src_ || src_+count_<=dst_, ("Memory regions may not overlap\r\n"));
  priv::reverse_move_construct(dst_, src_, count_, meta_bool<is_type_pod_move<T>::res>());
}
//----

template<typename T>
PFC_INLINE void reverse_destruct(T *dst_, usize_t count_)
{
  // destruct an array in reverse order
  priv::reverse_destruct(dst_, count_, meta_bool<is_type_pod_dtor<T>::res>());
}
//----------------------------------------------------------------------------


//============================================================================
// pair
//============================================================================
template<typename T, typename U>
pair<T, U>::pair()
{
}
//----

template<typename T, typename U>
pair<T, U>::pair(const T &first_, const U &second_)
  :first(first_)
  ,second(second_)
{
}
//----------------------------------------------------------------------------

template<typename T, typename U>
PFC_INLINE pair<T, U> make_pair(const T &p0_, const U &p1_)
{
  return pair<T, U>(p0_, p1_);
}
//----

template<typename T, typename U>
PFC_INLINE bool operator==(const pair<T, U> &p0_, const pair<T, U> &p1_)
{
  return p0_.first==p1_.first && p0_.second==p1_.second;
}
//----

template<typename T, typename U>
PFC_INLINE bool operator==(const pair<T, U> &p_, const T &v_)
{
  return p_.first==v_;
}
//----

template<typename T, typename U>
PFC_INLINE bool operator!=(const pair<T, U> &p0_, const pair<T, U> &p1_)
{
  return p0_.first!=p1_.first || p0_.second!=p1_.second;
}
//----

template<typename T, typename U>
PFC_INLINE bool operator!=(const pair<T, U> &p_, const T &v_)
{
  return p_.first!=v_;
}
//----------------------------------------------------------------------------


//============================================================================
// functor
//============================================================================
template<typename R>
functor<R()>::functor()
  :m_this(0)
  ,m_func(0)
{
}
//----

template<typename R>
functor<R()>::functor(R(*func_)())
  :m_this(0)
  ,m_func((void*)func_)
{
}
//----

template<typename R>
template<class T, class U>
functor<R()>::functor(T &this_, R(*func_)(U&))
  :m_this((void*)static_cast<U*>(&this_))
  ,m_func((void*)func_)
{
  PFC_CTF_ASSERT_MSG((is_type_derived<T, U>::res), object_does_not_have_compatible_type_with_the_function_signature);
}
//----

template<typename R>
R functor<R()>::operator()() const
{
  // call member/free function
  PFC_ASSERT_PEDANTIC(m_func!=0);
  return m_this?(*(R(*)(void*))m_func)(m_this):(*(R(*)())m_func)();
}
//----

template<typename R>
functor<R()>::operator bool() const
{
  return m_func!=0;
}
//----

template<typename R>
void functor<R()>::clear()
{
  m_func=0;
}
//----------------------------------------------------------------------------

template<typename R>
template<class T, R(T::*mem_fun)()>
R functor<R()>::call_mem_func(T &this_)
{
  return (this_.*mem_fun)();
}
//----

template<typename R>
template<class T, R(T::*mem_fun)() const>
R functor<R()>::call_cmem_func(const T &this_)
{
  return (this_.*mem_fun)();
}
//----

template<typename R>
template<class T, R(T::*mem_fun)() volatile>
R functor<R()>::call_vmem_func(volatile T &this_)
{
  return (this_.*mem_fun)();
}
//----

template<typename R>
template<class T, R(T::*mem_fun)() const volatile>
R functor<R()>::call_cvmem_func(const volatile T &this_)
{
  return (this_.*mem_fun)();
}
//----------------------------------------------------------------------------

template<class T, class U, typename R>
PFC_INLINE functor<R()> make_functor(T &v_, R(*func_)(U&))
{
  return functor<R()>(v_, func_);
}
//----------------------------------------------------------------------------

#define PFC_FUNCTOR_TMPL()\
  template<typename R, PFC_FUNCTOR_TEMPLATE_ARG_LIST>\
  class functor<R(PFC_FUNCTOR_TYPE_LIST)>\
  {\
  public:\
    PFC_INLINE functor()                                                                           :m_this(0), m_func(0) {}\
    PFC_INLINE functor(R(*func_)(PFC_FUNCTOR_TYPE_LIST))                                           :m_this(0), m_func((void*)func_) {}\
    template<class T, class U> PFC_INLINE functor(T &this_, R(*func_)(U&, PFC_FUNCTOR_TYPE_LIST))  :m_this((void*)static_cast<U*>(&this_)), m_func((void*)func_) {PFC_CTF_ASSERT_MSG((is_type_derived<T, U>::res), object_does_not_have_compatible_type_with_the_function_signature);}\
    inline R operator()(PFC_FUNCTOR_PROTO_ARG_LIST) const                                          {PFC_ASSERT_PEDANTIC(m_func!=0); return m_this?(*(R(*)(void*, PFC_FUNCTOR_TYPE_LIST))m_func)(m_this, PFC_FUNCTOR_ARG_LIST):(*(R(*)(PFC_FUNCTOR_TYPE_LIST))m_func)(PFC_FUNCTOR_ARG_LIST);}\
    PFC_INLINE operator bool() const                                                               {return m_func!=0;}\
    PFC_INLINE void clear()                                                                        {m_func=0;}\
    template<class T, R(T::*mem_fun)(PFC_FUNCTOR_TYPE_LIST)> static R call_mem_func(T &this_, PFC_FUNCTOR_PROTO_ARG_LIST) {return (this_.*mem_fun)(PFC_FUNCTOR_ARG_LIST);}\
    template<class T, R(T::*mem_fun)(PFC_FUNCTOR_TYPE_LIST) const> static R call_cmem_func(const T &this_, PFC_FUNCTOR_PROTO_ARG_LIST) {return (this_.*mem_fun)(PFC_FUNCTOR_ARG_LIST);}\
    template<class T, R(T::*mem_fun)(PFC_FUNCTOR_TYPE_LIST) volatile> static R call_vmem_func(volatile T &this_, PFC_FUNCTOR_PROTO_ARG_LIST) {return (this_.*mem_fun)(PFC_FUNCTOR_ARG_LIST);}\
    template<class T, R(T::*mem_fun)(PFC_FUNCTOR_TYPE_LIST) const volatile> static R call_cvmem_func(const volatile T &this_, PFC_FUNCTOR_PROTO_ARG_LIST) {return (this_.*mem_fun)(PFC_FUNCTOR_ARG_LIST);}\
  private:\
    void *m_this;\
    void *m_func;\
  };\
  template<class T, class U, typename R, PFC_FUNCTOR_TEMPLATE_ARG_LIST> functor<R(PFC_FUNCTOR_TYPE_LIST)> make_functor(T &v_, R(*func_)(U&, PFC_FUNCTOR_TYPE_LIST)) {return functor<R(PFC_FUNCTOR_TYPE_LIST)>(v_, func_);}
//----

// functor 1 argument implementation
#define PFC_FUNCTOR_TEMPLATE_ARG_LIST typename A0
#define PFC_FUNCTOR_TYPE_LIST A0
#define PFC_FUNCTOR_PROTO_ARG_LIST A0 a0_
#define PFC_FUNCTOR_ARG_LIST a0_
PFC_FUNCTOR_TMPL();
#undef PFC_FUNCTOR_TEMPLATE_ARG_LIST
#undef PFC_FUNCTOR_TYPE_LIST
#undef PFC_FUNCTOR_PROTO_ARG_LIST
#undef PFC_FUNCTOR_ARG_LIST
// functor 2 argument implementation
#define PFC_FUNCTOR_TEMPLATE_ARG_LIST typename A0, typename A1
#define PFC_FUNCTOR_TYPE_LIST A0, A1
#define PFC_FUNCTOR_PROTO_ARG_LIST A0 a0_, A1 a1_
#define PFC_FUNCTOR_ARG_LIST a0_, a1_
PFC_FUNCTOR_TMPL();
#undef PFC_FUNCTOR_TEMPLATE_ARG_LIST
#undef PFC_FUNCTOR_TYPE_LIST
#undef PFC_FUNCTOR_PROTO_ARG_LIST
#undef PFC_FUNCTOR_ARG_LIST
// functor 3 argument implementation
#define PFC_FUNCTOR_TEMPLATE_ARG_LIST typename A0, typename A1, typename A2
#define PFC_FUNCTOR_TYPE_LIST A0, A1, A2
#define PFC_FUNCTOR_PROTO_ARG_LIST A0 a0_, A1 a1_, A2 a2_
#define PFC_FUNCTOR_ARG_LIST a0_, a1_, a2_
PFC_FUNCTOR_TMPL();
#undef PFC_FUNCTOR_TEMPLATE_ARG_LIST
#undef PFC_FUNCTOR_TYPE_LIST
#undef PFC_FUNCTOR_PROTO_ARG_LIST
#undef PFC_FUNCTOR_ARG_LIST
// functor 4 argument implementation
#define PFC_FUNCTOR_TEMPLATE_ARG_LIST typename A0, typename A1, typename A2, typename A3
#define PFC_FUNCTOR_TYPE_LIST A0, A1, A2, A3
#define PFC_FUNCTOR_PROTO_ARG_LIST A0 a0_, A1 a1_, A2 a2_, A3 a3_
#define PFC_FUNCTOR_ARG_LIST a0_, a1_, a2_, a3_
PFC_FUNCTOR_TMPL();
#undef PFC_FUNCTOR_TEMPLATE_ARG_LIST
#undef PFC_FUNCTOR_TYPE_LIST
#undef PFC_FUNCTOR_PROTO_ARG_LIST
#undef PFC_FUNCTOR_ARG_LIST
// functor 5 argument implementation
#define PFC_FUNCTOR_TEMPLATE_ARG_LIST typename A0, typename A1, typename A2, typename A3, typename A4
#define PFC_FUNCTOR_TYPE_LIST A0, A1, A2, A3, A4
#define PFC_FUNCTOR_PROTO_ARG_LIST A0 a0_, A1 a1_, A2 a2_, A3 a3_, A4 a4_
#define PFC_FUNCTOR_ARG_LIST a0_, a1_, a2_, a3_, a4_
PFC_FUNCTOR_TMPL();
#undef PFC_FUNCTOR_TEMPLATE_ARG_LIST
#undef PFC_FUNCTOR_TYPE_LIST
#undef PFC_FUNCTOR_PROTO_ARG_LIST
#undef PFC_FUNCTOR_ARG_LIST
// functor 6 argument implementation
#define PFC_FUNCTOR_TEMPLATE_ARG_LIST typename A0, typename A1, typename A2, typename A3, typename A4, typename A5
#define PFC_FUNCTOR_TYPE_LIST A0, A1, A2, A3, A4, A5
#define PFC_FUNCTOR_PROTO_ARG_LIST A0 a0_, A1 a1_, A2 a2_, A3 a3_, A4 a4_, A5 a5_
#define PFC_FUNCTOR_ARG_LIST a0_, a1_, a2_, a3_, a4_, a5_
PFC_FUNCTOR_TMPL();
#undef PFC_FUNCTOR_TEMPLATE_ARG_LIST
#undef PFC_FUNCTOR_TYPE_LIST
#undef PFC_FUNCTOR_PROTO_ARG_LIST
#undef PFC_FUNCTOR_ARG_LIST
// functor 7 argument implementation
#define PFC_FUNCTOR_TEMPLATE_ARG_LIST typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6
#define PFC_FUNCTOR_TYPE_LIST A0, A1, A2, A3, A4, A5, A6
#define PFC_FUNCTOR_PROTO_ARG_LIST A0 a0_, A1 a1_, A2 a2_, A3 a3_, A4 a4_, A5 a5_, A6 a6_
#define PFC_FUNCTOR_ARG_LIST a0_, a1_, a2_, a3_, a4_, a5_, a6_
PFC_FUNCTOR_TMPL();
#undef PFC_FUNCTOR_TEMPLATE_ARG_LIST
#undef PFC_FUNCTOR_TYPE_LIST
#undef PFC_FUNCTOR_PROTO_ARG_LIST
#undef PFC_FUNCTOR_ARG_LIST
// functor 8 argument implementation
#define PFC_FUNCTOR_TEMPLATE_ARG_LIST typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7
#define PFC_FUNCTOR_TYPE_LIST A0, A1, A2, A3, A4, A5, A6, A7
#define PFC_FUNCTOR_PROTO_ARG_LIST A0 a0_, A1 a1_, A2 a2_, A3 a3_, A4 a4_, A5 a5_, A6 a6_, A7 a7_
#define PFC_FUNCTOR_ARG_LIST a0_, a1_, a2_, a3_, a4_, a5_, a6_, a7_
PFC_FUNCTOR_TMPL();
#undef PFC_FUNCTOR_TEMPLATE_ARG_LIST
#undef PFC_FUNCTOR_TYPE_LIST
#undef PFC_FUNCTOR_PROTO_ARG_LIST
#undef PFC_FUNCTOR_ARG_LIST
// functor 9 argument implementation
#define PFC_FUNCTOR_TEMPLATE_ARG_LIST typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8
#define PFC_FUNCTOR_TYPE_LIST A0, A1, A2, A3, A4, A5, A6, A7, A8
#define PFC_FUNCTOR_PROTO_ARG_LIST A0 a0_, A1 a1_, A2 a2_, A3 a3_, A4 a4_, A5 a5_, A6 a6_, A7 a7_, A8 a8_
#define PFC_FUNCTOR_ARG_LIST a0_, a1_, a2_, a3_, a4_, a5_, a6_, a7_, a8_
PFC_FUNCTOR_TMPL();
#undef PFC_FUNCTOR_TEMPLATE_ARG_LIST
#undef PFC_FUNCTOR_TYPE_LIST
#undef PFC_FUNCTOR_PROTO_ARG_LIST
#undef PFC_FUNCTOR_ARG_LIST
// functor 10 argument implementation
#define PFC_FUNCTOR_TEMPLATE_ARG_LIST typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9
#define PFC_FUNCTOR_TYPE_LIST A0, A1, A2, A3, A4, A5, A6, A7, A8, A9
#define PFC_FUNCTOR_PROTO_ARG_LIST A0 a0_, A1 a1_, A2 a2_, A3 a3_, A4 a4_, A5 a5_, A6 a6_, A7 a7_, A8 a8_, A9 a9_
#define PFC_FUNCTOR_ARG_LIST a0_, a1_, a2_, a3_, a4_, a5_, a6_, a7_, a8_, a9_
PFC_FUNCTOR_TMPL();
#undef PFC_FUNCTOR_TEMPLATE_ARG_LIST
#undef PFC_FUNCTOR_TYPE_LIST
#undef PFC_FUNCTOR_PROTO_ARG_LIST
#undef PFC_FUNCTOR_ARG_LIST
//----
#undef PFC_FUNCTOR_TMPL
//----------------------------------------------------------------------------
