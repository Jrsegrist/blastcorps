#include "common.h"
#include <ultra64.h>

/* func_802DBA60: `lui $k0,%hi(func_802DBA70); addiu $k0,$k0,%lo(func_802DBA70);
 * jr $k0; nop` - a raw kernel-register ($k0) tail-jump stub, same character
 * as __osExceptionPreamble in init/2330.c (confirmed hand-written SDK/boot
 * assembly there, not decompilable from idiomatic C). Permanently
 * GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/972A0/func_802DBA60.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/972A0/func_802DBA70.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/972A0/func_802DBFB8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/972A0/__osEnqueueAndYield.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/972A0/__osEnqueueThread.s")

/* __osPopThread: libultra __osPopThread (`*queue = (*queue)->next`,
 * returning the old head). Part of hand-written exceptasm.s, like its
 * neighbours __osEnqueueThread (__osEnqueueThread, also $t9-based) and the
 * $k0/`ld` context restore in __osDispatchThread - not compiled C, so the
 * $v0/$t9 register choice can't come from IDO. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/972A0/__osPopThread.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/972A0/__osDispatchThread.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/972A0/__osCleanupThread.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/972A0/func_802DC2D0.s")
