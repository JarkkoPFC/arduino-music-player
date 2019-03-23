//============================================================================
// Spin-X Platform
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================

#ifndef PFC_CORE_STR_H
#define PFC_CORE_STR_H
//----------------------------------------------------------------------------


//============================================================================
// interface
//============================================================================
// external
#include "cstr.h"
#include "meta.h"
#include <stdio.h>
namespace pfc
{
// new
template<class> class str_base;
template<typename T> struct is_type_str;
class heap_str;
template<unsigned capacity> class stack_str;
typedef stack_str<8> stack_str8;
typedef stack_str<16> stack_str16;
typedef stack_str<32> stack_str32;
typedef stack_str<64> stack_str64;
typedef stack_str<128> stack_str128;
typedef stack_str<256> stack_str256;
typedef stack_str<512> stack_str512;
typedef stack_str<1024> stack_str1024;
typedef stack_str1024 filepath_str;
// string operations
template<class S0, class S1> PFC_INLINE bool operator==(const str_base<S0>&, const str_base<S1>&);
template<class S> PFC_INLINE bool operator==(const str_base<S>&, const char*);
template<class S> PFC_INLINE bool operator==(const char*, const str_base<S>&);
template<class S> PFC_INLINE bool operator==(const str_base<S>&, char);
template<class S> PFC_INLINE bool operator==(char, const str_base<S>&);
template<class S0, class S1> PFC_INLINE bool operator!=(const str_base<S0>&, const str_base<S1>&);
template<class S> PFC_INLINE bool operator!=(const str_base<S>&, const char*);
template<class S> PFC_INLINE bool operator!=(const char*, const str_base<S>&);
template<class S> PFC_INLINE bool operator!=(const str_base<S>&, char);
template<class S> PFC_INLINE bool operator!=(char, const str_base<S>&);
template<class S0, class S1> PFC_INLINE bool operator<(const str_base<S0>&, const str_base<S1>&);
template<class S> PFC_INLINE bool operator<(const str_base<S>&, const char*);
template<class S> PFC_INLINE bool operator<(const char*, const str_base<S>&);
template<class S> PFC_INLINE bool operator<(const str_base<S>&, char);
template<class S> PFC_INLINE bool operator<(char, const str_base<S>&);
template<class S0, class S1> PFC_INLINE bool operator>(const str_base<S0>&, const str_base<S1>&);
template<class S> PFC_INLINE bool operator>(const str_base<S>&, const char*);
template<class S> PFC_INLINE bool operator>(const char*, const str_base<S>&);
template<class S> PFC_INLINE bool operator>(const str_base<S>&, char);
template<class S> PFC_INLINE bool operator>(char, const str_base<S>&);
template<class S0, class S1> PFC_INLINE bool operator<=(const str_base<S0>&, const str_base<S1>&);
template<class S> PFC_INLINE bool operator<=(const str_base<S>&, const char*);
template<class S> PFC_INLINE bool operator<=(const char*, const str_base<S>&);
template<class S> PFC_INLINE bool operator<=(const str_base<S>&, char);
template<class S> PFC_INLINE bool operator<=(char, const str_base<S>&);
template<class S0, class S1> PFC_INLINE bool operator>=(const str_base<S0>&, const str_base<S1>&);
template<class S> PFC_INLINE bool operator>=(const str_base<S>&, const char*);
template<class S> PFC_INLINE bool operator>=(const char*, const str_base<S>&);
template<class S> PFC_INLINE bool operator>=(const str_base<S>&, char);
template<class S> PFC_INLINE bool operator>=(char, const str_base<S>&);
template<class S> bool replace_substr(str_base<S> &res_, const str_base<S> &src_, const char *replace_, const char *replacement_);
//----------------------------------------------------------------------------


//============================================================================
// str_base
//============================================================================
template<class S>
class str_base
{
public:
  // config
  enum {max_format_str_size=1024};
  //--------------------------------------------------------------------------

