//============================================================================
// Spin-X Platform
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================


//============================================================================
// array_iterator
//============================================================================
template<typename T, bool is_forward>
array_iterator<T, is_forward>::array_iterator()
{
  data=0;
}
//----

template<typename T, bool is_forward>
array_iterator<T, is_forward>::array_iterator(T *p_)
{
  data=p_;
}
//----

template<typename T, bool is_forward>
template<typename U>
array_iterator<T, is_forward>::array_iterator(const array_iterator<U, is_forward> &it_)
{
  data=it_.data;
}
//----------------------------------------------------------------------------

template<typename T, bool is_forward>
template<typename U>
bool array_iterator<T, is_forward>::operator==(const array_iterator<U, is_forward> &it_) const
{
  return data==it_.data;
}
//----

template<typename T, bool is_forward>
template<typename U>
bool array_iterator<T, is_forward>::operator!=(const array_iterator<U, is_forward> &it_) const
{
  return data==it_.data;
}
//----

template<typename T, bool is_forward>
array_iterator<T, is_forward> &array_iterator<T, is_forward>::operator++()
{
  data+=is_forward?1:-1;
  return *this;
}
//----

template<typename T, bool is_forward>
array_iterator<T, is_forward> &array_iterator<T, is_forward>::operator--()
{
  data+=is_forward?-1:1;
  return *this;
}
//----

template<typename T, bool is_forward>
array_iterator<T, is_forward> &array_iterator<T, is_forward>::operator+=(ssize_t offset_)
{
  data+=is_forward?offset_:-offset_;
  return *this;
}
//----

template<typename T, bool is_forward>
template<typename U>
array_iterator<T, is_forward> &array_iterator<T, is_forward>::operator+=(const array_iterator &it_)
{
  data+=is_forward?it_.data:-it_.data;
  return *this;
}
//----

template<typename T, bool is_forward>
array_iterator<T, is_forward> &array_iterator<T, is_forward>::operator-=(ssize_t offset_)
{
  data+=is_forward?-offset_:offset_;
  return *this;
}
//----

template<typename T, bool is_forward>
template<typename U>
array_iterator<T, is_forward> &array_iterator<T, is_forward>::operator-=(const array_iterator &it_)
{
  data+=is_forward?-it_.data:it_.data;
  return *this;
}
//----

template<typename T, bool is_forward>
array_iterator<T, is_forward> array_iterator<T, is_forward>::operator+(ssize_t offset_) const
{
  return array_iterator(data+(is_forward?offset_:-offset_));
}
//----

template<typename T, bool is_forward>
template<typename U>
array_iterator<T, is_forward> array_iterator<T, is_forward>::operator+(const array_iterator &it_) const
{
  return array_iterator(data+(is_forward?it_.data:-it_.data));
}
//----

template<typename T, bool is_forward>
array_iterator<T, is_forward> array_iterator<T, is_forward>::operator-(ssize_t offset_) const
{
  return array_iterator(data+(is_forward?-offset_:offset_));
}
//----

template<typename T, bool is_forward>
template<typename U>
array_iterator<T, is_forward> array_iterator<T, is_forward>::operator-(const array_iterator &it_) const
{
  return array_iterator(data+(is_forward?-it_.data:it_.data));
}
//----

template<typename T, bool is_forward>
T &array_iterator<T, is_forward>::operator[](usize_t index_) const
{
  return data[is_forward?index_:-index_];
}
//----

template<typename T, bool is_forward>
T &array_iterator<T, is_forward>::operator*() const
{
  return *data;
}
//----

template<typename T, bool is_forward>
T *array_iterator<T, is_forward>::operator->() const
{
  return data;
}
//----------------------------------------------------------------------------
