//============================================================================
// Spin-X Platform
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================

#ifndef PFC_CORE_CONTAINERS_H
#define PFC_CORE_CONTAINERS_H
//----------------------------------------------------------------------------


//============================================================================
// interface
//============================================================================
// external
#include "utils.h"
#include "iterators.h"
namespace pfc
{

// new
struct compare_predicate;
template<typename> class owner_array;
template<typename> class array;
template<typename> class deque;
// swap functions
template<typename T> PFC_INLINE void swap(owner_array<T>&, owner_array<T>&);
template<typename T> PFC_INLINE void swap(array<T>&, array<T>&);
template<typename T> PFC_INLINE void swap(deque<T>&, deque<T>&);
//----------------------------------------------------------------------------


//============================================================================
// compare_predicate
//============================================================================
struct compare_predicate
{
  template<typename T, typename U> PFC_INLINE bool before(const T&, const U&) const;
  template<typename T, typename U> PFC_INLINE bool equal(const T&, const U&) const;
};
//----------------------------------------------------------------------------


//============================================================================
// owner_array
//============================================================================
template<typename T>
class owner_array
{ PFC_MONO(owner_array) PFC_INTROSPEC_DECL;
public:
  // construction
  PFC_INLINE owner_array();
  PFC_INLINE owner_array(T*, usize_t size_);
  PFC_INLINE owner_array(const owner_array&);
  PFC_INLINE void set(T*, usize_t size_);
  PFC_INLINE void operator=(const owner_array&);
  PFC_INLINE ~owner_array();
  PFC_INLINE void clear();
  PFC_INLINE void swap(owner_array&);
  owner_data steal_data();
  //--------------------------------------------------------------------------

  // accessors
  PFC_INLINE friend bool is_valid(const owner_array &arr_)  {return arr_.data!=0;}
  PFC_INLINE friend void *ptr(const owner_array &arr_)      {return arr_.data;}
  //--------------------------------------------------------------------------

  mutable usize_t size;
  mutable T *data;
  //--------------------------------------------------------------------------

private:
  template<class PE> void stream_array(PE&, meta_case<0> is_type_void_);
  template<class PE> void stream_array(PE&, meta_case<1> default_);
};
PFC_SET_TYPE_TRAIT_PARTIAL(typename T, owner_array<T>, is_type_pod_def_ctor, true);
PFC_SET_TYPE_TRAIT_PARTIAL(typename T, owner_array<T>, is_type_pod_move, true);
//----------------------------------------------------------------------------


//============================================================================
// array
//============================================================================
template<typename T>
class array
{ PFC_MONO(array) PFC_INTROSPEC_DECL;
public:
  // nested types
  typedef const T *const_iterator;
  typedef T *iterator;
  typedef array_iterator<typename add_const<T>::res, false> const_reverse_iterator;
  typedef array_iterator<T, false> reverse_iterator;
  //--------------------------------------------------------------------------

  // construction
  array(memory_allocator_base *alloc_=0);
  array(const array&);
  array(const array&, memory_allocator_base*);
  explicit array(usize_t size_, memory_allocator_base *alloc_=0);
  array(usize_t size_, const T&, memory_allocator_base *alloc_=0);
  PFC_INLINE ~array();
  void operator=(const array&);
  void set_allocator(memory_allocator_base*);
  void clear();
  void reset_size(usize_t size_);
  void reset_size(usize_t size_, const T&);
  void resize(usize_t size_);
  void resize(usize_t size_, const T&);
  void resize_to_zero();
  void reserve(usize_t capacity_);
  void trim(usize_t permitted_slack_=0);
  PFC_INLINE void swap(array&);
  //--------------------------------------------------------------------------