  // construction
  PFC_INLINE void operator=(const str_base&);
  template<class S2> PFC_INLINE void operator=(const str_base<S2>&);
  PFC_INLINE void operator=(const char*);
  PFC_INLINE void set(const char*, usize_t num_chars_);
  PFC_INLINE void clear();
  //--------------------------------------------------------------------------

  // accessors and mutators
  PFC_INLINE usize_t size() const;
  PFC_INLINE usize_t capacity() const;
  PFC_INLINE const char &operator[](usize_t idx_) const;
  PFC_INLINE char &operator[](usize_t idx_);
  PFC_INLINE const char &front() const;
  PFC_INLINE char &front();
  PFC_INLINE const char &back() const;
  PFC_INLINE char &back();
  PFC_INLINE const char *c_str() const;
  PFC_INLINE char *c_str();
  PFC_INLINE const char *data() const;
  PFC_INLINE char *data();
  template<class S2> PFC_INLINE void operator+=(const str_base<S2>&);
  PFC_INLINE void operator+=(const char*);
  PFC_INLINE void operator+=(char);
  void format(const char*, ...);
  PFC_INLINE void push_front(char);
  PFC_INLINE void push_front(char, usize_t num_chars_);
  PFC_INLINE void push_front(const char*);
  PFC_INLINE void push_front(const char*, usize_t num_chars_);
  PFC_INLINE void push_back(char);
  PFC_INLINE void push_back(char, usize_t num_chars_);
  PFC_INLINE void push_back(const char*, usize_t num_chars_);
  void push_back_format(const char*, ...);
  PFC_INLINE void remove_front(usize_t num_chars_);
  PFC_INLINE void remove_back(usize_t num_chars_);
  //--------------------------------------------------------------------------

protected:
  // construction
  PFC_INLINE str_base();
  PFC_INLINE str_base(const str_base&);
  PFC_INLINE ~str_base();
};
//----------------------------------------------------------------------------


//============================================================================
// heap_str
//============================================================================
class heap_str: public str_base<heap_str>
{ PFC_MONO(heap_str) PFC_INTROSPEC_DECL;
public:
  // construction
  PFC_INLINE heap_str(memory_allocator_base *alloc_=0);
  PFC_INLINE heap_str(const heap_str&, memory_allocator_base *alloc_=0);
  template<class S> PFC_INLINE heap_str(const str_base<S>&, memory_allocator_base *alloc_=0);
  PFC_INLINE heap_str(const char*, memory_allocator_base *alloc_=0);
  PFC_INLINE heap_str(const char*, usize_t num_chars_, memory_allocator_base *alloc_=0);
  PFC_INLINE heap_str(char, memory_allocator_base *alloc_=0);
  PFC_INLINE ~heap_str();
  PFC_INLINE void set_allocator(memory_allocator_base*);
  PFC_INLINE void operator=(const heap_str&);
  template<class S> PFC_INLINE void operator=(const str_base<S>&);
  PFC_INLINE void operator=(const char*);
  PFC_INLINE void set(const char*, usize_t num_chars_);
  PFC_INLINE void clear();
  PFC_INLINE void reset_size(usize_t size_);
  PFC_INLINE void resize(usize_t size_);
  PFC_INLINE void reserve(usize_t capacity_);
  void swap(heap_str&);
  //--------------------------------------------------------------------------

