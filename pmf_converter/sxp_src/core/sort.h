//============================================================================
// Spin-X Platform
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================

#ifndef PFC_CORE_SORT_H
#define PFC_CORE_SORT_H
//----------------------------------------------------------------------------


//============================================================================
// interface
//============================================================================
// external
//#include "iterators.h"
#include "utils.h"
namespace pfc
{

// new
// search functions
template<typename Iterator, typename U> PFC_INLINE Iterator linear_search(Iterator seq_, Iterator end_, const U&); // linearly search value from a sequence of (unsorted) values: O(n)
template<typename Iterator, typename U, class Predicate> Iterator linear_search(Iterator seq_, Iterator end_, const U&, Predicate);
template<typename Iterator, typename U> PFC_INLINE Iterator linear_search(Iterator seq_, usize_t num_values_, const U&);
template<typename Iterator, typename U, class Predicate> Iterator linear_search(Iterator seq_, usize_t num_values_, const U&, Predicate);
template<typename Iterator> PFC_INLINE Iterator find_min(Iterator seq_, Iterator end_);
template<typename Iterator, class Predicate> Iterator find_min(Iterator seq_, Iterator end_, Predicate);
template<typename Iterator> PFC_INLINE Iterator find_min(Iterator seq_, usize_t num_values_);
template<typename Iterator, class Predicate> Iterator find_min(Iterator seq_, usize_t num_values_, Predicate);
template<typename Iterator> PFC_INLINE Iterator find_max(Iterator seq_, Iterator end_);
template<typename Iterator, class Predicate> Iterator find_max(Iterator seq_, Iterator end_, Predicate);
template<typename Iterator> PFC_INLINE Iterator find_max(Iterator seq_, usize_t num_values_);
template<typename Iterator, class Predicate> Iterator find_max(Iterator seq_, usize_t num_values_, Predicate);
// predicates
template<typename T, typename U=T> struct search_predicate;
//----------------------------------------------------------------------------


//============================================================================
// search_predicate
//============================================================================
template<typename T, typename U>
struct search_predicate
{
  // search predicate interface
  PFC_INLINE bool before(const T&, const U&) const;
  PFC_INLINE bool equal(const T&, const U&) const;
};
//----------------------------------------------------------------------------


//============================================================================
#include "sort.inl"
} // namespace pfc
#endif
