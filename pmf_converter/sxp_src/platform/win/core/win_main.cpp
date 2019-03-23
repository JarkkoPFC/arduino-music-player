//============================================================================
// Spin-X Platform
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================

#include "sxp_src/core/main.h"
#include "sxp_src/core/str.h"
#include "sxp_src/core/utils.h"
using namespace pfc;
//----------------------------------------------------------------------------


//============================================================================
// win_main
//============================================================================
int pfc::win_main(int argc_, const char *argv_[])
{
  // get command-line arguments
  enum {max_args=256};
  const char *args[max_args]={0};
  heap_str cmd_line=GetCommandLine();
  unsigned num_args=tokenize_command_line(cmd_line.c_str(), args, max_args);
  return entry(args+1, num_args-1);
}
//----

int pfc::win_main(HINSTANCE inst_, HINSTANCE prev_inst_, LPSTR cmd_, int show_flags_)
{
  // get command-line arguments
  enum {max_args=256};
  const char *args[max_args]={0};
  unsigned num_args=tokenize_command_line(cmd_, args, max_args);
  return entry(args, num_args);
}
//----------------------------------------------------------------------------
