//============================================================================
// Spin-X Platform
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================


//============================================================================
// align_type
//============================================================================
template<unsigned> struct align_type {PFC_CTC_ASSERT_MSG(0, given_alignment_doesnt_have_type);};
template<> struct align_type<0>  {typedef void res;};
template<> struct align_type<1>  {typedef align_type_1 res;};
template<> struct align_type<2>  {typedef align_type_2 res;};
template<> struct align_type<4>  {typedef align_type_4 res;};
template<> struct align_type<8>  {typedef align_type_8 res;};
template<> struct align_type<16> {typedef align_type_16 res;};
//----------------------------------------------------------------------------


//============================================================================
// memory management functions
//============================================================================
PFC_INLINE void *mem_alloc(usize_t num_bytes_, const alloc_site_info *site_info_)
{
  return PFC_ALIGNED_MALLOC(num_bytes_, memory_align);
}
//----

template<typename T>
PFC_INLINE T *mem_alloc(const alloc_site_info *site_info_)
{
  return (T*)PFC_ALIGNED_MALLOC(sizeof(T), memory_align);
}
//----

PFC_INLINE void mem_free(void *p_)
{
  PFC_ALIGNED_FREE(p_);
}
//----

PFC_INLINE void mem_copy(void *dst_, const void *src_, usize_t num_bytes_)
{
  PFC_ASSERT_PEDANTIC(!num_bytes_ || (dst_ && src_));
  PFC_ASSERT_PEDANTIC_MSG(!num_bytes_ || dst_==src_ || (dst_<src_ && ((char*)dst_)+num_bytes_<=src_) || (dst_>src_ && ((char*)src_)+num_bytes_<=dst_),
                          ("Overlapping memory regions for mem_copy() result in undefined behavior\r\n"));
  ::memcpy(dst_, src_, (size_t)num_bytes_);
}
//----

PFC_INLINE void mem_move(void *dst_, const void *src_, usize_t num_bytes_)
{
  PFC_ASSERT_PEDANTIC(!num_bytes_ || (dst_ && src_));
  ::memmove(dst_, src_, (size_t)num_bytes_);
}
//----

PFC_INLINE void mem_zero(void *p_, usize_t num_bytes_)
{
  PFC_ASSERT_PEDANTIC(!num_bytes_ || p_);
  ::memset(p_, 0, (size_t)num_bytes_);
}
//----

PFC_INLINE void mem_set(void *p_, unsigned char c_, usize_t num_bytes_)
{
  PFC_ASSERT_PEDANTIC(!num_bytes_ || p_);
  ::memset(p_, c_, (size_t)num_bytes_);
}
//----

PFC_INLINE bool mem_eq(const void *p0_, const void *p1_, usize_t num_bytes_)
{
  PFC_ASSERT_PEDANTIC(!num_bytes_ || (p0_ && p1_));
  return ::memcmp(p0_, p1_, (size_t)num_bytes_)==0;
}
//----

PFC_INLINE bool is_mem_zero(void *p_, usize_t num_bytes_)
{
  PFC_ASSERT_PEDANTIC(!num_bytes_ || p_);
  const char *p=(const char*)p_;
  return *p==0 && ::memcmp(p, p+1, num_bytes_-1)==0;
}
//----

PFC_INLINE int mem_diff(const void *p0_, const void *p1_, usize_t num_bytes_)
{
  PFC_ASSERT_PEDANTIC(!num_bytes_ || (p0_ && p1_));
  return ::memcmp(p0_, p1_, (size_t)num_bytes_);
}
//----------------------------------------------------------------------------


//============================================================================
// memory_allocator_base
//============================================================================
memory_allocator_base::memory_allocator_base()
{
}
//----

memory_allocator_base::~memory_allocator_base()
{
}
//----------------------------------------------------------------------------


//============================================================================
// default_memory_allocator
//============================================================================
PFC_INLINE default_memory_allocator &default_memory_allocator::inst()
{
  static default_memory_allocator s_allocator;
  return s_allocator;
}
//----------------------------------------------------------------------------


//============================================================================
// pointer ops
//============================================================================
template<typename T> T *ptr(T *p_)
{
  return p_;
}
//----

template<typename T> bool is_valid(T *p_)
{
  return p_!=0;
}
//----------------------------------------------------------------------------