  // accessors and mutators
  PFC_INLINE memory_allocator_base &allocator() const;
  PFC_INLINE usize_t size() const;
  PFC_INLINE usize_t capacity() const;
  PFC_INLINE const T &operator[](usize_t idx_) const;
  PFC_INLINE T &operator[](usize_t idx_);
  PFC_INLINE const T &front() const;
  PFC_INLINE T &front();
  PFC_INLINE const T &back() const;
  PFC_INLINE T &back();
  PFC_INLINE const T *data() const;
  PFC_INLINE T *data();
  PFC_INLINE const_iterator begin() const;
  PFC_INLINE iterator begin();
  PFC_INLINE const_reverse_iterator rbegin() const;
  PFC_INLINE reverse_iterator rbegin();
  PFC_INLINE const_iterator last() const;
  PFC_INLINE iterator last();
  PFC_INLINE const_reverse_iterator rlast() const;
  PFC_INLINE reverse_iterator rlast();
  PFC_INLINE const_iterator end() const;
  PFC_INLINE iterator end();
  PFC_INLINE const_reverse_iterator rend() const;
  PFC_INLINE reverse_iterator rend();
  PFC_INLINE void get(usize_t start_idx_, T*, usize_t num_items_) const;
  PFC_INLINE void set(usize_t start_idx_, const T*, usize_t num_items_);
  PFC_INLINE void set(usize_t start_idx_, const T&, usize_t num_items_);
  PFC_INLINE void push_front(const T&);
  PFC_INLINE T &push_front();
  PFC_INLINE void push_back(const T&);
  PFC_INLINE T &push_back();
  PFC_INLINE void insert_front(usize_t num_items_);
  PFC_INLINE void insert_front(usize_t num_items_, const T&);
  PFC_INLINE void insert_front(usize_t num_items_, const T*);
  PFC_INLINE void insert_back(usize_t num_items_);
  PFC_INLINE void insert_back(usize_t num_items_, const T&);
  PFC_INLINE void insert_back(usize_t num_items_, const T*);
  PFC_INLINE void pop_front();
  PFC_INLINE void pop_back();
  PFC_INLINE void remove_front(usize_t num_items_);
  PFC_INLINE void remove_back(usize_t num_items_);
  PFC_INLINE void remove_at(usize_t idx_);
  PFC_INLINE void remove_at(usize_t idx_, usize_t num_items_);
  owner_array<T> steal_data();
  //--------------------------------------------------------------------------

private:
  void reserve(usize_t capacity_, usize_t offset_);
  //--------------------------------------------------------------------------

  memory_allocator_base *m_allocator;
  usize_t m_size;
  usize_t m_capacity;
  T *m_data;
};
PFC_SET_TYPE_TRAIT_PARTIAL(typename T, array<T>, is_type_pod_move, true);
//----------------------------------------------------------------------------


//============================================================================
// deque
//============================================================================
template<typename T>
struct deque_traits
{
  // properties
  enum {sshift=meta_max<meta_log2<1024/sizeof(T)>::res, 4>::res,
        ssize=1<<sshift,
        smask=ssize-1,
        block_alloc_size=ssize*sizeof(T),
        block_alloc_align=meta_alignof<T>::res};
};
//----------------------------------------------------------------------------

template<typename T>
class deque
{ PFC_MONO(deque) PFC_INTROSPEC_DECL;
  template<bool is_forward> class const_iterator_impl;
  template<bool is_forward> class iterator_impl;
public:
  // nested types
  typedef const_iterator_impl<true> const_iterator;
  typedef const_iterator_impl<false> const_reverse_iterator;
  typedef iterator_impl<true> iterator;
  typedef iterator_impl<false> reverse_iterator;
  //--------------------------------------------------------------------------

  // construction
  deque(memory_allocator_base *alloc_=0);
  deque(const deque&);
  deque(const deque&, memory_allocator_base*);
  explicit deque(usize_t size_, memory_allocator_base *alloc_=0);
  deque(usize_t size_, const T&, memory_allocator_base *alloc_=0);
  PFC_INLINE ~deque();
  void operator=(const deque&);
  void set_allocator(memory_allocator_base*);
  void clear();
  void resize(usize_t size_);
  void resize(usize_t size_, const T&);
  PFC_INLINE void swap(deque&);
  //--------------------------------------------------------------------------

