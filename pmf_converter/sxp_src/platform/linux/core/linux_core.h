//============================================================================
// Spin-X Platform
//
// Copyright (c) 2016, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================

#ifndef PFC_LINUX_CORE_H
#define PFC_LINUX_CORE_H
//----------------------------------------------------------------------------
                 

//============================================================================
// interface
//============================================================================
// external
#include "sxp_src/core/config.h"
#include <cmath>
#include <stdio.h>
namespace pfc
{

// new
#define PFC_BIG_ENDIAN 0 /*todo*/
#define PFC_SNPRINTF snprintf
#define PFC_ALLOCA alloca
#define PFC_MKDIR(dir__) mkdir(dir__, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH)
//----------------------------------------------------------------------------


//============================================================================
} // namespace pfc
#endif
