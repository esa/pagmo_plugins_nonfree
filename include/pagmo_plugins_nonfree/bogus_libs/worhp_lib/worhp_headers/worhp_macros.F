#ifndef WORHP_MACROS_F
#define WORHP_MACROS_F

#include "worhp_macros_WMT.F"
#include "worhp_version.h"

!
! This is necessary because the gfortran fpp does not behave like standard cpp.
! Apparently the one of ifort does, though.
!
#if (defined(__INTEL_COMPILER))
# define QUOTE(x) #x
#else
# define QUOTE(x) "x"
#endif

!
! This macro adds functionality similar to C assert to Fortran
!
#ifdef NDEBUG
# define ASSERT(expr)
#else
# define ASSERT(expr) IF (.NOT.(expr)) CALL WORHP_ASSERT("expr", __FILE__, __LINE__)
#endif

!
! This macro makes the executing code check for matching library version
!
#define CHECK_WORHP_VERSION IF (F_CheckWorhpVersion(WORHP_MAJOR, WORHP_MINOR, WORHP_PATCH) /= 0) STOP

#define WORHP_CNM(x) BIND(C, name = QUOTE(x))
#define WORHP_API(x)

#ifdef MEM_DEBUG
!
! Have C_realloc calls by Fortran actually call C_DebugRealloc,
! which is defined in std.F90 as alias to DebugRealloc in C_std.c
!
#define C_realloc(p,n) C_DebugRealloc(p,n,F2CString(__FILE__),__LINE__)
#endif

#endif