//============================================================================
// object construction and destruction
//============================================================================
template<typename T>
PFC_INLINE T *array_new(usize_t num_items_, const alloc_site_info *site_info_)
{
  // alloc memory and setup memory info
  memory_info *info=(memory_info*)PFC_ALIGNED_MALLOC(num_items_*sizeof(T)+memory_info_size, memory_align);
  info->num_items=num_items_;

  // construct array
  T *p=(T*)(((char*)info)+memory_info_size);
  if(num_items_)
  {
    eh_array_dtor<T> v(p, p);
    T *end=p+num_items_;
    do
    {
      PFC_PNEW(v.dst)T;
    } while(++v.dst!=end);
    v.begin=0;
  }
  return p;
}
//----

template<typename T>
PFC_INLINE void array_delete(const T *p_)
{
  // read memory block info
  if(!p_)
    return;
  memory_info *info=(memory_info*)(((char*)p_)-memory_info_size);
  usize_t num_items=info->num_items;
  PFC_ASSERT_MSG((num_items&memory_flag_typeless)==0, ("Trying to release typeless data with array_delete()\r\n"));

  // destruct objects and free memory
  for(usize_t i=0; i<num_items; ++i)
  {
    p_->~T();
    ++p_;
  }
  PFC_ALIGNED_FREE(info);
}
//----

template<typename T>
PFC_INLINE usize_t array_size(const T *p_)
{
  // get number of items from memory block info
  if(!p_)
    return 0;
  const memory_info *info=(const memory_info*)(((char*)p_)-memory_info_size);
  return info->num_items&~memory_flag_typeless;
}
//----

template<typename T>
PFC_INLINE void *destruct(const T *p_)
{
  if(p_)
    p_->~T();
  return (void*)p_;
}
//----------------------------------------------------------------------------


//============================================================================
// type ID
//============================================================================
namespace priv
{
  PFC_INLINE unsigned new_type_id()
  {
    static unsigned s_typeid=0;
    return ++s_typeid;
  }
} // namespace priv
//----

template<typename T> const unsigned type_id<T>::id=type_id<const T>::id?type_id<const T>::id:type_id<volatile T>::id?type_id<volatile T>::id:type_id<const volatile T>::id?type_id<const volatile T>::id:priv::new_type_id();
//----

template<typename T> struct type_id<const T> {static const unsigned id;};
template<typename T> const unsigned type_id<const T>::id=type_id<T>::id?type_id<T>::id:type_id<volatile T>::id?type_id<volatile T>::id:type_id<const volatile T>::id?type_id<const volatile T>::id:priv::new_type_id();
template<typename T> struct type_id<volatile T> {static const unsigned id;};
template<typename T> const unsigned type_id<volatile T>::id=type_id<T>::id?type_id<T>::id:type_id<const T>::id?type_id<const T>::id:type_id<const volatile T>::id?type_id<const volatile T>::id:priv::new_type_id();
template<typename T> struct type_id<const volatile T> {static const unsigned id;};
template<typename T> const unsigned type_id<const volatile T>::id=type_id<T>::id?type_id<T>::id:type_id<const T>::id?type_id<const T>::id:type_id<volatile T>::id?type_id<volatile T>::id:priv::new_type_id();
//----

template<typename T>
struct ptr_id<T*>
{
  static PFC_INLINE unsigned id()
  {
    return type_id<T>::id;
  }
};
//----

template<typename T>
struct ref_id<T&>
{
  static PFC_INLINE unsigned id()
  {
    return type_id<T>::id;
  }
};
//----------------------------------------------------------------------------


//============================================================================
// introspection
//============================================================================
template<class PE>
template<class T>
bool prop_enum_interface_base<PE>::subclass(T*)
{
  return true;
}
//----

template<class PE>
template<class T>
unsigned prop_enum_interface_base<PE>::set_custom_streaming(T&, unsigned version_)
{
  return version_;
}
//----

template<class PE>
template<typename T>
bool prop_enum_interface_base<PE>::var(const T&, unsigned flags_, const char *mvar_name_)
{
  return true;
}
//----

template<class PE>
template<typename T, class C>
bool prop_enum_interface_base<PE>::var(const T&, unsigned flags_, const char *mvar_name_, C&)
{
  return true;
}
//----