  // accessors and mutators
  PFC_INLINE memory_allocator_base &allocator() const;
  PFC_INLINE usize_t size() const;
  PFC_INLINE const T &operator[](usize_t idx_) const;
  PFC_INLINE T &operator[](usize_t idx_);
  PFC_INLINE const T &front() const;
  PFC_INLINE T &front();
  PFC_INLINE const T &back() const;
  PFC_INLINE T &back();
  PFC_INLINE const_iterator begin() const;
  PFC_INLINE iterator begin();
  PFC_INLINE const_reverse_iterator rbegin() const;
  PFC_INLINE reverse_iterator rbegin();
  PFC_INLINE const_iterator last() const;
  PFC_INLINE iterator last();
  PFC_INLINE const_reverse_iterator rlast() const;
  PFC_INLINE reverse_iterator rlast();
  PFC_INLINE const_iterator end() const;
  PFC_INLINE iterator end();
  PFC_INLINE const_reverse_iterator rend() const;
  PFC_INLINE reverse_iterator rend();
  PFC_INLINE void get(usize_t start_idx_, T*, usize_t num_items_) const;
  PFC_INLINE void set(usize_t start_idx_, const T*, usize_t num_items_);
  PFC_INLINE void set(usize_t start_idx_, const T&, usize_t num_items_);
  PFC_INLINE void push_front(const T&);
  PFC_INLINE T &push_front();
  PFC_INLINE void push_back(const T&);
  PFC_INLINE T &push_back();
  PFC_INLINE void insert_front(usize_t num_items_);
  PFC_INLINE void insert_front(usize_t num_items_, const T&);
  PFC_INLINE void insert_front(usize_t num_items_, const T*);
  PFC_INLINE void insert_back(usize_t num_items_);
  PFC_INLINE void insert_back(usize_t num_items_, const T&);
  PFC_INLINE void insert_back(usize_t num_items_, const T*);
  PFC_INLINE void pop_front();
  PFC_INLINE void pop_back();
  PFC_INLINE void remove_front(usize_t num_items_);
  PFC_INLINE void remove_back(usize_t num_items_);
  //--------------------------------------------------------------------------

private:
  void cctor(const deque&);
  void alloc_stride_front();
  void alloc_stride_back();
  void dealloc_stride_front();
  void dealloc_stride_back();
  //--------------------------------------------------------------------------

  memory_allocator_base *m_allocator;
  usize_t m_size;
  usize_t m_offset;
  T *m_first, *m_last, *m_stride_last;
  array<T*> m_strides;
};
PFC_SET_TYPE_TRAIT_PARTIAL(typename T, deque<T>, is_type_pod_move, true);
//----------------------------------------------------------------------------

//============================================================================
// deque::const_iterator_impl
//============================================================================
template<typename T>
template<bool is_forward>
class deque<T>::const_iterator_impl
{
public:
  // nested types
  typedef T value_t;
  //--------------------------------------------------------------------------

  // construction
  PFC_INLINE const_iterator_impl();
  PFC_INLINE const_iterator_impl(const iterator_impl<is_forward>&);
  PFC_INLINE void reset();
  //--------------------------------------------------------------------------

  // iteration
  PFC_INLINE friend bool is_valid(const const_iterator_impl &it_)  {return it_.m_deque && it_.m_index<it_.m_deque->size();}
  PFC_INLINE bool operator==(const const_iterator_impl&) const;
  PFC_INLINE bool operator==(const iterator_impl<is_forward>&) const;
  PFC_INLINE bool operator!=(const const_iterator_impl&) const;
  PFC_INLINE bool operator!=(const iterator_impl<is_forward>&) const;
  PFC_INLINE const_iterator_impl &operator++();
  PFC_INLINE const_iterator_impl &operator--();
  PFC_INLINE const_iterator_impl &operator+=(ssize_t offset_);
  PFC_INLINE const_iterator_impl &operator+=(const const_iterator_impl&);
  PFC_INLINE const_iterator_impl &operator+=(const iterator_impl<is_forward>&);
  PFC_INLINE const_iterator_impl &operator-=(ssize_t offset_);
  PFC_INLINE const_iterator_impl &operator-=(const const_iterator_impl&);
  PFC_INLINE const_iterator_impl &operator-=(const iterator_impl<is_forward>&);
  PFC_INLINE const_iterator_impl operator+(ssize_t offset_) const;
  PFC_INLINE const_iterator_impl operator+(const const_iterator_impl&) const;
  PFC_INLINE const_iterator_impl operator+(const iterator_impl<is_forward>&) const;
  PFC_INLINE const_iterator_impl operator-(ssize_t offset_) const;
  PFC_INLINE const_iterator_impl operator-(const const_iterator_impl&) const;
  PFC_INLINE const_iterator_impl operator-(const iterator_impl<is_forward>&) const;
  PFC_INLINE const T &operator[](usize_t index_) const;
  PFC_INLINE const T &operator*() const;
  PFC_INLINE const T *operator->() const;
  PFC_INLINE friend const T *ptr(const const_iterator_impl &it_)   {return it_.m_deque?&(*it_.m_deque)[it_.m_index]:0;}
  PFC_INLINE usize_t index() const;
  //--------------------------------------------------------------------------

private:
  friend class deque<T>;
  friend class iterator_impl<is_forward>;
  PFC_INLINE const_iterator_impl(const deque<T>&, usize_t index_);
  //--------------------------------------------------------------------------

