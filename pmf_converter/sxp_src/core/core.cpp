//============================================================================
// Spin-X Platform
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================

#include "containers.h"
#include "sort.h"
using namespace pfc;
//----------------------------------------------------------------------------


//============================================================================
// log/warn/error
//============================================================================
namespace
{
  enum {log_indention_size=2};
  void(*s_logf_func)(const char*, usize_t)=&default_logging_func;
  void(*s_warnf_func)(const char*, usize_t)=&default_logging_func;
  void(*s_errorf_func)(const char*, usize_t)=&default_logging_func;
  volatile bool s_aborted=false;
  volatile unsigned s_log_indention=0;
  //----

  void va_log(void(*log_func_)(const char*, usize_t), const char *fmt_, va_list args_)
  {
    while(fmt_)
    {
      // find next formatting position and output preceding string
      const char *s=str_find(fmt_, '%');
      usize_t num_chars=s?usize_t(s-fmt_):usize_t(-1);
      if(num_chars)
        (*log_func_)(fmt_, num_chars);
      if(!s)
        break;

      // build formatting string
      bool terminate_format_str=false;
      char fmt_string[64];
      char *fmt_pos=fmt_string;
      *fmt_pos++='%';
      do
      {
        // check for format string overflow
        *fmt_pos++=*++s;
        if(fmt_pos==fmt_string+sizeof(fmt_string)-2)
        {
          *fmt_pos++=0;
          (*log_func_)(fmt_string, usize_t(-1));
          break;
        }

        switch(*s)
        {
          // unfinished string formatting
          case 0:
          {
            (*log_func_)(fmt_string, usize_t(-1));
            terminate_format_str=true;
            --s;
          } break;

          // integer value formatting
          case 'c': case 'd': case 'i': case 'u': case 'o': case 'x': case 'X':
          {
            char buf[64];
            int v=va_arg(args_, int);
            *fmt_pos++=0;
            PFC_SNPRINTF(buf, sizeof(buf), fmt_string, v);
            (*log_func_)(buf, usize_t(-1));
            terminate_format_str=true;
          } break;

          // floating point value formatting
          case 'f': case 'F': case 'e': case 'E': case 'g': case 'G':  case 'a': case 'A':
          {
            char buf[64];
            double v=va_arg(args_, double);
            *fmt_pos++=0;
            PFC_SNPRINTF(buf, sizeof(buf), fmt_string, v);
            (*log_func_)(buf, usize_t(-1));
            terminate_format_str=true;
          } break;

          // pointer formatting
          case 'p':
          {
            char buf[64];
            void *v=va_arg(args_, void*);
            *fmt_pos++=0;
            PFC_SNPRINTF(buf, sizeof(buf), fmt_string, v);
            (*log_func_)(buf, usize_t(-1));
            terminate_format_str=true;
          } break;

          // none or % formatting
          case 'n': case '%':
          {
            char buf[64];
            *fmt_pos++=0;
            PFC_SNPRINTF(buf, sizeof(buf), fmt_string, 0);
            (*log_func_)(buf, usize_t(-1));
            terminate_format_str=true;
          } break;

          // string formatting
          case 's':
          {
            char *v=va_arg(args_, char*);
            (*log_func_)(v, usize_t(-1));
            terminate_format_str=true;
          } break;
        }
      } while(!terminate_format_str);
      fmt_=s+1;
    }
  }
} // namespace <anonymous>
//----

void pfc::log(const char *str_)
{
  if(!s_aborted)
  {
    (*s_logf_func)(str_, usize_t(-1));
  }
}
//----

void pfc::logf(const char *fs_, ...)
{
  // write string to log window
  if(!s_aborted)
  {
    va_list args;
    va_start(args, fs_);
    va_log(s_logf_func, fs_, args);
    va_end(args);
  }
}
//----

void pfc::log_indention()
{
  if(!s_aborted)
  {
    // write indention with log function
    char str[64];
    unsigned indention=s_log_indention;
    indention=min(indention, unsigned((sizeof(str)-1)/log_indention_size));
    mem_set(str, ' ', indention*log_indention_size);
    str[indention*log_indention_size]=0;
    (*s_logf_func)(str, indention*log_indention_size);
  }
}
//----

void pfc::warn(const char *str_)
{
  if(!s_aborted)
  {
    (*s_warnf_func)(str_, usize_t(-1));
  }
}
//----

