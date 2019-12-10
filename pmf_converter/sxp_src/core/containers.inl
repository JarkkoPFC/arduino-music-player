//============================================================================
// Spin-X Platform
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================


//============================================================================
// compare_predicate
//============================================================================
template<typename T, typename U>
bool compare_predicate::before(const T &v0_, const U &v1_) const
{
  return v0_<v1_;
}
//----

template<typename T, typename U>
bool compare_predicate::equal(const T &v0_, const U &v1_) const
{
  return v0_==v1_;
}
//----------------------------------------------------------------------------


//============================================================================
// owner_array
//============================================================================
PFC_INTROSPEC_INL_TDEF1(typename T, owner_array<T>)
{
  PFC_CUSTOM_STREAMING(0);
  switch(unsigned(PE::pe_type))
  {
    case penum_input:
    {
      // read data
      clear();
      uint32 s;
      pe_.var(s);
      size=s;
      enum {item_size=is_type_equal<typename add_cv<T>::res, const volatile void>::res?1:meta_sizeof<T>::res};
      data=(T*)PFC_MEM_ALLOC(size*item_size);
      default_construct(data, size);
      stream_array(pe_, meta_case<is_type_equal<typename add_cv<T>::res, const volatile void>::res?0:1>());
    } break;

    case penum_output:
    case penum_display:
    {
      // write/display data
      PFC_CHECK_MSG(size<=0xffffffff, ("Unable to serialize owner_array<%s> that contains more than 2^32-1 elements\r\n", typeid(T).name()));
      uint32 s=(uint32)size;
      pe_.var(s, 0, "size");
      stream_array(pe_, meta_case<is_type_equal<typename add_cv<T>::res, const volatile void>::res?0:1>());
    } break;
  }
}
//----------------------------------------------------------------------------

template<typename T>
owner_array<T>::owner_array()
{
  size=0;
  data=0;
}
//----

template<typename T>
owner_array<T>::owner_array(T *data_, usize_t size_)
{
  size=size_;
  data=data_;
}
//----

template<typename T>
owner_array<T>::owner_array(const owner_array &arr_)
{
  size=arr_.size;
  data=arr_.data;
  arr_.size=0;
  arr_.data=0;
}
//----

template<typename T>
void owner_array<T>::set(T *data_, usize_t size_)
{
  if(data!=data_)
  {
    reverse_destruct(data, size);
    size=size_;
    data=data_;
  }
}
//----

template<typename T>
void owner_array<T>::operator=(const owner_array &arr_)
{
  if(this!=&arr_)
  {
    reverse_destruct(data, size);
    size=arr_.size;
    data=arr_.data;
    arr_.size=0;
    arr_.data=0;
  }
}
//----

template<typename T>
owner_array<T>::~owner_array()
{
  reverse_destruct(data, size);
  PFC_MEM_FREE(data);
}
//----

template<typename T>
void owner_array<T>::clear()
{
  reverse_destruct(data, size);
  PFC_MEM_FREE(data);
  data=0;
  size=0;
}
//----

template<typename T>
void owner_array<T>::swap(owner_array<T> &arr_)
{
  pfc::swap(size, arr_.size);
  pfc::swap(data, arr_.data);
}
//----

template<typename T>
owner_data owner_array<T>::steal_data()
{
  PFC_CTF_ASSERT_MSG(is_type_pod<T>::res, unable_to_get_data_of_non_pod_type);
  owner_data d(data);
  size=0;
  data=0;
  return d;
}
//----------------------------------------------------------------------------

template<typename T>
template<class PE>
void owner_array<T>::stream_array(PE &pe_, meta_case<0> is_type_void_)
{
  pe_.data(data, size);
}
//----

template<typename T>
template<class PE>
void owner_array<T>::stream_array(PE &pe_, meta_case<1> default_)
{
  pe_.avar(data, size, mvarflag_mutable|mvarflag_mutable_ptr);
}
//----------------------------------------------------------------------------


//============================================================================
// array
//============================================================================
PFC_INTROSPEC_INL_TDEF1(typename T, array<T>)
{
  PFC_CUSTOM_STREAMING(0);
  switch(unsigned(PE::pe_type))
  {
    case penum_input:
    {
      // read data
      clear();
      uint32 size;
      pe_.var(size);
      resize(size);
      pe_.avar(m_data, size);
    } break;

    case penum_output:
    case penum_display:
    {
      // write/display data
      PFC_CHECK_MSG(m_size<=0xffffffff, ("Unable to serialize array<%s> that contains more than 2^32-1 elements\r\n", typeid(T).name()));
      uint32 size=(uint32)m_size;
      pe_.var(size, 0, "size");
      pe_.avar(m_data, size, mvarflag_mutable|mvarflag_mutable_ptr);
    } break;
  }
}
//----------------------------------------------------------------------------

template<typename T>
array<T>::array(memory_allocator_base *alloc_)
  :m_allocator(alloc_?alloc_:&default_memory_allocator::inst())
{
  m_size=0;
  m_capacity=0;
  m_data=0;
}
//----

template<typename T>
array<T>::array(const array &a_)
  :m_allocator(a_.m_allocator)
{
  // copy-construct the array from array
  eh_data<T> p(*m_allocator, a_.m_size, meta_alignof<T>::res);
  m_data=p.data;
  copy_construct(m_data, a_.m_data, a_.m_size);
  m_size=m_capacity=a_.m_size;
  p.reset();
}
//----

template<typename T>
array<T>::array(const array &a_, memory_allocator_base *alloc_)
  :m_allocator(alloc_?alloc_:&default_memory_allocator::inst())
{
  // copy-construct the array from array
  eh_data<T> p(*m_allocator, a_.m_size, meta_alignof<T>::res);
  m_data=p.data;
  copy_construct(m_data, a_.m_data, a_.m_size);
  m_size=m_capacity=a_.m_size;
  p.reset();
}
//----

template<typename T>
array<T>::array(usize_t size_, memory_allocator_base *alloc_)
  :m_allocator(alloc_?alloc_:&default_memory_allocator::inst())
{
  // default construct the array
  eh_data<T> p(*m_allocator, size_, meta_alignof<T>::res);
  m_data=p.data;
  default_construct(m_data, size_);
  m_size=m_capacity=size_;
  p.reset();
}
//----

template<typename T>
array<T>::array(usize_t size_, const T &v_, memory_allocator_base *alloc_)
  :m_allocator(alloc_?alloc_:&default_memory_allocator::inst())
{
  // copy-construct the array from value
  eh_data<T> p(*m_allocator, size_, meta_alignof<T>::res);
  m_data=p.data;
  copy_construct(m_data, v_, size_);
  m_size=m_capacity=size_;
  p.reset();
}
//----

template<typename T>
array<T>::~array()
{
  clear();
}
//----

template<typename T>
void array<T>::operator=(const array &a_)
{
  array<T> a(a_);
  swap(a);
}
//----

template<typename T>
void array<T>::set_allocator(memory_allocator_base *alloc_)
{
  PFC_ASSERT_MSG(!m_data, ("Unable to change the allocator of an array with capacity\r\n"));
  m_allocator=alloc_?alloc_:&default_memory_allocator::inst();
}
//----

template<typename T>
void array<T>::clear()
{
  // clear the array
  reverse_destruct(m_data, m_size);
  if(m_data)
    m_allocator->free(m_data);
  m_size=0;
  m_capacity=0;
  m_data=0;
}
//----

