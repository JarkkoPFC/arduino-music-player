//============================================================================
// Spin-X Platform
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================

#include "fsys.h"
using namespace pfc;
//----------------------------------------------------------------------------


//============================================================================
// config
//============================================================================
static const char *s_fileext_temp=".tmp";
//----------------------------------------------------------------------------


//============================================================================
// file name functions
//============================================================================
bool pfc::is_absolute_filepath(const char *filepath_)
{
  // check if given path is absolute or relative
  PFC_ASSERT(filepath_);
  if(*filepath_=='/' || *filepath_=='\\')
    return true;
  while(char c=*filepath_++)
  {
    switch(c)
    {
      case ':': return true;
      case '/':
      case '\\': return false;
    }
  }
  return false;
}
//----

const char *pfc::get_filename(const char *filepath_)
{
  // seek for slash/backslash character starting from the end of the file path
  if(!filepath_)
    return 0;
  unsigned pos=(unsigned)str_size(filepath_);
  while(pos)
  {
    // check for directory separator
    --pos;
    if(filepath_[pos]=='/' || filepath_[pos]=='\\')
      return filepath_+pos+1;
  }

  // no slash/backslash found
  return filepath_;
}
//----

char *pfc::get_filename(char *filepath_)
{
  return (char*)get_filename((const char*)filepath_);
}
//----

const char *pfc::get_fileext(const char *filename_)
{
  // seek for '.'-character starting from the end of the filename
  if(!filename_)
    return 0;
  unsigned pos=(unsigned)str_size(filename_);
  while(pos)
  {
    // check for '.'-character
    --pos;
    if(filename_[pos]=='.')
      return filename_+pos+1;
  }

  // no '.'-character found
  return filename_;
}
//----

char *pfc::get_fileext(char *filename_)
{
  return (char*)get_fileext((const char*)filename_);
}
//----

unsigned pfc::common_path_size(const char *path0_, const char *path1_, bool case_sensitive_)
{
  // return the length of common path between the two paths
  const char *begin=path0_;
  while(*path0_ && ((case_sensitive_?*path0_==*path1_:to_lower(*path0_)==to_lower(*path1_)) || (*path0_=='/' && *path1_=='\\') || (*path0_=='\\' && *path1_=='/')))
  {
    ++path0_;
    ++path1_;
  }
  if(*path0_)
  {
    // back-up to the previous common slash or backslash
    while(path0_!=begin && ((*path0_!='/' && *path0_!='\\') || (*path1_!='/' && *path1_!='\\')))
    {
      --path0_;
      --path1_;
    }
    if(path0_!=begin)
      ++path0_;
  }
  return unsigned(path0_-begin);
}
//----

char *pfc::split_filepath(char *filepath_)
{
  // split path from the filename
  char *filename=(char*)get_filename(filepath_);
  if(filename==filepath_)
    return filepath_;
  *(filename-1)=0;
  return filename;
}
//----

char *pfc::split_filename(char *filename_)
{
  // split extension from the filename
  char *fileext=(char*)get_fileext(filename_);
  if(fileext==filename_)
    return filename_;
  *(fileext-1)=0;
  return fileext;
}
//----

void pfc::convert_to_compatible_filepath(char *filepath_, char replacement_)
{
  // setup valid character set
  static char s_valid_char_lut[256];
  static bool s_is_init=false;
  if(!s_is_init)
  {
    memset(s_valid_char_lut, 1, 256);
    static const char *s_invalid_chars=":*?\"<>|";
    const char *c=s_invalid_chars;
    while(*c)
      s_valid_char_lut[uint8(*c++)]=0;
    for(unsigned i=0; i<32; ++i)
      s_valid_char_lut[i]=0;
    s_is_init=true;
  }

  // replace invalid characters in the filepath
  if(filepath_)
  {
    char *c=filepath_;
    while(*c)
    {
      if(!s_valid_char_lut[uint8(*c)])
        *c=replacement_;
      ++c;
    }
  }
}
//----

unsigned pfc::collapse_relative_dirs(char *filepath_)
{
  // remove relative directories from the filepath
  PFC_ASSERT(filepath_);
  char *cw=filepath_, *fpath=filepath_, *cr=cw, c;
  do
  {
    // check for relative path character
    if((c=*cr++)=='.')
    {
      switch(c=*cr++)
      {
        // previous directory
        case '.':
        {
          c=*cr++;
          PFC_CHECK(!c || c=='/' || c=='\\');
          if(cw>fpath)
          {
            cw-=2;
            while(cw>fpath && *cw!='/' && *cw!='\\')
              --cw;
            if(cw>fpath)
              ++cw;
          }
          else
          {
            *cw++='.'; *cw++='.'; *cw++='/';
            fpath=cw;
          }
        } break;

        // current directory
        case '/':
        case '\\': break;

        // extension
        default: *cw++='.'; *cw++=c;
      }
    }
    else
      *cw++=c;
  } while(c);
  return unsigned(cw-filepath_)-1;
}
//----

