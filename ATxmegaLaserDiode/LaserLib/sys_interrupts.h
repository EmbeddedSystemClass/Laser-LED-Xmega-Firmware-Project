#ifndef _SYS_INTERRUPTS_H_
#define _SYS_INTERRUPTS_H_

typedef void (*ISRCallback)(void* sender);

#include "intx128.h"
/*
#if defined (__AVR_ATmega128__)
#  include "int128a.h"
#elif defined (__AVR_ATmega8__)
#  include "int8a.h"
#else
#  if !defined(__COMPILING_AVR_LIBC__)
#    warning "device type not defined"
#  endif
#endif*/

#endif