template<typename T>
void array<T>::reset_size(usize_t size_)
{
  // reset array size to given size
  clear();
  eh_data<T> p(*m_allocator, size_, meta_alignof<T>::res);
  m_data=p.data;
  default_construct(m_data, size_);
  m_size=m_capacity=size_;
  p.reset();
}
//----

template<typename T>
void array<T>::reset_size(usize_t size_, const T &v_)
{
  // reset array size to given size of given value
  clear();
  eh_data<T> p(*m_allocator, size_, meta_alignof<T>::res);
  m_data=p.data;
  copy_construct(m_data, v_, size_);
  m_size=m_capacity=size_;
  p.reset();
}
//----

template<typename T>
void array<T>::resize(usize_t size_)
{
  // resize the array to given size
  if(size_>m_size)
  {
    if(m_capacity<size_)
      reserve(size_, 0);
    default_construct(m_data+m_size, size_-m_size);
  }
  else
    reverse_destruct(m_data+size_, m_size-size_);
  m_size=size_;
}
//----

template<typename T>
void array<T>::resize(usize_t size_, const T &v_)
{
  // resize the array to given size
  if(size_>m_size)
  {
    if(m_capacity<size_)
      reserve(size_, 0);
    copy_construct(m_data+m_size, v_, size_-m_size);
  }
  else
    reverse_destruct(m_data+size_, m_size-size_);
  m_size=size_;
}
//----

template<typename T>
void array<T>::resize_to_zero()
{
  reverse_destruct(m_data, m_size);
  m_size=0;
}
//----

template<typename T>
void array<T>::reserve(usize_t capacity_)
{
  if(m_capacity<capacity_)
  {
    // move content of the array to new memory location
    eh_data<T> p(*m_allocator, capacity_, meta_alignof<T>::res);
    move_construct(p.data, m_data, m_size);
    m_allocator->free(m_data);
    m_data=p.data;
    m_capacity=capacity_;
    p.reset();
  }
}
//----

template<typename T>
void array<T>::trim(usize_t permitted_slack_)
{
  // trim array capacity
  if(m_size<m_capacity+permitted_slack_)
  {
    array a(*this, m_allocator);
    swap(a);
  }
}
//----

template<typename T>
void array<T>::swap(array &a_)
{
  pfc::swap(m_allocator, a_.m_allocator);
  pfc::swap(m_size, a_.m_size);
  pfc::swap(m_capacity, a_.m_capacity);
  pfc::swap(m_data, a_.m_data);
}
//----------------------------------------------------------------------------

template<typename T>
memory_allocator_base &array<T>::allocator() const
{
  return *m_allocator;
}
//----

template<typename T>
usize_t array<T>::size() const
{
  return m_size;
}
//----

template<typename T>
usize_t array<T>::capacity() const
{
  return m_capacity;
}
//----

template<typename T>
const T &array<T>::operator[](usize_t idx_) const
{
  PFC_ASSERT_PEDANTIC_MSG(idx_<m_size, ("Trying to access element at index %u of array<%s> (size=%u)\r\n", idx_, typeid(T).name(), m_size));
  return m_data[idx_];
}
//----

template<typename T>
T &array<T>::operator[](usize_t idx_)
{
  PFC_ASSERT_PEDANTIC_MSG(idx_<m_size, ("Trying to access element at index %u of array<%s> (size=%u)\r\n", idx_, typeid(T).name(), m_size));
  return m_data[idx_];
}
//----

template<typename T>
const T &array<T>::front() const
{
  PFC_ASSERT_PEDANTIC(m_size);
  return *m_data;
}
//----

template<typename T>
T &array<T>::front()
{
  PFC_ASSERT_PEDANTIC(m_size);
  return *m_data;
}
//----

template<typename T>
const T &array<T>::back() const
{
  PFC_ASSERT_PEDANTIC(m_size);
  return m_data[m_size-1];
}
//----

template<typename T>
T &array<T>::back()
{
  PFC_ASSERT_PEDANTIC(m_size);
  return m_data[m_size-1];
}
//----

template<typename T>
const T *array<T>::data() const
{
  return m_data;
}
//----

template<typename T>
T *array<T>::data()
{
  return m_data;
}
//----

template<typename T>
typename array<T>::const_iterator array<T>::begin() const
{
  return m_data;
}
//----

template<typename T>
typename array<T>::iterator array<T>::begin()
{
  return m_data;
}
//----

template<typename T>
typename array<T>::const_reverse_iterator array<T>::rbegin() const
{
  return m_data+m_size-1;
}
//----

template<typename T>
typename array<T>::reverse_iterator array<T>::rbegin()
{
  return m_data+m_size-1;
}
//----

template<typename T>
typename array<T>::const_iterator array<T>::last() const
{
  return m_data+m_size-1;
}
//----

template<typename T>
typename array<T>::iterator array<T>::last()
{
  return m_data+m_size-1;
}
//----

template<typename T>
typename array<T>::const_reverse_iterator array<T>::rlast() const
{
  return m_data;
}
//----

template<typename T>
typename array<T>::reverse_iterator array<T>::rlast()
{
  return m_data;
}
//----

template<typename T>
typename array<T>::const_iterator array<T>::end() const
{
  return m_data+m_size;
}
//----

template<typename T>
typename array<T>::iterator array<T>::end()
{
  return m_data+m_size;
}
//----

template<typename T>
typename array<T>::const_reverse_iterator array<T>::rend() const
{
  return m_data-1;
}
//----

template<typename T>
typename array<T>::reverse_iterator array<T>::rend()
{
  return m_data-1;
}
//----

template<typename T>
void array<T>::get(usize_t start_idx_, T *p_, usize_t num_items_) const
{
  // get items to the given array from the container
  PFC_ASSERT_MSG(start_idx_+num_items_<=m_size, ("Reading values past the end of the container\r\n"));
  destruct(p_, num_items_);
  copy_construct(p_, m_data+start_idx_, num_items_);
}
//----

template<typename T>
void array<T>::set(usize_t start_idx_, const T *p_, usize_t num_items_)
{
  // set items from the given array to the container
  PFC_ASSERT_MSG(start_idx_+num_items_<=m_size, ("Writing values past the end of the container\r\n"));
  destruct(m_data+start_idx_, num_items_);
  copy_construct(m_data+start_idx_, p_, num_items_);
}
//----

template<typename T>
void array<T>::set(usize_t start_idx_, const T &v_, usize_t num_items_)
{
  // set items from the given array to the container
  PFC_ASSERT_MSG(start_idx_+num_items_<=m_size, ("Writing values past the end of the container\r\n"));
  destruct(m_data+start_idx_, num_items_);
  copy_construct(m_data+start_idx_, v_, num_items_);
}
//----

template<typename T>
void array<T>::push_front(const T &v_)
{
  // push the value to the beginning of the array
  if(m_size==m_capacity)
  {
    reserve(m_size?m_size*2:4, 1);
  }
  else
    move_construct(m_data+1, m_data, m_size);
  PFC_PNEW(m_data)T(v_);
  ++m_size;
}
//----

template<typename T>
T &array<T>::push_front()
{
  // push default to the beginning of the array
  if(m_size==m_capacity)
  {
    reserve(m_size?m_size*2:4, 1);
  }
  else
    move_construct(m_data+1, m_data, m_size);
  PFC_PNEW(m_data)T;
  ++m_size;
  return *m_data;
}
//----