void pfc::build_temp_filepath(filepath_str &res_, const filepath_str &input_)
{
  // build temporary filepath from input filepath (prefix filename with ~ and postfix with .tmp)
  res_=input_;
  char *fname=split_filepath(res_.c_str());
  if(fname!=res_.c_str())
  {
    res_.resize(unsigned(fname-res_.c_str())-1);
    res_+='/';
  }
  else
    res_.clear();
  res_+='~';
  res_+=get_filename(input_.c_str());
  res_+=s_fileext_temp;
}
//----

bool pfc::read_file(file_system_base &fsys_, heap_str &res_, const char *filename_, const char *path_, e_file_open_check check_)
{
  // read given file content to the string
  res_.clear();
  owner_ptr<bin_input_stream_base> file=fsys_.open_read(filename_, path_, check_);
  if(!file.data)
    return false;
  res_.resize(fsys_.file_size(filename_));
  file->read_bytes(res_.data(), res_.size());
  return true;
}
//----------------------------------------------------------------------------


//============================================================================
// active file system free-functions
//============================================================================
owner_ptr<bin_input_stream_base> pfc::afs_open_read(const char *filename_, const char *path_, e_file_open_check fopen_check_)
{
  return file_system_base::active().open_read(filename_, path_, fopen_check_);
}
//----

owner_ptr<bin_output_stream_base> pfc::afs_open_write(const char *filename_, const char *path_, e_file_open_write_mode mode_, uint64 fpos_, bool makedir_, e_file_open_check fopen_check_)
{
  return file_system_base::active().open_write(filename_, path_, mode_, fpos_, makedir_, fopen_check_);
}
//----

filepath_str pfc::afs_complete_path(const char *filename_, const char *path_, bool collapse_relative_dirs_)
{
  return file_system_base::active().complete_path(filename_, path_, collapse_relative_dirs_);
}
//----

filepath_str pfc::afs_complete_system_path(const char *filename_, const char *path_, bool collapse_relative_dirs_)
{
  return file_system_base::active().complete_system_path(filename_, path_, collapse_relative_dirs_);
}
//----------------------------------------------------------------------------


//============================================================================
// directory_monitor_base
//============================================================================
directory_monitor_base::directory_monitor_base()
{
}
//----

directory_monitor_base::~directory_monitor_base()
{
}
//----------------------------------------------------------------------------


//============================================================================
// file_system_base
//============================================================================
file_system_base *file_system_base::s_active=0;
//----------------------------------------------------------------------------

file_system_base::file_system_base(bool set_active_)
{
  if(set_active_)
    set_active(this);
}
//----

file_system_base::~file_system_base()
{
  if(s_active==this)
    s_active=0;
}
//----------------------------------------------------------------------------

void file_system_base::set_directory(const char *dir_)
{
  m_directory=dir_?dir_:"";
  unsigned s=(unsigned)m_directory.size();
  if(s)
  {
    // convert backslashes to slashes and expand slash to dir name if needed
    str_replace(m_directory.c_str(), '\\', '/');
    if(m_directory[s-1]!='/')
      m_directory+='/';
  }
}
//----

filepath_str file_system_base::complete_path(const char *filename_, const char *path_, bool collapse_relative_dirs_) const
{
  // construct complete path from active directory, optional path and given file/dir name
  filepath_str res;
  if(!filename_ || !is_absolute_filepath(filename_))
  {
    if(!path_ || !is_absolute_filepath(path_))
    {
      if(!is_absolute_filepath(m_directory.c_str()))
        res=m_system_root_dir;
      res+=m_directory;
    }
    if(path_ && *path_)
    {
      res+=path_;
      char last_char=res[res.size()-1];
      if(last_char!='/' && last_char!='\\')
        res+='/';
    }
  }
  if(filename_)
    res+=filename_;
  if(collapse_relative_dirs_)
    res.resize(collapse_relative_dirs(res.c_str()));
  return res;
}
//----

filepath_str file_system_base::complete_system_path(const char *filename_, const char *path_, bool collapse_relative_dirs_) const
{
  // construct full system path to given file/dir name
  PFC_ASSERT(filename_);
  filepath_str fpath=complete_path(filename_, path_);
  if(!is_absolute_filepath(fpath.c_str()))
    fpath.push_front(working_dir());
  if(collapse_relative_dirs_)
    fpath.resize(collapse_relative_dirs(fpath.c_str()));
  return fpath;
}
//----------------------------------------------------------------------------

void file_system_base::set_system_root_directory(const char *root_dir_)
{
  m_system_root_dir=root_dir_?root_dir_:"";
  unsigned s=(unsigned)m_system_root_dir.size();
  if(s)
  {
    // convert backslashes to slashes and expand slash to dir name if needed
    str_replace(m_system_root_dir.c_str(), '\\', '/');
    if(m_system_root_dir[s-1]!='/')
      m_system_root_dir+='/';
  }
}
//----------------------------------------------------------------------------
