#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025C5D0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025C878.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025CE74.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025D0B0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025D184.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025D2B4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025E1E0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025E2CC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025E67C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025EDF0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025F044.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025F0F0.s")

extern void *D_802E8CEC;

void func_802D74E0(s32, void *);
void func_802D7560(s32, void *);

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

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_80260148.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_80260210.s")

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

/* TODO: func_80260A30 - lock via func_802D7660(1) (saving the previous
 * state), walk the D_802E8CE0 linked list (next pointer at offset 0, a
 * vestigial unused counter incremented once per node), and for each node
 * whose unk8->unk4->unk2 byte (masked 0x3f) equals arg0, call
 * func_802608C8(node); then restore the saved lock state. Logic, every
 * field offset, and the linked-list walk are all confirmed correct (diff
 * score down to 265, zero inserts/deletes). Remaining gap is the same
 * "defer the next-pointer store into the loop branch's delay slot" pattern
 * documented repeatedly in hd_code/48D00.c: a plain `entry =
 * *(void**)entry` reload-then-store sequence leaves one spurious reload
 * in, and the usual m2c-style split-temp fix makes it worse here too
 * (gives the temp its own stack slot, score 377) because func_802608C8 is
 * called inside the loop body. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_80260A30.s")

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

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_80260B40.s")