template<class PE>
template<typename T, class C>
bool prop_enum_interface_base<PE>::var(const T&, unsigned flags_, const char *mvar_name_, C&, void(*post_mutate_func_)(C*))
{
  return true;
}
//----

template<class PE>
template<typename T, class C>
bool prop_enum_interface_base<PE>::var(const T&, unsigned flags_, const char *mvar_name_, C&, void(C::*mutate_func_)(const T&, unsigned var_index_), unsigned var_index_)
{
  return true;
}
//----

template<class PE>
template<typename T>
bool prop_enum_interface_base<PE>::avar(const T*, usize_t size_, unsigned flags_, const char *mvar_name_)
{
  return true;
}
//----

template<class PE>
template<typename T, class C>
bool prop_enum_interface_base<PE>::avar(const T*, usize_t size_, unsigned flags_, const char *mvar_name_, C&)
{
  return true;
}
//----

template<class PE>
template<typename T, class C>
bool prop_enum_interface_base<PE>::avar(const T*, usize_t size_, unsigned flags_, const char *mvar_name_, C&, void(*post_mutate_func_)(C*))
{
  return true;
}
//----

template<class PE>
template<typename T, class C>
bool prop_enum_interface_base<PE>::avar(const T*, usize_t size_, unsigned flags_, const char *mvar_name_, C&, void(C::*mutate_func_)(const T&, unsigned index_, unsigned var_index_), unsigned var_index_)
{
  return true;
}
//----

template<class PE>
bool prop_enum_interface_base<PE>::data(const void*, usize_t num_bytes_)
{
  return true;
}
//----

template<class PE>
void prop_enum_interface_base<PE>::skip(usize_t num_bytes_)
{
  PFC_ERROR_NOT_IMPL();
}
//----

template<class PE>
template<typename T>
bool prop_enum_interface_base<PE>::alias_var(const T&, unsigned flags_, const char *alias_)
{
  return true;
}
//----

template<class PE>
template<typename T>
bool prop_enum_interface_base<PE>::alias_avar(const T*, usize_t size_, unsigned flags_, const char *alias_)
{
  return true;
}
//----------------------------------------------------------------------------

template<class PE>
void prop_enum_interface_base<PE>::group_begin(const char *group_name_)
{
}
//----

template<class PE>
void prop_enum_interface_base<PE>::group_end()
{
}
//----

template<class PE>
void prop_enum_interface_base<PE>::name(const char*)
{
}
//----

template<class PE>
void prop_enum_interface_base<PE>::desc(const char*)
{
}
//----

template<class PE>
void prop_enum_interface_base<PE>::color(uint32 rgb_)
{
}
//----

template<class PE>
void prop_enum_interface_base<PE>::expanded()
{
}
//----

template<class PE>
template<typename T>
void prop_enum_interface_base<PE>::slider(const T &min_, const T &max_, const T &step_)
{
}
//----------------------------------------------------------------------------

template<class PE>
prop_enum_interface_base<PE>::prop_enum_interface_base()
{
}
//----

template<class PE>
prop_enum_interface_base<PE>::~prop_enum_interface_base()
{
}
//----------------------------------------------------------------------------


//============================================================================
// uint128 operations
//============================================================================
PFC_INLINE bool operator==(const uint128 &v0_, const uint128 &v1_)
{
  return v0_.lo==v1_.lo && v0_.hi==v1_.hi;
}
//----

PFC_INLINE bool operator==(const volatile uint128 &v0_, const uint128 &v1_)
{
  return v0_.lo==v1_.lo && v0_.hi==v1_.hi;
}
//----

PFC_INLINE bool operator==(const uint128 &v0_, const volatile uint128 &v1_)
{
  return v0_.lo==v1_.lo && v0_.hi==v1_.hi;
}
//----

PFC_INLINE bool operator==(const volatile uint128 &v0_, const volatile uint128 &v1_)
{
  return v0_.lo==v1_.lo && v0_.hi==v1_.hi;
}
//----

PFC_INLINE bool operator!=(const uint128 &v0_, const uint128 &v1_)
{
  return v0_.lo!=v1_.lo || v0_.hi!=v1_.hi;
}
//----