template<typename T>
void array<T>::push_back(const T &v_)
{
  // push the value to the end of the array
  if(m_size==m_capacity)
  {
    reserve(m_size?m_size*2:4, 0);
  }
  PFC_PNEW(m_data+m_size)T(v_);
  ++m_size;
}
//----

template<typename T>
T &array<T>::push_back()
{
  // push default value to the end of the array
  if(m_size==m_capacity)
  {
    reserve(m_size?m_size*2:4, 0);
  }
  PFC_PNEW(m_data+m_size)T;
  return m_data[m_size++];
}
//----

template<typename T>
void array<T>::insert_front(usize_t num_items_)
{
  // insert items to the beginning of the array
  usize_t new_size=m_size+num_items_;
  if(m_capacity<new_size)
  {
    reserve(max(usize_t(4), new_size, m_size*2), num_items_);
  }
  else
    move_construct(m_data+num_items_, m_data, m_size);
  default_construct(m_data, num_items_);
  m_size=new_size;
}
//----

template<typename T>
void array<T>::insert_front(usize_t num_items_, const T &v_)
{
  // insert items to the beginning of the array
  usize_t new_size=m_size+num_items_;
  if(m_capacity<new_size)
  {
    reserve(max(usize_t(4), new_size, m_size*2), num_items_);
  }
  else
    move_construct(m_data+num_items_, m_data, m_size);
  copy_construct(m_data, v_, num_items_);
  m_size=new_size;
}
//----

template<typename T>
void array<T>::insert_front(usize_t num_items_, const T *v_)
{
  // insert items to the beginning of the array
  usize_t new_size=m_size+num_items_;
  if(m_capacity<new_size)
  {
    reserve(max(usize_t(4), new_size, m_size*2), num_items_);
  }
  else
    move_construct(m_data+num_items_, m_data, m_size);
  copy_construct(m_data, v_, num_items_);
  m_size=new_size;
}
//----

template<typename T>
void array<T>::insert_back(usize_t num_items_)
{
  // insert items to the end of the array
  usize_t new_size=m_size+num_items_;
  if(m_capacity<new_size)
  {
    reserve(max(usize_t(4), new_size, m_size*2), 0);
  }
  default_construct(m_data+m_size, num_items_);
  m_size=new_size;
}
//----

template<typename T>
void array<T>::insert_back(usize_t num_items_, const T &v_)
{
  // insert items to the end of the array
  usize_t new_size=m_size+num_items_;
  if(m_capacity<new_size)
  {
    reserve(max(usize_t(4), new_size, m_size*2), 0);
  }
  copy_construct(m_data+m_size, v_, num_items_);
  m_size=new_size;
}
//----

template<typename T>
void array<T>::insert_back(usize_t num_items_, const T *v_)
{
  // insert items to the end of the array
  usize_t new_size=m_size+num_items_;
  if(m_capacity<new_size)
  {
    reserve(max(usize_t(4), new_size, m_size*2), 0);
  }
  copy_construct(m_data+m_size, v_, num_items_);
  m_size=new_size;
}
//----

template<typename T>
void array<T>::pop_front()
{
  // remove the first element from the beginning of the array
  PFC_ASSERT_PEDANTIC(m_size);
  m_data[0].~T();
  move_construct(m_data, m_data+1, --m_size);
}
//----

template<typename T>
void array<T>::pop_back()
{
  // remove the last element from the end of the array
  PFC_ASSERT_PEDANTIC(m_size);
  --m_size;
  m_data[m_size].~T();
}
//----

template<typename T>
void array<T>::remove_front(usize_t num_items_)
{
  // remove given number of items from the beginning of the array
  PFC_ASSERT_PEDANTIC(m_size>=num_items_);
  m_size-=num_items_;
  destruct(m_data, num_items_);
  move_construct(m_data, m_data+num_items_, m_size);
}
//----

template<typename T>
void array<T>::remove_back(usize_t num_items_)
{
  // remove given number of items from the end of the array
  PFC_ASSERT_PEDANTIC(m_size>=num_items_);
  reverse_destruct(m_data+m_size-num_items_, num_items_);
  m_size-=num_items_;
}
//----

template<typename T>
void array<T>::remove_at(usize_t idx_)
{
  // remove item at given index
  PFC_ASSERT_PEDANTIC(idx_<m_size);
  m_data[idx_].~T();
  move_construct(m_data+idx_, m_data+idx_+1, m_size-idx_-1);
  --m_size;
}
//----

template<typename T>
void array<T>::remove_at(usize_t idx_, usize_t num_items_)
{
  // remove given number of items starting at given index
  PFC_ASSERT_PEDANTIC(idx_+num_items_<=m_size);
  destruct(m_data+idx_, num_items_);
  move_construct(m_data+idx_, m_data+idx_+num_items_, m_size-idx_-num_items_);
  m_size-=num_items_;
}
//----

template<typename T>
owner_array<T> array<T>::steal_data()
{
  // pass ownership of the data to the caller
  owner_array<T> arr(m_data, m_size);
  m_size=0;
  m_capacity=0;
  m_data=0;
  return arr;
}
//----------------------------------------------------------------------------

template<typename T>
void array<T>::reserve(usize_t capacity_, usize_t offset_)
{
  // move content of the array to new memory location
  eh_data<T> p(*m_allocator, capacity_, meta_alignof<T>::res);
  move_construct(p.data+offset_, m_data, m_size);
  m_allocator->free(m_data);
  m_data=p.data;
  m_capacity=capacity_;
  p.reset();
}
//----------------------------------------------------------------------------


//============================================================================
// deque
//============================================================================
PFC_INTROSPEC_INL_TDEF1(typename T, deque<T>)
{
  PFC_CUSTOM_STREAMING(0);
  switch(unsigned(PE::pe_type))
  {
    case penum_input:
    {
      // read data
      clear();
      uint32 size;
      pe_.var(size);
      resize(size);
      for(uint32 i=0; i<size; ++i)
        pe_.var((*this)[i], i?mvarflag_array_tail:0);
    } break;

    case penum_output:
    case penum_display:
    {
      // write/display data
      PFC_CHECK_MSG(m_size<=0xffffffff, ("Unable to serialize deque<%s> that contains more than 2^32-1 elements\r\n", typeid(T).name()));
      uint32 size=(uint32)m_size;
      pe_.var(size, 0, "size");
      for(uint32 i=0; i<size; ++i)
        pe_.var((*this)[i], mvarflag_mutable|mvarflag_mutable_ptr|(i?mvarflag_array_tail:0));
    } break;
  }
}
//----------------------------------------------------------------------------

template<typename T>
deque<T>::deque(memory_allocator_base *alloc_)
  :m_allocator(alloc_?alloc_:&default_memory_allocator::inst())
  ,m_size(0)
  ,m_offset(0)
  ,m_first(0)
  ,m_last(0)
  ,m_stride_last(0)
{
  PFC_ASSERT_CALL(m_allocator->check_allocator(deque_traits<T>::block_alloc_size, deque_traits<T>::block_alloc_align));
}
//----

template<typename T>
deque<T>::deque(const deque &dq_)
  :m_allocator(&default_memory_allocator::inst())
{
  cctor(dq_);
}
//----

template<typename T>
deque<T>::deque(const deque &dq_, memory_allocator_base *alloc_)
  :m_allocator(alloc_?alloc_:&default_memory_allocator::inst())
{
  PFC_ASSERT_CALL(m_allocator->check_allocator(deque_traits<T>::block_alloc_size, deque_traits<T>::block_alloc_align));
  cctor(dq_);
}
//----

