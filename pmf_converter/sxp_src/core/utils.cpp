//============================================================================
// Spin-X Platform
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================

#include "utils.h"
using namespace pfc;
//----------------------------------------------------------------------------


//============================================================================
// tokenize_command_line
//============================================================================
unsigned pfc::tokenize_command_line(char *cmd_line_, const char **tokens_, unsigned max_tokens_)
{
  // split arguments in argument string
  bool is_quoted=false;
  unsigned num_args=0;
  while(num_args<max_tokens_ && *cmd_line_)
  {
    // skip white space and add new argument
    while(*cmd_line_ && is_whitespace(*cmd_line_))
      ++cmd_line_;
    if(*cmd_line_)
      tokens_[num_args++]=cmd_line_;

    // skip until non-quoted white space
    while(*cmd_line_ && (is_quoted || !is_whitespace(*cmd_line_)))
    {
      if(*cmd_line_=='\"')
        is_quoted=!is_quoted;
      ++cmd_line_;
    }

    // replace white space with string terminate
    if(*cmd_line_)
      *cmd_line_++=0;
  }
  return num_args;
}
//----------------------------------------------------------------------------
