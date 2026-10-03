#include "common.h"

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BC5E0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BC714.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BC7DC.s")

/* Confirmed hand-written assembly: a "preserve caller registers via explicit
 * stack save/restore" convention, distinct from every other hand-written-asm
 * family documented elsewhere in this project. Each function in this family
 * (33 of this file's 68) opens by `sd`-saving some subset of registers
 * (always including $v0/$v1; the largest, func_802BC888, saves literally
 * every register in the file including $at/$gp/$sp/$fp/$k0/$k1 as raw data)
 * to its own stack frame, is then free to clobber those same register
 * numbers as ordinary scratch for its real body, and `ld`-restores every
 * saved slot byte-for-byte before returning - the restored value is always
 * exactly what was saved, never read or influenced by the body in between.
 * No compiler emits this: IDO never saves/restores a register it doesn't
 * believe is a live local, and no C semantics could want $sp itself
 * round-tripped through memory as inert data (seen in func_802BC888). This
 * is a deliberate low-level convention - likely a shared boilerplate/macro
 * the original engine used for a family of dispatch-table callback/handler
 * functions that must leave the caller's register state untouched regardless
 * of their own internal register needs. Permanently GLOBAL_ASM; don't
 * attempt a C translation for any function flagged with this comment. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BC840.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BC888.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCA2C.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCBD8.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCC10.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCC48.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCCD4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCD20.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCD80.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCDE0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCE40.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BD064.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BD10C.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BD1F8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BD85C.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BD8C8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BD99C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BDDB4.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BE228.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BE3C8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BE574.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BE77C.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BE944.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BE9F8.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BEA30.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BEA70.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BEADC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BEBB0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BEF9C.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BEFF4.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF1F0.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF264.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF384.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF534.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF668.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF898.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF978.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFB50.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFBF4.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFD1C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFDAC.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFEE4.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFF6C.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C0284.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C038C.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C049C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C04F0.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C0574.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C08C4.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C09B8.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C0C64.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C0CBC.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C0E8C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1214.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C12E0.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1438.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C18D4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1A28.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1AA0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1B1C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1B9C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1DD0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1EE0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1F30.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C2054.s")
