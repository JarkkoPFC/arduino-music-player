//============================================================================
// Spin-X Platform
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================

#ifndef PFC_CORE_ITERATORS_H
#define PFC_CORE_ITERATORS_H
//----------------------------------------------------------------------------


//============================================================================
// interface
//============================================================================
// external
#include "core.h"
namespace pfc
{

// new
template<typename> struct iterator_trait;
template<typename T, bool is_forward> class array_iterator;
//----------------------------------------------------------------------------


//============================================================================
// iterator_trait
//============================================================================
template<typename T> struct iterator_trait
{
  typedef typename T::value_t value_t;
};
//----

template<typename T> struct iterator_trait<T*> {typedef T value_t;};
//----------------------------------------------------------------------------


//============================================================================
// array_iterator
//============================================================================
template<typename T, bool is_forward>
class array_iterator
{
public:
  // nested types
  typedef T value_t;
  //--------------------------------------------------------------------------

  // construction
  PFC_INLINE array_iterator();
  PFC_INLINE array_iterator(T*);
  template<typename U> PFC_INLINE array_iterator(const array_iterator<U, is_forward>&);
  //--------------------------------------------------------------------------

  // iteration and accessors
  PFC_INLINE friend bool is_valid(const array_iterator &it_)  {return it_.data!=0;}
  template<typename U> PFC_INLINE bool operator==(const array_iterator<U, is_forward>&) const;
  template<typename U> PFC_INLINE bool operator!=(const array_iterator<U, is_forward>&) const;
  PFC_INLINE array_iterator &operator++();
  PFC_INLINE array_iterator &operator--();
  PFC_INLINE array_iterator &operator+=(ssize_t offset_);
  template<typename U> PFC_INLINE array_iterator &operator+=(const array_iterator&);
  PFC_INLINE array_iterator &operator-=(ssize_t offset_);
  template<typename U> PFC_INLINE array_iterator &operator-=(const array_iterator&);
  PFC_INLINE array_iterator operator+(ssize_t offset_) const;
  template<typename U> PFC_INLINE array_iterator operator+(const array_iterator&) const;
  PFC_INLINE array_iterator operator-(ssize_t offset_) const;
  template<typename U> PFC_INLINE array_iterator operator-(const array_iterator&) const;
  PFC_INLINE T &operator[](usize_t index_) const;
  PFC_INLINE T &operator*() const;
  PFC_INLINE T *operator->() const;
  PFC_INLINE friend T *ptr(const array_iterator &it_)         {return it_.data;}
  //--------------------------------------------------------------------------

  T *data;
};
//----------------------------------------------------------------------------

//============================================================================
#include "iterators.inl"
} // namespace pfc
#endif