PFC_INLINE bool operator!=(const volatile uint128 &v0_, const uint128 &v1_)
{
  return v0_.lo!=v1_.lo || v0_.hi!=v1_.hi;
}
//----

PFC_INLINE bool operator!=(const uint128 &v0_, const volatile uint128 &v1_)
{
  return v0_.lo!=v1_.lo || v0_.hi!=v1_.hi;
}
//----

PFC_INLINE bool operator!=(const volatile uint128 &v0_, const volatile uint128 &v1_)
{
  return v0_.lo!=v1_.lo || v0_.hi!=v1_.hi;
}
//----------------------------------------------------------------------------


//============================================================================
// incomplete type forwarding
//============================================================================
// type traits
template<typename T> struct is_type_fwd_delete {enum {res=false};};
// pointer traits
template<typename T> struct is_ptr_fwd_delete {enum {res=false};};
template<typename T> struct is_ptr_fwd_delete<T*> {enum {res=is_type_fwd_delete<T>::res};};
// reference traits
template<typename T> struct is_ref_fwd_delete {enum {res=false};};
template<typename T> struct is_ref_fwd_delete<T&> {enum {res=is_type_fwd_delete<T>::res};};
//----------------------------------------------------------------------------

// clone forwarding
template<typename T>
struct fwd_type_func_clone
{
  static owner_ref<T>(*s_func)(const T&);
};
template<typename T> owner_ref<T>(*fwd_type_func_clone<T>::s_func)(const T&)=0;
//----------------------------------------------------------------------------

// delete forwarding
template<typename T>
struct fwd_type_func_delete
{
  static void(*s_func)(T*);
};
template<typename T> void(*fwd_type_func_delete<T>::s_func)(T*)=0;
//----

template<typename T> void register_fwd_delete()
{
  PFC_CTF_ASSERT_MSG(is_type_fwd_delete<T>::res, type_not_declared_as_forward_deletable);
  struct func_type
  {
    static void func(T *p_)
    {
      PFC_DELETE(p_);
    }
  };
  fwd_type_func_delete<T>::s_func=&func_type::func;
}
//----

template<typename T> PFC_INLINE void fwd_delete(T *p_)
{
  PFC_CTF_ASSERT_MSG(is_type_fwd_delete<T>::res, type_not_declared_as_forward_deletable);
  if(p_)
  {
    PFC_ASSERT_MSG((fwd_type_func_delete<T>::s_func), ("Forward delete not registered for the type\r\n"));
    (*fwd_type_func_delete<T>::s_func)(p_);
  }
}
//----------------------------------------------------------------------------


//============================================================================
// event system
//============================================================================
template<class EH, typename E>
PFC_INLINE void send_event(EH &v_, E &e_)
{
  v_.dispatch_event(pfc::type_id<E>::id, &e_);
}
//----------------------------------------------------------------------------


//============================================================================
// owner_ptr
//============================================================================
namespace priv
{
  template<bool is_fwd_delete>
  struct delete_helper
  {
    template<typename T> static PFC_INLINE void delete_ptr(T *p_)
    {
      PFC_FWD_DELETE(p_);
    }
  };

  template<>
  struct delete_helper<false>
  {
    template<typename T> static PFC_INLINE void delete_ptr(T *p_)
    {
      PFC_DELETE(p_);
    }
  };
}
//----------------------------------------------------------------------------

template<typename T>
owner_ptr<T>::owner_ptr()
  :data(0)
{
}
//----

template<typename T>
owner_ptr<T>::owner_ptr(T *p_)
  :data(p_)
{
}
//----

template<typename T>
owner_ptr<T>::owner_ptr(const owner_ptr &ptr_)
  :data(ptr_.data)
{
  ptr_.data=0;
}
//----

template<typename T>
template<typename U>
owner_ptr<T>::owner_ptr(const owner_ptr<U> &ptr_)
  :data(ptr_.data)
{
  ptr_.data=0;
}
//----

template<typename T>
template<typename U>
owner_ptr<T>::owner_ptr(const owner_ref<U> &ref_)
  :data(ref_.data)
{
  PFC_ASSERT_PEDANTIC(ref_.data);
  ref_.data=0;
}
//----