template<typename T>
deque<T>::deque(usize_t size_, memory_allocator_base *alloc_)
  :m_allocator(alloc_?alloc_:&default_memory_allocator::inst())
{
  // default construct the deque
  PFC_ASSERT_CALL(m_allocator->check_allocator(deque_traits<T>::block_alloc_size, deque_traits<T>::block_alloc_align));
  m_size=0;
  m_offset=0;
  m_strides.resize((size_+deque_traits<T>::smask)>>deque_traits<T>::sshift);
  PFC_EDTOR(*this, &deque<T>::clear);
  usize_t idx=0;
  while(m_size<size_)
  {
    // construct a stride
    eh_data<T> p(*m_allocator, deque_traits<T>::ssize, deque_traits<T>::block_alloc_align);
    m_strides[idx++]=p.data;
    usize_t num_vals=min<usize_t>(size_-m_size, deque_traits<T>::ssize);
    default_construct(p.data, num_vals);
    p.reset();
    m_size+=num_vals;
  }
  PFC_EDTOR_RESET();

  // setup first & last pointers
  if(m_size)
  {
    m_first=m_strides.front()+m_offset;
    m_last=m_strides.back()+((m_size-1)&deque_traits<T>::smask);
    m_stride_last=m_strides.back()+deque_traits<T>::smask;
  }
  else
    m_first=m_last=m_stride_last=0;
}
//----

template<typename T>
deque<T>::deque(usize_t size_, const T &v_, memory_allocator_base *alloc_)
  :m_allocator(alloc_?alloc_:&default_memory_allocator::inst())
{
  // copy-construct the deque from the value
  PFC_ASSERT_CALL(m_allocator->check_allocator(deque_traits<T>::block_alloc_size, deque_traits<T>::block_alloc_align));
  m_size=0;
  m_offset=0;
  m_strides.resize((size_+deque_traits<T>::smask)>>deque_traits<T>::sshift);
  PFC_EDTOR(*this, &deque<T>::clear);
  usize_t idx=0;
  while(m_size<size_)
  {
    // construct a stride
    eh_data<T> p(*m_allocator, deque_traits<T>::ssize, deque_traits<T>::block_alloc_align);
    m_strides[idx++]=p.data;
    usize_t num_vals=min<usize_t>(size_-m_size, deque_traits<T>::ssize);
    copy_construct(p.data, v_, num_vals);
    p.reset();
    m_size+=num_vals;
  }
  PFC_EDTOR_RESET();

  // setup first & last pointers
  if(m_size)
  {
    m_first=m_strides.front()+m_offset;
    m_last=m_strides.back()+((m_size-1)&deque_traits<T>::smask);
    m_stride_last=m_strides.back()+deque_traits<T>::smask;
  }
  else
    m_first=m_last=m_stride_last=0;
}
//----

template<typename T>
deque<T>::~deque()
{
  clear();
}
//----

template<typename T>
void deque<T>::operator=(const deque &dq_)
{
  deque<T> dq(dq_);
  swap(dq);
}
//----

template<typename T>
void deque<T>::set_allocator(memory_allocator_base *alloc_)
{
  PFC_ASSERT_MSG(!m_size, ("Unable to change the allocator of a non-empty deque\r\n"));
  m_allocator=alloc_?alloc_:&default_memory_allocator::inst();
  PFC_ASSERT_CALL(m_allocator->check_allocator(deque_traits<T>::block_alloc_size, deque_traits<T>::block_alloc_align));
}
//----

template<typename T>
void deque<T>::clear()
{
  // clear the deque
  usize_t idx=0;
  while(m_size)
  {
    usize_t num_vals=min(m_size, deque_traits<T>::ssize-m_offset);
    destruct(m_strides[idx]+m_offset, num_vals);
    m_allocator->free(m_strides[idx]);
    m_size-=num_vals;
    m_offset=0;
    ++idx;
  }
  m_first=m_last=m_stride_last=0;
  m_strides.clear();
}
//----

template<typename T>
void deque<T>::resize(usize_t size_)
{
  if(size_<m_size)
  {
    // release elements from the end of the deque until the size matches
    do
    {
      usize_t num_vals=min(((m_offset+m_size-1)&deque_traits<T>::smask)+1, m_size-size_);
      m_size-=num_vals;
      usize_t offset=(m_offset+m_size)&deque_traits<T>::smask;
      reverse_destruct(m_strides.back()+offset, num_vals);
      if(!offset || !m_size)
        dealloc_stride_back();
    } while(m_size>size_);
    if(!size_)
      m_offset=0;
  }
  else
  {
    // add elements to the end of the deque until the size matches
    while(m_size<size_)
    {
      usize_t offset=(m_offset+m_size)&deque_traits<T>::smask;
      usize_t num_vals=min(deque_traits<T>::ssize-offset, size_-m_size);
      if(!offset)
        alloc_stride_back();
      default_construct(m_strides.back()+offset, num_vals);
      m_size+=num_vals;
    }
  }

  // setup first & last pointers
  if(m_size)
  {
    m_first=m_strides.front()+m_offset;
    m_last=m_strides.back()+((m_size-1)&deque_traits<T>::smask);
    m_stride_last=m_strides.back()+deque_traits<T>::smask;
  }
  else
    m_first=m_last=m_stride_last=0;
}
//----

template<typename T>
void deque<T>::resize(usize_t size_, const T &v_)
{
  if(size_<m_size)
  {
    // release elements from the end of the deque until the size matches
    do
    {
      usize_t num_vals=min(((m_offset+m_size-1)&deque_traits<T>::smask)+1, m_size-size_);
      m_size-=num_vals;
      usize_t offset=(m_offset+m_size)&deque_traits<T>::smask;
      reverse_destruct(m_strides.back()+offset, num_vals);
      if(!offset || !m_size)
        dealloc_stride_back();
    } while(m_size>size_);
    if(!size_)
      m_offset=0;
  }
  else
  {
    // add elements to the end of the deque until the size matches
    while(m_size<size_)
    {
      usize_t offset=(m_offset+m_size)&deque_traits<T>::smask;
      usize_t num_vals=min(deque_traits<T>::ssize-offset, size_-m_size);
      if(!offset)
        alloc_stride_back();
      copy_construct(m_strides.back()+offset, v_, num_vals);
      m_size+=num_vals;
    }
  }

  // setup first & last pointers
  if(m_size)
  {
    m_first=m_strides.front()+m_offset;
    m_last=m_strides.back()+((m_size-1)&deque_traits<T>::smask);
    m_stride_last=m_strides.back()+deque_traits<T>::smask;
  }
  else
    m_first=m_last=m_stride_last=0;
}
//----

template<typename T>
void deque<T>::swap(deque &dq_)
{
  // swap content of deques
  pfc::swap(m_allocator, dq_.m_allocator);
  pfc::swap(m_size, dq_.m_size);
  pfc::swap(m_offset, dq_.m_offset);
  pfc::swap(m_first, dq_.m_first);
  pfc::swap(m_last, dq_.m_last);
  pfc::swap(m_stride_last, dq_.m_stride_last);
  m_strides.swap(dq_.m_strides);
}
//----------------------------------------------------------------------------

template<typename T>
memory_allocator_base &deque<T>::allocator() const
{
  return *m_allocator;
}
//----

