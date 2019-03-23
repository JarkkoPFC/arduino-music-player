//============================================================================
// Spin-X Platform
//
// Copyright (c) 2016, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================

#include "sxp_src/core/cstr.h"
#include <sys/wait.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
using namespace pfc;
//----------------------------------------------------------------------------


//============================================================================
// default_logging_func
//============================================================================
void pfc::default_logging_func(const char *str_, usize_t num_chars_)
{
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
      std::cout<<buf;
    }
  }
  else
    std::cout<<str_;
}
//----------------------------------------------------------------------------


//============================================================================
// get_global_time
//============================================================================
udouble pfc::get_global_time()
{
  // retrieve time
  timespec t;
  PFC_VERIFY_MSG(clock_gettime(CLOCK_REALTIME, &t)==0, ("Unable to retrieve current time with clock_gettime()"));
  return double(t.tv_sec)+double(t.tv_sec)/1000000000.0;
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
  PFC_VERIFY_MSG(chdir(s_current_working_dir)==0, ("Unable to set working directory to \"%s\"", s_current_working_dir));
}
//----

const char *pfc::executable_dir()
{
  // get executable directory
  static char s_executable_dir[1024]={0};
  if(!*s_executable_dir)
  {
    PFC_VERIFY_MSG(readlink("/proc/self/exe", s_executable_dir, sizeof(s_executable_dir))!=-1, ("Unable to get executable directory"));
    unsigned len=str_size(s_executable_dir);
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
    PFC_VERIFY_MSG(readlink("/proc/self/exe", s_executable_name, sizeof(s_executable_name))==0, ("Unable to get executable name"));
    unsigned len=str_size(s_executable_name);
    while(--len && s_executable_name[len]!='/');
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
    PFC_VERIFY_MSG(readlink("/proc/self/exe", s_executable_filepath, sizeof(s_executable_filepath))==0, ("Unable to get executable filepath"));
  }
  return s_executable_filepath;
}
//----

const char *pfc::working_dir()
{
  if(!*s_current_working_dir)
  {
    PFC_VERIFY_MSG(getcwd(s_current_working_dir, sizeof(s_current_working_dir)), ("Unable to get current working directory"));
    usize_t len=str_size(s_current_working_dir);
    s_current_working_dir[len++]='/';
    s_current_working_dir[len]=0;
  }
  return s_current_working_dir;
}
//----------------------------------------------------------------------------