template<typename T>
void owner_ptr<T>::operator=(T *p_)
{
  if(data!=p_)
  {
    priv::delete_helper<is_type_fwd_delete<T>::res>::delete_ptr(data);
    data=p_;
  }
}
//----

template<typename T>
void owner_ptr<T>::operator=(const owner_ptr &ptr_)
{
  if(this!=&ptr_)
  {
    priv::delete_helper<is_type_fwd_delete<T>::res>::delete_ptr(data);
    data=ptr_.data;
    ptr_.data=0;
  }
}
//----

template<typename T>
template<typename U>
void owner_ptr<T>::operator=(const owner_ptr<U> &ptr_)
{
  priv::delete_helper<is_type_fwd_delete<T>::res>::delete_ptr(data);
  data=ptr_.data;
  ptr_.data=0;
}
//----

template<typename T>
template<typename U>
void owner_ptr<T>::operator=(const owner_ref<U> &ref_)
{
  PFC_ASSERT_PEDANTIC(ref_.data);
  priv::delete_helper<is_type_fwd_delete<T>::res>::delete_ptr(data);
  data=ref_.data;
  ref_.data=0;
}
//----

template<typename T>
owner_ptr<T>::~owner_ptr()
{
  priv::delete_helper<is_type_fwd_delete<T>::res>::delete_ptr(data);
}
//----------------------------------------------------------------------------

template<typename T>
T *owner_ptr<T>::operator->() const
{
  PFC_ASSERT_PEDANTIC_MSG(data, ("Dereferencing \"%s\" NULL pointer\r\n", typeid(T).name()));
  return data;
}
//----

template<typename T>
T &owner_ptr<T>::operator*() const
{
  PFC_ASSERT_PEDANTIC_MSG(data, ("Dereferencing \"%s\" NULL pointer\r\n", typeid(T).name()));
  return *data;
}
//----------------------------------------------------------------------------


//============================================================================
// owner_ref
//============================================================================
template<typename T>
owner_ref<T>::owner_ref(T *p_)
  :data(p_)
{
  PFC_ASSERT_PEDANTIC(p_);
}
//----

template<typename T>
owner_ref<T>::owner_ref(const owner_ref &ref_)
  :data(ref_.data)
{
  PFC_ASSERT_PEDANTIC(ref_.data);
  ref_.data=0;
}
//----

template<typename T>
template<typename U>
owner_ref<T>::owner_ref(const owner_ref<U> &ref_)
  :data(ref_.data)
{
  PFC_ASSERT_PEDANTIC(ref_.data);
  ref_.data=0;
}
//----

template<typename T>
template<typename U>
owner_ref<T>::owner_ref(const owner_ptr<U> &ptr_)
  :data(ptr_.data)
{
  PFC_ASSERT_PEDANTIC(ptr_.data);
  ptr_.data=0;
}
//----

template<typename T>
owner_ref<T>::~owner_ref()
{
  priv::delete_helper<is_type_fwd_delete<T>::res>::delete_ptr(data);
}
//----------------------------------------------------------------------------

template<typename T>
T *owner_ref<T>::operator->() const
{
  PFC_ASSERT_PEDANTIC_MSG(data, ("Dereferencing \"%s\" NULL pointer\r\n", typeid(T).name()));
  return data;
}
//----

template<typename T>
T &owner_ref<T>::operator*() const
{
  PFC_ASSERT_PEDANTIC_MSG(data, ("Dereferencing \"%s\" NULL pointer\r\n", typeid(T).name()));
  return *data;
}
//----------------------------------------------------------------------------


//============================================================================
// owner_data
//============================================================================
owner_data::owner_data()
{
  data=0;
}
//----

owner_data::owner_data(void *p_)
{
  data=p_;
}
//----

owner_data::owner_data(const owner_data &ptr_)
{
  data=ptr_.data;
  ptr_.data=0;
}
//----

void owner_data::operator=(void *p_)
{
  if(data!=p_)
  {
    PFC_MEM_FREE(data);
    data=p_;
  }
}
//----

void owner_data::operator=(const owner_data &ptr_)
{
  if(this!=&ptr_)
  {
    PFC_MEM_FREE(data);
    data=ptr_.data;
    ptr_.data=0;
  }
}
//----

owner_data::~owner_data()
{
  PFC_MEM_FREE(data);
}
//----------------------------------------------------------------------------