template<typename T>
usize_t deque<T>::size() const
{
  return m_size;
}
//----

template<typename T>
const T &deque<T>::operator[](usize_t idx_) const
{
  // return nth element in the deque
  PFC_ASSERT_PEDANTIC_MSG(idx_<m_size, ("Trying to access element at index %u of deque<%s> (size=%u)\r\n", idx_, typeid(T).name(), m_size));
  idx_+=m_offset;
  return m_strides[idx_>>deque_traits<T>::sshift][idx_&deque_traits<T>::smask];
}
//----

template<typename T>
T &deque<T>::operator[](usize_t idx_)
{
  // return nth element in the deque
  PFC_ASSERT_PEDANTIC_MSG(idx_<m_size, ("Trying to access element at index %u of deque<%s> (size=%u)\r\n", idx_, typeid(T).name(), m_size));
  idx_+=m_offset;
  return m_strides[idx_>>deque_traits<T>::sshift][idx_&deque_traits<T>::smask];
}
//----

template<typename T>
const T &deque<T>::front() const
{
  // return the first element in the deque
  PFC_ASSERT_PEDANTIC(m_size);
  return *m_first;
}
//----

template<typename T>
T &deque<T>::front()
{
  // return the first element in the deque
  PFC_ASSERT_PEDANTIC(m_size);
  return *m_first;
}
//----

template<typename T>
const T &deque<T>::back() const
{
  // return the last element in the deque
  PFC_ASSERT_PEDANTIC(m_size);
  return *m_last;
}
//----

template<typename T>
T &deque<T>::back()
{
  // return the last element in the deque
  PFC_ASSERT_PEDANTIC(m_size);
  return *m_last;
}
//----

template<typename T>
typename deque<T>::const_iterator deque<T>::begin() const
{
  return const_iterator(*this, 0);
}
//----

template<typename T>
typename deque<T>::iterator deque<T>::begin()
{
  return iterator(*this, 0);
}
//----

template<typename T>
typename deque<T>::const_reverse_iterator deque<T>::rbegin() const
{
  return const_reverse_iterator(*this, m_size-1);
}
//----

template<typename T>
typename deque<T>::reverse_iterator deque<T>::rbegin()
{
  return reverse_iterator(*this, m_size-1);
}
//----

template<typename T>
typename deque<T>::const_iterator deque<T>::last() const
{
  return const_iterator(*this, m_size?m_size-1:0);
}
//----

template<typename T>
typename deque<T>::iterator deque<T>::last()
{
  return iterator(*this, m_size?m_size-1:0);
}
//----

template<typename T>
typename deque<T>::const_reverse_iterator deque<T>::rlast() const
{
  return const_reverse_iterator(*this, 0);
}
//----

template<typename T>
typename deque<T>::reverse_iterator deque<T>::rlast()
{
  return reverse_iterator(*this, 0);
}
//----

template<typename T>
typename deque<T>::const_iterator deque<T>::end() const
{
  return const_iterator(*this, m_size);
}
//----

template<typename T>
typename deque<T>::iterator deque<T>::end()
{
  return iterator(*this, m_size);
}
//----

template<typename T>
typename deque<T>::const_reverse_iterator deque<T>::rend() const
{
  return const_reverse_iterator(*this, usize_t(-1));
}
//----

template<typename T>
typename deque<T>::reverse_iterator deque<T>::rend()
{
  return reverse_iterator(*this, usize_t(-1));
}
//----

template<typename T>
void deque<T>::get(usize_t start_idx_, T *p_, usize_t num_items_) const
{
  // get items to the given array from the container
  PFC_ASSERT_MSG(start_idx_+num_items_<=m_size, ("Reading values past the end of the container\r\n"));
  destruct(p_, num_items_);
  start_idx_+=m_offset;
  while(num_items_)
  {
    // copy items in the stride to the given array
    usize_t num_stride_items=min(num_items_, deque_traits<T>::ssize-(start_idx_&deque_traits<T>::smask));
    copy_construct(p_, m_strides[start_idx_>>deque_traits<T>::sshift]+(start_idx_&deque_traits<T>::smask), num_stride_items);
    start_idx_+=num_stride_items;
    p_+=num_stride_items;
    num_items_-=num_stride_items;
  }
}
//----

template<typename T>
void deque<T>::set(usize_t start_idx_, const T *p_, usize_t num_items_)
{
  // set items from the given array to the container
  PFC_ASSERT_MSG(start_idx_+num_items_<=m_size, ("Writing values past the end of the container\r\n"));
  start_idx_+=m_offset;
  while(num_items_)
  {
    // copy items to the stride from the given array
    usize_t num_stride_items=min(num_items_, deque_traits<T>::ssize-(start_idx_&deque_traits<T>::smask));
    T *data=m_strides[start_idx_>>deque_traits<T>::sshift]+(start_idx_&deque_traits<T>::smask);
    destruct(data, num_stride_items);
    copy_construct(data, p_, num_stride_items);
    start_idx_+=num_stride_items;
    p_+=num_stride_items;
    num_items_-=num_stride_items;
  }
}
//----

template<typename T>
void deque<T>::set(usize_t start_idx_, const T &v_, usize_t num_items_)
{
  // set array of given item to the container
  PFC_ASSERT_MSG(start_idx_+num_items_<=m_size, ("Writing values past the end of the container\r\n"));
  start_idx_+=m_offset;
  while(num_items_)
  {
    // copy items to the stride from the given array
    usize_t num_stride_items=min(num_items_, deque_traits<T>::ssize-(start_idx_&deque_traits<T>::smask));
    T *data=m_strides[start_idx_>>deque_traits<T>::sshift]+(start_idx_&deque_traits<T>::smask);
    destruct(data, num_stride_items);
    copy_construct(data, v_, num_stride_items);
    start_idx_+=num_stride_items;
    num_items_-=num_stride_items;
  }
}
//----

template<typename T>
void deque<T>::push_front(const T &v_)
{
  // push new element to the beginning of the deque
  if(!m_offset)
    alloc_stride_front();
  --m_offset;
  PFC_PNEW(--m_first)T(v_);  // todo: handle exceptions
  ++m_size;
}
//----

template<typename T>
T &deque<T>::push_front()
{
  // push default value to the beginning of the deque
  if(!m_offset)
    alloc_stride_front();
  --m_offset;
  PFC_PNEW(--m_first)T;  // todo: handle exceptions
  ++m_size;
  return *m_first;
}
//----

template<typename T>
void deque<T>::push_back(const T &v_)
{
  // push new element to the end of the deque
  if(m_last==m_stride_last)
    alloc_stride_back();
  PFC_PNEW(++m_last)T(v_);  // todo: handle exceptions
  ++m_size;
}
//----

template<typename T>
T &deque<T>::push_back()
{
  // push default value to the end of the deque
  if(m_last==m_stride_last)
    alloc_stride_back();
  PFC_PNEW(++m_last)T;  // todo: handle exceptions
  ++m_size;
  return *m_last;
}
//----

template<typename T>
void deque<T>::insert_front(usize_t num_items_)
{
  // insert given number of default constructed items to the front of the deque
  m_size+=num_items_;
  while(num_items_)
  {
    if(!m_offset)
      alloc_stride_front();
    usize_t ni=min(m_offset, num_items_);
    num_items_-=ni;
    m_first-=ni;
    m_offset-=ni;
    reverse_default_construct(m_first, ni);
  }
}
//----

