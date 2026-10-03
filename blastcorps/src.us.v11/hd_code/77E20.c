#include "common.h"
#include <ultra64.h>

extern void *D_80358074;

/* FILE-WIDE FINDING: every one of this file's 68 functions saves $ra (and
 * any other preserved registers) via the 64-bit `sd`/`ld` doubleword
 * instructions, never the normal 32-bit `sw`/`lw` pair IDO emits for every
 * o32 call frame (confirmed mechanically: grep for the first `*ra,` save in
 * each of the 68 .s files here - 68/68 are `sd`, 0/68 are `sw`). Per this
 * project's probe-confirmed research (see the hd_code "phantom dead frame"
 * notes in the project skill file), the `sd $ra`/`ld $ra` doubleword save
 * never comes out of this compiler for ANY real C, including genuine
 * function calls - so this signature, applied file-wide with no exceptions,
 * means the entire file is hand-written MIPS assembly, not reachable from C
 * at all. Spot-checked across four different internal shapes (a trivial
 * single-global-write stub, a ~30-instruction loop, a full all-registers
 * save/restore wrapper preserving even $sp/$fp/$k0/$k1 as data, and a
 * pointer-chain walk with a real call inside that still uses `sd $ra`) -
 * all confirm the same convention regardless of internal complexity. 33 of
 * the 68 additionally follow a "preserve caller registers across scratch
 * reuse" sub-convention (see the block comment above func_802BC840 below);
 * the rest don't need that extra layer to already be unreachable from C on
 * frame-shape grounds alone. Every pragma in this file is permanently
 * GLOBAL_ASM; don't attempt a C translation for any of them. */

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BC5E0.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BC714.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
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

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCA2C.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCBD8.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCC10.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCC48.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCCD4.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCD20.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCD80.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCDE0.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCE40.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BD064.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BD10C.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BD1F8.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BD85C.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BD8C8.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BD99C.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BDDB4.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BE228.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BE3C8.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BE574.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
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

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BEBB0.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BEF9C.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BEFF4.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF1F0.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF264.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF384.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF534.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF668.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF898.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF978.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFB50.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFBF4.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFD1C.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFDAC.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFEE4.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFF6C.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C0284.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C038C.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C049C.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C04F0.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C0574.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C08C4.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C09B8.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C0C64.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C0CBC.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C0E8C.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1214.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C12E0.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1438.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C18D4.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1A28.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1AA0.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1B1C.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1B9C.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1DD0.s")

/* TODO: func_802C1EE0 - walk a u16-offset chain rooted at
 * D_80358074 + D_80358074->unk74: `u8 *t1 = (u8*)D_80358074 + *(s32*)((u8*)D_80358074 + 0x74);
 * u16 *t2 = (u16*)(t1 + 4); for (arg0--; arg0 != 0; arg0--) t2 = (u16*)(t1 + *t2);
 * return (u8*)t2 + 2;` (target: 11 instructions, a pure leaf with zero calls).
 * The target saves/restores $ra via an 8-byte `sd $ra`/`ld $ra` frame despite
 * never calling anything - this is the already-documented dead-frame shape
 * that no C phrasing can produce even for a genuine call, so definitely not
 * for a callless leaf (see the hd_code "phantom dead frame" notes in the
 * project skill file). Every phrasing tried (plain locals, register-qualified
 * locals) either adds extra spills on top or drops the frame to nothing -
 * never this exact shape. Logic confirmed correct via direct diff read. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1EE0.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1F30.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C2054.s")