  const deque<T> *m_deque;
  usize_t m_index;
};
//----------------------------------------------------------------------------

//============================================================================
// deque::iterator_impl
//============================================================================
template<typename T>
template<bool is_forward>
class deque<T>::iterator_impl
{
public:
  // nested types
  typedef T value_t;
  //--------------------------------------------------------------------------

  // construction
  PFC_INLINE iterator_impl();
  PFC_INLINE void reset();
  //--------------------------------------------------------------------------

  // iteration
  PFC_INLINE friend bool is_valid(const iterator_impl &it_)  {return it_.m_deque && it_.m_index<it_.m_deque->size();}
  PFC_INLINE bool operator==(const const_iterator_impl<is_forward>&) const;
  PFC_INLINE bool operator==(const iterator_impl&) const;
  PFC_INLINE bool operator!=(const const_iterator_impl<is_forward>&) const;
  PFC_INLINE bool operator!=(const iterator_impl&) const;
  PFC_INLINE iterator_impl &operator++();
  PFC_INLINE iterator_impl &operator--();
  PFC_INLINE iterator_impl &operator+=(ssize_t offset_);
  PFC_INLINE iterator_impl &operator+=(const const_iterator_impl<is_forward>&);
  PFC_INLINE iterator_impl &operator+=(const iterator_impl&);
  PFC_INLINE iterator_impl &operator-=(ssize_t offset_);
  PFC_INLINE iterator_impl &operator-=(const const_iterator_impl<is_forward>&);
  PFC_INLINE iterator_impl &operator-=(const iterator_impl&);
  PFC_INLINE iterator_impl operator+(ssize_t offset_) const;
  PFC_INLINE iterator_impl operator+(const const_iterator_impl<is_forward>&) const;
  PFC_INLINE iterator_impl operator+(const iterator_impl&) const;
  PFC_INLINE iterator_impl operator-(ssize_t offset_) const;
  PFC_INLINE iterator_impl operator-(const const_iterator_impl<is_forward>&) const;
  PFC_INLINE iterator_impl operator-(const iterator_impl&) const;
  PFC_INLINE T &operator[](usize_t index_) const;
  PFC_INLINE T &operator*() const;
  PFC_INLINE T *operator->() const;
  PFC_INLINE friend T *ptr(const iterator_impl &it_)         {return it_.m_deque?&(*it_.m_deque)[it_.m_index]:0;}
  PFC_INLINE usize_t index() const;
  //--------------------------------------------------------------------------

private:
  friend class deque<T>;
  friend class const_iterator_impl<is_forward>;
  PFC_INLINE iterator_impl(deque<T>&, usize_t index_);
  PFC_INLINE friend void move_construct(const iterator_impl &dst_, const iterator_impl &src_, usize_t count_)  {move_construct_impl(dst_, src_, count_);}
  static void move_construct_impl(const iterator_impl &dst_, const iterator_impl &src_, usize_t count_);
  //--------------------------------------------------------------------------

  deque<T> *m_deque;
  usize_t m_index;
};
//----------------------------------------------------------------------------

//============================================================================
#include "containers.inl"
} // namespace pfc
#endif
