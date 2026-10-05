#include "common.h"
#include <ultra64.h>

#ifdef NON_MATCHING
/* Shared declarations for the NON_MATCHING (port) rewrites below. Most of
 * these functions take a level object (in t0 or t4 in the asm) whose header
 * holds offsets of its sub-lists: OBJ_PTR(obj, off) = obj + *(s32 *)(obj + off). */
extern u8 *D_80358070; /* heap pointer */
extern s32 D_802E8BDC; /* current level */
extern u64 D_80364A98; /* game mode */
extern u16 D_803BE714;
extern u16 D_803BE716;
extern OSIoMesg D_80370C58;
extern OSMesgQueue D_803150A0;

#define OBJ_PTR(obj, off) ((obj) + *(s32 *) ((obj) + (off)))
/* Big-endian 16/32-bit fields at any alignment (the asm assembles them from bytes). */
#define BE16U(p) (((p)[0] << 8) | (p)[1])
#define BE16S(p) ((s16) BE16U(p))
#define BE32(p) (((u32) (p)[0] << 24) | ((p)[1] << 16) | ((p)[2] << 8) | (p)[3])
#endif

/* func_802A1320: `mfc0 $v0, $12` (read COP0 Status register) wrapped in a
 * dead $ra save/restore frame - same hand-written COP0-leaf-stub character
 * as __osGetSR in init/2330.c. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
u32 __osGetSR(void);

/* Returns the COP0 Status register (the asm is a bare `mfc0 $v0, $12`);
 * the port's platform layer supplies __osGetSR. */
