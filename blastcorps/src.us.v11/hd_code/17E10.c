#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025C5D0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025C878.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025CE74.s")

/* TODO: func_8025D0B0 - for arg0==0, load a (baseAddr=0x487050,
 * size=0x2E20) pair; for arg0==1, (0x489E70, 0x5730) (0x489E70 is both
 * mode 0's end and mode 1's start - these read like adjacent slots in a
 * ROM resource table, each size = next_start - this_start); call
 * func_8028B4C4(baseAddr, D_80358070, &size, 0xc), then set
 * D_80366BB0[arg0] = D_80358070 & 0x1FFFFFFF and bump D_80358070 by size.
 * Logic and every field/offset are confirmed correct (every phrasing tried
 * lands on the right values) - two compiler-behavior gaps block a match
 * rather than any logic error. First: this compiler constant-folds
 * `end - baseAddr` into a single `li` the moment both operands are
 * literal-initialized locals in the same block, while target has a real
 * runtime `subu` of two lui/addiu-built registers - tried separating them
 * into distinct named locals (still folds) and `volatile` (stops the fold
 * but adds load/store pairs target doesn't have either). Second: target
 * keeps arg0 live in the callee-saved $s0 across the whole function
 * (saved/restored around the one call) instead of reloading its stack
 * byte 3 times like every C phrasing produces; `register` on the
 * parameter had no effect. Likely needs direct evidence of which pass
 * made each choice (decomp-workbench's register-role-audit) rather than
 * more source guessing. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025D0B0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025D184.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025D2B4.s")

/* TODO: func_8025E1E0 - writes 3 fixed 8-byte records (a flags word,
 * then a base+size word) into *arg0, *arg0+8 and *arg0+0x10 - record i's
 * flags are 0x0103/0x0101/0x0102 | 0x40, and its second word is
 * 0x02000000 + {0x80,0x140,0x1c0} (consecutive, suggesting real adjacent
 * ucode-task slot sizes) - then advances a cursor past the third record
 * and runs it through func_8024FC2C(&cursor, 0/1/2) before writing the
 * final cursor back to *arg0. Logic, every field value, and the frame
 * (4 pointer locals, matches target byte-for-byte) are all confirmed
 * correct (down to a 11-word edit distance, zero register-class diffs -
 * the register allocator's work is fully reproduced). The one gap: each
 * second word's `0x02000000 + offset` value is built by target as a pure
 * register temporary (lui, then a no-op addiu +0, then the real addiu
 * +offset - never spilled), while any source phrasing that avoids
 * compile-time constant folding (needed, since a bare literal sum folds
 * immediately) requires a real local, and assigning a local always homes
 * it to the stack here, inserting one extra `sw` per occurrence (+3
 * instructions total) and swapping the add for an `ori` (equivalent since
 * the low 16 bits never overlap, but not what target emits). Tried:
 * plain literal (folds to one `ori`), splitting into a separate pointer
 * local written then added to (closest: 11-word distance, but the
 * mandatory stack home remains), `register` on that local (reallocates
 * the whole function into callee-saved $s0-$s3 with a totally different
 * save/restore shape instead). */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025E1E0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025E2CC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025E67C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025EDF0.s")

/* TODO: func_8025F044 - repeat {if entry->unk28==0x20, post a {code=0x20,
 * param=entry+0x14} event with entry->unk4c as the 3rd field, else call
 * func_8025F0F0(entry, entry+0x28); then entry->unk50 =
 * func_802D6DB0(entry+0x14, entry+0x28)} until unk50 becomes nonzero, then
 * entry->unk54 += unk50 and return unk50 (a polling/retry loop with a
 * fixed `entry`, never advancing - the retries are driven entirely by the
 * three calls' side effects). Logic, every field offset, and the loop
 * condition are all confirmed correct (diff score down to 460, frame size
 * exact, zero inserts/deletes) - the one gap: inside the `unk28==0x20`
 * branch, target reloads `entry` from its stack home a second time right
 * before using it for the call's a0/a2 despite having just loaded it for
 * the preceding comparison, while every phrasing tried keeps reusing the
 * already-loaded register instead, cascading into register-rename diffs
 * for the rest of the function. Tried moving the inner `eventCode` local
 * to function scope; no effect (score 462, same shape). */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025F044.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025F0F0.s")

