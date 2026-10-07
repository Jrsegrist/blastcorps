#include "common.h"
#include <ultra64.h>
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_80364460 ((u8 *) D_80364460)
#ifdef NON_MATCHING
#define D_80365330 (*(s32 *) &D_80365330)
#define D_803BDB08 (*(u8 * N64P *) &D_803BDB08)
#define D_803BE6E0 (*(u8 * N64P *) &D_803BE6E0)
#define D_803BE6E4 (*(u8 * N64P *) &D_803BE6E4)
#define D_803BE6E8 (*(u8 * N64P *) &D_803BE6E8)
#define D_803BE6EC (*(u8 * N64P *) &D_803BE6EC)
#endif
/* end of views */

#ifdef NON_MATCHING
/* Shared declarations for the NON_MATCHING (port) rewrites below. Most of
 * these functions take a level object (in t0 or t4 in the asm) whose header
 * holds offsets of its sub-lists: OBJ_PTR(obj, off) = obj + *(s32 *)(obj + off). */

#define OBJ_PTR(obj, off) ((obj) + *(s32 *) ((obj) + (off)))
/* Big-endian 16/32-bit fields at any alignment (the asm assembles them from bytes). */
#define BE16U(p) (((p)[0] << 8) | (p)[1])
#define BE16S(p) ((s16) BE16U(p))
#define BE32(p) (((u32) (p)[0] << 24) | ((p)[1] << 16) | ((p)[2] << 8) | (p)[3])

/* Register results of func_802A32CC its asm caller reads. */
typedef struct {
    /* 0x0 */ u8 * N64P s2; /* the loaded data (old heap pointer) */
    /* 0x4 */ u8 * N64P s3; /* func_802A08E4's s3 */
    /* 0x8 */ u8 * N64P s4; /* func_802A08E4's s4 (in: the ROM start) */
} Out802A32CC;


void func_802A32CC(s32 type, Out802A32CC *out);

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
void func_802A1558(Gfx *src, Gfx *end, Gfx * N64P *dstp);

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
    *(u8 * N64P *) (rec + 0x0) = OBJ_PTR(hdr, 0x14);
    for (i = 0; i < 9; i++) {
        *(u8 * N64P *) (rec + 0xC + i * 4) = OBJ_PTR(hdr, 0x24 + i * 4);
    }
    *(u8 * N64P *) (rec + 0x54) = OBJ_PTR(hdr, 0x48);
    src = (u64 *) OBJ_PTR(hdr, 0x1C);
    end = (u64 *) OBJ_PTR(hdr, 0x20);
    dst = (u64 *) D_80358070;
    delta = (u32) dst - *(u32 *) (rec + 0xC);
    while (src != end) {
        *dst++ = *src++;
    }
#ifdef PORT_HOST
    /* the copy is display-list commands (host-order words, like the model's
     * own): record that for the stale-byte emulation (level 6's collision
     * records land on a copy made on the bonus-level screen) */
    {
        void port_unit_mark(void *p, u32 n, s32 width);
        port_unit_mark(D_80358070, (u32) ((u8 *) dst - (u8 *) D_80358070) / 4, 4);
    }
