#ifndef STDARG_H
#define STDARG_H

// When not building with IDO, use the builtin vaarg macros for portability.
#if defined(_MSC_VER) && !defined(__sgi)
// MSVC (the native port's x64 build): the Windows x64 calling convention's
// va_list, a char * walking 8-byte argument slots (larger or odd-sized
// arguments are passed by reference); __va_start is a compiler intrinsic
typedef char *va_list;
void __cdecl __va_start(va_list *, ...);
#define va_start(ap, v) ((void) __va_start(&(ap), (v)))
#define va_arg(ap, t) \
    ((sizeof(t) > 8 || (sizeof(t) & (sizeof(t) - 1)) != 0) ? **(t **) (((ap) += 8) - 8) : *(t *) (((ap) += 8) - 8))
#define va_end(ap) ((void) ((ap) = (va_list) 0))
#elif !defined(__sgi)
#define va_list __builtin_va_list
#define va_start __builtin_va_start
#define va_arg __builtin_va_arg
#define va_end __builtin_va_end
#else

typedef char *va_list;
#define _FP 1
#define _INT 0
#define _STRUCT 2

#define _VA_FP_SAVE_AREA 0x10
#define _VA_ALIGN(p, a) (((unsigned int)(((char *)p) + ((a) > 4 ? (a) : 4) - 1)) & -((a) > 4 ? (a) : 4))
#define va_start(vp, parmN) (vp = ((va_list)&parmN + sizeof(parmN)))

#define __va_stack_arg(list, mode)                                 \
  (                                                                \
      ((list) = (char *)_VA_ALIGN(list, __builtin_alignof(mode)) + \
                _VA_ALIGN(sizeof(mode), 4)),                       \
      (((char *)list) - (_VA_ALIGN(sizeof(mode), 4) - sizeof(mode))))

#define __va_double_arg(list, mode)                                                                  \
  (                                                                                                  \
      (((long)list & 0x1) /* 1 byte aligned? */                                                      \
           ? (list = (char *)((long)list + 7), (char *)((long)list - 6 - _VA_FP_SAVE_AREA))          \
           : (((long)list & 0x2) /* 2 byte aligned? */                                               \
                  ? (list = (char *)((long)list + 10), (char *)((long)list - 24 - _VA_FP_SAVE_AREA)) \
                  : __va_stack_arg(list, mode))))

#define va_arg(list, mode) ((mode *)(((__builtin_classof(mode) == _FP &&          \
                                       __builtin_alignof(mode) == sizeof(double)) \
                                          ? __va_double_arg(list, mode)           \
                                          : __va_stack_arg(list, mode))))[-1]
#define va_end(__list)

#endif
#endif