//============================================================================
// raw_cast
//============================================================================
template<typename T, typename U>
PFC_INLINE T raw_cast(U v_)
{
  PFC_CTF_ASSERT_MSG(sizeof(T)==sizeof(U), source_and_destination_types_must_have_equal_size);
  return reinterpret_cast<const T&>(v_);
}
//----------------------------------------------------------------------------


//============================================================================
// min/max
//============================================================================
template<typename T>
PFC_INLINE T min(T v0_, T v1_)
{
  // return minimum of the two
  return v0_<v1_?v0_:v1_;
}
//----

template<typename T>
PFC_INLINE T min(T v0_, T v1_, T v2_)
{
  // return minimum of the three
  T v=v0_<v1_?v0_:v1_;
  return v<v2_?v:v2_;
}
//----

template<typename T>
PFC_INLINE T min(T v0_, T v1_, T v2_, T v3_)
{
  // return minimum of the four
  T v=v0_<v1_?v0_:v1_;
  v=v<v2_?v:v2_;
  return v<v3_?v:v3_;
}
//----

template<typename T>
PFC_INLINE T max(T v0_, T v1_)
{
  // return maximum of the two
  return v0_>v1_?v0_:v1_;
}
//----

template<typename T>
PFC_INLINE T max(T v0_, T v1_, T v2_)
{
  // return maximum of the three
  T v=v0_>v1_?v0_:v1_;
  return v>v2_?v:v2_;
}
//----

template<typename T>
PFC_INLINE T max(T v0_, T v1_, T v2_, T v3_)
{
  // return maximum of the four
  T v=v0_>v1_?v0_:v1_;
  v=v>v2_?v:v2_;
  return v>v3_?v:v3_;
}
//----------------------------------------------------------------------------


//============================================================================
// eh_data
//============================================================================
template<typename T>
eh_data<T>::eh_data(memory_allocator_base &alloc_, usize_t size_, usize_t align_)
#ifdef PFC_BUILDOP_EXCEPTIONS
  :m_allocator(alloc_)
#endif
{
  data=size_?(T*)alloc_.alloc(sizeof(T)*size_, align_):0;
}
//----

template<typename T>
eh_data<T>::~eh_data()
{
#ifdef PFC_BUILDOP_EXCEPTIONS
  m_allocator.free(data);
#endif
}
//----

template<typename T>
void eh_data<T>::reset()
{
#ifdef PFC_BUILDOP_EXCEPTIONS
  data=0;
#endif
}
//----------------------------------------------------------------------------


//============================================================================
// eh_array_dtor
//============================================================================
template<typename T>
eh_array_dtor<T>::eh_array_dtor()
{
  begin=0;
  dst=0;
}
//----

template<typename T>
eh_array_dtor<T>::eh_array_dtor(T *begin_, T *dst_)
{
  begin=begin_;
  dst=dst_;
}
//----

template<typename T>
eh_array_dtor<T>::~eh_array_dtor()
{
#ifdef PFC_BUILDOP_EXCEPTIONS
  if(begin)
  {
    if(dst>begin)
      while(dst--!=begin)
        dst->~T();
    else
      while(dst++!=begin)
        dst->~T();
  }
#endif
}
//----------------------------------------------------------------------------


//============================================================================
// enumerated value string
//============================================================================
template<typename T>
PFC_INLINE const char *enum_string(T eval_)
{
  return enum_strings(eval_)[enum_string_index(eval_)];
}
//----

template<typename T>
PFC_INLINE const char *enum_display_string(T eval_)
{
  return enum_display_strings(eval_)[enum_string_index(eval_)];
}
//----

template<typename T>
PFC_INLINE bool enum_value(T &v_, const char *enum_str_)
{
  extern int str_find_substr(const char*, const char*const*);
  unsigned eidx=str_find_substr(enum_str_, enum_strings(v_)+1)+1;
  if(!eidx)
  {
    if(enum_dep_value(v_, enum_str_))
      return true;
    PFC_WARN(("No value found for enum %s = %s\r\n", enum_type_name(v_), enum_str_));
    return false;
  }
  const T *evals=enum_values(v_);
  v_=evals?evals[eidx]:T(eidx-1);
  return true;
}
//----------------------------------------------------------------------------
