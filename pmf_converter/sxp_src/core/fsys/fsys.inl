//============================================================================
// Spin-X Platform
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================


//============================================================================
// file_time
//============================================================================
file_time::file_time()
  :year(0)
  ,month(0)
  ,day(0)
  ,hour(0)
  ,minute(0)
  ,second(0)
{
}
//----

file_time::file_time(uint16 year_, uint8 month_, uint8 day_, uint8 hour_, uint8 minute_, uint8 second_)
  :year(year_)
  ,month(month_)
  ,day(day_)
  ,hour(hour_)
  ,minute(minute_)
  ,second(second_)
{
}
//----------------------------------------------------------------------------


//============================================================================
// file time operators
//============================================================================
PFC_INLINE bool operator==(const file_time &ft0_, const file_time &ft1_)
{
  return    ft0_.year==ft1_.year
         && ft0_.month==ft1_.month
         && ft0_.day==ft1_.day
         && ft0_.hour==ft1_.hour
         && ft0_.minute==ft1_.minute
         && ft0_.second==ft1_.second;
}
//----

PFC_INLINE bool operator!=(const file_time &ft0_, const file_time &ft1_)
{
  return    ft0_.year!=ft1_.year
         || ft0_.month!=ft1_.month
         || ft0_.day!=ft1_.day
         || ft0_.hour!=ft1_.hour
         || ft0_.minute!=ft1_.minute
         || ft0_.second!=ft1_.second;
}
//----

PFC_INLINE bool operator<(const file_time &ft0_, const file_time &ft1_)
{
  uint64 t0=(uint64((ft0_.year<<8)|ft0_.month)<<32)|uint64((ft0_.day<<24)|(ft0_.hour<<16)|(ft0_.minute<<8)|ft0_.second);
  uint64 t1=(uint64((ft1_.year<<8)|ft1_.month)<<32)|uint64((ft1_.day<<24)|(ft1_.hour<<16)|(ft1_.minute<<8)|ft1_.second);
  return t0<t1;
}
//----

PFC_INLINE bool operator>(const file_time &ft0_, const file_time &ft1_)
{
  uint64 t0=(uint64((ft0_.year<<8)|ft0_.month)<<32)|uint64((ft0_.day<<24)|(ft0_.hour<<16)|(ft0_.minute<<8)|ft0_.second);
  uint64 t1=(uint64((ft1_.year<<8)|ft1_.month)<<32)|uint64((ft1_.day<<24)|(ft1_.hour<<16)|(ft1_.minute<<8)|ft1_.second);
  return t0>t1;
}
//----

PFC_INLINE bool operator<=(const file_time &ft0_, const file_time &ft1_)
{
  uint64 t0=(uint64((ft0_.year<<8)|ft0_.month)<<32)|uint64((ft0_.day<<24)|(ft0_.hour<<16)|(ft0_.minute<<8)|ft0_.second);
  uint64 t1=(uint64((ft1_.year<<8)|ft1_.month)<<32)|uint64((ft1_.day<<24)|(ft1_.hour<<16)|(ft1_.minute<<8)|ft1_.second);
  return t0<=t1;
}
//----

PFC_INLINE bool operator>=(const file_time &ft0_, const file_time &ft1_)
{
  uint64 t0=(uint64((ft0_.year<<8)|ft0_.month)<<32)|uint64((ft0_.day<<24)|(ft0_.hour<<16)|(ft0_.minute<<8)|ft0_.second);
  uint64 t1=(uint64((ft1_.year<<8)|ft1_.month)<<32)|uint64((ft1_.day<<24)|(ft1_.hour<<16)|(ft1_.minute<<8)|ft1_.second);
  return t0>=t1;
}
//----------------------------------------------------------------------------


//============================================================================
// file_system_base
//============================================================================
void file_system_base::set_active(file_system_base *fs_)
{
  PFC_CHECK_MSG(!s_active || !fs_, ("File system has already been activated\r\n"));
  s_active=fs_;
}
//----

file_system_base &file_system_base::active()
{
  PFC_ASSERT_MSG(s_active, ("No file system has been activated\r\n"));
  return *s_active;
}
//----------------------------------------------------------------------------

const stack_str256 &file_system_base::directory() const
{
  return m_directory;
}
//----------------------------------------------------------------------------


//============================================================================
// file_system_base::iterator
//============================================================================
file_system_base::iterator::iterator()
{
  reset();
}
//----------------------------------------------------------------------------

PFC_INLINE bool is_valid(const file_system_base::iterator &it_)
{
  return it_.m_iterator.data!=0;
}
//----

const char *file_system_base::iterator::name() const
{
  PFC_ASSERT_PEDANTIC(m_iterator.data);
  return m_name;
}
//----

bool file_system_base::iterator::is_dir() const
{
  PFC_ASSERT_PEDANTIC(m_iterator.data);
  return m_is_dir;
}
//----

bool file_system_base::iterator::is_file() const
{
  PFC_ASSERT_PEDANTIC(m_iterator.data);
  return m_is_file;
}
//----

void file_system_base::iterator::operator++()
{
  // advance iterator
  PFC_ASSERT_PEDANTIC(m_iterator.data);
  if(!m_iterator.data->advance(*this))
    reset();
}
//----------------------------------------------------------------------------

void file_system_base::iterator::reset()
{
  m_iterator=0;
  m_name=0;
  m_is_dir=false;
  m_is_file=false;
}
//----------------------------------------------------------------------------


//============================================================================
// file_system_base::iterator_impl_base
//============================================================================
file_system_base::iterator_impl_base::iterator_impl_base()
{
}
//----

file_system_base::iterator_impl_base::~iterator_impl_base()
{
}
//----------------------------------------------------------------------------

void file_system_base::iterator_impl_base::init_iterator(iterator &it_)
{
  it_.m_iterator=this;
}
//----

void file_system_base::iterator_impl_base::update(iterator &it_, const char *name_, bool is_dir_, bool is_file_)
{
  it_.m_name=name_;
  it_.m_is_dir=is_dir_;
  it_.m_is_file=is_file_;
}
//----------------------------------------------------------------------------