template<typename T>
void deque<T>::insert_front(usize_t num_items_, const T &v_)
{
  // insert given number of given items to the front of the deque
  m_size+=num_items_;
  while(num_items_)
  {
    if(!m_offset)
      alloc_stride_front();
    usize_t ni=min(m_offset, num_items_);
    num_items_-=ni;
    m_first-=ni;
    m_offset-=ni;
    reverse_copy_construct(m_first, v_, ni);
  }
}
//----

template<typename T>
void deque<T>::insert_front(usize_t num_items_, const T *v_)
{
  // insert given number of given items to the front of the deque
  m_size+=num_items_;
  while(num_items_)
  {
    if(!m_offset)
      alloc_stride_front();
    usize_t ni=min(m_offset, num_items_);
    num_items_-=ni;
    m_first-=ni;
    m_offset-=ni;
    reverse_copy_construct(m_first, v_, ni);
    v_+=ni;
  }
}
//----

template<typename T>
void deque<T>::insert_back(usize_t num_items_)
{
  resize(m_size+num_items_);
}
//----

template<typename T>
void deque<T>::insert_back(usize_t num_items_, const T &v_)
{
  resize(m_size+num_items_, v_);
}
//----

template<typename T>
void deque<T>::insert_back(usize_t num_items_, const T *v_)
{
  // insert array of values to the deque
  for(usize_t i=0; i<num_items_; ++i)
  {
    if(m_last==m_stride_last)
      alloc_stride_back();
    PFC_PNEW(++m_last)T(*v_++);  // todo: handle exceptions
  }
  m_size+=num_items_;
}
//----

template<typename T>
void deque<T>::pop_front()
{
  // remove the first element from the beginning of the deque
  PFC_ASSERT_PEDANTIC(m_size);
  --m_size;
  m_first++->~T();
  if(!((++m_offset)&deque_traits<T>::smask))
    dealloc_stride_front();
}
//----

template<typename T>
void deque<T>::pop_back()
{
  // remove the last element from the end of the deque
  PFC_ASSERT_PEDANTIC(m_size);
  --m_size;
  m_last--->~T();
  if(m_last==m_stride_last-deque_traits<T>::ssize)
    dealloc_stride_back();
}
//----

template<typename T>
void deque<T>::remove_front(usize_t num_items_)
{
  // remove given number of items from the beginning of the deque
  PFC_ASSERT_PEDANTIC(m_size>=num_items_);
  m_size-=num_items_;
  while(num_items_)
  {
    usize_t ni=min(num_items_, deque_traits<T>::ssize-m_offset);
    num_items_-=ni;
    destruct(m_first, ni);
    m_first+=ni;
    m_offset+=ni;
    if(!(m_offset&deque_traits<T>::smask))
      dealloc_stride_front();
  }
}
//----

template<typename T>
void deque<T>::remove_back(usize_t num_items_)
{
  // remove given number of items from the end of the deque
  PFC_ASSERT_PEDANTIC(m_size>=num_items_);
  m_size-=num_items_;
  while(num_items_)
  {
    usize_t ni=min(num_items_, usize_t(m_last-(m_stride_last-deque_traits<T>::ssize)));
    num_items_-=ni;
    m_last-=ni,
    reverse_destruct(m_last+1, ni);
    if(m_last==m_stride_last-deque_traits<T>::ssize)
      dealloc_stride_back();
  }
}
//----------------------------------------------------------------------------

template<typename T>
void deque<T>::cctor(const deque &dq_)
{
  // copy-construct the deque
  m_size=0;
  m_offset=dq_.m_offset;
  m_strides.resize(dq_.m_strides.size());
  usize_t idx=0, offset=m_offset;
  PFC_EDTOR(*this, &deque<T>::clear);
  while(m_size<dq_.m_size)
  {
    eh_data<T> p(*m_allocator, deque_traits<T>::ssize, deque_traits<T>::block_alloc_align);
    m_strides[idx]=p.data;
    usize_t num_vals=min(deque_traits<T>::ssize-offset, dq_.m_size-m_size);
    copy_construct(m_strides[idx]+offset, dq_.m_strides[idx]+offset, num_vals);
    p.reset();
    offset=0;
    ++idx;
    m_size+=num_vals;
  }
  PFC_EDTOR_RESET();

  // setup first & last pointers
  if(m_size)
  {
    m_first=m_strides.front()+m_offset;
    m_last=m_strides.back()+((m_size-1)&deque_traits<T>::smask);
    m_stride_last=m_strides.back()+deque_traits<T>::smask;
  }
  else
    m_first=m_last=m_stride_last=0;
}
//----

template<typename T>
void deque<T>::alloc_stride_front()
{
  // allocate new first stride
  eh_data<T> p(*m_allocator, deque_traits<T>::ssize, deque_traits<T>::block_alloc_align);
  m_strides.push_back(0);
  mem_move(m_strides.data()+1, m_strides.data(), sizeof(T*)*(m_strides.size()-1));
  m_strides.front()=p.data;
  m_first=p.data+deque_traits<T>::ssize;
  if(!m_size)
    m_last=m_stride_last=m_first-1;
  m_offset=deque_traits<T>::ssize;
  p.reset();
}
//----

template<typename T>
void deque<T>::alloc_stride_back()
{
  // allocate new last stride
  eh_data<T> p(*m_allocator, deque_traits<T>::ssize, deque_traits<T>::block_alloc_align);
  m_strides.push_back(p.data);
  m_last=p.data-1;
  m_stride_last=p.data+deque_traits<T>::smask;
  if(!m_size)
    m_first=p.data;
  p.reset();
}
//----

template<typename T>
void deque<T>::dealloc_stride_front()
{
  // deallocate first stride
  m_allocator->free(m_strides.front());
  mem_move(m_strides.data(), m_strides.data()+1, sizeof(T*)*(m_strides.size()-1));
  m_strides.pop_back();
  m_offset=0;
  if(!m_size)
    m_first=m_last=m_stride_last=0;
  else
    m_first=m_strides.front();
}
//----

template<typename T>
void deque<T>::dealloc_stride_back()
{
  // deallocate last stride
  m_allocator->free(m_strides.back());
  m_strides.pop_back();
  if(!m_size)
    m_first=m_last=m_stride_last=0;
  else
    m_last=m_stride_last=m_strides.back()+deque_traits<T>::smask;
}
//----------------------------------------------------------------------------


//============================================================================
// deque::const_iterator_impl
//============================================================================
template<typename T>
template<bool is_forward>
deque<T>::const_iterator_impl<is_forward>::const_iterator_impl()
  :m_deque(0)
  ,m_index(0)
{
}
//----

template<typename T>
template<bool is_forward>
deque<T>::const_iterator_impl<is_forward>::const_iterator_impl(const iterator_impl<is_forward> &it_)
  :m_deque(it_.m_deque)
  ,m_index(it_.m_index)
{
}
//----

template<typename T>
template<bool is_forward>
void deque<T>::const_iterator_impl<is_forward>::reset()
{
  m_deque=0;
  m_index=0;
}
//----------------------------------------------------------------------------

template<typename T>
template<bool is_forward>
bool deque<T>::const_iterator_impl<is_forward>::operator==(const const_iterator_impl &it_) const
{
  PFC_ASSERT_PEDANTIC(m_deque==it_.m_deque);
  return m_index==it_.m_index;
}
//----