extern void *D_802E8CEC;

void func_802D74E0(s32, void *);
void func_802D7560(s32, void *);
void func_80260148(void *, void *, u16);

void func_8026005C(void *arg0) {
    if (*((u8 *) arg0 + 0x3e) & 4) {
        func_802D74E0(*(s32 *) ((u8 *) D_802E8CEC + 0x38), (u8 *) arg0 + 0xc);
        func_802D7560(*(s32 *) ((u8 *) D_802E8CEC + 0x38), (u8 *) arg0 + 0xc);
    }
    func_802604FC(arg0);
    func_80260148((u8 *) D_802E8CEC + 0x14, arg0, 0xffff);
}

/* TODO: func_802600D8 - looks up arg0->unk8->unk4->unk5 (a signed byte),
 * passes it to func_802D7610 (a float-returning lookup/conversion), scales
 * the result by arg0->unk2c, and posts a {code=0x10, param=arg0, extra=the
 * scaled float's raw bits} event via func_802D6C8C. Logic, the pointer
 * chase, and the event-struct field layout are all confirmed correct
 * (diff score down to 675, zero logic errors). The one gap: this compiler
 * always promotes a float local to a callee-saved register ($f20, with a
 * matching sdc1/ldc1 prologue/epilogue save) the moment its address is
 * taken to read its bits as an integer, even when - as here - the value is
 * fully consumed well before any later call and a bare stack home would
 * suffice; target has no such save at all. Tried: reordering the
 * assignments every way, `register` (illegal combined with `&`), and a
 * union in place of the pointer-cast reinterpret (same promotion either
 * way) - this looks like a blanket cfe rule for address-taken floats, not
 * something a source rephrasing routes around. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_802600D8.s")

s32 func_802D6EB0(void *);
s32 func_802D6EE0(void *, void *);
s32 func_802D7660(s32);

void func_80260148(void *arg0, void *arg1, u16 arg2) {
    void *list;
    void *next;
    void *entry2;
    void *dup;
    void *addr;
    s32 savedState;

    savedState = func_802D7660(1);
    list = *(void **) ((u8 *) arg0 + 8);
    while (list != NULL) {
        next = *(void **) list;
        addr = (u8 *) list + 0xc;
        entry2 = list;
        dup = next;

        if (*(void **) ((u8 *) list + 0x10) == arg1) {
            if ((*(u16 *) ((u8 *) list + 0xc) & arg2) != 0) {
                if (next != NULL) {
                    *(s32 *) ((u8 *) next + 8) += *(s32 *) ((u8 *) list + 8);
                }
                func_802D6EB0(list);
                func_802D6EE0(list, arg0);
            }
        }
        list = next;
    }

    func_802D7660(savedState);
}

extern void *D_802E8CE0;
extern void *D_802E8CE4;
extern void *D_802E8CE8;
s32 func_802D7660(s32);

u16 func_80260210(u16 *arg0, u16 *arg1) {
    s32 savedState;
    u16 count1;
    u16 count2;
    u16 count3;
    void *list1;
    void *list2;
    void *list3;

    savedState = func_802D7660(1);

    list1 = D_802E8CE0;
    list2 = D_802E8CE8;
    list3 = D_802E8CE4;

    count1 = 0;
    if (list1 != NULL) {
        do {
            count1 = count1 + 1;
        } while ((list1 = *(void **) list1) != NULL);
    }

    count2 = 0;
    if (list2 != NULL) {
        do {
            count2 = count2 + 1;
        } while ((list2 = *(void **) list2) != NULL);
    }

    count3 = 0;
    if (list3 != NULL) {
        do {
            count3 = count3 + 1;
        } while ((list3 = *(void **) ((u8 *) list3 + 4)) != NULL);
    }

    *arg0 = count2;
    *arg1 = count1;

    func_802D7660(savedState);
    return count3;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_80260300.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_802604FC.s")

void func_80260618(void *arg0, u8 arg1) {
    if (arg0 != NULL) {
        *(s8 *)((u8 *) arg0 + 0x36) = (s8)(s16)(u8) arg1;
    }
}

u8 func_80260634(void *arg0) {
    if (arg0 != NULL) {
        return *(u8 *) ((u8 *) arg0 + 0x3f);
    }
    return 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_80260650.s")

extern void *D_802E8CEC;
extern u8 D_8030914C;

s32 func_802D6C8C(void *, void *, s32);

/* eventTail/eventHead are dead stack space never read or written here - the
 * real source likely shares one ~0x10-byte event-record local across sites
 * like this one and only fills the leading eventCode/eventParam fields,
 * leaving the rest as reserved/unused padding (same shape as a plain array
 * would need, but writing it as one keeps the 0x24/0x20-offset slots from
 * ever materializing their own address, which doesn't match target). */