  // accessors and mutators
  PFC_INLINE memory_allocator_base &allocator() const;
  PFC_INLINE usize_t size() const;
  PFC_INLINE usize_t capacity() const;
  PFC_INLINE const char &operator[](usize_t idx_) const;
  PFC_INLINE char &operator[](usize_t idx_);
  PFC_INLINE const char &front() const;
  PFC_INLINE char &front();
  PFC_INLINE const char &back() const;
  PFC_INLINE char &back();
  PFC_INLINE const char *c_str() const;
  PFC_INLINE char *c_str();
  PFC_INLINE const char *data() const;
  PFC_INLINE char *data();
  template<class S> PFC_INLINE void operator+=(const str_base<S>&);
  PFC_INLINE void operator+=(const char*);
  PFC_INLINE void operator+=(char);
  PFC_INLINE void push_front(char);
  PFC_INLINE void push_front(char, usize_t num_chars_);
  PFC_INLINE void push_front(const char*);
  PFC_INLINE void push_front(const char*, usize_t num_chars_);
  PFC_INLINE void push_back(char);
  PFC_INLINE void push_back(char, usize_t num_chars_);
  PFC_INLINE void push_back(const char*, usize_t num_chars_);
  PFC_INLINE void remove_front(usize_t num_chars_);
  PFC_INLINE void remove_back(usize_t num_chars_);
  //--------------------------------------------------------------------------

private:
  enum {min_grow_alloc=15};
  PFC_INLINE void reserve_offset(usize_t capacity_, usize_t offset_);
  //--------------------------------------------------------------------------

  memory_allocator_base *m_allocator;
  char *m_data;
  usize_t m_size;
  usize_t m_capacity;
};
PFC_SET_TYPE_TRAIT(heap_str, is_type_pod_move, true);
//----------------------------------------------------------------------------


//============================================================================
// stack_str
//============================================================================
template<unsigned cap>
class stack_str: public str_base<stack_str<cap> >
{ PFC_MONO(stack_str) PFC_INTROSPEC_DECL;
public:
  // properties
  enum {str_capacity=cap};
  //--------------------------------------------------------------------------

  // construction
  PFC_INLINE stack_str();
  PFC_INLINE stack_str(const stack_str&);
  template<class S> PFC_INLINE stack_str(const str_base<S>&);
  PFC_INLINE stack_str(const char*);
  PFC_INLINE stack_str(char);
  PFC_INLINE void operator=(const stack_str&);
  template<class S> PFC_INLINE void operator=(const str_base<S>&);
  PFC_INLINE void operator=(const char*);
  PFC_INLINE void set(const char *str_, usize_t size_);
  PFC_INLINE void clear();
  PFC_INLINE void resize(usize_t size_);
  //--------------------------------------------------------------------------

  // accessors and mutators
  PFC_INLINE usize_t size() const;
  PFC_INLINE usize_t capacity() const;
  PFC_INLINE const char &operator[](usize_t idx_) const;
  PFC_INLINE char &operator[](usize_t idx_);
  PFC_INLINE const char &front() const;
  PFC_INLINE char &front();
  PFC_INLINE const char &back() const;
  PFC_INLINE char &back();
  PFC_INLINE const char *c_str() const;
  PFC_INLINE char *c_str();
  PFC_INLINE const char *data() const;
  PFC_INLINE char *data();
  template<class S> PFC_INLINE void operator+=(const str_base<S>&);
  PFC_INLINE void operator+=(const char*);
  PFC_INLINE void operator+=(char);
  PFC_INLINE void push_front(char);
  PFC_INLINE void push_front(char, usize_t num_chars_);
  PFC_INLINE void push_front(const char*);
  PFC_INLINE void push_front(const char*, usize_t num_chars_);
  PFC_INLINE void push_back(char);
  PFC_INLINE void push_back(char, usize_t num_chars_);
  PFC_INLINE void push_back(const char*, usize_t num_chars_);
  PFC_INLINE void remove_front(usize_t num_chars_);
  PFC_INLINE void remove_back(usize_t num_chars_);
  //--------------------------------------------------------------------------

private:
  typedef typename meta_if<cap<256, uint8, uint16>::res size_type;
  size_type m_size;
  char m_data[cap+1];
};
PFC_SET_TYPE_TRAIT_PARTIAL(unsigned cap, stack_str<cap>, is_type_pod, true);
//----------------------------------------------------------------------------

//============================================================================
#include "str.inl"
} // namespace pfc
#endif