template<typename T>
template<bool is_forward>
bool deque<T>::const_iterator_impl<is_forward>::operator==(const iterator_impl<is_forward> &it_) const
{
  PFC_ASSERT_PEDANTIC(m_deque==it_.m_deque);
  return m_index==it_.m_index;
}
//----

template<typename T>
template<bool is_forward>
bool deque<T>::const_iterator_impl<is_forward>::operator!=(const const_iterator_impl &it_) const
{
  PFC_ASSERT_PEDANTIC(m_deque==it_.m_deque);
  return m_index!=it_.m_index;
}
//----

template<typename T>
template<bool is_forward>
bool deque<T>::const_iterator_impl<is_forward>::operator!=(const iterator_impl<is_forward> &it_) const
{
  PFC_ASSERT_PEDANTIC(m_deque==it_.m_deque);
  return m_index!=it_.m_index;
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template const_iterator_impl<is_forward> &deque<T>::const_iterator_impl<is_forward>::operator++()
{
  m_index+=is_forward?1:usize_t(-1);
  return *this;
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template const_iterator_impl<is_forward> &deque<T>::const_iterator_impl<is_forward>::operator--()
{
  m_index+=is_forward?usize_t(-1):1;
  return *this;
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template const_iterator_impl<is_forward> &deque<T>::const_iterator_impl<is_forward>::operator+=(ssize_t offset_)
{
  m_index+=is_forward?offset_:-offset_;
  return *this;
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template const_iterator_impl<is_forward> &deque<T>::const_iterator_impl<is_forward>::operator+=(const const_iterator_impl &it_)
{
  PFC_ASSERT_PEDANTIC(m_deque==it_.m_deque);
  m_index+=is_forward?it_.m_index:m_deque->m_size-1-it_.m_index;
  return *this;
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template const_iterator_impl<is_forward> &deque<T>::const_iterator_impl<is_forward>::operator+=(const iterator_impl<is_forward> &it_)
{
  PFC_ASSERT_PEDANTIC(m_deque==it_.m_deque);
  m_index+=is_forward?it_.m_index:m_deque->m_size-1-it_.m_index;
  return *this;
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template const_iterator_impl<is_forward> &deque<T>::const_iterator_impl<is_forward>::operator-=(ssize_t offset_)
{
  m_index+=is_forward?-offset_:offset_;
  return *this;
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template const_iterator_impl<is_forward> &deque<T>::const_iterator_impl<is_forward>::operator-=(const const_iterator_impl &it_)
{
  PFC_ASSERT_PEDANTIC(m_deque==it_.m_deque);
  m_index+=is_forward?-it_.m_index:it_.m_index-m_deque->m_size+1;
  return *this;
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template const_iterator_impl<is_forward> &deque<T>::const_iterator_impl<is_forward>::operator-=(const iterator_impl<is_forward> &it_)
{
  PFC_ASSERT_PEDANTIC(m_deque==it_.m_deque);
  m_index+=is_forward?-it_.m_index:it_.m_index-m_deque->m_size+1;
  return *this;
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template const_iterator_impl<is_forward> deque<T>::const_iterator_impl<is_forward>::operator+(ssize_t offset_) const
{
  return const_iterator_impl(*m_deque, m_index+(is_forward?offset_:-offset_));
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template const_iterator_impl<is_forward> deque<T>::const_iterator_impl<is_forward>::operator+(const const_iterator_impl &it_) const
{
  PFC_ASSERT_PEDANTIC(m_deque==it_.m_deque);
  return const_iterator_impl(*m_deque, m_index+(is_forward?it_.m_index:m_deque->m_size-1-it_.m_index));
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template const_iterator_impl<is_forward> deque<T>::const_iterator_impl<is_forward>::operator+(const iterator_impl<is_forward> &it_) const
{
  PFC_ASSERT_PEDANTIC(m_deque==it_.m_deque);
  return const_iterator_impl(*m_deque, m_index+(is_forward?it_.m_index:m_deque->m_size-1-it_.m_index));
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template const_iterator_impl<is_forward> deque<T>::const_iterator_impl<is_forward>::operator-(ssize_t offset_) const
{
  return const_iterator_impl(*m_deque, m_index+(is_forward?-offset_:offset_));
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template const_iterator_impl<is_forward> deque<T>::const_iterator_impl<is_forward>::operator-(const const_iterator_impl &it_) const
{
  PFC_ASSERT_PEDANTIC(m_deque==it_.m_deque);
  return const_iterator_impl(*m_deque, m_index+(is_forward?-it_.m_index:it_.m_index-m_deque->m_size+1));
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template const_iterator_impl<is_forward> deque<T>::const_iterator_impl<is_forward>::operator-(const iterator_impl<is_forward> &it_) const
{
  PFC_ASSERT_PEDANTIC(m_deque==it_.m_deque);
  return const_iterator_impl(*m_deque, m_index+(is_forward?-it_.m_index:it_.m_index-m_deque->m_size+1));
}
//----

template<typename T>
template<bool is_forward>
const T &deque<T>::const_iterator_impl<is_forward>::operator[](usize_t index_) const
{
  return (*m_deque)[m_index+index_];
}
//----

template<typename T>
template<bool is_forward>
const T &deque<T>::const_iterator_impl<is_forward>::operator*() const
{
  return (*m_deque)[m_index];
}
//----

template<typename T>
template<bool is_forward>
const T *deque<T>::const_iterator_impl<is_forward>::operator->() const
{
  return &(*m_deque)[m_index];
}
//----

template<typename T>
template<bool is_forward>
usize_t deque<T>::const_iterator_impl<is_forward>::index() const
{
  return m_index;
}
//----------------------------------------------------------------------------

template<typename T>
template<bool is_forward>
deque<T>::const_iterator_impl<is_forward>::const_iterator_impl(const deque<T> &deque_, usize_t index_)
  :m_deque(&deque_)
  ,m_index(index_)
{
}
//----------------------------------------------------------------------------


//============================================================================
// deque::iterator_impl
//============================================================================
template<typename T>
template<bool is_forward>
deque<T>::iterator_impl<is_forward>::iterator_impl()
  :m_deque(0)
  ,m_index(0)
{
}
//----

template<typename T>
template<bool is_forward>
void deque<T>::iterator_impl<is_forward>::reset()
{
  m_deque=0;
  m_index=0;
}
//----------------------------------------------------------------------------

template<typename T>
template<bool is_forward>
bool deque<T>::iterator_impl<is_forward>::operator==(const const_iterator_impl<is_forward> &it_) const
{
  PFC_ASSERT_PEDANTIC(m_deque==it_.m_deque);
  return m_index==it_.m_index;
}
//----

template<typename T>
template<bool is_forward>
bool deque<T>::iterator_impl<is_forward>::operator==(const iterator_impl &it_) const
{
  PFC_ASSERT_PEDANTIC(m_deque==it_.m_deque);
  return m_index==it_.m_index;
}
//----

template<typename T>
template<bool is_forward>
bool deque<T>::iterator_impl<is_forward>::operator!=(const const_iterator_impl<is_forward> &it_) const
{
  PFC_ASSERT_PEDANTIC(m_deque==it_.m_deque);
  return m_index!=it_.m_index;
}
//----

template<typename T>
template<bool is_forward>
bool deque<T>::iterator_impl<is_forward>::operator!=(const iterator_impl &it_) const
{
  PFC_ASSERT_PEDANTIC(m_deque==it_.m_deque);
  return m_index!=it_.m_index;
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template iterator_impl<is_forward> &deque<T>::iterator_impl<is_forward>::operator++()
{
  m_index+=is_forward?1:usize_t(-1);
  return *this;
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template iterator_impl<is_forward> &deque<T>::iterator_impl<is_forward>::operator--()
{
  m_index+=is_forward?usize_t(-1):1;
  return *this;
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template iterator_impl<is_forward> &deque<T>::iterator_impl<is_forward>::operator+=(ssize_t offset_)
{
  m_index+=is_forward?offset_:-offset_;
  return *this;
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template iterator_impl<is_forward> &deque<T>::iterator_impl<is_forward>::operator+=(const const_iterator_impl<is_forward> &it_)
{
  PFC_ASSERT_PEDANTIC(m_deque==it_.m_deque);
  m_index+=is_forward?it_.m_index:m_deque->m_size-1-it_.m_index;
  return *this;
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template iterator_impl<is_forward> &deque<T>::iterator_impl<is_forward>::operator+=(const iterator_impl &it_)
{
  PFC_ASSERT_PEDANTIC(m_deque==it_.m_deque);
  m_index+=is_forward?it_.m_index:m_deque->m_size-1-it_.m_index;
  return *this;
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template iterator_impl<is_forward> &deque<T>::iterator_impl<is_forward>::operator-=(ssize_t offset_)
{
  m_index-=offset_;
  return *this;
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template iterator_impl<is_forward> &deque<T>::iterator_impl<is_forward>::operator-=(const const_iterator_impl<is_forward> &it_)
{
  PFC_ASSERT_PEDANTIC(m_deque==it_.m_deque);
  m_index+=is_forward?-it_.m_index:it_.m_index-m_deque->m_size+1;
  return *this;
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template iterator_impl<is_forward> &deque<T>::iterator_impl<is_forward>::operator-=(const iterator_impl &it_)
{
  PFC_ASSERT_PEDANTIC(m_deque==it_.m_deque);
  m_index+=is_forward?-it_.m_index:it_.m_index-m_deque->m_size+1;
  return *this;
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template iterator_impl<is_forward> deque<T>::iterator_impl<is_forward>::operator+(ssize_t offset_) const
{
  return iterator_impl(*m_deque, m_index+(is_forward?offset_:-offset_));
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template iterator_impl<is_forward> deque<T>::iterator_impl<is_forward>::operator+(const const_iterator_impl<is_forward> &it_) const
{
  PFC_ASSERT_PEDANTIC(m_deque==it_.m_deque);
  return iterator_impl(*m_deque, m_index+(is_forward?it_.m_index:m_deque->m_size-1-it_.m_index));
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template iterator_impl<is_forward> deque<T>::iterator_impl<is_forward>::operator+(const iterator_impl &it_) const
{
  PFC_ASSERT_PEDANTIC(m_deque==it_.m_deque);
  return iterator(*m_deque, m_index+(is_forward?it_.m_index:m_deque->m_size-1-it_.m_index));
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template iterator_impl<is_forward> deque<T>::iterator_impl<is_forward>::operator-(ssize_t offset_) const
{
  return iterator_impl(*m_deque, m_index+(is_forward?-offset_:offset_));
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template iterator_impl<is_forward> deque<T>::iterator_impl<is_forward>::operator-(const const_iterator_impl<is_forward> &it_) const
{
  PFC_ASSERT_PEDANTIC(m_deque==it_.m_deque);
  return iterator_impl(*m_deque, m_index+(is_forward?-it_.m_index:it_.m_index-m_deque->m_size+1));
}
//----

template<typename T>
template<bool is_forward>
typename deque<T>::template iterator_impl<is_forward> deque<T>::iterator_impl<is_forward>::operator-(const iterator_impl &it_) const
{
  PFC_ASSERT_PEDANTIC(m_deque==it_.m_deque);
  return iterator_impl(*m_deque, m_index+(is_forward?-it_.m_index:it_.m_index-m_deque->m_size+1));
}
//----

template<typename T>
template<bool is_forward>
T &deque<T>::iterator_impl<is_forward>::operator[](usize_t index_) const
{
  return (*m_deque)[m_index+index_];
}
//----

template<typename T>
template<bool is_forward>
T &deque<T>::iterator_impl<is_forward>::operator*() const
{
  return (*m_deque)[m_index];
}
//----

template<typename T>
template<bool is_forward>
T *deque<T>::iterator_impl<is_forward>::operator->() const
{
  return &(*m_deque)[m_index];
}
//----

template<typename T>
template<bool is_forward>
usize_t deque<T>::iterator_impl<is_forward>::index() const
{
  return m_index;
}
//----------------------------------------------------------------------------

template<typename T>
template<bool is_forward>
deque<T>::iterator_impl<is_forward>::iterator_impl(deque<T> &deque_, usize_t index_)
  :m_deque(&deque_)
  ,m_index(index_)
{
}
//----------------------------------------------------------------------------

template<typename T>
template<bool is_forward>
void deque<T>::iterator_impl<is_forward>::move_construct_impl(const iterator_impl &dst_, const iterator_impl &src_, usize_t count_)
{
  if(count_)
  {
    // move-construct deque sequence in passes
    PFC_ASSERT(dst_.m_deque && src_.m_deque);
    usize_t dst_idx=dst_.m_index+dst_.m_deque->m_offset;
    usize_t src_idx=src_.m_index+src_.m_deque->m_offset;
    if(src_idx>dst_idx)
    {
      // move items starting from the beginning of the deque
      do
      {
        // copy longest continuous sequence from source to target deque
        enum {smask=deque_traits<T>::smask, ssize=deque_traits<T>::ssize, sshift=deque_traits<T>::sshift};
        usize_t dst_stride_idx=dst_idx&smask;
        usize_t dst_stride_left=ssize-dst_stride_idx;
        usize_t src_stride_idx=src_idx&smask;
        usize_t src_stride_left=ssize-src_stride_idx;
        usize_t num_pass_values=min(dst_stride_left, src_stride_left, count_);
        move_construct(dst_.m_deque->m_strides[dst_idx>>sshift]+dst_stride_idx,
                       src_.m_deque->m_strides[src_idx>>sshift]+src_stride_idx,
                       num_pass_values);
        dst_idx+=num_pass_values;
        src_idx+=num_pass_values;
        count_-=num_pass_values;
      } while(count_);
    }
    else
    {
      // move items starting from the end of the deque
      dst_idx+=count_-1;
      src_idx+=count_-1;
      do
      {
        // copy longest continuous sequence from source to target deque
        enum {smask=deque_traits<T>::smask, ssize=deque_traits<T>::ssize, sshift=deque_traits<T>::sshift};
        usize_t dst_stride_idx=dst_idx&smask;
        usize_t src_stride_idx=src_idx&smask;
        usize_t num_pass_values=min(dst_stride_idx+1, src_stride_idx+1, count_);
        move_construct(dst_.m_deque->m_strides[dst_idx>>sshift]+dst_stride_idx-num_pass_values+1,
                       src_.m_deque->m_strides[src_idx>>sshift]+src_stride_idx-num_pass_values+1,
                       num_pass_values);
        dst_idx-=num_pass_values;
        src_idx-=num_pass_values;
        count_-=num_pass_values;
      } while(count_);
    }
  }
}
//----------------------------------------------------------------------------