void func_802608C8(void *arg0) {
    s32 eventTail;
    s32 eventHead;
    s32 eventParam;
    s16 pad;
    s16 eventCode;

    eventCode = 0x400;
    eventParam = (s32) arg0;

    if (arg0 != NULL) {
        *((u8 *) arg0 + 0x3e) &= ~0x10;
        func_802D6C8C((u8 *) D_802E8CEC + 0x14, &eventCode, 0);
    } else {
        func_8029A7E4(&D_8030914C);
    }
}

extern void *D_802E8CE0;
s32 func_802D7660(s32);

void func_80260934(u8 arg0) {
    s32 savedState;
    s32 unused2;
    s32 unused1;
    s32 eventParam;
    s16 pad;
    s16 eventCode;
    void *entry;

    savedState = func_802D7660(1);
    entry = D_802E8CE0;
    if (entry != NULL) {
        do {
            eventCode = 0x400;
            eventParam = (s32) entry;
            if ((*((u8 *) entry + 0x3e) & arg0) == arg0) {
                *((u8 *) entry + 0x3e) &= ~0x10;
                func_802D6C8C((u8 *) D_802E8CEC + 0x14, &eventCode, 0);
            }
            entry = *(void **) entry;
        } while (entry != NULL);
    }
    func_802D7660(savedState);
}

void func_802609D0(void) {
    func_80260934(1);
}

void func_802609F0(void) {
    func_80260934(0x11);
}

void func_80260A10(void) {
    func_80260934(3);
}

void func_80260A30(u8 arg0) {
    s32 savedState;
    void *entry;
    s32 count;

    savedState = func_802D7660(1);
    entry = D_802E8CE0;
    count = 0;
    if (entry != NULL) {
        do {
            if ((*(u8 *) ((u8 *) (*(void **) ((u8 *) (*(void **) ((u8 *) entry + 8)) + 4)) + 2) & 0x3f) == arg0) {
                func_802608C8(entry);
            }
            count = count + 1;
        } while ((entry = *(void **) entry) != NULL);
    }
    func_802D7660(savedState);
}

extern u8 D_8030917C;

void func_80260AB8(void *arg0, s16 arg1, s32 arg2) {
    s32 unused;
    s32 eventExtra;
    s32 eventParam;
    s16 pad;
    s16 eventCode;

    eventCode = arg1;
    eventParam = (s32) arg0;
    eventExtra = arg2;

    if (arg0 != NULL) {
        func_802D6C8C((u8 *) D_802E8CEC + 0x14, &eventCode, 0);
    } else {
        func_8029A7E4(&D_8030917C);
    }
}

extern u16 *D_80366C28;

u16 func_80260B24(u8 arg0) {
    return D_80366C28[arg0];
}

extern void *D_802E8CE0;
extern void *D_802E8CEC;
s32 func_802D7660(s32);
s32 func_802D6C8C(void *, void *, s32);

void func_80260B40(u8 arg0, u16 arg1) {
    s32 savedState;
    void *entry;
    s32 count;
    s32 eventTail;
    s32 eventHead;
    s32 eventParam;
    s16 pad;
    s16 eventCode;

    savedState = func_802D7660(1);
    entry = D_802E8CE0;
    D_80366C28[arg0] = arg1;

    count = 0;
    while (entry != NULL) {
        if ((*(u8 *) ((u8 *) (*(void **) ((u8 *) (*(void **) ((u8 *) entry + 8)) + 4)) + 2) & 0x3f) == arg0) {
            eventCode = 0x800;
            eventParam = (s32) entry;
            func_802D6C8C((u8 *) D_802E8CEC + 0x14, &eventCode, 0);
        }
        count = count + 1;
        entry = *(void **) entry;
    }

    func_802D7660(savedState);
}