void pfc::warnf(const char *fs_, ...)
{
  // write string to warning window
  if(!s_aborted)
  {
    va_list args;
    va_start(args, fs_);
    va_log(s_warnf_func, fs_, args);
    va_end(args);
  }
}
//----

void pfc::warn_indention()
{
  if(!s_aborted)
  {
    // write indention with log function
    char str[64];
    unsigned indention=s_log_indention;
    indention=min(indention, unsigned((sizeof(str)-1)/log_indention_size));
    mem_set(str, ' ', indention*log_indention_size);
    str[indention*log_indention_size]=0;
    (*s_warnf_func)(str, indention*log_indention_size);
  }
}
//----

void pfc::error(const char *str_)
{
  if(!s_aborted)
  {
    (*s_errorf_func)(str_, usize_t(-1));
  }
}
//----

void pfc::errorf(const char *fs_, ...)
{
  // write string to error window
  if(!s_aborted)
  {
    va_list args;
    va_start(args, fs_);
    va_log(s_errorf_func, fs_, args);
    va_end(args);
  }
}
//----

void pfc::error_indention()
{
  if(!s_aborted)
  {
    // write indention with log function
    char str[64];
    unsigned indention=s_log_indention;
    indention=min(indention, unsigned((sizeof(str)-1)/log_indention_size));
    mem_set(str, ' ', indention*log_indention_size);
    str[indention*log_indention_size]=0;
    (*s_errorf_func)(str, indention*log_indention_size);
  }
}
//----

void pfc::set_logging_funcs(void(*logf_)(const char*, usize_t), void(*warnf_)(const char*, usize_t), void(*errorf_)(const char*, usize_t))
{
  // setup logging functions
  s_logf_func=logf_?logf_:&default_logging_func;
  s_warnf_func=warnf_?warnf_:&default_logging_func;
  s_errorf_func=errorf_?errorf_:&default_logging_func;
}
//----

void pfc::indent_log()
{
  ++s_log_indention;
}
//----

void pfc::unindent_log()
{
  unsigned old_indention=s_log_indention--;
  PFC_ASSERT(int(old_indention)>=0);
}
//----------------------------------------------------------------------------

namespace
{
  bool default_preabort_func()
  {
    return true;
  }
  //----

  bool(*s_preabort_func)()=&default_preabort_func;
} // namespace anonymous
//----

bool pfc::preabort()
{
  if(!s_aborted)
  {
    s_aborted=(*s_preabort_func)();
  }
  return s_aborted;
}
//----

void pfc::set_preabort_func(bool(*preabort_)())
{
  s_preabort_func=preabort_?preabort_:&default_preabort_func;
}
//----------------------------------------------------------------------------


//============================================================================
// default_memory_allocator
//============================================================================
void default_memory_allocator::check_allocator(usize_t num_bytes_, usize_t mem_align_)
{
  PFC_CHECK_MSG(mem_align_ && mem_align_<=memory_align && (mem_align_&(mem_align_-1))==0,
                ("default_memory_allocator memory alignment must be power-of-2 and in range [1, %u] (requesting %u byte alignment)\r\n", memory_align, mem_align_));
}
//----------------------------------------------------------------------------

void *default_memory_allocator::alloc(usize_t num_bytes_, usize_t mem_align_)
{
  PFC_ASSERT_PEDANTIC_MSG(mem_align_ && mem_align_<=memory_align && (mem_align_&(mem_align_-1))==0,
                          ("default_memory_allocator memory alignment must be power-of-2 and in range [1, %u] (requesting %u byte alignment)\r\n", memory_align, mem_align_));
  return PFC_MEM_ALLOC(num_bytes_);
}
//----

void default_memory_allocator::free(void *p_)
{
  PFC_MEM_FREE(p_);
}
//----------------------------------------------------------------------------

default_memory_allocator::default_memory_allocator()
{
}
//----

default_memory_allocator::~default_memory_allocator()
{
}
//----------------------------------------------------------------------------


//============================================================================
// type ID
//============================================================================
#define PFC_STATIC_TYPEID(type__, id__) const unsigned type_id<type__ >::id=id__;\
                                        const unsigned type_id<const type__ >::id=id__;\
                                        const unsigned type_id<volatile type__ >::id=id__;\
                                        const unsigned type_id<const volatile type__ >::id=id__
#include "typeid.inc"
#undef PFC_STATIC_TYPEID
//----------------------------------------------------------------------------
