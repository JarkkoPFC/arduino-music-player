//============================================================================
// Spin-X Platform
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================


//============================================================================
// linear_search
//============================================================================
template<typename Iterator, typename U>
PFC_INLINE Iterator linear_search(Iterator seq_, Iterator end_, const U &value_)
{
  return linear_search(seq_, end_, value_, search_predicate<typename iterator_trait<Iterator>::value_t, U>());
}
//----

template<typename Iterator, typename U, class Predicate>
Iterator linear_search(Iterator seq_, Iterator end_, const U &value_, Predicate pred_)
{
  // check for empty sequence
  if(seq_==end_)
    return Iterator();

  // linearly search the value in the sequence
  do
  {
    if(pred_.equal(*seq_, value_))
      return seq_;
    ++seq_;
  } while(seq_!=end_);
  return Iterator();
}
//----

template<typename Iterator, typename U>
PFC_INLINE Iterator linear_search(Iterator seq_, usize_t num_values_, const U &value_)
{
  return linear_search(seq_, num_values_, value_, search_predicate<typename iterator_trait<Iterator>::value_t, U>());
}
//----

template<typename Iterator, typename U, class Predicate>
Iterator linear_search(Iterator seq_, usize_t num_values_, const U &value_, Predicate pred_)
{
  // check for no values
  if(!num_values_)
    return Iterator();

  // linearly search the value in the sequence
  do 
  {
    if(pred_.equal(*seq_, value_))
      return seq_;
    ++seq_;
  } while (--num_values_);
  return Iterator();
}
//----------------------------------------------------------------------------


//============================================================================
// find_min
//============================================================================
template<typename Iterator>
PFC_INLINE Iterator find_min(Iterator seq_, Iterator end_)
{
  typedef typename iterator_trait<Iterator>::value_t value_t;
  return find_min(seq_, end_, search_predicate<value_t, value_t>());
}
//----

template<typename Iterator, class Predicate>
Iterator find_min(Iterator seq_, Iterator end_, Predicate pred_)
{
  // check for empty sequence
  if(seq_==end_)
    return Iterator();

  // search for the minimum value in the sequence
  Iterator min_it=seq_;
  ++seq_;
  if(seq_!=end_)
    do
    {
      if(pred_.before(*seq_, *min_it))
        min_it=seq_;
      ++seq_;
    } while(seq_!=end_);
  return min_it;
}
//----

template<typename Iterator>
PFC_INLINE Iterator find_min(Iterator seq_, usize_t num_values_)
{
  typedef typename iterator_trait<Iterator>::value_t value_t;
  return find_min(seq_, num_values_, search_predicate<value_t, value_t>());
}
//----

template<typename Iterator, class Predicate>
Iterator find_min(Iterator seq_, usize_t num_values_, Predicate pred_)
{
  // check for empty sequence
  if(!num_values_)
    return Iterator();

  // search for the minimum value in the sequence
  Iterator min_it=seq_;
  ++seq_;
  if(--num_values_)
    do
    {
      if(pred_.before(*seq_, *min_it))
        min_it=seq_;
      ++seq_;
    } while(--num_values_);
  return min_it;
}
//----------------------------------------------------------------------------


//============================================================================
// find_max
//============================================================================
template<typename Iterator>
PFC_INLINE Iterator find_max(Iterator seq_, Iterator end_)
{
  typedef typename iterator_trait<Iterator>::value_t value_t;
  return find_max(seq_, end_, search_predicate<value_t, value_t>());
}
//----

template<typename Iterator, class Predicate>
Iterator find_max(Iterator seq_, Iterator end_, Predicate pred_)
{
  // check for empty sequence
  if(seq_==end_)
    return Iterator();

  // search for the maximum value in the sequence
  Iterator max_it=seq_;
  ++seq_;
  if(seq_!=end_)
    do
    {
      if(pred_.before(*max_it, *seq_))
        max_it=seq_;
      ++seq_;
    } while(seq_!=end_);
  return max_it;
}
//----

template<typename Iterator>
PFC_INLINE Iterator find_max(Iterator seq_, usize_t num_values_)
{
  typedef typename iterator_trait<Iterator>::value_t value_t;
  return find_max(seq_, num_values_, search_predicate<value_t, value_t>());
}
//----

template<typename Iterator, class Predicate>
Iterator find_max(Iterator seq_, usize_t num_values_, Predicate pred_)
{
  // check for empty sequence
  if(!num_values_)
    return Iterator();

  // search for the maximum value in the sequence
  Iterator max_it=seq_;
  ++seq_;
  if(--num_values_)
    do
    {
      if(pred_.before(*max_it, *seq_))
        max_it=seq_;
      ++seq_;
    } while(--num_values_);
  return max_it;
}
//----------------------------------------------------------------------------


//============================================================================
// search_predicate
//============================================================================
template<typename T, typename U>
bool search_predicate<T, U>::before(const T &v0_, const U &v1_) const
{
  return v0_<v1_;
}
//----

template<typename T, typename U>
bool search_predicate<T, U>::equal(const T &v0_, const U &v1_) const
{
  return v0_==v1_;
}
//----------------------------------------------------------------------------
