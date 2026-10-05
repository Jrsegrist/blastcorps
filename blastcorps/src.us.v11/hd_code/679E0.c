#include "common.h"
#include <ultra64.h>

/* FILE-WIDE FINDING: this file's functions save $ra via the 64-bit `sd`/`ld`
 * doubleword form, not the normal 32-bit `sw`/`lw` pair - the same signature
 * documented in full in hd_code/77E20.c's top-of-file note and the project
 * skill file, confirming (with no exceptions found) that this file is part
 * of the same large hand-written-assembly module. Any pragma below without
 * a more specific comment follows this convention; a few have their own
 * more specific non-ABI explanation where one was already worked out. */
/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC1A0.s")

/* func_802AC284: trivial no-arg trampoline (`func_802AC3B8();`) using an
 * 8-byte `addiu sp,sp,-8`/`sd $ra,($sp)`/.../`addiu sp,sp,8` frame.
 * Confirmed via probe compile that IDO's own codegen for a real call
 * ALWAYS uses the 24-byte o32 argument-shadow frame (`sw $ra`) instead,
 * regardless of callee prototype visibility or any other phrasing tried
 * - the 8-byte `sd $ra` style literally never comes out of this
 * compiler for a genuine call. Same hand-written-stub family as
 * func_802BBE10/func_802C8AF0/func_802CE9A4 above. func_80260EC0 in
 * hd_code/1C460.c is an outwardly identical trampoline that matched
 * fine with the normal 24-byte form - that one really is compiler
 * output; this one isn't. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC284.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC2A4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC3B8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC4C4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC544.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC61C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC6FC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Serialize a 0xA6-byte block plus three words into `dst` (0xB2 bytes, dst
 * need not be aligned; `words` must be word-aligned and not overlap dst).
 * Returns the number of bytes written (always 0xB2; the asm leaves it in v0,
 * though its callers ignore it). Inverse of func_802AC85C. */
s32 func_802AC7DC(u8 *dst, u8 *src, u32 *words) {
    u8 *start = dst;
    s32 i;

    for (i = 0; i < 0xA6; i++) {
        *dst++ = *src++;
    }
    for (i = 0; i < 3; i++) {
        dst[0] = words[i] >> 24;
        dst[1] = words[i] >> 16;
        dst[2] = words[i] >> 8;
        dst[3] = words[i];
        dst += 4;
    }
    return dst - start;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC7DC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Deserialize what func_802AC7DC wrote: copy 0xA6 bytes from `src` to `dst`,
 * then the three (unaligned) big-endian words after them into `words`. */
void func_802AC85C(u8 *src, u8 *dst, u32 *words) {
    s32 i;

    for (i = 0; i < 0xA6; i++) {
        *dst++ = *src++;
    }
    for (i = 0; i < 3; i++) {
        words[i] = (src[0] << 24) | (src[1] << 16) | (src[2] << 8) | src[3];
        src += 4;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC85C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC8CC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACA60.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACAC4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACB50.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACBDC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACC68.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACCCC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACDB8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACE38.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACEB8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACF3C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACF64.s")