#endif
    D_80358070 = (u8 *) dst;
    for (i = 0; i < 9; i++) {
        *(s32 *) (rec + 0x30 + i * 4) = *(s32 *) (rec + 0xC + i * 4) + delta;
    }
    if (((s32) D_80364AA8) != 1 || a1Val != 0) {
        func_802A1558(*(Gfx * N64P *) (rec + 0xC), *(Gfx * N64P *) (rec + 0x18), (Gfx * N64P *) (rec + 0x58));
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1388.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

/* Register-preserving wrapper: func_80278BF0(src, end, dstp).
 * asm: src in t2, end in t3, dstp in t4; it saves and restores every
 * register (including v0 and s0-s7) around the call. */
void func_802A1558(Gfx *src, Gfx *end, Gfx * N64P *dstp) {
    func_80278BF0(src, end, dstp);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1558.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_803BE6F4; /* func_802A1674's second argument (a status stream pointer or 0) */
/* u64 game-mode constants in .hd_code_data (the asm loads them by raw
 * address with ld). */
#define D_8030D880 (*(u64 *) 0x8030D880)
#define D_8030D888 (*(u64 *) 0x8030D888)

void func_802A2D68(u8 *obj);
u8 *func_802A3008(u8 *obj);
void func_802A1C20(u8 *obj, u8 *param, s32 *s1io);
void func_802A1C88(u8 *obj);
u8 *func_802A3D54(u8 *obj, s32 id, s32 b4F, s32 *s1io);
u8 *func_802A3DF8(u8 *obj, s32 id, s32 b4F, s32 *s1io);
void func_802A3E9C(u8 *obj, s32 id, s32 b4F, s32 *s1io);
u8 *func_802A3F80(u8 *obj, s32 *s1io);
void func_802A4464(u8 *obj);
/* An Mtx's s16 halves (integer parts here): PORT_HALF swaps the two halves
 * of each word on the little-endian host, where the port keeps Mtx words
 * in host order (game/port.h). */
#define MTX_HALF(off) (PORT_HALF((off) / 2) * 2)

void func_802A1A9C(u8 *obj, s32 *s1io);
void func_802A1934(void);
void func_802A2C54(u8 *obj);
u8 *func_802A1D54(u8 *obj, u8 *param, s32 *s1io);
s32 func_802A350C(u8 *obj, s32 fp);
void func_802A303C(u8 *obj, s32 fp);
void func_802A30DC(void);
void func_802A3134(u8 *obj);
void func_802A3198(Out802A32CC *out);
void func_802A19F4(void);

/* What the asm's v0 and t9 hold on entry from the only caller, 00000.c's
 * func_80256A34 (initlevel): its last call before func_802A1674 that sets them
 * is func_8026F92C(D_80364A98) (the debug print after it is an empty stub),
 * which returns the index of the lowest set bit of the game mode (-1 for 0)
 * and leaves t9 = that index, or the mode's low word when bit 0 is set (the
 * loop doesn't run; 0 for mode 0). The 64-bit mask is doubled by adding so
 * this stays inline (no __ll_lshift call). */
static s32 port_initlevel_v0(s32 *t9) {
    u64 mode = D_80364A98;
    u64 bit = 1;
    s32 n = 0;

    if (mode == 0) {
        *t9 = 0;
        return -1;
    }
    if (mode & 1) {
        *t9 = (u32) mode;
        return 0;
    }
    do {
        bit += bit;
        n++;
    } while (!(mode & bit));
    *t9 = n;
    return n;
}

/* Level setup from the level object obj (heap data; offsets as OBJ_PTR):
 * D_803BE6F4 = arg1, then the level header (func_802A2D68), the texture
 * table (func_802A0700), display lists and textures (func_802A3008,
 * func_802A1C20, func_802A1C88), the animation channels (func_8029DEA0), the
 * collision triangles (func_802A3D54, func_802A3DF8, func_802A3E9C,
 * func_802A3F80), grids and tables (func_802A4464 .. func_802BC840), the
 * placed objects (func_802A1D54), func_802C049C, func_8028FDA0(obj words
 * 0x3C/0x40). Unless the game mode D_80364A98 is 0x80 the level's vehicles
 * and objects are spawned (func_802A350C). D_80364AC1 = D_803643DB =
 * D_803643DC = 0; then mode 2: the type-0xFF object (func_802A303C) unless
 * D_802E8BEC; other modes: unless the mode is 0x80 or arg1 is set, the
 * type-0xFF object and the 0xFD vehicle (func_802A30DC), then always the
 * 0xFE vehicle (func_802A3134). Then func_802A3198, func_802CEAA0,
 * func_802A19F4, D_803BE6FC / D_803BE700 = OBJ_PTR(obj, 0x58 / 0x5C),
 * func_8026FBB0 / func_8028D4C0 / func_8028C190 on the obj ranges
 * 0x34-0x38, 0x38-0x3C, 0x20-0x24, D_80370C50 = 0, and when the mode is
 * neither 2 nor D_8030D880 (0x100000000000) and arg1 is set,
 * func_802C4BF0(arg1) (arg1 is the saved-status stream it applies).
 * Registers the asm hands on (it saves every callee-saved register itself):
 *  - s1: func_802A3008's result, threaded through func_802A1C20 and the
 *    triangle builders (s1).
 *  - fp: func_802A1C20's param is the caller's fp (func_80256A34 and the
 *    IDO code above it never set $s8; its value comes from whatever hand asm
 *    last left it): NULL here (it only reaches D_803C4B58.param, read by the
 *    decoder for packed types 4/5). From func_802A3F80 on, fp is its result
 *    (the D_803B9890 record end), handed to func_802A1D54 and on to the
 *    spawns: func_802A350C returns the fp the next object would get, and
 *    that goes to func_802A303C.
 *  - v0 / t9 (triangle id and byte 0x4F of the D_803BDCA8 / D_803BDE40 /
 *    group records): the caller's, never changed before those calls (every
 *    callee in between preserves them); see port_initlevel_v0.
 *  - s2-s4 into func_802A3198 (in/out; its outputs are dead here): 0.
 *  - s1 also reaches func_8028FDA0's callee func_802CE9C8 (byte 0x58 of each
 *    record's first triangle; func_802BF264 tests it) after func_802A1A9C
 *    and func_802A1D54 change it: the NM build passes it to func_8028FDA0 as
 *    an extra argument.
 */
void func_802A1674(u8 *obj, s32 arg1) {
    Out802A32CC o;
    s32 s1;
    s32 id;
    s32 b4F;
    u8 *fp;

    D_803BE6F4 = arg1;
    func_802A2D68(obj);
    func_802A0700();
    s1 = (s32) func_802A3008(obj);
    func_802A1C20(obj, NULL, &s1);
    func_802A1C88(obj);
    func_8029DEA0();
    id = port_initlevel_v0(&b4F);
    func_802A3D54(obj, id, b4F, &s1);
    func_802A3DF8(obj, id, b4F, &s1);
    func_802A3E9C(obj, id, b4F, &s1);
    fp = func_802A3F80(obj, &s1);
    func_802A4464(obj);
    func_802A1A9C(obj, &s1);
    func_802A1934();
    func_8029DC80();
    func_802A4510();
    func_802A2C54(obj);
    func_802A5F30();
    func_802BC840();
    func_802A1D54(obj, fp, &s1);
    func_802C049C();
    func_8028FDA0(OBJ_PTR(obj, 0x3C), OBJ_PTR(obj, 0x40), s1);
    if (D_80364A98 != 0x80) {
        fp = (u8 *) N64_IPTR(func_802A350C(obj, (s32) fp));
    }
    D_80364AC1 = 0;
    D_803643DB = 0;
    D_803643DC = 0;
    if (D_80364A98 == 2) {
        if (D_802E8BEC == 0) {
            func_802A303C(obj, (s32) fp);
        }
    } else {
        if (D_80364A98 != 0x80 && D_803BE6F4 == 0) {
            func_802A303C(obj, (s32) fp);
            func_802A30DC();
        }
        func_802A3134(obj);
    }
    o.s2 = NULL;
    o.s3 = NULL;
    o.s4 = NULL;
    func_802A3198(&o);
    func_802CEAA0(obj);
    func_802A19F4();
    D_803BE6FC = OBJ_PTR(obj, 0x58);
    D_803BE700 = OBJ_PTR(obj, 0x5C);
    func_8026FBB0((s16 *) (OBJ_PTR(obj, 0x34)), (s16 *) (OBJ_PTR(obj, 0x38)));
    func_8028D4C0(OBJ_PTR(obj, 0x38), OBJ_PTR(obj, 0x3C));
    func_8028C190((struct BoxSpawn *) (OBJ_PTR(obj, 0x20)), (struct BoxSpawn *) (OBJ_PTR(obj, 0x24)));
    D_80370C50 = 0;
    if (D_80364A98 != 2 && D_80364A98 != D_8030D880 && D_803BE6F4 != 0) {
        func_802C4BF0((void *) N64_IPTR(D_803BE6F4));
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1674.s")
#endif

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
#ifdef NON_MATCHING

/* If D_8039CAB7 is set: load packed object 0x98 (func_802A396C), D_8039CAC0 =
 * OBJ_PTR(data, 0x14) and move each 16-byte vertex record from there up to
 * OBJ_PTR(data, 0x18) by (D_8039CAB0, D_8039CAB2, D_8039CAB4) (s16 x, y, z at
 * +0/+2/+4; the asm loops with `!=` and its adds trap); D_8039CABC =
 * OBJ_PTR(data, 0x24).
 * The asm saves t0 (asm caller func_802A1674 keeps it) and leaves
 * func_802A396C's s2/s4 (and s5-s7) changed and f12/f14 as the libultra
 * calls leave them (the survey lists those as read by func_802A1674; not
 * modelled). */
void func_802A19F4(void) {
    Out802A396C o;
    u8 *p;
    u8 *end;

    if (D_8039CAB7 != 0) {
        func_802A396C(0x98, &o);
        p = OBJ_PTR(o.s2, 0x14);
        D_8039CAC0 = p;
        end = OBJ_PTR(o.s2, 0x18);
        while (p != end) {
            *(s16 *) (p + 0) += D_8039CAB0;
            *(s16 *) (p + 2) += D_8039CAB2;
            *(s16 *) (p + 4) += D_8039CAB4;
            p += 0x10;
        }
        D_8039CABC = OBJ_PTR(o.s2, 0x24);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A19F4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

/* Allocates from the heap D_80358070: 2n bytes of identity matrices (0x40-
 * byte fixed-point Mtx; D_803F7820 = start, D_803F7824 = start + n), then
 * unpacks the item blocks after the count word n at OBJ_PTR(obj, 0x74) into
 * 0x28-byte records (nine s16 << 5 as words, then a byte at 0x24). Each block
 * is a 0x2C-byte header (s16 tag at 0, -1 ending the list; s32 item count at
 * 0x28) followed by that many 0x44-byte items. D_803F7828 = record start, D_803F782C = D_80358070 =
 * end. The asm loops with `!=`, so n must be a multiple of 0x20.
 * asm: obj in t0; it leaves scratch values in s0, s2, s3 (no caller reads
 * them) and counts the item loops down in s1, so s1 comes back 0 when n != 0
 * (unchanged otherwise). func_802A1674 hands that s1 on to func_8028FDA0's
 * func_802CE9C8, hence `s1io`.
 * Asm callers rely on preserved: func_802A1674 keeps t0, f12, f14. */
void func_802A1A9C(u8 *obj, s32 *s1io) {
    u8 *hdr = OBJ_PTR(obj, 0x74);
    s32 n = *(s32 *) hdr;
    u8 *m = D_80358070;
    u8 *mend;

    D_803F7820 = m;
    D_803F7824 = m + n;
    mend = m + n + n;
    while (m != mend) {
        *(s16 *) (m + MTX_HALF(0x00)) = 1;
        *(s16 *) (m + MTX_HALF(0x02)) = 0;
        *(s32 *) (m + 0x04) = 0;
        *(s16 *) (m + MTX_HALF(0x08)) = 0;
        *(s16 *) (m + MTX_HALF(0x0A)) = 1;
        *(s32 *) (m + 0x0C) = 0;
        *(s32 *) (m + 0x10) = 0;
        *(s16 *) (m + MTX_HALF(0x14)) = 1;
        *(s16 *) (m + MTX_HALF(0x16)) = 0;
        *(s32 *) (m + 0x18) = 0;
        *(s16 *) (m + MTX_HALF(0x1C)) = 0;
        *(s16 *) (m + MTX_HALF(0x1E)) = 1;
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
#ifdef PORT_HOST
                /* bytes 0x25-0x27 are never written: keep the N64's stale
                 * bytes (port_n64_byte, see PORT_STALE_51 below) */
                void port_unit_mark(void *p, u32 n, s32 width);
                u8 port_n64_byte(const void *p);
                void port_garbage(const void *p, u32 len);
                u8 old[3];
                s32 k;

                for (k = 0; k < 3; k++) {
                    old[k] = port_n64_byte(m + 0x25 + k);
                }
#endif
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
#ifdef PORT_HOST
                for (k = 0; k < 3; k++) {
                    m[0x25 + k] = old[k];
                }
                port_unit_mark(m, 9, 4);
                port_unit_mark(m + 0x24, 4, 1);
                port_garbage(m + 0x25, 3);
#endif
                src += 0x44;
                m += 0x28;
            }
        } while (tag != -1);
        *s1io = 0;
    }
    D_803F782C = m;
    D_80358070 = m;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1A9C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

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
#ifdef NON_MATCHING
void func_802A1EC8(void);
void func_802A2BB0(void);
u8 *func_802A2A98(s32 index);
void func_802A2164(s32 kind, u8 *obj);
void func_802A26A8(u8 *obj, s32 dx, s32 dy, s32 dz);
void func_802A20F4(u8 *obj, s32 flag);
void func_802A21AC(s32 type, u8 *obj, s32 x, s32 y, s32 z, s32 flag, s32 tag, u8 *param, s32 *s1io);

/* Level objects setup: D_8036EB93 = 0, func_802A1EC8 (per-level list), clear
 * a few flag bytes of the tables at D_803EFED0 (1 x 0xA30: bytes 0xA2A/B),
 * D_803F0900 (4 x 0x4B8: half 0x4B0, byte 0x4B2) and D_803F1BE0 (2 x 0x478:
 * bytes 0x470/0x472), D_803F7654 = D_803F4030 (record pool), func_802A2BB0
 * (offset table). Then for each 14-byte placement in [OBJ_PTR(obj, 0x5C),
 * OBJ_PTR(obj, 0x60)) {BE u16 x, y, z, type; u8 flag, count; u16 at 0xA,
 * 0xC}: data = func_802A2A98(type) (load), func_802A2164(type, data),
 * func_802A08E4 over data's display list [OBJ_PTR(data, 0x10),
 * OBJ_PTR(data, 0x14)), data's u16 at 0xE = the u16 at 0xA,
 * func_802A26A8(data, x, y, z), func_802A20F4(data, flag), data[6] = count
 * and D_8036EB93 += count, then func_802A21AC(type, data, x, y, z, flag, the
 * u16 at 0xC, param). The asm loops with `!=`. Returns the list end (the
 * asm's t1 and t2).
 * Register convention: obj t0, param fp; result t1. The asm leaves s0, s2-s7
 * (and the libultra calls' f12/f14, func_802A21AC's f20-f28) changed; the
 * survey lists those as read by func_802A1674 (dead there, not modelled).
 * s1 is modelled (`s1io`, unchanged without placements): per placement it is
 * the new D_8036EB93 sum (the register: old byte + count, not truncated),
 * then whatever func_802A21AC leaves; func_802A1674 hands it on to
 * func_8028FDA0's func_802CE9C8.
 * Asm caller func_802A1674 keeps t0 live. */
u8 *func_802A1D54(u8 *obj, u8 *param, s32 *s1io) {
    Unk802A08E4Regs r;
    u8 *p;
    u8 *end;
    u8 *data;
    s32 type;
    s32 x;
    s32 y;
    s32 z;
    s32 n;

    D_8036EB93 = 0;
    func_802A1EC8();
    for (p = D_803EFED0, n = 1; n != 0; n--, p += 0xA30) {
        p[0xA2A] = 0;
        p[0xA2B] = 0;
    }
    for (p = D_803F0900, n = 4; n != 0; n--, p += 0x4B8) {
        *(s16 *) (p + 0x4B0) = 0;
        p[0x4B2] = 0;
    }
    for (p = D_803F1BE0, n = 2; n != 0; n--, p += 0x478) {
        p[0x470] = 0;
        p[0x472] = 0;
    }
    D_803F7654 = D_803F4030;
    func_802A2BB0();
    p = OBJ_PTR(obj, 0x5C);
    end = OBJ_PTR(obj, 0x60);
    while (p != end) {
        type = BE16U(p + 6);
        data = func_802A2A98(type);
        func_802A2164(type, data);
        func_802A08E4((u32 *) OBJ_PTR(data, 0x10), (u32 *) OBJ_PTR(data, 0x14), &r);
        *(u16 *) (data + 0xE) = *(u16 *) (p + 0xA);
        x = BE16U(p + 0);
        y = BE16U(p + 2);
        z = BE16U(p + 4);
        func_802A26A8(data, x, y, z);
        func_802A20F4(data, p[8]);
        data[6] = p[9];
        *s1io = D_8036EB93 + p[9];
        D_8036EB93 = *s1io;
        func_802A21AC(type, data, x, y, z, p[8], *(u16 *) (p + 0xC), param, s1io);
        p += 0xE;
    }
    return p;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1D54.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 * N64P D_803BE704;
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
#ifdef NON_MATCHING
void func_802A2608(u8 *obj, u8 *param, s32 *s1io);
void func_802A2458(u8 *rec, u8 *obj);
void func_802A24BC(u8 *rec, u8 *param);
void func_802A23E0(u8 *rec);

#define REC_W(rec, off) (*(s32 *) ((rec) + (off)))

/* Set up a placed level object of `type` from its loaded data obj at (x, y, z)
 * (map units, << 5 into the record): func_802A2608(obj, param) resolves its
 * ids; type 0x38 also stores x/y/z in D_803F767C/7E/80. Takes the next
 * 0xFC-byte record from D_803F7654: func_802A2458 (centre), word 0 = obj, 0x30
 * = type, 0x34 = tag, 0xC = obj's u16 at 2 << 5, 0x10/0x1C = x, 0x14/0x20 =
 * y, 0x18/0x24 = z (<< 5), 0x38..0x40 = 0, bytes 0xEA = 0, 0xEB = flag, 0xE9 =
 * n (obj's u16 at 0) and n zero bytes at 0xEC, n zero halves at 0x48 and at
 * 0x68. With the per-level list D_803BE704 set and flag != 0 the record is
 * appended to it (stride 0x18). Byte 0xE8 = grid cell (z / (D_803BE710 >>
 * 5)) * D_803BE714 + x / (D_803BE70C >> 5) (unsigned). Then func_802A24BC(rec,
 * param), func_802A23E0(rec), and the 0x19-byte triangle entries
 * [OBJ_PTR(obj, 0x48), OBJ_PTR(obj, 0x4C)) are built at the heap D_80358070
 * with func_802A41B0 (byte 0x51 of the current output = entry byte 0x17 == 0;
 * the asm passes &D_803F7654, left in v0, as the id); word 4 / 8 = the heap
 * before / after, D_80358070 = the end. The asm's addi trap; the divides trap
 * on a zero cell size.
 * Register convention: type t3, obj t4, x t5, y t6, z t7, flag s0, tag t9,
 * param fp (conventions.txt). The asm restores t0-t2, sets s0 = 1, leaves
 * gp (the last entry's byte 0x18), func_802A41B0's s2-s7 and func_802A24BC's
 * f20-f28 changed (listed as read by func_802A1D54; dead there). s1 is
 * modelled (`s1io`): func_802A2608's, then per entry its byte 0x16 into
 * func_802A41B0's chain; the last value reaches func_802CE9C8 through
 * func_802A1D54 / func_802A1674 / func_8028FDA0. */
void func_802A21AC(s32 type, u8 *obj, s32 x, s32 y, s32 z, s32 flag, s32 tag, u8 *param, s32 *s1io) {
    u8 *rec;
    u8 *list;
    s32 n;
    s32 i;
    u8 *e;
    u8 *eEnd;
    u8 *out;

    func_802A2608(obj, param, s1io);
    if (type == 0x38) {
        D_803F767C = x;
        D_803F767E = y;
        D_803F7680 = z;
    }
    rec = D_803F7654;
    D_803F7654 = rec + 0xFC;
    func_802A2458(rec, obj);
    REC_W(rec, 0x34) = tag;
    REC_W(rec, 0x38) = 0;
    REC_W(rec, 0x3C) = 0;
    REC_W(rec, 0x40) = 0;
    REC_W(rec, 0x30) = type;
    rec[0xEB] = flag;
    rec[0xEA] = 0;
    *(u8 * N64P *) rec = obj;
    REC_W(rec, 0xC) = *(u16 *) (obj + 2) << 5;
    REC_W(rec, 0x10) = x << 5;
    REC_W(rec, 0x1C) = x << 5;
    REC_W(rec, 0x14) = y << 5;
    REC_W(rec, 0x20) = y << 5;
    REC_W(rec, 0x18) = z << 5;
    REC_W(rec, 0x24) = z << 5;
    n = *(u16 *) obj;
    rec[0xE9] = n;
    for (i = 0; i < n; i++) {
        rec[0xEC + i] = 0;
    }
    for (i = 0; i < n; i++) {
        *(s16 *) (rec + 0x48 + i * 2) = 0;
    }
    for (i = 0; i < n; i++) {
        *(s16 *) (rec + 0x68 + i * 2) = 0;
    }
    list = D_803BE704;
    if (list != NULL && flag != 0) {
        *(u8 * N64P *) list = rec;
        D_803BE704 = list + 0x18;
    }
    rec[0xE8] = ((u32) z / ((u32) D_803BE710 >> 5)) * ((u16) D_803BE714) + (u32) x / ((u32) D_803BE70C >> 5);
    func_802A24BC(rec, param);
    func_802A23E0(rec);
    e = OBJ_PTR(obj, 0x48);
    eEnd = OBJ_PTR(obj, 0x4C);
    out = D_80358070;
    REC_W(rec, 4) = (s32) out;
    while (e != eEnd) {
        out[0x51] = (e[0x17] != 0) ? 0 : 1;
        *s1io = e[0x16];
        out = func_802A41B0(out, e, (s32) &D_803F7654, e[0x15], e[0x17], e[0x14], s1io, tag, e[0x18]);
        e += 0x19;
    }
    REC_W(rec, 8) = (s32) out;
    D_80358070 = out;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A21AC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_8030631F[];

/* rec[0] points at an object; remaps byte 0x31 of every 0x38-byte entry in
 * [OBJ_PTR(obj, 0x30), OBJ_PTR(obj, 0x38)) through the byte table D_8030631F
 * (the asm loops with `!=`).
 * asm: rec in v1; it changes no register.
 * Asm callers rely on preserved: func_802A21AC keeps a2, a3, t4, t9, f12, f14. */
void func_802A23E0(u8 *rec) {
    u8 *obj = *(u8 * N64P *) rec;
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
#ifdef NON_MATCHING

/* For a record whose type (word 0x30) is 0xBA, 0xBB or 0xBC: D_80365330 =
 * func_802A0CFC(0xF81, param) and word 0x44 = the ground height
 * func_802CE6F8(word 0x10, word 0x18, word 0x14).
 * Register convention: rec in v1, param in fp (conventions.txt); the asm
 * saves and restores every integer register (f20-f28 are left as
 * func_802CE6F8 leaves them). Asm callers rely on preserved: func_802A21AC
 * keeps t4, t9 (and is listed as reading f12, f14, f20-f26 after it). */
void func_802A24BC(u8 *rec, u8 *param) {
    s32 type = *(s32 *) (rec + 0x30);

    if (type == 0xBA || type == 0xBB || type == 0xBC) {
        D_80365330 = func_802A0CFC(0xF81, param);
        *(s32 *) (rec + 0x44) = func_802CE6F8(*(s32 *) (rec + 0x10), *(s32 *) (rec + 0x18), *(s32 *) (rec + 0x14));
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A24BC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

/* Like func_802A1C20 for the records [OBJ_PTR(obj, 0x28), OBJ_PTR(obj, 0x2C)):
 * each record's word 0 is replaced by func_802A0CFC(word, param), then (the
 * count byte at +4 read after that call) count - 1 words from +0x10 likewise;
 * the next record follows the last word. Same preconditions (exact end,
 * count >= 1).
 * Register convention: obj in t4, param in fp (conventions.txt); the asm saves
 * t3, t5-t7, s0, t9, leaves t1/t2 changed and s1 = 0x80000000 when there is
 * a record (`s1io`; it can reach func_802CE9C8 through func_802A21AC and
 * func_802A1D54); its addi trap. Asm caller func_802A21AC keeps t3-t7, t9
 * live. */
void func_802A2608(u8 *obj, u8 *param, s32 *s1io) {
    u8 *rec = OBJ_PTR(obj, 0x28);
    u8 *end = OBJ_PTR(obj, 0x2C);
    u32 *w;
    s32 n;

    while (rec != end) {
        *(u32 *) rec = func_802A0CFC(*(u32 *) rec, param);
        *s1io = 0x80000000;
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
#ifdef NON_MATCHING
extern u8 * N64P D_803BE6F0; /* table of u32 offsets into the compressed block at ROM 0x6EC4C0 */
u32 func_802A44E4(u32 x);

/* The load shared by func_802A2A98 / func_802A32CC / func_802A396C: DMAs
 * `size` bytes from ROM `rom` into the buffer 0x8021ED00 (cache invalidated
 * first, waiting for completion), decompresses two streams from it to the
 * heap D_80358070 (func_802C4108 twice, window 0x8004B400), rounds the heap
 * end up (func_802A44E4, also left in *end: the asm's a1), and returns the
 * old heap pointer (the start of the data) after moving D_80358070 past it. */
static u8 *port_load_packed(u32 rom, u32 size, u8 **end) {
    u8 *src = (u8 *) 0x8021ED00;
    u8 *dst = D_80358070;
    u8 *old;

    osInvalDCache(src, size);
    osPiStartDma(&D_80370C58, OS_MESG_PRI_NORMAL, OS_READ, rom, src, size, &D_803150A0);
    osRecvMesg(&D_803150A0, NULL, OS_MESG_BLOCK);
    func_802C4108(&src, &dst, 0x8004B400);
    func_802C4108(&src, &dst, 0x8004B400);
#ifdef PORT_HOST
    {
        /* Windows port: byte order on load (port/src/load/port_load.h) */
        void port_on_load(u32 rom, u32 dst, u32 len);

        port_on_load(rom, (u32) D_80358070, (u32) (dst - D_80358070));
    }
#endif
    dst = (u8 *) func_802A44E4((u32) dst);
    old = D_80358070;
    D_80358070 = dst;
    *end = dst;
    return old;
}

/* Loads packed object `index`: port_load_packed of ROM 0x6EC4C0 + tbl[index]
 * .. + tbl[index + 1] (tbl = D_803BE6F0).
 * Register convention: index in t3, result in s0 (conventions.txt); the asm
 * saves t0-t6 and s1, leaves s2 (size) and s7 (buffer) changed and f12 /
 * f14 as the libultra calls leave them. Asm callers rely on preserved:
 * func_802A1D54 keeps t1, t2, t3. */
u8 *func_802A2A98(s32 index) {
    u32 *t = (u32 *) (D_803BE6F0 + index * 4);
    u8 *end;

    return port_load_packed(0x6EC4C0 + t[0], t[1] - t[0], &end);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A2A98.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 * N64P D_803BE6F0;

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
extern u16 D_803BE72E;

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
/* (Unk802A08E4Regs and func_802A08E4 are declared at the top of the file.) */

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
#ifdef NON_MATCHING

/* The level's type-0xFF object: the 9-byte record at OBJ_PTR(obj, 0x54) (just
 * past func_802A350C's list) {u8 speed, BE s16 x, z, heading, limit}; when
 * its first byte is set, load model 0xFF (func_802A396C) and set it up with
 * func_802B9C50(model, x << 5, z << 5, speed, limit << 5, heading, fp), then
 * D_803643DB = 1.
 * Register convention: obj in t0, fp (the previous object's leftover: here
 * what func_802A350C returns) (conventions.txt); the asm saves t0 and leaves
 * s1, s2, s4 and the setup's s0-s7, fp, gp and FP registers changed. Its add
 * traps (a heap address: in range). */
void func_802A303C(u8 *obj, s32 fp) {
    u8 *p = OBJ_PTR(obj, 0x54);
    Out802A396C o;
    s32 speed = p[0];

    if (speed != 0) {
        func_802A396C(0xFF, &o);
        func_802B9C50(o.s2, BE16S(p + 1) << 5, BE16S(p + 3) << 5, speed, BE16S(p + 7) << 5, BE16S(p + 5), fp);
        D_803643DB = 1;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A303C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

/* If func_80268EE8(D_802E8BDC) (the current level has it): D_80364AC1 = 1,
 * load model 0xFD (func_802A396C) and set up vehicle 0xFD with it
 * (func_802D2570). The asm saves t0 (not read) and leaves func_802A396C's s2,
 * s4 and the setup's registers changed (conventions.txt: clobbers). */
void func_802A30DC(void) {
    Out802A396C o;

    if (func_80268EE8(D_802E8BDC) != 0) {
        D_80364AC1 = 1;
        func_802A396C(0xFD, &o);
        func_802D2570(o.s2);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A30DC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

/* When the record at OBJ_PTR(obj, 0x54) (func_802A303C's) has a nonzero first
 * byte and the game mode D_80364AA8 isn't 0x80: load model 0xFE
 * (func_802A396C) and set up the flying vehicle with func_802B8480(heap end,
 * model) - its a1 input is the heap end func_802A396C leaves in a1 - then
 * D_803643DC = 1.
 * Register convention: obj in t0 (conventions.txt); the asm saves t0 and
 * leaves s2, s4 and the setup's registers changed. Its add traps. */
void func_802A3134(u8 *obj) {
    Out802A396C o;

    if (*OBJ_PTR(obj, 0x54) != 0 && ((s32) D_80364AA8) != 0x80) {
        func_802A396C(0xFE, &o);
        func_802B8480((s32) o.a1, o.s2);
        D_803643DC = 1;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A3134.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 * N64P D_803BDAFC;

/* If D_8039CA61 is set: load packed object D_8039CA7E (func_802A32CC; its s2,
 * s3, s4 results go to *out); D_803BDAFC = data, D_803BDB04 =
 * OBJ_PTR(data, 0x14), D_803BDB08 = OBJ_PTR(data, 0x24), D_803BDB00 = the heap
 * D_80358070. The block at OBJ_PTR(data, 0x18) is {s32 size, s32 bytes,
 * words[]}: its `bytes` bytes of words are copied to the heap, and the rest of
 * `size` is filled with 0x40-byte identity matrices (the asm loops with `!=`:
 * size - bytes must be a multiple of 0x40); D_80358070 = the end. Then
 * func_80295AE0(OBJ_PTR(data, 0x24), OBJ_PTR(data, 0x28)). The asm's adds
 * trap (heap addresses: in range).
 * Register convention: s2, s3, s4 out through `out` when loaded (in/out:
 * unchanged otherwise); the asm sets s0 = 1 then (clobbers s0) and saves t0
 * (asm caller func_802A1674 keeps it). f12/f14 as the callees leave them are
 * listed as read by func_802A1674 (not modelled). */
void func_802A3198(Out802A32CC *out) {
    u8 *data;
    s32 *src;
    s32 *srcEnd;
    s32 *dst;
    s32 size;

    if (D_8039CA61 == 0) {
        return;
    }
    func_802A32CC(D_8039CA7E, out);
    data = out->s2;
    D_803BDAFC = data;
    D_803BDB04 = OBJ_PTR(data, 0x14);
    D_803BDB08 = OBJ_PTR(data, 0x24);
    dst = (s32 *) D_80358070;
    D_803BDB00 = (u8 *) dst;
    src = (s32 *) OBJ_PTR(data, 0x18);
    size = src[0];
    srcEnd = (s32 *) ((u8 *) (src + 2) + src[1]);
    src += 2;
    while (src != srcEnd) {
        *dst++ = *src++;
        size -= 4;
    }
    while (size != 0) {
        u8 *m = (u8 *) dst;

        *(s16 *) (m + MTX_HALF(0x00)) = 1;
        *(s16 *) (m + MTX_HALF(0x02)) = 0;
        *(s32 *) (m + 0x04) = 0;
        *(s16 *) (m + MTX_HALF(0x08)) = 0;
        *(s16 *) (m + MTX_HALF(0x0A)) = 1;
        *(s32 *) (m + 0x0C) = 0;
        *(s32 *) (m + 0x10) = 0;
        *(s16 *) (m + MTX_HALF(0x14)) = 1;
        *(s16 *) (m + MTX_HALF(0x16)) = 0;
        *(s32 *) (m + 0x18) = 0;
        *(s16 *) (m + MTX_HALF(0x1C)) = 0;
        *(s16 *) (m + MTX_HALF(0x1E)) = 1;
        *(s32 *) (m + 0x20) = 0;
        *(s32 *) (m + 0x24) = 0;
        *(s32 *) (m + 0x28) = 0;
        *(s32 *) (m + 0x2C) = 0;
        *(s32 *) (m + 0x30) = 0;
        *(s32 *) (m + 0x34) = 0;
        *(s32 *) (m + 0x38) = 0;
        *(s32 *) (m + 0x3C) = 0;
        dst += 0x10;
        size -= 0x40;
    }
    D_80358070 = (u8 *) dst;
    func_80295AE0((Gfx *) OBJ_PTR(data, 0x24), (Gfx *) OBJ_PTR(data, 0x28));
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A3198.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* (Out802A32CC, the register results its asm caller reads, is declared at
 * the top of the file.) */

/* Loads the packed block of `type` (ROM ranges below; any other type hits
 * the asm's `syscall` debug trap and then runs as type 3, kept) with
 * port_load_packed and resolves the G_SETTIMG ids of its display list
 * [data + *(data + 0x1C), data + *(data + 0x20)) with func_802A08E4 (in/out
 * s4 = the ROM start). out: the data, func_802A08E4's s3 / s4.
 * Register convention: type in t3; s2, s3, s4 through `out`
 * (conventions.txt). The asm saves t0-t7, s0, s1 and leaves s5-s7 changed
 * (and f12 / f14 as the callees leave them). */
void func_802A32CC(s32 type, Out802A32CC *out) {
    u32 rom;
    u32 romEnd;
    u8 *end;
    u8 *data;
    Unk802A08E4Regs r;

    switch (type) {
        default: /* syscall */
        case 3:
            rom = 0x490AC0, romEnd = 0x491E00;
            break;
        case 4:
            rom = 0x496AD0, romEnd = 0x497AF0;
            break;
        case 5:
            rom = 0x497AF0, romEnd = 0x4989E0;
            break;
        case 8:
            rom = 0x49BCE0, romEnd = 0x49C480;
            break;
        case 9:
            rom = 0x49C480, romEnd = 0x49E8E0;
            break;
        case 0xA:
            rom = 0x49E8E0, romEnd = 0x49F7A0;
            break;
        case 0xD:
            rom = 0x49FF70, romEnd = 0x4A0720;
            break;
        case 0xE:
            rom = 0x4A0720, romEnd = 0x4A1000;
            break;
        case 0xF:
            rom = 0x4A1000, romEnd = 0x4A1690;
            break;
    }
    data = port_load_packed(rom, romEnd - rom, &end);
    r.s4 = (u8 *) rom;
    func_802A08E4((u32 *) OBJ_PTR(data, 0x1C), (u32 *) OBJ_PTR(data, 0x20), &r);
    out->s2 = data;
    out->s3 = r.s3;
    out->s4 = r.s4;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A32CC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* The FP registers the spawns hand to func_802A992C (f12, f14, f20-f26; FP
 * side results only), as their TriSideOut. */
typedef struct {
    f32 f12;
    f32 f14;
    f32 f20;
    f32 f22;
    f32 f24;
    f32 f26;
} SpawnFP;

void func_802A3824(u8 *obj);

/* Spawn the level's vehicles and objects: func_802A3824(obj) (picks
 * D_803BE73A), D_803ED3F5 = D_803ED40F = 0, then for each 9-byte record in
 * [OBJ_PTR(obj, 0x50), OBJ_PTR(obj, 0x54)) {u8 type, BE s16 x, y, z,
 * heading}: in game mode D_80364A98 == 2 (and == the u64 D_8030D888) only
 * records of type D_8036698C are spawned; type 1 becomes the player's
 * vehicle D_803643D4 unless D_80364AA8 is 1 or 0x80; types 6, 7, 0xB, 0x11,
 * 0x12 set D_803ED40F. The model is loaded (func_802A396C(type)) and the
 * type's setup runs with (x, y, z) << 5 and the heading (0 func_802AE370, 1
 * func_802AFC60, 2 func_802B0DA0, 3 func_802B29C0, 4 func_802B4100, 5
 * func_802B5900, 6 func_802BAD80, 7 func_802BBA60, 8 func_802B7340, 9
 * func_802C5120, 0xA func_802C9B90, 0xB/0x11/0x12 func_802C80D0(type, ..),
 * 0xD func_802CB720, 0xE func_802CC920, 0xF func_802CF6A0, 0x10
 * func_802D07E0). Any other type hits the asm's `syscall` debug trap, after
 * which it returns (the rest of the list is not spawned): return here.
 * The asm's adds trap (heap addresses: in range).
 * Leftover registers (the setups take them as by-value inputs):
 *  - fp: the ground byte func_802A992C stores into D_803ED3F2[] (and so
 *    veh+0x50) for a wheel slot whose scan finds no fp of its own. The first
 *    object gets the dispatcher's fp: func_802A1674's, i.e. func_802A3F80's
 *    result (the D_803B9890 collision record end; its low byte). In the asm
 *    each later object gets whatever the previous setup left in fp (most
 *    end with func_8029E558 -> func_8029E5AC, which leaves byte 0x15, the
 *    interpolation mode, of the last active animation channel there; others
 *    a func_802AE104 / scan result), and the dispatcher returns the last
 *    setup's (func_802A1674 hands it to func_802A303C). Traced in the
 *    original over the nine attract-demo levels (45 spawns, 16 setup types):
 *    every setup leaves 0 (linear channels), so here every object after the
 *    first gets 0, and 0 is returned once a setup has run (the C setups
 *    don't return their fp).
 *  - t6 (func_802B0DA0, func_802CF6A0, func_802D07E0: only passed through to
 *    their per-frame zone scans) and the FP state f12-f26 (only FP side
 *    results of func_802A992C; func_802CB720 / func_802CC920 write f22-f26
 *    back, chained here): 0.
 * Register convention: obj in t0, fp in/out (conventions.txt). The asm saves
 * t0; the setups save t0-t5 and leave s0-s7, gp and the FP registers
 * changed. Asm caller func_802A1674 keeps t0. */
s32 func_802A350C(u8 *obj, s32 fp) {
    SpawnFP f;
    Out802A396C o;
    u8 *p;
    u8 *end;
    s32 type;
    s32 x;
    s32 y;
    s32 z;
    s32 heading;

    f.f12 = 0.0f;
    f.f14 = 0.0f;
    f.f20 = 0.0f;
    f.f22 = 0.0f;
    f.f24 = 0.0f;
    f.f26 = 0.0f;
    func_802A3824(obj);
    D_803ED3F5 = 0;
    D_803ED40F = 0;
    p = OBJ_PTR(obj, 0x50);
    end = OBJ_PTR(obj, 0x54);
    while (p != end) {
        type = p[0];
        if (D_80364A98 == 2 && D_80364A98 == D_8030D888 && D_8036698C != type) {
            p += 9;
            continue;
        }
        if (type == 1 && ((s32) D_80364AA8) != 1 && ((s32) D_80364AA8) != 0x80) {
            type = D_803643D4;
        }
        if (type == 6 || type == 7 || type == 0xB || type == 0x11 || type == 0x12) {
            D_803ED40F = 1;
        }
        x = BE16S(p + 1) << 5;
        y = BE16S(p + 3) << 5;
        z = BE16S(p + 5) << 5;
        heading = BE16S(p + 7);
        p += 9;
        func_802A396C(type, &o);
        switch (type) {
            case 0:
                func_802AE370(o.s2, x, y, z, heading, fp, (TriSideOut *) &f);
                break;
            case 1:
                func_802AFC60(o.s2, x, y, z, heading, fp);
                break;
            case 2:
                func_802B0DA0(o.s2, x, y, z, heading, fp, 0, (TriSideOut *) &f);
                break;
            case 3:
                func_802B29C0(o.s2, x, y, z, heading, fp);
                break;
            case 4:
                func_802B4100(o.s2, x, y, z, heading, fp);
                break;
            case 5:
                func_802B5900(o.s2, x, y, z, heading, fp);
                break;
            case 6:
                func_802BAD80(o.s2, x, y, z, heading, fp);
                break;
            case 7:
                func_802BBA60(x, y, z, heading, o.s2, fp);
                break;
            case 8:
                func_802B7340(o.s2, x, y, z, heading, fp, (TriSideOut *) &f);
                break;
            case 9:
                func_802C5120(x, y, z, heading, o.s2, fp);
                break;
            case 0xA:
                func_802C9B90(x, y, z, heading, o.s2, fp);
                break;
            case 0xB:
            case 0x11:
            case 0x12:
                func_802C80D0(type, x, y, z, heading, o.s2, fp);
                break;
            case 0xD:
                func_802CB720(o.s2, x, y, z, heading, fp, (TriSideOut *) &f);
                break;
            case 0xE:
                func_802CC920(o.s2, x, y, z, heading, fp, (TriSideOut *) &f);
                break;
            case 0xF:
                func_802CF6A0(o.s2, x, y, z, heading, fp, 0, (TriSideOut *) &f);
                break;
            case 0x10:
                func_802D07E0(o.s2, x, y, z, heading, fp, 0, (TriSideOut *) &f);
                break;
            default: /* the asm's syscall, then its epilogue */
                return fp;
        }
        fp = 0; /* what the setup leaves in the asm's fp (see above) */
    }
    return fp;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A350C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

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
            if (((s32) D_80364AA8) != 1 && ((s32) D_80364AA8) != 0x80) {
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
#ifdef NON_MATCHING

/* (Out802A396C is declared at the top of the file.) */

/* As func_802A32CC for the object types below (any other type hits the
 * asm's `syscall` debug trap and then runs as type 0, kept), with
 * func_8029DF78(dl, dlEnd, type) on the display list before func_802A08E4.
 * out: the data, func_802A08E4's s4 and the new heap end.
 * Register convention: type in t3; s2, s4, a1 through `out`
 * (conventions.txt). The asm saves t0-t7, s0, s1, s3 and leaves s5-s7
 * changed (and f12 / f14 as the callees leave them). Asm callers rely on
 * preserved: func_802A303C keeps t1, t2, t4-t7; func_802A30DC /
 * func_802CEAA0 keep t1, t2; func_802A3134 keeps t1; func_802A350C keeps
 * t1-t3, t7. */
void func_802A396C(s32 type, Out802A396C *out) {
    u32 rom;
    u32 romEnd;
    u8 *end;
    u8 *data;
    Unk802A08E4Regs r;

    switch (type) {
        default: /* syscall */
        case 0:
            rom = 0x491E00, romEnd = 0x4929D0;
            break;
        case 1:
            rom = 0x4929D0, romEnd = 0x494390;
            break;
        case 2:
            rom = 0x494390, romEnd = 0x496AD0;
            break;
        case 0x10:
            rom = 0x4A1690, romEnd = 0x4A4120;
            break;
        case 3:
            rom = 0x490AC0, romEnd = 0x491E00;
            break;
        case 4:
            rom = 0x496AD0, romEnd = 0x497AF0;
            break;
        case 5:
            rom = 0x497AF0, romEnd = 0x4989E0;
            break;
        case 6:
            rom = 0x49AD20, romEnd = 0x49B630;
            break;
        case 7:
            rom = 0x49B630, romEnd = 0x49BCE0;
            break;
        case 8:
            rom = 0x49BCE0, romEnd = 0x49C480;
            break;
        case 9:
            rom = 0x49C480, romEnd = 0x49E8E0;
            break;
        case 0xA:
            rom = 0x49E8E0, romEnd = 0x49F7A0;
            break;
        case 0xB:
        case 0x11:
        case 0x12:
            rom = 0x49F7A0, romEnd = 0x49FF70;
            break;
        case 0xD:
            rom = 0x49FF70, romEnd = 0x4A0720;
            break;
        case 0xE:
            rom = 0x4A0720, romEnd = 0x4A1000;
            break;
        case 0xF:
            rom = 0x4A1000, romEnd = 0x4A1690;
            break;
        case 0xFE:
            rom = 0x4989E0, romEnd = 0x499690;
            break;
        case 0xFF:
            rom = 0x499690, romEnd = 0x49AD20;
            break;
        case 0xFD:
            rom = 0x4A4120, romEnd = 0x4A5660;
            break;
        case 0x96:
            rom = 0x4903C0, romEnd = 0x490AC0;
            break;
        case 0x98:
            rom = 0x48FE90, romEnd = 0x4903C0;
            break;
    }
    data = port_load_packed(rom, romEnd - rom, &end);
    func_8029DF78(OBJ_PTR(data, 0x1C), OBJ_PTR(data, 0x20), type);
    r.s4 = (u8 *) rom;
    func_802A08E4((u32 *) OBJ_PTR(data, 0x1C), (u32 *) OBJ_PTR(data, 0x20), &r);
    out->s2 = data;
    out->s4 = r.s4;
    out->a1 = end;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A396C.s")
#endif

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
s32 func_802A4168(s32 id, u8 *end);

#if defined(NON_MATCHING) && defined(PORT_HOST)
/* Windows port: the triangle builders below (the grids, func_802A3E9C,
 * func_802A3F80) never set a record's byte 0x51 (the "collides" flag that
 * 56040.c's scans test), and no builder sets bytes 0x5A-0x5F, so the original
 * reads (and leaves) whatever the heap held there: on the N64 a big-endian
 * byte of the previous level's triangle records or assets (attract demo 1
 * disables some of its grid triangles that way).  The host holds the same
 * values in host order; port_n64_byte (port/src/load/swap.c) gives the byte
 * the N64 would hold from the unit the load layer, or func_802A41B0 for its
 * records (port_tri_mark), last put there.  Memory other code wrote since is
 * misread -- only in these never-initialised bytes. */
void port_unit_mark(void *p, u32 n, s32 width);
u8 port_n64_byte(const void *p);
void port_garbage(const void *p, u32 len);

/* the layout of a record func_802A41B0 built (Unk803B9890, 77E20.c) */
static void port_tri_mark(u8 *rec) {
    port_unit_mark(rec, 4, 8);
    port_unit_mark(rec + 0x20, 11, 4);
    port_unit_mark(rec + 0x4C, 1, 2);
    port_unit_mark(rec + 0x4E, 4, 1);
    port_unit_mark(rec + 0x52, 1, 2);
    port_unit_mark(rec + 0x54, 12, 1);
}

#define PORT_STALE_51(rec) ((rec)[0x51] = port_n64_byte((rec) + 0x51))
#else
#define PORT_STALE_51(rec)
#endif

/* Grid of (s16) obj[0x10] * (s16) obj[0x12] cells (must be >= 1: the asm
 * counts down with `!=`) at OBJ_PTR(obj, off): each cell is an unaligned BE
 * word (end of its triangles, relative to the grid) followed by 0x16-byte
 * packed triangles. table[cell] = the cell's first record (table[cells] =
 * the end). Per triangle: func_802A41B0(rec, tri, id, h52 = cells left,
 * b57 = the cell end pointer, b56 = tri[0x14], s1io, b4F, 0), then
 * rec[0x59] = tri[0x15]. Returns the last cell's end pointer (the asm's t6). */
static u8 *port_build_grid(u8 *obj, s32 off, u8 * N64P *table, s32 id, s32 b4F, s32 *s1io) {
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
            PORT_STALE_51(rec);
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
extern u8 * N64P D_803BDAF0;  /* end of the groups */

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
    u8 * N64P *slot;
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
        slot = (u8 * N64P *) (grp + 8);
        while (n != 0) {
            n--;
            *slot++ = rec;
            PORT_STALE_51(rec);
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
#ifdef PORT_HOST
                /* read back as (channel, state) byte pairs (56040.c
                 * func_8029DB7C): keep the N64 byte order */
                ((u8 *) w)[0] = p[0];
                ((u8 *) w)[1] = p[1];
#else
                *(s16 *) w = last;
#endif
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
                PORT_STALE_51(rec);
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

#ifdef PORT_HOST
    {
        /* bytes 0x5A-0x5F are never written either (nothing reads them):
         * the N64's stale bytes too, so RDRAM compares equal */
        u8 old[6];
        s32 k;
        for (k = 0; k < 6; k++) {
            old[k] = port_n64_byte(rec + 0x5A + k);
        }
        for (k = 0; k < 6; k++) {
            rec[0x5A + k] = old[k];
        }
        port_garbage(rec + 0x5A, 6);
    }
    port_tri_mark(rec);
#endif
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
    u8 * N64P *out = D_803BDB10;

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