u32 func_802A1320(void) {
    return __osGetSR();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1320.s")
#endif

/* FILE-WIDE FINDING: this file's functions save $ra via the 64-bit `sd`/`ld`
 * doubleword form, not the normal 32-bit `sw`/`lw` pair - the same signature
 * documented in full in hd_code/77E20.c's top-of-file note and the project
 * skill file, confirming (with no exceptions found) that this file is part
 * of the same large hand-written-assembly module. Any pragma below without
 * a more specific comment follows this convention; a few have their own
 * more specific non-ABI explanation where one was already worked out. */
/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_80364460[]; /* 0x74-byte records */

/* Finds the 0x74-byte record of D_80364460 whose word at 0x5C is `id` (no
 * bound: it must exist) and stores v0Val, v1Val, a0Val and obj[0x9B] in its
 * words at 0x64/0x68/0x6C/0x70.
 * asm: a0Val in a0, id in a1, v0Val in v0, v1Val in v1, obj in gp (each
 * caller points gp at its own vehicle object). It changes no register.
 * Asm callers rely on preserved: a0, a1 (func_802B3180 & co), a1-a3, f12,
 * f14 (func_802AEEC8 & co), a3, t0, t6 (func_802B6294). */
void func_802A133C(s32 a0Val, s32 id, s32 v0Val, s32 v1Val, u8 *obj) {
    u8 *rec = D_80364460;

    while (*(s32 *) (rec + 0x5C) != id) {
        rec += 0x74;
    }
    *(s32 *) (rec + 0x64) = v0Val;
    *(s32 *) (rec + 0x68) = v1Val;
    *(s32 *) (rec + 0x6C) = a0Val;
    *(s32 *) (rec + 0x70) = obj[0x9B];
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A133C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 *D_803649D0; /* end of the D_80364460 records in use */
extern s32 D_80364AA8;
void func_802A1558(Gfx *src, Gfx *end, Gfx **dstp);

/* Appends a 0x74-byte record to D_80364460 (cursor D_803649D0) for the model
 * whose header is `hdr` (offsets in it are relative to hdr): word 0x70 = 0,
 * 4 = v0Val, 8 = v1Val, 0x5C = a0Val; word 0 and 0xC..0x2C = hdr + header
 * words 0x14 and 0x24..0x44, 0x54 = hdr + word 0x48. Copies the 8-byte units
 * [hdr + w[0x1C], hdr + w[0x20]) to the heap D_80358070 (advanced; the asm
 * loops with `!=`), and sets words 0x30..0x50 to words 0xC..0x2C moved by the
 * copy's displacement (heap - word 0xC). Unless D_80364AA8 == 1 and a1Val ==
 * 0, it then runs func_802A1558(word 0xC, word 0x18, rec + 0x58).
 * Register convention: a0Val/a1Val in a0/a1, v0Val/v1Val in v0/v1, hdr in s2
 * (conventions.txt); the asm saves t0-t6. Asm callers keep a1, a3, t7, f12,
 * f14 (some a2, t4-t6) live (a mixed build would need a thunk). */
void func_802A1388(s32 a0Val, s32 a1Val, s32 v0Val, s32 v1Val, u8 *hdr) {
    u8 *rec = D_803649D0;
    u64 *src;
    u64 *end;
    u64 *dst;
    s32 delta;
    s32 i;

    D_803649D0 = rec + 0x74;
    *(s32 *) (rec + 0x70) = 0;
    *(s32 *) (rec + 0x4) = v0Val;
    *(s32 *) (rec + 0x8) = v1Val;
    *(s32 *) (rec + 0x5C) = a0Val;
    *(u8 **) (rec + 0x0) = OBJ_PTR(hdr, 0x14);
    for (i = 0; i < 9; i++) {
        *(u8 **) (rec + 0xC + i * 4) = OBJ_PTR(hdr, 0x24 + i * 4);
    }
    *(u8 **) (rec + 0x54) = OBJ_PTR(hdr, 0x48);
    src = (u64 *) OBJ_PTR(hdr, 0x1C);
    end = (u64 *) OBJ_PTR(hdr, 0x20);
    dst = (u64 *) D_80358070;
    delta = (u32) dst - *(u32 *) (rec + 0xC);
    while (src != end) {
        *dst++ = *src++;
    }
    D_80358070 = (u8 *) dst;
    for (i = 0; i < 9; i++) {
        *(s32 *) (rec + 0x30 + i * 4) = *(s32 *) (rec + 0xC + i * 4) + delta;
    }
    if (D_80364AA8 != 1 || a1Val != 0) {
        func_802A1558(*(Gfx **) (rec + 0xC), *(Gfx **) (rec + 0x18), (Gfx **) (rec + 0x58));
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1388.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
void func_80278BF0(Gfx *src, Gfx *end, Gfx **dstp);

/* Register-preserving wrapper: func_80278BF0(src, end, dstp).
 * asm: src in t2, end in t3, dstp in t4; it saves and restores every
 * register (including v0 and s0-s7) around the call. */
void func_802A1558(Gfx *src, Gfx *end, Gfx **dstp) {
    func_80278BF0(src, end, dstp);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1558.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1674.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* An entry of the table at 0x80306480 (stride 0x30), ended by an entry
 * whose `end` byte is -1. The asm builds the address as a raw lui/addiu. */
typedef struct {
    /* 0x00 */ s32 x;
    /* 0x04 */ s32 y;
    /* 0x08 */ s32 z;
    /* 0x0C */ u8 pad0C[8];
    /* 0x14 */ u8 radius;
    /* 0x15 */ s8 end;
    /* 0x16 */ s16 box[12]; /* four corners (x, y, z) with x/z = centre -/+ radius */
    /* 0x2E */ u8 pad2E[2];
} BoxEntry; /* size 0x30 */

#define D_80306480 ((BoxEntry *) 0x80306480)

/* For each entry of the table at 0x80306480 converts the centre (x, y, z)
 * >> 5 (arithmetic) to the four corner points of a square of half-size
 * `radius`: (x-r, y, z-r), (x+r, y, z-r), (x-r, y, z+r) at 0x16/0x1C/...,
 * i.e. box = {x-r, y, z-r, x+r, y, z-r, x-r, y, z+r, x+r, y, z+r}.
 * Asm callers rely on preserved: func_802A1674 keeps t0, f12, f14 (the asm
 * saves v0, v1, a0-a3). */
void func_802A1934(void) {
    BoxEntry *e;

    for (e = D_80306480; e->end != -1; e++) {
        s32 x = e->x >> 5;
        s32 y = e->y >> 5;
        s32 z = e->z >> 5;
        s32 r = e->radius;

        e->box[0] = x - r;
        e->box[1] = y;
        e->box[2] = z - r;
        e->box[3] = x + r;
        e->box[4] = y;
        e->box[5] = z - r;
        e->box[6] = x - r;
        e->box[7] = y;
        e->box[8] = z + r;
        e->box[9] = x + r;
        e->box[10] = y;
        e->box[11] = z + r;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1934.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A19F4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 *D_803F7820;
extern u8 *D_803F7824;
extern u8 *D_803F7828;
extern u8 *D_803F782C;

/* Allocates from the heap D_80358070: 2n bytes of identity matrices (0x40-
 * byte fixed-point Mtx; D_803F7820 = start, D_803F7824 = start + n), then
 * unpacks the item blocks after the count word n at OBJ_PTR(obj, 0x74) into
 * 0x28-byte records (nine s16 << 5 as words, then a byte at 0x24). Each block
 * is a 0x2C-byte header (s16 tag at 0, -1 ending the list; s32 item count at
 * 0x28) followed by that many 0x44-byte items. D_803F7828 = record start, D_803F782C = D_80358070 =
 * end. The asm loops with `!=`, so n must be a multiple of 0x20.
 * asm: obj in t0; it leaves scratch values in s0-s3 (no caller reads them).
 * Asm callers rely on preserved: func_802A1674 keeps t0, f12, f14. */
void func_802A1A9C(u8 *obj) {
    u8 *hdr = OBJ_PTR(obj, 0x74);
    s32 n = *(s32 *) hdr;
    u8 *m = D_80358070;
    u8 *mend;

    D_803F7820 = m;
    D_803F7824 = m + n;
    mend = m + n + n;
    while (m != mend) {
        *(s16 *) (m + 0x00) = 1;
        *(s16 *) (m + 0x02) = 0;
        *(s32 *) (m + 0x04) = 0;
        *(s16 *) (m + 0x08) = 0;
        *(s16 *) (m + 0x0A) = 1;
        *(s32 *) (m + 0x0C) = 0;
        *(s32 *) (m + 0x10) = 0;
        *(s16 *) (m + 0x14) = 1;
        *(s16 *) (m + 0x16) = 0;
        *(s32 *) (m + 0x18) = 0;
        *(s16 *) (m + 0x1C) = 0;
        *(s16 *) (m + 0x1E) = 1;
        *(s32 *) (m + 0x20) = 0;
        *(s32 *) (m + 0x24) = 0;
        *(s32 *) (m + 0x28) = 0;
        *(s32 *) (m + 0x2C) = 0;
        *(s32 *) (m + 0x30) = 0;
        *(s32 *) (m + 0x34) = 0;
        *(s32 *) (m + 0x38) = 0;
        *(s32 *) (m + 0x3C) = 0;
        m += 0x40;
    }
    D_803F7828 = m;
    if (n != 0) {
        u8 *src = hdr + 4;
        s32 tag;

        do {
            s32 count;

            tag = *(s16 *) src;
            count = *(s32 *) (src + 0x28);
            src += 0x2C;
            while (count != 0) {
                count--;
                *(s32 *) (m + 0x00) = *(s16 *) (src + 0x00) << 5;
                *(s32 *) (m + 0x04) = *(s16 *) (src + 0x02) << 5;
                *(s32 *) (m + 0x08) = *(s16 *) (src + 0x04) << 5;
                *(s32 *) (m + 0x0C) = *(s16 *) (src + 0x06) << 5;
                *(s32 *) (m + 0x10) = *(s16 *) (src + 0x08) << 5;
                *(s32 *) (m + 0x14) = *(s16 *) (src + 0x0A) << 5;
                *(s32 *) (m + 0x18) = *(s16 *) (src + 0x0C) << 5;
                *(s32 *) (m + 0x1C) = *(s16 *) (src + 0x0E) << 5;
                *(s32 *) (m + 0x20) = *(s16 *) (src + 0x10) << 5;
                m[0x24] = src[0x12];
                src += 0x44;
                m += 0x28;
            }
        } while (tag != -1);
    }
    D_803F782C = m;
    D_80358070 = m;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1A9C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
u32 func_802A0CFC(s32 id, u8 *param);

/* Resolves the texture ids of the level object's records
 * [OBJ_PTR(obj, 0x2C), OBJ_PTR(obj, 0x30)): each record has a count byte
 * at +4 and count - 1 words from +0xC, each replaced by
 * func_802A0CFC(word, param) (its physical address); the next record follows
 * the last word. The walk stops only at exactly the end, and a count byte of
 * 0 loops (about 2^32 times) in the asm too, so both are preconditions.
 * The asm also sets s1 = 0x80000000 for every record (a leftover that its
 * caller func_802A1674 hands on to func_802A3D54's in/out s1; modelled
 * through `s1io`) and leaves t6 = the last id.
 * Register convention: obj in t0, param in fp (conventions.txt); its add/addi
 * trap. Asm caller func_802A1674 keeps t0, t9 live. */
void func_802A1C20(u8 *obj, u8 *param, s32 *s1io) {
    u8 *rec = OBJ_PTR(obj, 0x2C);
    u8 *end = OBJ_PTR(obj, 0x30);
    u32 *w;
    s32 n;

    while (rec != end) {
        n = rec[4];
        w = (u32 *) (rec + 0xC);
        *s1io = 0x80000000;
        n--;
        while (n != 0) {
            n--;
            *w = func_802A0CFC(*w, param);
            w++;
        }
        rec = (u8 *) w;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1C20.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 *D_803BE6E0;
extern u8 *D_803BE6E4;
extern u8 *D_803BE6E8;
extern u8 *D_803BE6EC;

/* Relocates the object's display lists: every gSPDisplayList (opcode 6) in
 * [OBJ_PTR(obj, 0x90), OBJ_PTR(obj, 0x84)) gets OBJ_PTR(obj, 0x78) - 0x80000000
 * added to its address word; then D_803BE6E0/E4/E8/EC = OBJ_PTR(obj, 0x90/
 * 0x94/0x98/0x9C). The asm loops with `!=` over 8-byte commands.
 * asm: obj in t0; it saves every register it uses but at.
 * Asm callers rely on preserved: func_802A1674 keeps t0, t6, t9, f12, f14. */
void func_802A1C88(u8 *obj) {
    u32 reloc = (u32) OBJ_PTR(obj, 0x78) - 0x80000000;
    u32 *cmd = (u32 *) OBJ_PTR(obj, 0x90);
    u32 *end = (u32 *) OBJ_PTR(obj, 0x84);

    while (cmd != end) {
        if ((cmd[0] >> 24) == 6) {
            cmd[1] += reloc;
        }
        cmd += 2;
    }
    D_803BE6E0 = OBJ_PTR(obj, 0x90);
    D_803BE6E4 = OBJ_PTR(obj, 0x94);
    D_803BE6E8 = OBJ_PTR(obj, 0x98);
    D_803BE6EC = OBJ_PTR(obj, 0x9C);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1C88.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1D54.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_802E8BDC; /* current level */
extern u8 *D_803BE704;
extern u8 *D_803BE708;
extern u8 D_802D30D0[], D_802D3194[], D_802D32A0[], D_802D331C[], D_802D33C8[];
extern u8 D_802D3444[], D_802D3538[], D_802D3614[], D_802D36C0[], D_802D3784[];
extern u8 D_802D3890[], D_802D393C[], D_802D3A00[], D_802D3A4C[], D_802D3BA0[];
extern u8 D_802D3BD4[], D_802D3CE0[], D_802D3DD4[], D_802D3EB0[], D_802D3F74[];

/* Picks the per-level data block for the current level D_802E8BDC:
 * D_803BE708 = block, D_803BE704 = block + 4; both NULL for levels without
 * one.
 * Asm callers rely on preserved: func_802A1D54 keeps t0, f12, f14 (the asm
 * saves t2). */
void func_802A1EC8(void) {
    u8 *p;

    switch (D_802E8BDC) {
        case 0x00: p = D_802D30D0; break;
        case 0x01: p = D_802D3194; break;
        case 0x02: p = D_802D32A0; break;
        case 0x03: p = D_802D331C; break;
        case 0x04: p = D_802D33C8; break;
        case 0x05: p = D_802D3444; break;
        case 0x09: p = D_802D3538; break;
        case 0x0A: p = D_802D3614; break;
        case 0x0C: p = D_802D36C0; break;
        case 0x0D: p = D_802D3784; break;
        case 0x0E: p = D_802D3890; break;
        case 0x0F: p = D_802D393C; break;
        case 0x10: p = D_802D3A00; break;
        case 0x11: p = D_802D3A4C; break;
        case 0x12: p = D_802D3BA0; break;
        case 0x1A: p = D_802D3BD4; break;
        case 0x1D: p = D_802D3CE0; break;
        case 0x21: p = D_802D3DD4; break;
        case 0x39: p = D_802D3EB0; break;
        case 0x3A: p = D_802D3F74; break;
        default:
            D_803BE704 = NULL;
            D_803BE708 = NULL;
            return;
    }
    D_803BE708 = p;
    D_803BE704 = p + 4;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1EC8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* If flag is set and the game mode D_80364A98 is 0x800, sets bytes 0xC/0xD/0xE
 * of every 0x10-byte entry in [obj + 0x50, OBJ_PTR(obj, 0x1C)) to 0xFF, 0, 0
 * (the asm loops with `!=`).
 * asm: obj in t4, flag in s0; it changes no register but at.
 * Asm callers rely on preserved: func_802A1D54 keeps a2, a3, t1-t7, f12, f14. */
void func_802A20F4(u8 *obj, s32 flag) {
    u8 *p;
    u8 *end;

    if (flag != 0 && D_80364A98 == 0x800) {
        end = OBJ_PTR(obj, 0x1C);
        for (p = obj + 0x50; p != end; p += 0x10) {
            p[0xC] = 0xFF;
            p[0xD] = 0;
            p[0xE] = 0;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A20F4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* On level 0x31, sets obj[4] = 2 for objects of kind 0x24.
 * asm: kind in t3, obj in s0; it changes no register but at.
 * Asm callers rely on preserved: func_802A1D54 keeps a2, a3, t1-t3, f12, f14. */
void func_802A2164(s32 kind, u8 *obj) {
    if (D_802E8BDC == 0x31 && kind == 0x24) {
        obj[4] = 2;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A2164.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A21AC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_8030631F[];

/* rec[0] points at an object; remaps byte 0x31 of every 0x38-byte entry in
 * [OBJ_PTR(obj, 0x30), OBJ_PTR(obj, 0x38)) through the byte table D_8030631F
 * (the asm loops with `!=`).
 * asm: rec in v1; it changes no register.
 * Asm callers rely on preserved: func_802A21AC keeps a2, a3, t4, t9, f12, f14. */
void func_802A23E0(u8 *rec) {
    u8 *obj = *(u8 **) rec;
    u8 *p = OBJ_PTR(obj, 0x30);
    u8 *end = OBJ_PTR(obj, 0x38);

    while (p != end) {
        p[0x31] = D_8030631F[p[0x31]];
        p += 0x38;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A23E0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* rec's words at 0x28 / 0x2C = the midpoints (x0 + x3) / 2 and (z0 + z6) / 2
 * (arithmetic shift), << 5, of the s16 array at OBJ_PTR(obj, 0x1C).
 * asm: rec in v1, obj in t4; it changes no register.
 * Asm callers rely on preserved: func_802A21AC keeps a2, a3, t3-t7, t9, f12, f14. */
void func_802A2458(u8 *rec, u8 *obj) {
    s16 *v = (s16 *) OBJ_PTR(obj, 0x1C);

    *(s32 *) (rec + 0x28) = ((v[0] + v[3]) >> 1) << 5;
    *(s32 *) (rec + 0x2C) = ((v[2] + v[8]) >> 1) << 5;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A2458.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A24BC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
u32 func_802A0CFC(s32 id, u8 *param);

/* Like func_802A1C20 for the records [OBJ_PTR(obj, 0x28), OBJ_PTR(obj, 0x2C)):
 * each record's word 0 is replaced by func_802A0CFC(word, param), then (the
 * count byte at +4 read after that call) count - 1 words from +0x10 likewise;
 * the next record follows the last word. Same preconditions (exact end,
 * count >= 1).
 * Register convention: obj in t4, param in fp (conventions.txt); the asm saves
 * t3, t5-t7, s0, t9, leaves s1 = 0x80000000 (dead in its caller) and t1/t2
 * changed; its addi trap. Asm caller func_802A21AC keeps t3-t7, t9 live. */
void func_802A2608(u8 *obj, u8 *param) {
    u8 *rec = OBJ_PTR(obj, 0x28);
    u8 *end = OBJ_PTR(obj, 0x2C);
    u32 *w;
    s32 n;

    while (rec != end) {
        *(u32 *) rec = func_802A0CFC(*(u32 *) rec, param);
        n = rec[4];
        w = (u32 *) (rec + 0x10);
        n--;
        while (n != 0) {
            n--;
            *w = func_802A0CFC(*w, param);
            w++;
        }
        rec = (u8 *) w;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A2608.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Adds (dx, dy, dz) to (x, y, z) triples, per pattern, in arrays
 * of a level object (all loops use `!=` ends):
 *  s16 y values in [0x40, 0x44); the s16 (x, z, x, z) at 0x20;
 *  0x14-byte entries of three s16 (x, y, z) in [0x24, 0x28);
 *  four s16 triples at 0x1C; 0x10-byte entries in [obj + 0x50, 0x1C) unless
 *  the u16 at obj + 0xE is set; 0x19-byte entries of three unaligned big-
 *  endian s16 triples in [0x48, 0x4C); 0x38-byte entries in [0x30, 0x34) and
 *  [0x34, 0x38) whose words 0/1/2 become (w + dx/dy/dz) << 16 and word 10
 *  (w + dy) << 16; 8-byte entries of an s16 triple in [0x2C, 0x30).
 * asm: obj in t4, dx/dy/dz in t5/t6/t7; it changes no register but at.
 * Asm callers rely on preserved: func_802A1D54 keeps a2, a3, t1-t7, f12, f14. */
void func_802A26A8(u8 *obj, s32 dx, s32 dy, s32 dz) {
    s16 *h;
    s16 *hend;
    u8 *b;
    u8 *bend;
    s32 *w;
    s32 *wend;
    s32 i;
    s32 k;
    s32 d[3];

    d[0] = dx;
    d[1] = dy;
    d[2] = dz;

    h = (s16 *) OBJ_PTR(obj, 0x40);
    hend = (s16 *) OBJ_PTR(obj, 0x44);
    for (; h != hend; h++) {
        h[0] += dy;
    }

    h = (s16 *) OBJ_PTR(obj, 0x20);
    h[0] += dx;
    h[1] += dz;
    h[2] += dx;
    h[3] += dz;

    h = (s16 *) OBJ_PTR(obj, 0x24);
    hend = (s16 *) OBJ_PTR(obj, 0x28);
    for (; h != hend; h += 10) {
        for (k = 0; k < 9; k++) {
            h[k] += d[k % 3];
        }
    }

    h = (s16 *) OBJ_PTR(obj, 0x1C);
    for (i = 4; i != 0; i--) {
        h[0] += dx;
        h[1] += dy;
        h[2] += dz;
        h += 3;
    }

    if (*(u16 *) (obj + 0xE) == 0) {
        h = (s16 *) (obj + 0x50);
        hend = (s16 *) OBJ_PTR(obj, 0x1C);
        for (; h != hend; h += 8) {
            h[0] += dx;
            h[1] += dy;
            h[2] += dz;
        }
    }

    b = OBJ_PTR(obj, 0x48);
    bend = OBJ_PTR(obj, 0x4C);
    for (; b != bend; b += 0x19) {
        for (k = 0; k < 9; k++) {
            s32 v = BE16S(b + k * 2) + d[k % 3];

            b[k * 2] = v >> 8;
            b[k * 2 + 1] = v;
        }
    }

    for (i = 0x30; i != 0x38; i += 4) {
        w = (s32 *) OBJ_PTR(obj, i);
        wend = (s32 *) OBJ_PTR(obj, i + 4);
        for (; w != wend; w += 14) {
            w[0] = (w[0] + dx) << 16;
            w[1] = (w[1] + dy) << 16;
            w[2] = (w[2] + dz) << 16;
            w[10] = (w[10] + dy) << 16;
        }
    }

    h = (s16 *) OBJ_PTR(obj, 0x2C);
    hend = (s16 *) OBJ_PTR(obj, 0x30);
    for (; h != hend; h += 4) {
        h[0] += dx;
        h[1] += dy;
        h[2] += dz;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A26A8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A2A98.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 *D_803BE6F0;

/* Takes 0x800 bytes from the heap D_80358070 (start kept in D_803BE6F0) and
 * DMAs ROM 0x6EC4C0..+0x800 into it, waiting for completion.
 * asm: leaves the buffer address in s0 (unsaved; no caller reads it); its
 * survey "outputs" a2/a3/f12/f14 are just what the libultra calls leave.
 * Asm callers rely on preserved: func_802A1D54 keeps t0. */
void func_802A2BB0(void) {
    u8 *buf = D_80358070;

    D_803BE6F0 = buf;
    D_80358070 = buf + 0x800;
    osInvalDCache(buf, 0x800);
    osPiStartDma(&D_80370C58, OS_MESG_PRI_NORMAL, OS_READ, 0x6EC4C0, buf, 0x800, &D_803150A0);
    osRecvMesg(&D_803150A0, NULL, OS_MESG_BLOCK);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A2BB0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_80364A6E;
extern u8 D_803BDFD8[]; /* 0x24-byte entries */
extern u8 *D_803BDFD4;  /* end of the D_803BDFD8 entries */

/* Unpacks the byte stream [OBJ_PTR(obj, 0x60), OBJ_PTR(obj, 0x64)): the first
 * byte goes to D_80364A6E, then each record (four big-endian u16, two bytes,
 * a name of byte-count n, two bytes) becomes a 0x24-byte D_803BDFD8 entry:
 * words 0/4/8/0xC = u16 << 5, byte 0x10, n at 0x13, the name at 0x15, the
 * last two bytes at 0x11/0x14 and 1 at 0x12. D_803BDFD4 = end of the entries.
 * The asm loops with `!=`, so the stream must end on a record boundary.
 * asm: obj in t0; it clobbers t1-t6.
 * Asm callers rely on preserved: func_802A1674 keeps t0, f12, f14. */
void func_802A2C54(u8 *obj) {
    u8 *p = OBJ_PTR(obj, 0x60);
    u8 *end = OBJ_PTR(obj, 0x64);
    u8 *e = D_803BDFD8;
    u8 *name;
    u32 n;

    D_80364A6E = *p++;
    while (p != end) {
        *(s32 *) (e + 0x0) = BE16U(p + 0) << 5;
        *(s32 *) (e + 0x4) = BE16U(p + 2) << 5;
        *(s32 *) (e + 0x8) = BE16U(p + 4) << 5;
        *(s32 *) (e + 0xC) = BE16U(p + 6) << 5;
        e[0x10] = p[8];
        n = p[9];
        e[0x13] = n;
        p += 10;
        name = e + 0x15;
        while (n != 0) {
            *name++ = *p++;
            n--;
        }
        e[0x11] = p[0];
        e[0x14] = p[1];
        e[0x12] = 1;
        p += 2;
        e += 0x24;
    }
    D_803BDFD4 = e;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A2C54.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern f32 D_803EBBF0;
extern u8 *D_80364458;
extern u8 *D_803BDAF4;
extern u8 *D_803BDAF8;
extern s32 D_803BE70C;
extern s32 D_803BE710;
extern u16 D_803BE720;
extern u16 D_803BE722;
extern s32 D_803BE718;
extern s32 D_803BE71C;
extern u16 D_803BE72C;
extern u16 D_803BE72E;
extern s32 D_803BE724;
extern s32 D_803BE728;
extern u8 *D_803EBBEC;
extern u8 D_803EBDB0[];
extern u8 D_803A6B30[];  /* 100 x 0x14 */
extern u8 D_803A7300[];  /* 12 x 0x14 */
extern u8 D_803ED3B8[];  /* 12 words */
extern u8 D_803EBC10[];  /* 26 x 0x10 */
extern u8 *D_803BE6F8;
extern s16 D_803BE730;
extern s16 D_803BE732;
extern s16 D_803BE734;
extern s16 D_803BE736;
extern u8 D_803A7426;
extern s16 D_803ED400;
extern u8 D_803F7804;
extern u8 D_803F7805;
extern u8 D_803F7806;
extern u8 D_803BE738;
extern u8 D_803BE739;
extern u8 D_803EFECB;
extern u8 D_803EF6FF;
extern u8 D_803ED40C;
extern s32 D_803A740C;
extern s32 D_803F77F8;
extern u8 D_803F780A;
extern u8 D_803F780B;
extern u8 D_803F780C;
extern u8 D_803F7810;
extern u8 D_803F7812;

/* Level setup from the level header obj: copies its sizes and bounds into
 * globals (u16 fields at 4/6/0xC/0xE/0x14/0x16 are stored << 5), points
 * D_80364458/D_803BDAF4/D_803BDAF8/D_803BE6F8 into it, marks four tables
 * empty (-1 bytes/words) and clears a set of per-level flags.
 * asm: obj in t0; it clobbers t1-t3 and f0.
 * Asm callers rely on preserved: func_802A1674 keeps t0, t6, t9, f12, f14. */
void func_802A2D68(u8 *obj) {
    s16 *v;
    s32 i;

    D_803EBBF0 = *(s32 *) (obj + 0x18);
    D_80364458 = obj + 0xC8;
    D_803BDAF4 = OBJ_PTR(obj, 0x24);
    D_803BDAF8 = OBJ_PTR(obj, 0x28);
    D_803BE714 = *(u16 *) (obj + 0x0);
    D_803BE716 = *(u16 *) (obj + 0x2);
    D_803BE70C = *(u16 *) (obj + 0x4) << 5;
    D_803BE710 = *(u16 *) (obj + 0x6) << 5;
    D_803BE720 = *(u16 *) (obj + 0x8);
    D_803BE722 = *(u16 *) (obj + 0xA);
    D_803BE718 = *(u16 *) (obj + 0xC) << 5;
    D_803BE71C = *(u16 *) (obj + 0xE) << 5;
    D_803BE72C = *(u16 *) (obj + 0x10);
    D_803BE72E = *(u16 *) (obj + 0x12);
    D_803BE724 = *(u16 *) (obj + 0x14) << 5;
    D_803BE728 = *(u16 *) (obj + 0x16) << 5;
    D_803EBBEC = D_803EBDB0;
    for (i = 0; i < 100; i++) {
        D_803A6B30[i * 0x14 + 0x13] = 0xFF;
    }
    for (i = 0; i < 12; i++) {
        D_803A7300[i * 0x14 + 0x11] = 0xFF;
    }
    for (i = 0; i < 12 * 4; i++) {
        D_803ED3B8[i] = 0xFF;
    }
    for (i = 0; i < 26; i++) {
        *(s32 *) (D_803EBC10 + i * 0x10) = -1;
    }
    D_803BE6F8 = OBJ_PTR(obj, 0x50);
    v = (s16 *) OBJ_PTR(obj, 0x4C);
    D_803BE730 = v[0];
    D_803BE734 = v[1];
    D_803BE732 = v[2];
    D_803BE736 = v[3];
    D_803A7426 = 0;
    D_803ED400 = 0;
    D_803F7805 = 0;
    D_803F7806 = 0;
    D_803BE738 = 0;
    D_803BE739 = *(s32 *) (obj + 0x1C);
    D_803EFECB = 0;
    D_803EF6FF = 0;
    D_803ED40C = 0;
    D_803A740C = 0;
    D_803F77F8 = 0;
    D_803F780A = 0;
    D_803F780B = 0;
    D_803F780C = 0;
    D_803F7810 = 0;
    D_803F7804 = 0;
    D_803F7812 = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A2D68.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
typedef struct {
    /* 0x0 */ u8 **s2;
    /* 0x4 */ u8 *s3;
    /* 0x8 */ u8 *s4; /* in/out */
} Unk802A08E4Regs; /* as in 5BF40.c */
void func_802A08E4(u32 *dl, u32 *end, Unk802A08E4Regs *r);

/* Resolves the G_SETTIMG texture ids in the level object's display list
 * [OBJ_PTR(obj, 0x78), OBJ_PTR(obj, 0x84)) with func_802A08E4. Returns the
 * list end, which the asm leaves in s1 and its caller func_802A1674 hands on
 * to func_802A3D54's in/out s1 when func_802A1C20 doesn't overwrite it.
 * Register convention: obj in t0, result in s1 (conventions.txt); the asm
 * saves t0 and passes its caller's s4 through as func_802A08E4's in/out s4
 * (`r` is a local here); s0, s2-s6 are left changed (dead in the caller).
 * Asm caller func_802A1674 keeps t0, t6, t9 live. */
u8 *func_802A3008(u8 *obj) {
    Unk802A08E4Regs r;
    u8 *end = OBJ_PTR(obj, 0x84);

    func_802A08E4((u32 *) OBJ_PTR(obj, 0x78), (u32 *) end, &r);
    return end;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A3008.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A303C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A30DC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A3134.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A3198.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A32CC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A350C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_80364AA8;
extern u8 D_803643D4;
extern u8 D_803BE73A;

/* 9-byte records in [OBJ_PTR(obj, 0x50), OBJ_PTR(obj, 0x54)): a kind byte and
 * three unaligned big-endian s16 (x, y, z). Finds the first record of kind 0
 * (searched from the start with no end bound: it must exist), then for every
 * nonzero-kind record at the same position sets D_803BE73A to its kind, or
 * to D_803643D4 unless D_80364AA8 is 1 or 0x80. The asm loops with `!=`.
 * asm: obj in t0; it changes no register but at.
 * Asm callers rely on preserved: func_802A350C keeps a1-a3, t0, f12, f14. */
void func_802A3824(u8 *obj) {
    u8 *p = OBJ_PTR(obj, 0x50);
    u8 *end = OBJ_PTR(obj, 0x54);
    u8 *r = p;
    s32 x;
    s32 y;
    s32 z;
    u8 kind;

    while (r[0] != 0) {
        r += 9;
    }
    x = BE16S(r + 1);
    y = BE16S(r + 3);
    z = BE16S(r + 5);
    for (; p != end; p += 9) {
        kind = p[0];
        if (kind != 0 && BE16S(p + 1) == x && BE16S(p + 3) == y && BE16S(p + 5) == z) {
            if (D_80364AA8 != 1 && D_80364AA8 != 0x80) {
                kind = D_803643D4;
            }
            D_803BE73A = kind;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A3824.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A396C.s")

/* func_802A3D54 .. func_802A3F80: build the level's collision triangle
 * records (func_802A41B0, 0x60 bytes each) at the heap D_80358070 from the
 * level object's packed data. Several of func_802A41B0's byte arguments are
 * whatever the asm happens to hold in that register (loop counters, low
 * bytes of pointers); the rewrites pass the same values.
 * Register convention (conventions.txt): obj in t0, triangle id in v0, kind
 * byte in t9, and the in/out s1 that func_802A41B0 threads from one call to
 * the next (through `s1io`). The asm saves t0 only, sets gp (0 or 1, passed
 * as func_802A41B0's b55) and leaves func_802A41B0's s2-s7 scratch. Its asm
 * caller func_802A1674 keeps t0, t9, f12, f14 live. */
#ifdef NON_MATCHING
u8 *func_802A41B0(u8 *rec, u8 *v, s32 id, s32 h52, s32 b57, s32 b56, s32 *s1io, s32 b4F, s32 b55);
s32 func_802A4168(s32 id, u8 *end);
extern u8 *D_803BDCA8[];
extern u8 *D_803BDE40[];

/* Grid of (s16) obj[0x10] * (s16) obj[0x12] cells (must be >= 1: the asm
 * counts down with `!=`) at OBJ_PTR(obj, off): each cell is an unaligned BE
 * word (end of its triangles, relative to the grid) followed by 0x16-byte
 * packed triangles. table[cell] = the cell's first record (table[cells] =
 * the end). Per triangle: func_802A41B0(rec, tri, id, h52 = cells left,
 * b57 = the cell end pointer, b56 = tri[0x14], s1io, b4F, 0), then
 * rec[0x59] = tri[0x15]. Returns the last cell's end pointer (the asm's t6). */
static u8 *port_build_grid(u8 *obj, s32 off, u8 **table, s32 id, s32 b4F, s32 *s1io) {
    u8 *base = OBJ_PTR(obj, off);
    u8 *t = base;
    u8 *cellEnd;
    u8 *rec = D_80358070;
    s32 cells = *(s16 *) (obj + 0x10) * *(s16 *) (obj + 0x12);

    do {
        *table++ = rec;
        cellEnd = base + BE32(t);
        t += 4;
        while (t != cellEnd) {
            rec = func_802A41B0(rec, t, id, cells, (s32) cellEnd, t[0x14], s1io, b4F, 0);
            rec[-7] = t[0x15];
            t += 0x16;
        }
        cells--;
    } while (cells != 0);
    *table = rec;
    D_80358070 = rec;
    return cellEnd;
}

/* Grid at obj word 0x6C, cell table D_803BDCA8. */
u8 *func_802A3D54(u8 *obj, s32 id, s32 b4F, s32 *s1io) {
    return port_build_grid(obj, 0x6C, D_803BDCA8, id, b4F, s1io);
}
#else
/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A3D54.s")
#endif

#ifdef NON_MATCHING
/* Grid at obj word 0x70, cell table D_803BDE40 (otherwise func_802A3D54). */
u8 *func_802A3DF8(u8 *obj, s32 id, s32 b4F, s32 *s1io) {
    return port_build_grid(obj, 0x70, D_803BDE40, id, b4F, s1io);
}
#else
/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A3DF8.s")
#endif

#ifdef NON_MATCHING
extern u8 *D_803BD308;
extern u8 *D_803BD30C;
extern u8 D_803BD310[]; /* 0xFC-byte groups */
extern u8 *D_803BDAF0;  /* end of the groups */

/* Named triangle groups in [OBJ_PTR(obj, 0x64), OBJ_PTR(obj, 0x68)): a tag
 * byte, a name (length byte + bytes), a count byte, then that many 0x14-byte
 * packed triangles. Each becomes a 0xFC-byte group in D_803BD310: [0] = name
 * length, [1..] = name, [8 + 4i] = record of triangle i, [0xF8] = count,
 * [0xF9] = tag. Records start at the heap D_80358070 (also kept in
 * D_803BD308; the end goes to D_803BD30C and the heap pointer); the group
 * end to D_803BDAF0. Per triangle: func_802A41B0(rec, tri, id, h52 = the
 * end pointer, b57 = triangles left after this one, b56 = the next pointer
 * slot, s1io, b4F, 1). The asm loops with `!=` / count-downs. */
void func_802A3E9C(u8 *obj, s32 id, s32 b4F, s32 *s1io) {
    u8 *p = OBJ_PTR(obj, 0x64);
    u8 *end = OBJ_PTR(obj, 0x68);
    u8 *grp = D_803BD310;
    u8 *rec = D_80358070;
    u8 **slot;
    u8 *q;
    s32 n;

    D_803BD308 = rec;
    while (p != end) {
        grp[0xF9] = p[0];
        n = p[1];
        p += 2;
        grp[0] = n;
        for (q = grp + 1; n != 0; n--) {
            *q++ = *p++;
        }
        n = *p++;
        grp[0xF8] = n;
        slot = (u8 **) (grp + 8);
        while (n != 0) {
            n--;
            *slot++ = rec;
            rec = func_802A41B0(rec, p, id, (s32) end, n, (s32) slot, s1io, b4F, 1);
            p += 0x14;
        }
        grp += 0xFC;
    }
    D_803BDAF0 = grp;
    D_803BD30C = rec;
    D_80358070 = rec;
}
#else
/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A3E9C.s")
#endif

#ifdef NON_MATCHING
extern u8 D_803BC1D0[]; /* 0xDC-byte groups */
extern u8 D_803B9890[]; /* 0x60-byte records */
extern u8 *D_803BD300;  /* end of the D_803B9890 records */
extern u8 *D_803BD304;  /* end of the groups */

/* Object groups in [OBJ_PTR(obj, 0x68), OBJ_PTR(obj, 0x6C)), each becoming a
 * 0xDC-byte group in D_803BC1D0: bytes [0xC4..0xC6] = a, kind, n from the
 * data. Kind 0: n (>= 1) sets of six BE s16 (each << 5, as words; the third
 * of the last set is also left in s1, i.e. *s1io) and then a BE word; other
 * kinds: n (>= 1) BE s16 as halfwords. Then a count of 0x15-byte packed
 * triangles: [0xC7] = count, [0xC8 + i] = triangle i's id (its byte 0x14);
 * a record is built (func_802A41B0 at the D_803B9890 cursor, b4F 0, b55 1)
 * only for ids no record before this group has (func_802A4168). Then a
 * length byte and that many bytes at [0xD0], [0xD1..]. h52 is the end
 * pointer, b57 the triangles left, b56 the last value read in the first part
 * (the BE word / the last s16). The new record end goes to D_803BD300 and is
 * returned (the asm's fp); the group end to D_803BD304. */
u8 *func_802A3F80(u8 *obj, s32 *s1io) {
    u8 *p = OBJ_PTR(obj, 0x68);
    u8 *end = OBJ_PTR(obj, 0x6C);
    u8 *grp = D_803BC1D0;
    u8 *recEnd = D_803B9890;
    u8 *rec;
    u8 *q;
    s32 *w;
    s32 last;
    s32 kind;
    s32 n;
    s32 id;

    while (p != end) {
        w = (s32 *) grp;
        grp[0xC4] = p[0];
        kind = p[1];
        grp[0xC5] = kind;
        n = p[2];
        grp[0xC6] = n;
        p += 3;
        if (kind == 0) {
            do {
                w[0] = BE16S(p + 0) << 5;
                w[1] = BE16S(p + 2) << 5;
                w[2] = *s1io = BE16S(p + 4) << 5;
                w[3] = BE16S(p + 6) << 5;
                w[4] = BE16S(p + 8) << 5;
                w[5] = BE16S(p + 10) << 5;
                n--;
                p += 12;
                w += 6;
            } while (n != 0);
            last = BE32(p);
            p += 4;
            *w = last;
        } else {
            do {
                last = BE16S(p);
                *(s16 *) w = last;
                n--;
                p += 2;
                w = (s32 *) ((u8 *) w + 2);
            } while (n != 0);
        }
        n = *p++;
        grp[0xC7] = n;
        q = grp + 0xC8;
        rec = recEnd;
        for (; n != 0; n--) {
            id = p[0x14];
            *q++ = id;
            if (func_802A4168(id, recEnd) == 0) {
                rec = func_802A41B0(rec, p, id, (s32) end, n, last, s1io, 0, 1);
            }
            p += 0x15;
        }
        recEnd = rec;
        n = *p++;
        grp[0xD0] = n;
        for (q = grp + 0xD1; n != 0; n--) {
            *q++ = *p++;
        }
        grp += 0xDC;
    }
    D_803BD304 = grp;
    D_803BD300 = recEnd;
    return recEnd;
}
#else
/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A3F80.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803B9890[]; /* 0x60-byte records */

/* Returns 1 if a 0x60-byte record in [D_803B9890, end) has byte 0x50 == id,
 * else 0. The asm loops with `!=`.
 * asm: id in v0, end in fp, result in t5; it saves t0/t1.
 * Asm callers rely on preserved: func_802A3F80 keeps t1-t4, t6, t7, t9. */
s32 func_802A4168(s32 id, u8 *end) {
    u8 *p;

    for (p = D_803B9890; p != end; p += 0x60) {
        if (p[0x50] == id) {
            return 1;
        }
    }
    return 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A4168.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Sets up a 0x60-byte collision triangle record from 0x14 bytes of packed
 * vertex data: three unaligned big-endian s16 (x, y, z) points P0..P2 (each
 * << 3, stored as words at 0x28..0x48) and a u16 at 0x12 (stored at 0x4C).
 * The plane normal n = (P0 - P1) x (P0 - P2) and -(n . P1) are s64 at 0, 8,
 * 0x10 and 0x18; |n|^2 and |n| as floats at 0x24 / 0x20; byte 0x4E is the
 * dominant normal axis (0 = z, 1 = y, else 2). Bytes 0x4F..0x59 come from the
 * arguments. Returns rec + 0x60.
 * asm: rec in t4 (returned advanced in t4), v in t5, id in v0, h52 in t2,
 * b57 in t6, b56 in t7, b4F in t9, b55 in gp; b58 in s1, and s1 comes back
 * as P0.x - P2.x (func_802A3F80 and func_802A3D54 feed it to the next
 * call). It also leaves the P1.x/P1.y in a2/a3 and other products in s2-s7
 * (no caller reads them).
 * Asm callers rely on preserved: func_802A21AC keeps t0, t1, t5, t9, f12,
 * f14; func_802A3D54/3DF8/3E9C/3F80 keep t1-t3, t5-t7, t9; func_802CE9C8
 * keeps t0, t2, t5, t6, t9. */
u8 *func_802A41B0(u8 *rec, u8 *v, s32 id, s32 h52, s32 b57, s32 b56, s32 *s1io, s32 b4F, s32 b55) {
    s32 x0, y0, z0, x1, y1, z1, x2, y2, z2;
    s32 dx1, dy1, dz1, dx2, dy2, dz2;
    s64 nx, ny, nz;
    s64 ax, ay, az;
    f32 f;
    f32 sum;

    rec[0x59] = 0;
    rec[0x55] = b55;
    rec[0x56] = b56;
    *(s16 *) (rec + 0x52) = h52;
    rec[0x4F] = b4F;
    rec[0x50] = id;
    rec[0x57] = b57;
    rec[0x58] = *s1io;

    x0 = BE16S(v + 0x0) << 3;
    y0 = BE16S(v + 0x2) << 3;
    z0 = BE16S(v + 0x4) << 3;
    x1 = BE16S(v + 0x6) << 3;
    y1 = BE16S(v + 0x8) << 3;
    z1 = BE16S(v + 0xA) << 3;
    x2 = BE16S(v + 0xC) << 3;
    y2 = BE16S(v + 0xE) << 3;
    z2 = BE16S(v + 0x10) << 3;
    dx1 = x0 - x1;
    dy1 = y0 - y1;
    dz1 = z0 - z1;
    dx2 = x0 - x2;
    dy2 = y0 - y2;
    dz2 = z0 - z2;
    *(s32 *) (rec + 0x28) = x0;
    *(s32 *) (rec + 0x2C) = y0;
    *(s32 *) (rec + 0x30) = z0;
    *(s32 *) (rec + 0x34) = x1;
    *(s32 *) (rec + 0x38) = y1;
    *(s32 *) (rec + 0x3C) = z1;
    *(s32 *) (rec + 0x40) = x2;
    *(s32 *) (rec + 0x44) = y2;
    *(s32 *) (rec + 0x48) = z2;

    nx = (s64) dy1 * dz2 - (s64) dz1 * dy2;
    ny = (s64) dz1 * dx2 - (s64) dx1 * dz2;
    nz = (s64) dx1 * dy2 - (s64) dy1 * dx2;
    *(s64 *) (rec + 0x0) = nx;
    *(s64 *) (rec + 0x8) = ny;
    *(s64 *) (rec + 0x10) = nz;
    f = nx;
    sum = f * f;
    f = ny;
    sum = sum + f * f;
    f = nz;
    sum = sum + f * f;
    *(s64 *) (rec + 0x18) = -(nx * x1 + ny * y1 + nz * z1);
    *(f32 *) (rec + 0x24) = sum;
    *(f32 *) (rec + 0x20) = sqrtf(sum);

    ax = (nx < 0) ? -nx : nx;
    ay = (ny < 0) ? -ny : ny;
    az = (nz < 0) ? -nz : nz;
    if (!(az < ax) && !(az < ay)) {
        rec[0x4E] = 0;
    } else if (!(ay < ax) && !(ay < az)) {
        rec[0x4E] = 1;
    } else {
        rec[0x4E] = 2;
    }
    rec[0x54] = 0;
    *(u16 *) (rec + 0x4C) = BE16U(v + 0x12);
    *s1io = dx2;
    return rec + 0x60;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A41B0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 *D_803BDB10[];

/* Builds a pointer table for a chain of length-prefixed blocks: with
 * base = OBJ_PTR(obj, 0x30) and n = (s16 at 8) * (s16 at 0xA), walks
 * p = base, p = base + (unaligned big-endian word at p), storing p + 4 for
 * each of the n blocks and once more at the end (n + 1 entries in
 * D_803BDB10). The asm counts n down with `!= 0`, so n must be >= 0.
 * asm: obj in t0; it clobbers t1-t3, t5, t6, hi/lo.
 * Asm callers rely on preserved: func_802A1674 keeps t0, f12, f14. */
void func_802A4464(u8 *obj) {
    s32 n = *(s16 *) (obj + 8) * *(s16 *) (obj + 0xA);
    u8 *base = OBJ_PTR(obj, 0x30);
    u8 *p = base;
    u8 **out = D_803BDB10;

    while (n != 0) {
        *out++ = p + 4;
        p = base + BE32(p);
        n--;
    }
    *out = p + 4;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A4464.s")
#endif

/* TODO: func_802A44E4 - round up to a multiple of 8 if not already a
 * multiple of 4: `if ((arg1 & 3) != 0) arg1 = (arg1 & ~7) + 8; return
 * arg1;` (target: 11 instructions, a `nop` in the branch's delay slot -
 * $a1 is the real parameter, with an unused leading parameter shadowing
 * $a0). Every type tried for the unused first parameter (void*, s32,
 * f32) makes IDO spill it into the delay slot anyway (`sw`/`swc1` at
 * 0(sp), no actual stack frame reserved for it) - the inverse of the
 * usual "unused argument still gets a dead stack home" pattern seen
 * elsewhere (func_80272C40 in init/26570.c): there the target WANTS the
 * dead spill and plain C gives it one; here the target doesn't want it
 * and plain C can't be talked out of it. Logic confirmed correct. */
#ifdef NON_MATCHING
/* Port rewrite (not matching): round x up to a multiple of 8 unless it is a
 * multiple of 4. asm: x in a1; the result in v0 and also left in a1, which
 * its asm callers (func_802A2A98, func_802A32CC, func_802A396C) read back
 * as the new heap pointer.
 * Asm callers rely on preserved: a2, a3, f12, f14. */
u32 func_802A44E4(u32 x) {
    if ((x & 3) != 0) {
        x = (x & ~7) + 8;
    }
    return x;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A44E4.s")
#endif
