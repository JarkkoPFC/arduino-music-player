//============================================================================
// Spin-X Platform
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================

#include "sxp_src/core/cstr.h"
#include <iostream>
#include <ShellAPI.h>
using namespace pfc;
//----------------------------------------------------------------------------


//============================================================================
// default_logging_func
//============================================================================
void pfc::default_logging_func(const char *str_, usize_t num_chars_)
{
  static bool is_debugger=IsDebuggerPresent()==TRUE;
  if(num_chars_!=usize_t(-1))
  {
    char buf[1024];
    while(num_chars_)
    {
      // copy characters to the buffer
      usize_t size=str_copy(buf, str_, min<usize_t>(num_chars_+1, sizeof(buf)));
      str_+=size;
      num_chars_-=size;

      // print buffer
      if(is_debugger)
        OutputDebugString(buf);
      else
        std::cout<<buf;
    }
  }
  else
  {
    if(is_debugger)
      OutputDebugString(str_);
    else
      std::cout<<str_;
  }
}
//----------------------------------------------------------------------------


//============================================================================
// get_global_time
//============================================================================
udouble pfc::get_global_time()
{
  // get performance counter factor if not yet queried
  static udouble s_counter_factor=0.0f;
  if(!s_counter_factor)
  {
    LARGE_INTEGER freq={{0, 0}};
    PFC_VERIFY_MSG(QueryPerformanceFrequency(&freq) && freq.QuadPart, ("Unable to query performance counter frequency\r\n"));
    s_counter_factor=1.0/udouble(freq.QuadPart);
  }

  // return elapsed time in seconds
  LARGE_INTEGER pc;
  QueryPerformanceCounter(&pc);
  return udouble(pc.QuadPart)*s_counter_factor;
}
//----------------------------------------------------------------------------


//============================================================================
// init_working_dir
//============================================================================
static char s_current_working_dir[1024]={0};
void pfc::init_working_dir()
{
  // set working directory to the executable directory
  str_copy(s_current_working_dir, executable_dir(), sizeof(s_current_working_dir));
  PFC_VERIFY_MSG(SetCurrentDirectory(s_current_working_dir), ("Unable to set working directory to \"%s\"\r\n", s_current_working_dir));
}
//----

const char *pfc::executable_dir()
{
  // get executable directory
  static char s_executable_dir[1024]={0};
  if(!*s_executable_dir)
  {
    GetModuleFileName(0, s_executable_dir, sizeof(s_executable_dir));
    unsigned len=(unsigned)str_size(s_executable_dir);
    str_replace(s_executable_dir, '\\', '/');
    while(--len && s_executable_dir[len]!='/');
    s_executable_dir[++len]=0;
  }
  return s_executable_dir;
}
//----

const char *pfc::executable_name()
{
  // get executable directory
  static char s_executable_name[1024]={0};
  if(!*s_executable_name)
  {
    GetModuleFileName(0, s_executable_name, sizeof(s_executable_name));
    unsigned len=(unsigned)str_size(s_executable_name);
    while(--len && s_executable_name[len]!='/' && s_executable_name[len]!='\\');
    ++len;
    mem_move(s_executable_name, s_executable_name+len, sizeof(s_executable_name)-len);
  }
  return s_executable_name;
}
//----

const char *pfc::executable_filepath()
{
  // get executable directory
  static char s_executable_filepath[1024]={0};
  if(!*s_executable_filepath)
  {
    GetModuleFileName(0, s_executable_filepath, sizeof(s_executable_filepath));
    str_replace(s_executable_filepath, '\\', '/');
  }
  return s_executable_filepath;
}
//----

const char *pfc::working_dir()
{
  if(!*s_current_working_dir)
  {
    PFC_VERIFY_MSG(GetCurrentDirectory(sizeof(s_current_working_dir), s_current_working_dir), ("Unable to get current working directory\r\n"));
    str_replace(s_current_working_dir, '\\', '/');
    usize_t len=str_size(s_current_working_dir);
    s_current_working_dir[len++]='/';
    s_current_working_dir[len]=0;
  }
  return s_current_working_dir;
}
//----------------------------------------------------------------------------
