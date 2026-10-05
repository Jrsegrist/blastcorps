#include "common.h"
#include <ultra64.h>

/* FILE-WIDE FINDING: this file's functions save $ra via the 64-bit `sd`/`ld`
 * doubleword form, not the normal 32-bit `sw`/`lw` pair - the same signature
 * documented in full in hd_code/77E20.c's top-of-file note and the project
 * skill file, confirming (with no exceptions found) that this file is part
 * of the same large hand-written-assembly module. Any pragma below without
 * a more specific comment follows this convention; a few have their own
 * more specific non-ABI explanation where one was already worked out. */

#ifdef NON_MATCHING
/* D_803F4030: array of 0xFC-byte records (55 slots); D_803F7654 points one
 * past the last record in use. Only the fields below are known. */
typedef struct {
    /* 0x00 */ u8 *unk0;    /* points at something whose byte +4 is a kind (1 = excluded) */
    /* 0x04 */ u8 pad4[0x2C];
    /* 0x30 */ s32 unk30;   /* 0x38 = excluded */
    /* 0x34 */ u8 pad34[0xB5];
    /* 0xE9 */ u8 numParts; /* number of per-part bytes at +0xEC */
    /* 0xEA */ u8 padEA;
    /* 0xEB */ u8 unkEB;    /* nonzero = already done */
    /* 0xEC */ u8 parts[0x10]; /* 100 = part destroyed */
} Rec7FB50; /* size 0xFC */

extern Rec7FB50 D_803F4030[];
extern Rec7FB50 *D_803F7654;
extern u16 D_8036EB90;
extern u8 D_803063F0[];

u8 func_8026FE6C(s32 arg0);

/* Bit writer used by the status packers below: MSB-first into whole bytes.
 * (Macros rather than functions: the originals are single asm leaves.) */
typedef struct {
    u8 *out;
    u32 bits;
    s32 count;
} BitWriter7FB50;

#define BITW_PUT(w, bit)                \
    {                                   \
        (w).bits = ((w).bits << 1) | (bit); \
        (w).count++;                    \
        if ((w).count == 8) {           \
            *(w).out++ = (w).bits;      \
            (w).bits = 0;               \
            (w).count = 0;              \
        }                               \
    }

/* Flush a partial byte, left-aligned. */
#define BITW_FLUSH(w)                                  \
    {                                                  \
        if ((w).count != 0) {                          \
            *(w).out++ = (w).bits << (8 - (w).count);  \
        }                                              \
    }

/* The player-vehicle block. The asm reaches it through gp (callers
 * func_802C5A14 / func_802C5AFC set gp = &D_803F7B50); the C reads it
 * directly. Only the fields below are known. */
extern u8 D_803F7B50[];
#define VEH_S32(off) (*(s32 *) (D_803F7B50 + (off)))
#define VEH_S16(off) (*(s16 *) (D_803F7B50 + (off)))
#define VEH_U8(off) (D_803F7B50[off])
/* +0x28/0x2C/0x30, +0x34/0x38/0x3C, +0x40/0x44/0x48: three s32 triples
 * +0x76: s16 (signed, decays towards 0)
 * +0x78..0x95: 15 s16 tuning values
 * +0x96..0x98: u8 flags; +0x9F: u8 (100 = ...); +0xA1: u8 mode (0..5); +0xA2: u8 previous mode */

extern s8 D_80370C2C;
extern s8 D_80370C2D;
extern u8 D_80370C35;
extern s32 D_803F7BFC;
extern s32 D_803F7C10;
extern s32 D_803F7C14;
extern void *D_803F7C18;
extern void *D_803F7C1C;
extern f32 D_803F7C28;
extern f32 D_803F7C2C;
extern u8 D_803F7C3C;
extern u8 D_803F7C3E;
extern u8 D_803F7C48;
extern f32 D_803EBBF0;
extern f32 D_803EBBF4;
extern u8 D_803ED3F6;
extern u8 D_803ED3F7;
extern s16 D_8036444C;
extern s16 D_80364450;
extern void *D_80367738;
extern f32 D_8030D968;
extern f32 D_8030D96C;
extern f32 D_8030D970;
extern f32 D_8030D974;

s32 func_80258500(u8 id);
void *func_80260650(void *arg0, s16 arg1, void *arg2);
void func_802608C8(void *arg0);

/* This vehicle's animation channel table and the channel-field setters /
 * getter of 56040.c (Unk8029DEA0Entry; asm conventions in
 * tools_port/conventions.txt). func_802A04BC's out[0] is (s8) field 0x10. */
extern u8 D_803F7850[];
void func_802A0290(void *base, s32 idx, s32 val);
void func_802A02E4(s32 idx, void *base);
void func_802A0360(f32 f, void *base, s32 idx, s32 val);
void func_802A039C(void *base, s32 idx, s32 val);
void func_802A03D4(void *base, s32 idx, s32 val);
void func_802A040C(void *base, s32 idx, s32 val);
void func_802A0480(f32 f, void *base, s32 idx, s32 val);
void func_802A04BC(s32 idx, void *base, s32 *out);

/* cvt.w.s under the default FCSR: round to nearest, ties to even (a C cast
 * truncates). */
#define CVT_W_S(out, x)                                       \
    {                                                         \
        f32 _x = (x);                                         \
        s32 _r = (s32) _x;                                    \
        f32 _d = _x - _r;                                     \
                                                              \
        if (_d > 0.5f || (_d == 0.5f && (_r & 1))) {          \
            _r++;                                             \
        } else if (_d < -0.5f || (_d == -0.5f && (_r & 1))) { \
            _r--;                                             \
        }                                                     \
        (out) = _r;                                           \
    }
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* record_status: pack into `out`, one bit each, MSB-first, (1) for every part
 * of every record in use whether it is 100, then (2) func_8026FE6C(i) for
 * i = 0 .. D_8036EB90-1. Each section is padded to a whole byte. Returns the
 * number of bytes written. */
s32 func_802C4A40(u8 *out) {
    BitWriter7FB50 w;
    Rec7FB50 *r;
    u8 *part;
    s32 count;
    s32 i;
    s32 n;

    w.out = out;
    w.bits = 0;
    w.count = 0;
    for (r = D_803F4030; r != D_803F7654; r++) {
        part = r->parts;
        for (n = r->numParts; n != 0; n--) {
            BITW_PUT(w,*part++ == 100);
        }
    }
    BITW_FLUSH(w);

    w.bits = 0;
    w.count = 0;
    count = D_8036EB90; /* read once, before any call */
    for (i = 0; i < count; i++) {
        BITW_PUT(w,func_8026FE6C(i));
    }
    BITW_FLUSH(w);

    return w.out - out;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C4A40.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_80368040;
void func_802BF1F0(void *e, s32 id);
void func_8026FE8C(s32 arg0);

/* apply_status: reads a bitstream in func_802C4A40's format (MSB-first; the
 * bit reader runs on across the records and restarts on a fresh byte for
 * section 2). Section 1, per record in use and per part i (1-based): a set
 * bit destroys the part (parts[i-1] = 100, func_802BF1F0(record, i)), a
 * clear bit restores it (0) and clears the record's "all intact" flag
 * (+0xEA) unless i equals byte +0xD of the record's +0 object; +0xEA is
 * written for every record. D_80368040 = the sum of byte +6 of the +0 object
 * of every record that had a part destroyed. Then, per destroyed part i of
 * each record, every 0x60-byte entry of [+4, +8) whose u16 at +0x52 is i
 * gets byte +0x51 = 0, and one whose byte +0x57 is i gets byte +0x51 = 1
 * when the part named by its +0x52 (1-based; 0 reads the byte before the
 * parts) isn't destroyed. Section 2: D_8036EB90 bits, func_8026FE8C(i) for
 * each set bit i. The asm saves every register it uses; its loops use `!=`
 * (the entry walk must reach +8 exactly). */
void func_802C4BF0(void *in) {
    u8 *p = in;
    Rec7FB50 *r;
    Rec7FB50 *end;
    u32 mask = 1;
    u32 byte = 0;
    s32 sum = 0;
    s32 n;
    s32 i;

    end = D_803F7654;
    for (r = D_803F4030; r != end; r++) {
        u8 *part = r->parts;
        s32 allIntact = 1;
        s32 anyDestroyed = 0;

        n = r->numParts;
        i = 0;
        while (n != 0) {
            u8 v;

            n--;
            i++;
            if (mask == 1) {
                byte = *p++;
                mask = 0x100;
            }
            mask >>= 1;
            if (!(byte & mask)) {
                if (r->unk0[0xD] != i) {
                    allIntact = 0;
                }
                v = 0;
            } else {
                v = 100;
                func_802BF1F0(r, i);
                anyDestroyed = 1;
            }
            *part++ = v;
        }
        r->padEA = allIntact;
        if (anyDestroyed) {
            sum += r->unk0[6];
        }
    }
    D_80368040 = sum;

    end = D_803F7654;
    for (r = D_803F4030; r != end; r++) {
        n = r->numParts;
        for (i = 0; n != 0; i++) {
            n--;
            if (r->parts[i] == 100) {
                u8 *e = *(u8 **) ((u8 *) r + 4);
                u8 *eEnd = *(u8 **) ((u8 *) r + 8);

                for (; e != eEnd; e += 0x60) {
                    s32 id = *(u16 *) (e + 0x52);

                    if (id == i + 1) {
                        e[0x51] = 0;
                    }
                    if (e[0x57] == i + 1 && r->parts[id - 1] != 100) {
                        e[0x51] = 1;
                    }
                }
            }
        }
    }

    n = D_8036EB90;
    mask = 1;
    for (i = 0; n != 0; i++) {
        n--;
        if (mask == 1) {
            byte = *p++;
            mask = 0x100;
        }
        mask >>= 1;
        if (byte & mask) {
            func_8026FE8C(i);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C4BF0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* A record counts toward a random status unless it is excluded. */
#define REC7FB50_ELIGIBLE(r) ((r)->unk30 != 0x38 && (r)->unk0[4] != 1)

/* create_status: build a status bitstream in the format func_802C4A40 writes,
 * for difficulty/percentage level `level` (1-based index into D_803063F0, a
 * table of percentages).
 * Section 1: for every record in use, numParts copies of one bit. The bit is
 * 1 for eligible records that are already done (unkEB), plus enough further
 * eligible records, in order, to make D_803063F0[level-1] percent of the
 * eligible ones, plus one (none at all when level == 5).
 * Section 2: D_8036EB90 bits, the first D_803063F0[level-1] percent of them 1.
 * Each section is padded to a whole byte. Returns the number of bytes written. */
u32 func_802C4E58(void *outp, u8 level) {
    BitWriter7FB50 w;
    Rec7FB50 *r;
    u32 pct;
    u32 eligible;
    s32 done;
    s32 extra;
    u32 ones;
    s32 count;
    u32 bit;
    s32 n;

    pct = D_803063F0[level - 1];

    eligible = 0;
    for (r = D_803F4030; r != D_803F7654; r++) {
        if (REC7FB50_ELIGIBLE(r)) {
            eligible++;
        }
    }
    /* Note: unlike `eligible`, this counts done records whether eligible or not. */
    done = 0;
    for (r = D_803F4030; r != D_803F7654; r++) {
        if (r->unkEB != 0) {
            done++;
        }
    }
    extra = (pct * eligible) / 100 + 1;
    if (level - 1 == 4) {
        extra = 0;
    } else {
        extra -= done;
        if (extra < 0) {
            extra = 0;
        }
    }

    w.out = outp;
    w.bits = 0;
    w.count = 0;
    for (r = D_803F4030; r != D_803F7654; r++) {
        bit = 0;
        if (REC7FB50_ELIGIBLE(r)) {
            if (r->unkEB != 0) {
                bit = 1;
            } else if (extra != 0) {
                extra--;
                bit = 1;
            }
        }
        for (n = r->numParts; n != 0; n--) {
            BITW_PUT(w,bit);
        }
    }
    BITW_FLUSH(w);

    count = D_8036EB90;
    ones = (pct * D_8036EB90) / 100;
    w.bits = 0;
    w.count = 0;
    for (; count != 0; count--) {
        bit = 0;
        if (ones != 0) {
            bit = 1;
            ones--;
        }
        BITW_PUT(w,bit);
    }
    BITW_FLUSH(w);

    return w.out - (u8 *) outp;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C4E58.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C5120.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Exit check for this vehicle (called from func_8024B4B8 in hd.c, which
 * declares it void and returns the leftover v0). 0 when a byte +0x96/7/8 of
 * the block is 1. Mode (+0xA1) 5: resets the mode to 0, stops channels 7
 * and 8, sets channel 6 (0.0, 2) and re-arms channel 9 (0.0 / 3 / 0 / 0.0 /
 * 1, restart with 1), returns 2. Other nonzero modes return 2. Mode 0:
 * 2 when channel 31's field 0x10 is 1, else 1 when channel 9's field 0x10 is
 * 0, else 2. Returns s32 (the asm's v0); the asm saves and restores $gp and
 * leaves v1 changed, which the C caller doesn't use. */
s32 func_802C5508(void) {
    s32 ch[8];

    if (VEH_U8(0x96) == 1 || VEH_U8(0x97) == 1 || VEH_U8(0x98) == 1) {
        return 0;
    }
    if (VEH_U8(0xA1) == 5) {
        VEH_U8(0xA1) = 0;
        func_802A02E4(7, D_803F7850);
        func_802A02E4(8, D_803F7850);
        func_802A0360(0.0f, D_803F7850, 6, 2);
        func_802A0360(0.0f, D_803F7850, 9, 0);
        func_802A039C(D_803F7850, 9, 3);
        func_802A03D4(D_803F7850, 9, 0);
        func_802A0480(0.0f, D_803F7850, 9, 0);
        func_802A040C(D_803F7850, 9, 1);
        func_802A0290(D_803F7850, 9, 1);
        return 2;
    }
    if (VEH_U8(0xA1) != 0) {
        return 2;
    }
    func_802A04BC(0x1F, D_803F7850, ch);
    if (ch[0] == 1) {
        return 2;
    }
    func_802A04BC(9, D_803F7850, ch);
    if (ch[0] == 0) {
        return 1;
    }
    return 2;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C5508.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u64 *D_803F7C08; /* save copy pair (func_802A7764) */
extern u64 *D_803F7C0C;
void func_802A7764(u64 *a, u64 *b, s32 size);

/* Teardown for this vehicle (called from func_8024B188 in hd.c): clears the
 * s16 at +0x76, func_802A7764(D_803F7C08, D_803F7C0C, 0x1000), stops
 * channel 31 and the sounds D_803F7C18 / D_803F7C1C that are playing (the
 * handles are left as they are). The asm saves and restores $gp and leaves
 * v1 = &D_803F7850 or func_802608C8's leftover; the C caller uses neither. */
void func_802C5688(void) {
    VEH_S16(0x76) = 0;
    func_802A7764(D_803F7C08, D_803F7C0C, 0x1000);
    func_802A02E4(0x1F, D_803F7850);
    if (D_803F7C18 != NULL) {
        func_802608C8(D_803F7C18);
    }
    if (D_803F7C1C != NULL) {
        func_802608C8(D_803F7C1C);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C5688.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Enter this vehicle (called from hd.c and 17210.c): clears the byte at
 * +0x99 and starts channels 3 and 4 (fields 0 / 0 / 1, 1.0 with 1, restart
 * with -1) and channel 1 (0 / 0 / 0, restart with -1). The asm points $gp at
 * D_803F7B50 and leaves it there (conventions.txt: clobbers gp) and leaves
 * v1 = -1; C callers use neither. */
void func_802C5714(void) {
    VEH_U8(0x99) = 0;
    func_802A039C(D_803F7850, 3, 0);
    func_802A03D4(D_803F7850, 3, 0);
    func_802A040C(D_803F7850, 3, 1);
    func_802A0480(1.0f, D_803F7850, 3, 1);
    func_802A0290(D_803F7850, 3, -1);
    func_802A039C(D_803F7850, 4, 0);
    func_802A03D4(D_803F7850, 4, 0);
    func_802A0480(1.0f, D_803F7850, 4, 1);
    func_802A040C(D_803F7850, 4, 1);
    func_802A0290(D_803F7850, 4, -1);
    func_802A039C(D_803F7850, 1, 0);
    func_802A03D4(D_803F7850, 1, 0);
    func_802A040C(D_803F7850, 1, 0);
    func_802A0290(D_803F7850, 1, -1);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C5714.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef TRI_SCAN_TYPES_DEFINED
#define TRI_SCAN_TYPES_DEFINED
/* func_802AC0BC's FP and integer register results (62740.c), in and out. */
typedef struct {
    f32 pz;    /* f12 */
    f32 cross; /* f14 */
    f32 cz;    /* f20 */
    f32 side;  /* f22 */
    f32 sideZ; /* f24 */
    f32 dz;    /* f26 */
} TriSideOut;
typedef struct {
    s32 a1; /* found flag */
    s32 a3;
    s32 t6;
    s32 t7;
    s32 fp;
    s32 s1;
    s32 s2;
    s32 s3;
    s32 s4;
} TriScanRegs;
s32 func_802AC0BC(s32 x, s32 z, s32 y, TriSideOut *f, TriScanRegs *r);
#endif
#ifndef MTX_CHAIN_REGS_DEFINED
#define MTX_CHAIN_REGS_DEFINED
/* func_802AA890's extra registers (62740.c), as func_8029C454 and
 * func_802ABBEC pass them through. */
typedef struct {
    s32 v1; /* out */
    s32 a0; /* out */
    s32 a3; /* in/out */
    s32 s1; /* in/out */
    s32 s2; /* in/out */
    s32 s0; /* in */
} MtxChainRegs;
#endif
#ifndef OUT_802A9A60_DEFINED
#define OUT_802A9A60_DEFINED
/* func_802A9A60's pointer results (the asm's s1 and s3; 62740.c). */
typedef struct {
    s32 *s1; /* dst + 9 */
    s32 *s3; /* dst */
} Out802A9A60;
#endif
extern u32 D_803F7BF8[]; /* x, y, z */
s32 func_802A9A60(s16 *tbl, s32 y, s32 x, s32 z, s32 *dst, s32 *mid, s16 *angle, s32 key, s32 fp, u8 *veh,
                  TriSideOut *f, Out802A9A60 *out);
void func_802A133C(s32 a0Val, s32 id, s32 v0Val, s32 v1Val, u8 *obj);
void func_802C7CB0(MtxChainRegs *regs);

/* Move this vehicle while it isn't the player's (func_8024B618, case 9):
 * ground heights for its three slots (func_802A9A60 with table +0x52, angle
 * +0x4C, heights to +4.., the middle y to D_803F7BFC, key 9), place the
 * model (func_802C7CB0), then func_802A133C(z, 9, x, y, block).
 * Register convention (conventions.txt): `fp` is the asm's $fp, which goes
 * on to func_802A9A60 (which stores it into D_803ED3F2[0..2], so the
 * block's +0x50 becomes (u8) fp). The only caller, C func_8024B618, passes
 * nothing: in the game this is whatever $fp its callers left (FIDELITY:
 * see tools_port port_followups), so the native port must pass it
 * explicitly. f12-f26 only pass through func_802A9A60 (not read first; the
 * asm restores f20-f30), so the C starts them at 0. func_802C7CB0 gets s0 =
 * z and s1 = func_802A9A60's s1. The asm saves every s-register, gp, fp and
 * f20-f30; it leaves v1 = y (func_802A133C preserves it), which the survey
 * lists as read by func_8024B618 (C: not a real use). */
void func_802C5860(s32 fp) {
    TriSideOut f;
    Out802A9A60 out;
    MtxChainRegs regs;
    s32 z = D_803F7BF8[2];

    f.pz = f.cross = f.cz = f.side = f.sideZ = f.dz = 0.0f;
    func_802A9A60((s16 *) (D_803F7B50 + 0x52), D_803F7BF8[1], D_803F7BF8[0], z, (s32 *) (D_803F7B50 + 4),
                  (s32 *) &D_803F7BF8[1], (s16 *) (D_803F7B50 + 0x4C), 9, fp, D_803F7B50, &f, &out);
    regs.s0 = z;
    regs.s1 = (s32) out.s1;
    func_802C7CB0(&regs);
    func_802A133C(D_803F7BF8[2], 9, D_803F7BF8[0], D_803F7BF8[1], D_803F7B50);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C5860.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef ZONE_SCAN_REGS_DEFINED
#define ZONE_SCAN_REGS_DEFINED
/* func_802ABD54's scan registers (62740.c), in and out. */
typedef struct {
    s32 t6; /* zone x */
    s32 t7; /* zone y */
    s32 s0; /* zone z */
    s32 s1; /* distance / level term */
    s32 s2; /* zone radius */
    s32 s3; /* scan counter / zone byte */
    s32 s4; /* scan pointer / zone byte */
} ZoneScanRegs;
s32 func_802ABD54(s32 id, s32 x, s32 y, s32 z, ZoneScanRegs *r);
#endif
extern u32 D_803F7BF8[]; /* x, y, z */
/* Zone level lookup (func_802ABD54) for vehicle id 9 at its position
 * D_803F7BF8..+8; returns func_802ABD54's v1 (the zone list end).
 * Register convention: the asm passes func_802ABD54's scan registers t6, t7,
 * s0-s4 through (in and out; here through r), leaves a3 = id and clobbers
 * s5, s6 (conventions.txt). Asm caller func_802C5AFC keeps f12 and f14 live
 * (a mixed N64 build would need a thunk; the native port won't). */
s32 func_802C5970(ZoneScanRegs *r) {
    return func_802ABD54(9, D_803F7BF8[0], D_803F7BF8[1], D_803F7BF8[2], r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C5970.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef INTERP_REGS_DEFINED
#define INTERP_REGS_DEFINED
/* func_802AAD0C's register results (62740.c). */
typedef struct {
    s32 t3;  /* interpolated u */
    s32 t4;  /* interpolated w */
    f32 f12;
    s32 f14; /* raw bits */
    f32 f20;
    f32 f22;
    f32 f24; /* also an input (kept on some paths) */
    f32 f26;
} InterpRegs;
#endif
void func_802AAD0C(s32 id, s32 x, s32 z, InterpRegs *r);

/* Value pair at this vehicle's x/z (D_803F7BF8[0], [2]) on the triangle
 * `id` (func_802AAD0C), stored as s16s at +0x6A/+0x6C of the vehicle block.
 * Register convention: id in a3; func_802AAD0C's FP results (f12-f26, f24
 * also in) pass through to the caller, here through r (conventions.txt). The
 * asm saves and restores t0, t1, t3, t4 (asm caller func_802AB50C keeps t0
 * and t1 live: a mixed N64 build would need a thunk) and points $gp at
 * D_803F7B50 without restoring it. */
void func_802C59B4(s32 id, InterpRegs *r) {
    func_802AAD0C(id, ((s32 *) D_803F7BF8)[0], ((s32 *) D_803F7BF8)[2], r);
    VEH_S16(0x6A) = r->t3;
    VEH_S16(0x6C) = r->t4;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C59B4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef REGS_802A8768_DEFINED
#define REGS_802A8768_DEFINED
/* func_802A8768's register results besides the FP state (62740.c). */
typedef struct {
    s32 s3;  /* in/out */
    s16 *s4; /* out */
    s32 fp;  /* out */
} Regs802A8768;
#endif
extern u8 D_803ED40B;
void func_802AAE54(s32 id, s32 x, s32 z, InterpRegs *r);
void func_802A8768(u8 *veh, s32 id, s32 *px, s32 *py, s32 *pz, s32 x, s32 z, s32 divB, s32 divA, s16 *angle,
                   u8 *flags, s16 *tbl, s32 *a, s32 *b, s32 *c, s32 *ys, Regs802A8768 *r, TriSideOut *f);
void func_802C7ECC(void);
void func_802C7F28(void);

/* Set this vehicle down at the point on triangle `id` given by the value
 * pair at +0x6A/+0x6C (func_802AAE54): D_803EBBF4 (func_802C7ECC) and the
 * tuning values (func_802C7F28) for its mode, D_803ED40B = 0, ground
 * contact (func_802A8768: id 9, position D_803F7BF8..+8, divisors 0x78 /
 * 0x78, angle +0x4C, flags +0x96, table +0x52, records +0x28/+0x40/+0x34,
 * heights +4), place the model (func_802C7CB0) and func_802A133C(z, 9, x,
 * y, block).
 * Register convention (conventions.txt): id in a3; s3 in/out (func_802A8768's
 * slot result, through *s3); the FP state chains through func_802AAE54 and
 * func_802A8768 (r: f24 in; f20-f26 out, f12/f14 are left as func_802AA764's
 * cos/sin temporaries, not modelled). func_802C7CB0 gets s0 = &+0x96 and
 * s1 = &D_803F7C00, the flags/pz pointers func_802A8768 leaves. The asm
 * saves and restores a3, t0, t1, t2 (asm caller func_802AB714 keeps a3, t0,
 * t1 live: a mixed N64 build would need a thunk) and points $gp at
 * D_803F7B50 without restoring it. */
void func_802C5A14(s32 id, s32 *s3, InterpRegs *r) {
    Regs802A8768 r8768;
    MtxChainRegs regs;

    func_802AAE54(id, VEH_S16(0x6A), VEH_S16(0x6C), r);
    func_802C7ECC();
    func_802C7F28();
    D_803ED40B = 0;
    r8768.s3 = *s3;
    func_802A8768(D_803F7B50, 9, (s32 *) &D_803F7BF8[0], (s32 *) &D_803F7BF8[1], (s32 *) &D_803F7BF8[2], r->t3,
                  r->t4, 0x78, 0x78, &VEH_S16(0x4C), &VEH_U8(0x96), &VEH_S16(0x52), &VEH_S32(0x28),
                  &VEH_S32(0x40), &VEH_S32(0x34), &VEH_S32(4), &r8768, (TriSideOut *) &r->f12);
    *s3 = r8768.s3;
    regs.s0 = (s32) &VEH_U8(0x96);
    regs.s1 = (s32) &D_803F7BF8[2];
    func_802C7CB0(&regs);
    func_802A133C(D_803F7BF8[2], 9, D_803F7BF8[0], D_803F7BF8[1], D_803F7B50);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C5A14.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef OUT_802A860C_DEFINED
#define OUT_802A860C_DEFINED
/* func_802A860C's results besides t0 (62740.c). */
typedef struct {
    s32 t1; /* new z */
    s32 s3; /* *px as read */
    s32 fp; /* the cosine */
} Out802A860C;
#endif
extern u8 D_8035805C;  /* which of the save copy pair is current */
extern u8 *D_803F7C04; /* this vehicle's model */
extern u8 D_803F7C36;  /* "turning back to the ring" flag */
extern u8 D_803F7C37;  /* bounced off the ring end (the bounce code is unreachable here) */
extern u8 D_803F7C38;  /* throttle hold-off countdown */
extern s16 D_803F7C30; /* ring target heading */
extern u8 D_803F7C39;
extern u8 D_803F7C3F;
extern s32 D_803F7C20;
extern s32 D_803F7C24;
extern u8 D_803A742B;
extern u8 D_803A7424;
extern u8 D_803A7425;
extern void *D_803F77D0;
extern s32 D_803643E0;
extern s32 D_803643E4;
extern s32 D_803643E8;
extern s16 D_8036443C;
extern s16 D_8036443E;
extern s16 D_80364440;
extern u8 D_80306400[];
void func_8029A800(s32 z, s32 a1, s32 b2, s32 b3, s32 x, s32 y, s32 b0, s32 h1, s32 h2, s32 b4, s32 b8,
                   u8 *veh);
void func_8029A914(void);
void func_8029AA10(s32 kind);
void func_8029C52C(s32 tag);
void func_8029E558(u8 *base, u8 *other, void *ch);
s32 func_802A6F6C(void);
void func_802A6FE4(u8 *veh, s32 limit);
void func_802A7070(u8 *veh, s16 *angle);
void func_802A70D8(u8 *veh);
s32 func_802A71DC(u8 *veh, s32 cur, s32 target, s32 *curOut, f32 scale);
s32 func_802A746C(u8 *veh, s32 delta, s32 v1, s32 *targetOut);
void func_802A77D0(u8 *veh);
void func_802A785C(u8 *veh, s16 *speed, s32 mode, u8 *flags, s16 *bands, s32 delta);
void func_802A7E70(s32 rate, u16 *angle);
f32 func_802A83B8(s16 *div, u8 *f, s32 *p, f32 *out);
void func_802A843C(u8 *veh, s16 *speed, s32 kind, s8 *f, s32 *p, s32 clamp, f32 div);
s32 func_802A860C(f32 f, s32 angle, s16 *len, s32 *px, s32 *pz, Out802A860C *out);
void func_802BE77C(s32 id, u8 *vehicle);
void func_802C61F0(s32 s0, TriSideOut *f, TriScanRegs *r);
void func_802C617C(void);
void func_802C71FC(s32 x, s32 z, TriSideOut *f);
void func_802C7410(void);
s32 func_802C7DFC(void);

/* Per-frame update of vehicle 9 when it is the player's (func_8024B7AC and
 * its init func_802C5120): zone lookup (func_802C5970), D_803EBBF4
 * (func_802C7ECC), unless +0x9A is set the mode machine (func_802C61F0) and
 * the D_8036444C pair (func_802C7410), the tuning values (func_802C7F28),
 * steering (func_802A7E70 at rate func_802C7DFC()), throttle (func_802A785C
 * with delta D_803F7C48, unless D_803F7C37; held off while D_803F7C38 counts
 * down), braking (func_802A77D0 in mode 0, else func_802C617C), heading
 * target = heading, slope (func_802A83B8 unless +0x96 is 1) and drag
 * (func_802A843C in mode 0; other modes move the speed 1 toward 0) unless
 * D_803F7C37, ring turn-back (func_802A7070 when D_803F7C36), the forward
 * step (func_802A860C), the ceiling check func_802C71FC and ground contact
 * (func_802A8768), the model (func_8029E558 on the current copy pair,
 * func_802C7CB0). Outside mode 5: collision (func_8029A800 with the mode
 * class 1 (modes 1-3) / 2 (mode 4, or modes 0/6+ while D_803F7C24 < 3) / 0,
 * func_8029C52C, func_8029AA10; a hit, D_803A742B, in modes 1-4 -> mode 2,
 * D_803F7C39 = 1), then either the collision pass func_802BE77C (no ring
 * hit) whose ring-end hit (D_803A7424, or +0x9C) sends modes 1-3 (and modes
 * 0/5+ while D_803F7C20 < 3 or with +0x96 == 1) to mode 2 and otherwise
 * falls into the turn back, or (ring hit) the turn back toward the ring
 * midpoint (func_8029A914, D_803F7C36 = 1, func_802A70D8, func_802A71DC,
 * func_802A746C, func_802A6FE4(0)). Mode 4 and the quiet paths clear
 * D_803F7C36/37. Finally the position, speed and angles go to D_803643E0.. /
 * D_8036443C.., func_802A133C(z, 9, x, y, block), and D_803F7C3F = mode in
 * 1..4. (The asm also holds a bounce-off-the-ring-end block like
 * func_802CA4E0's, but nothing branches to it.)
 * Register notes (FIDELITY, see tools_port/port_followups.md): func_802C5970's
 * scan registers start as the caller's t6, t7, s0-s4 (passed on to
 * func_802C61F0 -> func_802C7544 -> func_802A6274 where func_802ABD54 leaves
 * them) and the FP block f12-f26 as the caller's (only passed through):
 * the C starts them at 0. func_8029A800 gets b0 = t0 = 9 and h2 = t2 = the
 * model's point list end, which func_802C7CB0 (func_802ABBEC) leaves:
 * modelled. func_802C7CB0 gets s0 = &+0x96, s1 = &D_803F7C00 (left after
 * func_802A8768; func_8029E558's own leftovers in the game; only used for a
 * count-0 point record). func_8029A914 / func_8029C52C take the block in
 * $gp but their C rewrites (56040) read D_803EEA90: reported. The asm leaves
 * v1, f12, f14 as its callees leave them (survey: read by func_8024B7AC /
 * func_802C5120; not real uses). Integer sums trap in the asm. */
void func_802C5AFC(void) {
    ZoneScanRegs zr;
    TriScanRegs tr;
    TriSideOut f;
    Out802A860C o860c;
    Regs802A8768 r8768;
    MtxChainRegs regs;
    f32 slope;
    s32 x;
    s32 z;
    s32 v;
    s32 mid;
    s32 cur;
    s32 turn;
    s32 tgt;
    s32 cls;
    s32 mode;
    u8 *mdl;

    zr.t6 = zr.t7 = zr.s0 = zr.s1 = zr.s2 = zr.s3 = zr.s4 = 0;
    func_802C5970(&zr);
    func_802C7ECC();
    if ((s8) VEH_U8(0x9A) == 0) {
        f.pz = f.cross = f.cz = f.side = f.sideZ = f.dz = 0.0f;
        tr.a1 = 0;
        tr.a3 = 9;
        tr.t6 = zr.t6;
        tr.t7 = 0;
        tr.fp = 0;
        tr.s1 = zr.s1;
        tr.s2 = zr.s2;
        tr.s3 = zr.s3;
        tr.s4 = zr.s4;
        func_802C61F0(zr.s0, &f, &tr);
        func_802C7410();
    }
    func_802C7F28();
    func_802A7E70(func_802C7DFC(), (u16 *) &VEH_S16(0x4C));
    if (D_803F7C37 == 0) {
        if (D_803F7C38 == 0) {
            func_802A785C(D_803F7B50, &VEH_S16(0x76), 4, &VEH_U8(0x96), &VEH_S16(0x78), D_803F7C48);
        } else {
            D_803F7C38--;
        }
    }
    if (VEH_U8(0xA1) == 0) {
        func_802A77D0(D_803F7B50);
    } else {
        func_802C617C();
    }
    VEH_S16(0x4E) = VEH_S16(0x4C);
    if (VEH_U8(0x96) == 1) {
        slope = 0.0f;
    } else {
        slope = func_802A83B8(&VEH_S16(0x76), &VEH_U8(0x96), &VEH_S32(4), (f32 *) &VEH_S32(0));
    }
    if ((s8) D_803F7C37 == 0) {
        if (VEH_U8(0xA1) == 0) {
            func_802A843C(D_803F7B50, &VEH_S16(0x76), 9, (s8 *) &VEH_U8(0x96), &VEH_S32(4), 1, 120.0f);
        } else {
            v = VEH_S16(0x76);
            if (v > 0) {
                v--;
            } else if (v < 0) {
                v++;
            }
            VEH_S16(0x76) = v;
        }
    }
    if ((s8) D_803F7C36 != 0) {
        func_802A7070(D_803F7B50, &D_803F7C30);
    }
    x = func_802A860C(slope, *(u16 *) &VEH_S16(0x4E), &VEH_S16(0x76), (s32 *) &D_803F7BF8[0],
                      (s32 *) &D_803F7BF8[2], &o860c);
    z = o860c.t1;
    f.pz = f.cross = f.cz = f.side = f.sideZ = f.dz = 0.0f;
    func_802C71FC(x, z, &f);
    D_803ED40B = 0;
    r8768.s3 = o860c.s3;
    func_802A8768(D_803F7B50, 9, (s32 *) &D_803F7BF8[0], (s32 *) &D_803F7BF8[1], (s32 *) &D_803F7BF8[2], x, z,
                  0x78, 0x78, &VEH_S16(0x4C), &VEH_U8(0x96), &VEH_S16(0x52), &VEH_S32(0x28), &VEH_S32(0x40),
                  &VEH_S32(0x34), &VEH_S32(4), &r8768, &f);
    if (D_8035805C != 0) {
        func_8029E558((u8 *) D_803F7C08, (u8 *) D_803F7C0C, D_803F7850);
    } else {
        func_8029E558((u8 *) D_803F7C0C, (u8 *) D_803F7C08, D_803F7850);
    }
    regs.s0 = (s32) &VEH_U8(0x96);
    regs.s1 = (s32) &D_803F7BF8[2];
    func_802C7CB0(&regs);
    D_803F7C39 = 0;
    mode = VEH_U8(0xA1);
    if (mode == 5) {
        goto quiet;
    }
    if (mode == 1 || mode == 2 || mode == 3) {
        cls = 1;
    } else if (mode == 4 || D_803F7C24 < 3) {
        cls = 2;
    } else {
        cls = 0;
    }
    mdl = D_803F7C04; /* t2 as func_802ABBEC (in func_802C7CB0) leaves it */
    func_8029A800(D_803F7BF8[2], (s32) D_80306400, 1, 0, D_803F7BF8[0], D_803F7BF8[1], 9, VEH_S16(0x76),
                  (s32) (mdl + *(s32 *) (mdl + 4)), cls, 9, D_803F7B50);
    func_8029C52C(9);
    func_8029AA10(9);
    if (D_803A742B != 0 && VEH_U8(0xA1) != 0 && VEH_U8(0xA1) != 5) {
        VEH_U8(0xA1) = 2;
        D_803F7C39 = 1;
    }
    if (D_803A7425 == 0) {
        D_803A7424 = 0;
        D_803F77D0 = D_803F7850;
        func_802BE77C(9, D_803F7B50);
        if (D_803A7424 == 0 && VEH_U8(0x9C) == 0) {
            goto quiet;
        }
        mode = VEH_U8(0xA1);
        if (mode == 1 || mode == 2 || mode == 3) {
            goto toMode2;
        }
        if (mode == 4) {
            goto quiet;
        }
        if (D_803F7C20 < 3 || VEH_U8(0x96) == 1) {
            goto toMode2;
        }
    }
    /* turn back toward the ring */
    func_8029A914();
    D_803F7C36 = 1;
    mid = func_802A6F6C();
    v = *(u16 *) &VEH_S16(0x4E) - 0x800;
    if (v < 0) {
        v += 0xFFF;
    }
    v -= mid;
    if (v < 0) {
        v = -v;
    }
    if (!(v < 0x801)) {
        v = 0xFFF - v;
    }
    func_802A70D8(D_803F7B50);
    turn = func_802A71DC(D_803F7B50, *(u16 *) &VEH_S16(0x4E), *(u16 *) &VEH_S16(0x4C), &cur, 0.25f);
    D_803F7C30 = cur;
    VEH_S16(0x4E) = cur;
    VEH_S16(0x74) = cur;
    func_802A746C(D_803F7B50, turn, v, &tgt);
    func_802A6FE4(D_803F7B50, 0);
    goto done;
toMode2:
    VEH_U8(0xA1) = 2;
    D_803F7C39 = 1;
    goto done;
quiet:
    D_803F7C36 = 0;
    D_803F7C37 = 0;
done:
    D_803643E0 = D_803F7BF8[0];
    D_803643E4 = D_803F7BF8[1];
    D_803643E8 = D_803F7BF8[2];
    D_8036443C = VEH_S16(0x76);
    D_8036443E = *(u16 *) &VEH_S16(0x4E);
    D_80364440 = *(u16 *) &VEH_S16(0x4C);
    func_802A133C(D_803F7BF8[2], 9, D_803F7BF8[0], D_803F7BF8[1], D_803F7B50);
    mode = VEH_U8(0xA1);
    D_803F7C3F = (mode != 0 && mode != 5) ? 1 : 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C5AFC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Unless D_80370C35 or D_80370C2D is set, moves the s16 at +0x76 of the
 * vehicle block 8 towards 0 (stopping at 0). The asm saves s7, the only
 * register it uses; asm caller func_802C5AFC keeps a0-a3 and t6 live (a
 * mixed N64 build would need a thunk, the native port doesn't). */
void func_802C617C(void) {
    s32 v;

    if (D_80370C35 != 0 || D_80370C2D != 0) {
        return;
    }
    v = VEH_S16(0x76);
    if (v >= 0) {
        v -= 8;
        if (v < 0) {
            v = 0;
        }
    } else {
        v += 8;
        if (v > 0) {
            v = 0;
        }
    }
    VEH_S16(0x76) = v;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C617C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef TRI_SCAN_TYPES_DEFINED
#define TRI_SCAN_TYPES_DEFINED
/* func_802AC0BC's FP and integer register results (62740.c), in and out. */
typedef struct {
    f32 pz;    /* f12 */
    f32 cross; /* f14 */
    f32 cz;    /* f20 */
    f32 side;  /* f22 */
    f32 sideZ; /* f24 */
    f32 dz;    /* f26 */
} TriSideOut;
typedef struct {
    s32 a1; /* found flag */
    s32 a3;
    s32 t6;
    s32 t7;
    s32 fp;
    s32 s1;
    s32 s2;
    s32 s3;
    s32 s4;
} TriScanRegs;
s32 func_802AC0BC(s32 x, s32 z, s32 y, TriSideOut *f, TriScanRegs *r);
#endif
#ifndef IO_802A6274_DEFINED
#define IO_802A6274_DEFINED
/* func_802A6274's in/out registers (60F60.c). */
typedef struct {
    s32 a3;
    s32 t6;
    s32 s1;
} Io802A6274;
#endif
extern s32 D_803F7C20; /* frames since the countdown D_803F7C40 was at 2/3 */
extern s32 D_803F7C24; /* frames since it was at 4 */
extern s16 D_803F7C32; /* channel 6's last field 0x18 */
extern u8 D_803F7C39;
extern u8 D_803F7C3D; /* countdown */
extern u8 D_803F7C40; /* countdown */
extern u8 D_803F7C41; /* countdown (reloaded with 9 outside modes 1-3) */
extern u8 D_803F7C42; /* previous frame's D_80370C1D */
extern u8 D_803F7C43; /* previous frame's +0x96 */
extern u8 D_803F7C44;
extern u8 D_803F7C45;
extern u8 D_803F7C46;
extern u8 D_803F7C47;
extern u8 D_803F7C4A;
extern u8 D_80370C1A; /* inputs */
extern u8 D_80370C1B;
extern u8 D_80370C1C;
extern u8 D_80370C1D;
extern u8 D_802E8BE4;
extern s32 D_802E8BE8;
extern f32 D_8030D950;
extern u8 D_802C28E4[]; /* definition handed to func_802A6274 */
void func_8029F9D4(s32 a, s32 b, void *base);
void func_8029FC74(s32 a, s32 b, void *base);
s32 func_802A6274(Io802A6274 *io, u8 *def, s32 data, s32 type, s32 x, s32 y, s32 z, s32 w24, s32 w28,
                  s32 w18, s32 w1C, s32 w2C, s32 b35);
s32 func_802C6FD8(void);
s32 func_802C70E8(TriSideOut *f, TriScanRegs *r);
void func_802C7354(void);
void func_802C7544(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4);
void func_802C6DAC(void);
void func_802C6ECC(void);
void func_802C770C(void);
void func_802C7864(s32 *up);

/* Three s32 copies of one value at +off, +off+4, +off+8 of the vehicle block. */
#define VEH_TRIPLE(off, v) (VEH_S32(off) = VEH_S32((off) + 4) = VEH_S32((off) + 8) = (v))

/* Per-frame update of this vehicle's mode machine (+0xA1: 0 idle/driving,
 * 1 airborne?, 2/3, 4, 5; +0xA2 keeps the mode the frame started in), its
 * countdowns and animation channels (D_803F7850), then the ground check
 * func_802C70E8, func_802C7354, the steering channels (func_802C7864, modes
 * 1-3), the ramp/records func_802C7544, the idle check func_802C6DAC and
 * the looping sounds func_802C6ECC. Mode values above 5 hit a `syscall` in
 * the asm (not reached in the game; the C does nothing there).
 * Register convention (conventions.txt): s0 comes in and goes on to
 * func_802C7544; the FP registers f12-f26 and a3, t6, s1-s4 come in for
 * func_802C70E8 (here through f and r), and its f20-f26 are what the caller
 * func_802C5AFC reads back. t6, t7, s1-s4 after func_802C70E8 (or the
 * zeros/func_802A6274 results on the mode-4 exit path, s4 = |speed| / 16 in
 * mode 0, s2 from func_802C7864) are what func_802C7544 gets. Where an asm
 * callee leaves an undocumented register behind (func_802C7864 leaves s3 =
 * 270 on its steering paths, func_80260650 is C and may change t6/t7) the
 * C passes the documented value instead. $gp (= D_803F7B50) is read as the
 * global. Integer counters use trapping `addi` in the asm. */
void func_802C61F0(s32 s0, TriSideOut *f, TriScanRegs *r) {
    s32 mode = VEH_U8(0xA1);
    s32 v;
    s32 t6;
    s32 t7;
    s32 s1;
    s32 s2;
    s32 s3;
    s32 s4;
    s16 old;
    s32 ch[8];
    Io802A6274 io;

    VEH_U8(0xA2) = mode;
    if (mode == 1 || mode == 2 || mode == 3) {
        if (D_803F7C41 != 0) {
            D_803F7C41--;
        }
    } else {
        D_803F7C41 = 9;
    }
    v = D_803F7C40;
    if (v != 0) {
        v--;
        D_803F7C40 = v;
    }
    if (v == 2 || v == 3) {
        D_803F7C20 = 0;
    } else {
        D_803F7C20++;
    }
    if (v == 4) {
        D_803F7C24 = 0;
    } else {
        D_803F7C24++;
    }

    D_803F7C3E = 0;
    if (!(D_803F7C14 < D_803F7BFC) && VEH_U8(0xA1) != 4 && VEH_U8(0xA1) != 2) {
        if (D_803F7C3D != 0) {
            D_803F7C3D--;
        } else if (D_80370C1C != 0 || D_80370C1A != 0 || D_80370C1B != 0) {
            if (VEH_U8(0x96) != 1) {
                VEH_TRIPLE(0x34, D_803F7BFC);
                VEH_TRIPLE(0x40, 1);
                VEH_TRIPLE(0x28, 30);
                VEH_U8(0x96) = 1;
                VEH_U8(0x97) = 1;
                VEH_U8(0x98) = 1;
            } else {
                v = func_802C6FD8();
                VEH_TRIPLE(0x40, 1);
                VEH_TRIPLE(0x28, v);
                VEH_TRIPLE(0x34, D_803F7BFC);
            }
            D_803F7C3E = 1;
        }
    }

    if (VEH_U8(0xA1) == 5 && (VEH_S16(0x76) != 0 || VEH_U8(0x96) != 0)) {
        VEH_U8(0xA1) = 0;
        func_802A02E4(7, D_803F7850);
        func_802A02E4(8, D_803F7850);
        func_802A0360(0.0f, D_803F7850, 6, 2);
        func_802A0360(0.0f, D_803F7850, 9, 0);
        func_802A039C(D_803F7850, 9, 3);
        func_802A03D4(D_803F7850, 9, 0);
        func_802A0480(0.0f, D_803F7850, 9, 0);
        func_802A040C(D_803F7850, 9, 1);
        func_802A0290(D_803F7850, 9, 1);
    }

    if (VEH_U8(0x9E) != 0) {
        VEH_U8(0xA1) = 3;
        D_803F7C39 = 1;
    } else if (D_803F7C43 != 1) {
        if (VEH_U8(0x96) == 1) {
            func_802A02E4(6, D_803F7850);
            func_802A02E4(9, D_803F7850);
            D_803F7C28 = (VEH_U8(0xA2) == 5) ? D_8030D950 : 0.5f;
            D_803F7C2C = 0.5f;
            VEH_U8(0xA1) = 1;
        }
    } else if (VEH_U8(0x96) == 0 && D_803F7C39 == 0) {
        /* landed: reset the position triples */
        v = D_803F7BFC;
        VEH_S32(0x04) = v;
        VEH_S32(0x08) = v;
        VEH_S32(0x0C) = v;
        VEH_S32(0x10) = v;
        VEH_S32(0x14) = v;
        VEH_S32(0x18) = v;
        VEH_S32(0x1C) = v;
        VEH_S32(0x20) = v;
        VEH_S32(0x24) = v;
        func_802A0360(0.0f, D_803F7850, 6, 2);
        func_802A02E4(5, D_803F7850);
        func_802A02E4(4, D_803F7850);
        func_802A02E4(3, D_803F7850);
        if (VEH_U8(0xA2) != 4) {
            func_8029FC74(3, 4, D_803F7850);
            func_8029F9D4(0x1E, 6, D_803F7850);
            func_802A039C(D_803F7850, 0x1F, 0x32);
            func_802A03D4(D_803F7850, 0x1F, 0);
            func_802A040C(D_803F7850, 0x1F, 0);
            func_802A0290(D_803F7850, 0x1F, 1);
        }
        s4 = VEH_S16(0x76);
        if (s4 >= 0x97) {
            s4 = 0x96;
            VEH_S16(0x76) = s4;
        } else if (s4 < -0x64) {
            s4 = -0x64;
            VEH_S16(0x76) = s4;
        }
        r->s4 = s4; /* the asm's s4, on into func_802C70E8 */
        VEH_U8(0xA1) = 0;
    }

    func_802C70E8(f, r);
    func_802C7354();
    t6 = r->t6;
    t7 = r->t7;
    s1 = r->s1;
    s2 = r->s2;
    s3 = r->s3;
    s4 = r->s4;

    if (VEH_U8(0xA2) == 4 && VEH_U8(0xA1) == 0) {
        func_80260650(D_80367738, 0x7D, NULL);
        D_802E8BE4 = 0x14;
        D_802E8BE8 = 0x320;
        io.a3 = 0;
        io.t6 = 0;
        io.s1 = 0;
        func_802A6274(&io, D_802C28E4, 0x222E0, 0, ((s32 *) D_803F7BF8)[0] << 11, D_803F7BFC << 11,
                      ((s32 *) D_803F7BF8)[2] << 11, 0, 0, 0, 0, 0, 0);
        t6 = io.t6;
        s1 = io.s1;
        t7 = 0;
        s0 = 0;
        s2 = 0;
        s3 = 0;
        s4 = 0;
    }

    switch (VEH_U8(0xA1)) {
        case 5:
            func_802A04BC(7, D_803F7850, ch);
            if (ch[0] == 1) {
                break;
            }
            func_802A04BC(8, D_803F7850, ch);
            if (ch[0] == 1) {
                break;
            }
            func_802A0360(0.0f, D_803F7850, 8, 0);
            func_802A039C(D_803F7850, 8, 1);
            func_802A03D4(D_803F7850, 8, 0);
            func_802A0480(0.5f, D_803F7850, 8, 1);
            func_802A040C(D_803F7850, 8, 1);
            func_802A0290(D_803F7850, 8, -1);
            break;

        case 4:
            if (D_803F7C4A != 0) {
                func_802A04BC(5, D_803F7850, ch);
                if (ch[0] != 1) {
                    VEH_TRIPLE(0x34, D_803F7BFC);
                    VEH_TRIPLE(0x40, 1);
                    VEH_TRIPLE(0x28, -0x4B0);
                    D_803F7C4A = 0;
                }
            }
            D_803F7C3D = 10;
            break;

        case 0:
            func_802A04BC(0x1F, D_803F7850, ch);
            if (ch[0] == 1) {
                break;
            }
            func_802A04BC(9, D_803F7850, ch);
            if (ch[0] == 1) {
                break;
            }
            s4 = VEH_S16(0x76);
            func_802A03D4(D_803F7850, 6, (s4 < 0) ? 1 : 0);
            if (s4 < 0) {
                s4 = -s4;
            }
            s4 = (u32) s4 / 16;
            func_802A039C(D_803F7850, 6, s4);
            func_802A0480(0.5f, D_803F7850, 6, 1);
            func_802A040C(D_803F7850, 6, 0);
            func_802A0290(D_803F7850, 6, -1);
            func_802A04BC(6, D_803F7850, ch);
            /* ch[6] is the asm's t1 (the whole word; the store keeps 16 bits) */
            old = D_803F7C32;
            D_803F7C32 = ch[6];
            if (ch[6] != old) {
                if (ch[6] == 0) {
                    func_80260650(D_80367738, 0x5A, NULL);
                } else if (ch[6] == 3) {
                    func_80260650(D_80367738, 0x5B, NULL);
                }
            }
            break;

        case 2:
            D_803F7C3D = 10;
            if (D_80370C1C != 0 || D_80370C1A != 0 || D_80370C1B != 0) {
                v = func_802C6FD8();
                VEH_TRIPLE(0x40, 1);
                VEH_TRIPLE(0x28, v);
                VEH_TRIPLE(0x34, D_803F7BFC);
                D_803F7C3E = 1;
                VEH_U8(0xA1) = 1;
            }
            /* fall through */
        case 1:
        case 3:
            func_802A02E4(0x1F, D_803F7850);
            func_802A02E4(6, D_803F7850);
            func_802A0290(D_803F7850, 4, -1);
            func_802A0290(D_803F7850, 3, -1);
            func_802C7864(&s2);
            if (D_803F7C39 != 0) {
                if (VEH_U8(0x9C) == 0) {
                    v = VEH_S16(0x76);
                    if (v >= 0x15) {
                        v = 0x14;
                    }
                    if (v < -0x14) {
                        v = -0x14;
                    }
                    VEH_S16(0x76) = v;
                }
                VEH_TRIPLE(0x34, D_803F7BFC);
                VEH_TRIPLE(0x40, 1);
                VEH_TRIPLE(0x28, 0x14A);
                VEH_U8(0x96) = 1;
                VEH_U8(0x97) = 1;
                VEH_U8(0x98) = 1;
                break;
            }
            if (D_803F7C40 != 0 || D_803F7C41 != 0 || VEH_U8(0x9F) == 100) {
                break;
            }
            func_802C770C();
            if (D_803F7C44 != 3 && (D_803F7C42 != 0 || D_80370C1D == 0)) {
                break;
            }
            /* both press/release trackers done, or a new D_80370C1D press: mode 4 */
            D_803F7C45 = 0;
            D_803F7C44 = 0;
            D_803F7C47 = 0;
            D_803F7C46 = 0;
            D_803F7C40 = 20;
            D_803F7C4A = 1;
            VEH_U8(0xA1) = 4;
            VEH_TRIPLE(0x28, 0);
            VEH_TRIPLE(0x34, D_803F7BFC);
            VEH_TRIPLE(0x40, 1);
            func_802A02E4(4, D_803F7850);
            func_802A02E4(3, D_803F7850);
            func_802A039C(D_803F7850, 5, 10);
            func_802A03D4(D_803F7850, 5, 0);
            func_802A040C(D_803F7850, 5, 1);
            func_802A0360(0.0f, D_803F7850, 5, 0);
            func_802A0290(D_803F7850, 5, 1);
            func_80260650(D_80367738, 0x5C, NULL);
            break;

        default:
            /* the asm executes `syscall` here (no mode above 5 exists) */
            break;
    }

    func_802C7544(t6, t7, s0, s1, s2, s3, s4);
    func_802C6DAC();
    func_802C6ECC();
    D_803F7C43 = VEH_U8(0x96);
    D_803F7C42 = D_80370C1D;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C61F0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803F7C4B; /* idle countdown */

/* Idle check: while the vehicle moves (+0x76 != 0) or is in a mode
 * (+0xA1 != 0) the countdown D_803F7C4B is reloaded with 30, else it counts
 * down to 0. In mode 0, stopped, with the countdown at 0: mode 5, channel 6
 * stopped and channel 7 armed (0.0 / 0, 3, 0, 0.0 / 0, 1, restart with 1).
 * The asm reads the block through $gp (= D_803F7B50, set by its caller);
 * asm caller func_802C61F0 keeps a1-a3 live (a mixed N64 build would need a
 * thunk; the native port won't). */
void func_802C6DAC(void) {
    if (VEH_S16(0x76) != 0 || VEH_U8(0xA1) != 0) {
        D_803F7C4B = 30;
    } else if (D_803F7C4B != 0) {
        D_803F7C4B--;
    }
    if (VEH_U8(0xA1) != 0 || VEH_S16(0x76) != 0 || D_803F7C4B != 0) {
        return;
    }
    VEH_U8(0xA1) = 5;
    func_802A02E4(6, D_803F7850);
    func_802A0360(0.0f, D_803F7850, 7, 0);
    func_802A039C(D_803F7850, 7, 3);
    func_802A03D4(D_803F7850, 7, 0);
    func_802A0480(0.0f, D_803F7850, 7, 0);
    func_802A040C(D_803F7850, 7, 1);
    func_802A0290(D_803F7850, 7, 1);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C6DAC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Starts/stops the two looping vehicle sounds (handles D_803F7C18 and
 * D_803F7C1C) for the current mode (+0xA1) and D_803F7C3E. The survey lists
 * a1-a3 as outputs read by func_802C61F0's callers: they are only what the
 * sound calls leave behind, nothing this function sets. */
void func_802C6ECC(void) {
    s32 mode = VEH_U8(0xA1);

    if (mode == 0 || mode == 4) {
        if (D_803F7C1C != NULL) {
            func_802608C8(D_803F7C1C);
        }
    }
    if (D_803F7C3E == 0) {
        if (D_803F7C18 != NULL) {
            func_802608C8(D_803F7C18);
        }
        mode = VEH_U8(0xA1);
        if (mode == 1 || mode == 2 || mode == 3) {
            if (D_803F7C1C == NULL) {
                func_80260650(D_80367738, 0xF, &D_803F7C1C);
            }
        }
    } else {
        if (D_803F7C18 == NULL) {
            func_80260650(D_80367738, 2, &D_803F7C18);
        }
        if (D_803F7C1C != NULL) {
            func_802608C8(D_803F7C1C);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C6ECC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* With a = +0x28, n = +0x40 of the vehicle block and k = D_803EBBF4:
 * (a*n + round(k*n*n)) - (a*(n-1) + round(k*(n-1)^2)) + bonus, where bonus
 * is 30 when D_803F7BFC < D_803F7C10, else
 * 30 - (u32)((D_803F7BFC - D_803F7C10) * 30) / (u32)(D_803F7C14 - D_803F7C10).
 * Products are 32-bit; round() is cvt.w.s (nearest, ties to even).
 * The asm returns the value in s6 and also changes s7 and a1 (scratch);
 * asm caller func_802C61F0 keeps a2, a3, t6, t7, f12 and f14 live (a mixed
 * N64 build would need a thunk, the native port doesn't). */
s32 func_802C6FD8(void) {
    s32 a = VEH_S32(0x28);
    s32 n = VEH_S32(0x40);
    s32 r;
    s32 sum;
    s32 bonus;

    CVT_W_S(r, D_803EBBF4 * (f32) (n * n));
    sum = a * n + r;
    n = VEH_S32(0x40) - 1;
    a = VEH_S32(0x28);
    CVT_W_S(r, D_803EBBF4 * (f32) (n * n));
    sum -= a * n + r;
    if (D_803F7BFC < D_803F7C10) {
        bonus = 30;
    } else {
        bonus = 30 - (u32) ((D_803F7BFC - D_803F7C10) * 30) / (u32) (D_803F7C14 - D_803F7C10);
    }
    return sum + bonus;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C6FD8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef TRI_SCAN_TYPES_DEFINED
#define TRI_SCAN_TYPES_DEFINED
/* func_802AC0BC's FP and integer register results (62740.c), in and out. */
typedef struct {
    f32 pz;    /* f12 */
    f32 cross; /* f14 */
    f32 cz;    /* f20 */
    f32 side;  /* f22 */
    f32 sideZ; /* f24 */
    f32 dz;    /* f26 */
} TriSideOut;
typedef struct {
    s32 a1; /* found flag */
    s32 a3;
    s32 t6;
    s32 t7;
    s32 fp;
    s32 s1;
    s32 s2;
    s32 s3;
    s32 s4;
} TriScanRegs;
s32 func_802AC0BC(s32 x, s32 z, s32 y, TriSideOut *f, TriScanRegs *r);
#endif
extern u32 D_803F7BF8[]; /* x, y, z */
extern s32 D_803EBBFC;   /* height of func_802AC0BC's best triangle */

/* Ground check below the vehicle (D_803F7BF8 x/z, D_803F7BFC y) with
 * func_802AC0BC: when a triangle was found whose height h >= y, and h - y is
 * below 200, the vehicle snaps to mode 1 (+0xA1 = 1, triples +0x34.. = y,
 * +0x40.. = 1, +0x28.. = 30, flags +0x96..0x98 = 1, D_803F7C3E = 1); below
 * 3000 the same with +0x28.. = 0 and D_803F7C3E left alone. Returns the asm's
 * v1: 1 when it switched, else func_802AC0BC's result.
 * Register convention: func_802AC0BC's FP results (f12-f26) and scan
 * registers (a3, t6, t7, fp, s1-s4) pass through to the caller, here through
 * f and r (conventions.txt); $gp (= D_803F7B50) is read as the global. */
s32 func_802C70E8(TriSideOut *f, TriScanRegs *r) {
    s32 best = func_802AC0BC(D_803F7BF8[0], D_803F7BF8[2], D_803F7BFC, f, r);
    s32 d;
    s32 w;

    if (r->a1 == 0 || D_803EBBFC < D_803F7BFC) {
        return best;
    }
    d = D_803EBBFC - D_803F7BFC;
    if (d < 200) {
        w = 30;
    } else if (d < 3000) {
        w = 0;
    } else {
        return best;
    }
    VEH_U8(0xA1) = 1;
    VEH_S32(0x34) = D_803F7BFC;
    VEH_S32(0x38) = D_803F7BFC;
    VEH_S32(0x3C) = D_803F7BFC;
    VEH_S32(0x40) = 1;
    VEH_S32(0x44) = 1;
    VEH_S32(0x48) = 1;
    VEH_S32(0x28) = w;
    VEH_S32(0x2C) = w;
    VEH_S32(0x30) = w;
    VEH_U8(0x96) = 1;
    VEH_U8(0x97) = 1;
    VEH_U8(0x98) = 1;
    if (d < 200) {
        D_803F7C3E = 1;
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C70E8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803F7C49;

/* D_803F7C49 = 1 when func_802AC0BC at (x, z, D_803F7BFC) finds ground at
 * least 200 above D_803F7BFC, else 0.
 * Register convention: x t0, z t1; func_802AC0BC's FP results (f12-f26) are
 * left for the caller, here through f (in/out; conventions.txt). The asm
 * saves and restores every integer register; asm caller func_802C5AFC keeps
 * t0, t1 and t7 live (a mixed N64 build would need a thunk; the native port
 * won't). func_802AC0BC's scan-register inputs are whatever the caller left
 * in a3/t6/s1-s4; they only pass through, so the C hands it zeros. */
void func_802C71FC(s32 x, s32 z, TriSideOut *f) {
    TriScanRegs r;

    r.a1 = 0;
    r.a3 = 0;
    r.t6 = 0;
    r.t7 = 0;
    r.fp = 0;
    r.s1 = 0;
    r.s2 = 0;
    r.s3 = 0;
    r.s4 = 0;
    D_803F7C49 = 0;
    func_802AC0BC(x, z, D_803F7BFC, f, &r);
    if (r.a1 != 0 && D_803EBBFC >= D_803F7BFC && D_803EBBFC - D_803F7BFC >= 200) {
        D_803F7C49 = 1;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C71FC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* When +0x9F of the vehicle block is 100: outside mode 1, resets
 * D_803F7C28/D_803F7C2C to 0.5; then, if D_803F7BFC < func_80258500(9) +
 * 2500, switches to mode 1 (triples +0x34.. = D_803F7BFC, +0x40.. = 1,
 * +0x28.. = 30, flags +0x96..0x98 = 1, D_803F7C3E = 1).
 * The survey lists v1, t6, t7 as outputs read by func_802C61F0: v1 is only
 * scratch on the taken path and t6/t7 are untouched. Asm caller
 * func_802C61F0 keeps a1-a3, f12 and f14 live (a mixed N64 build would need
 * a thunk, the native port doesn't). */
void func_802C7354(void) {
    s32 limit;
    s32 cur;

    if (VEH_U8(0x9F) != 100) {
        return;
    }
    if (VEH_U8(0xA1) != 1) {
        D_803F7C28 = 0.5f;
        D_803F7C2C = 0.5f;
    }
    limit = func_80258500(9) + 2500;
    cur = D_803F7BFC;
    if (cur < limit) {
        VEH_S32(0x34) = cur;
        VEH_S32(0x38) = cur;
        VEH_S32(0x3C) = cur;
        VEH_S32(0x40) = 1;
        VEH_S32(0x44) = 1;
        VEH_S32(0x48) = 1;
        VEH_S32(0x28) = 30;
        VEH_S32(0x2C) = 30;
        VEH_S32(0x30) = 30;
        VEH_U8(0x96) = 1;
        VEH_U8(0x97) = 1;
        VEH_U8(0x98) = 1;
        VEH_U8(0xA1) = 1;
        D_803F7C3E = 1;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C7354.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Sets the pair D_8036444C / D_80364450 from where D_803F7BFC lies in
 * [D_803F7C10, D_803F7C14): (0xD48, 0x258) below it, (1000, 3000) at or
 * past the end, else interpolated (0xD48 - 2400*t, 0x258 + 2400*t with
 * t = (cur - lo) / (hi - lo), signed division). The survey lists a0-a3 as
 * outputs read by func_802C5AFC: they're scratch (a0 = D_803F7C14). */
void func_802C7410(void) {
    s32 cur = D_803F7BFC;
    s32 lo = D_803F7C10;
    s32 hi = D_803F7C14;

    if (cur < lo) {
        D_8036444C = 0xD48;
        D_80364450 = 0x258;
    } else if (cur >= hi) {
        D_8036444C = 1000;
        D_80364450 = 3000;
    } else {
        s32 num = cur - lo;
        s32 den = hi - lo;

        D_8036444C = num * -0x960 / den + 0xD48;
        D_80364450 = num * 0x960 / den + 0x258;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C7410.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef IO_802A6274_DEFINED
#define IO_802A6274_DEFINED
/* func_802A6274's in/out registers (60F60.c). */
typedef struct {
    s32 a3;
    s32 t6;
    s32 s1;
} Io802A6274;
#endif

extern u8 D_803F7C43; /* previous frame's +0x96 */
extern s8 D_803F7C3B; /* 0..50 ramp */
extern u8 D_803F7C3A; /* countdown */
extern u8 D_802C3804[]; /* definition handed to func_802A6274 */
s32 func_802584BC(u8 id);
s32 func_802ABC88(s32 id, s32 n, u8 **recOut);
s32 func_80288284(u8 type, s32 x, s32 y, s32 z, s32 floor);
s32 func_802A6274(Io802A6274 *io, u8 *def, s32 data, s32 type, s32 x, s32 y, s32 z, s32 w24, s32 w28,
                  s32 w18, s32 w1C, s32 w2C, s32 b35);

/* When +0x96 just became 1 (D_803F7C43 == 0) with D_803F7C3E set:
 * func_80288284(4, record (9, 1) of func_802ABC88, func_802584BC(9)). Then
 * the ramp D_803F7C3B steps up (to 50) in mode 5 or with D_803F7C3E set,
 * else down (to 0), and channel 1 of D_803F7850 gets ramp / 4 (divu, as the
 * asm). The countdown D_803F7C3A, when nonzero, just counts down; at zero it
 * restarts at 4 in mode 5 (1 with D_803F7C3E set, else nothing more happens)
 * and two func_802A6274 records are set up (def D_802C3804, data 0x15F90,
 * type 1 at (9, 1, 1) and (9, 2, 1)).
 * Register convention: the asm passes t6, t7, s0-s4 through to
 * func_802A6274 (t6 and s1 in its in/out block; s3 is func_802C7C1C's 270);
 * $gp (= D_803F7B50) is read as the global. It leaves func_802A6274's a3,
 * t6 and s1 and changes s5-s7 (conventions.txt: clobbers). */
void func_802C7544(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4) {
    Io802A6274 io;
    u8 *rec;
    s32 floor;
    s32 v;

    if (D_803F7C43 == 0 && VEH_U8(0x96) == 1 && D_803F7C3E != 0) {
        floor = func_802584BC(9);
        func_802ABC88(9, 1, &rec);
        func_80288284(4, ((s32 *) rec)[0], ((s32 *) rec)[1], ((s32 *) rec)[2], floor);
    }
    if (VEH_U8(0xA1) == 5 || D_803F7C3E != 0) {
        v = D_803F7C3B + 1;
        if (v >= 0x33) {
            v = 0x32;
        }
    } else {
        v = D_803F7C3B - 1;
        if (v < 0) {
            v = 0;
        }
    }
    D_803F7C3B = v;
    func_802A039C(D_803F7850, 1, (u32) (s32) D_803F7C3B / 4);
    if (D_803F7C3A != 0) {
        D_803F7C3A--;
        return;
    }
    if (VEH_U8(0xA1) == 5) {
        D_803F7C3A = 4;
    } else if (D_803F7C3E != 0) {
        D_803F7C3A = 1;
    } else {
        return;
    }
    io.a3 = 1;
    io.t6 = t6;
    io.s1 = s1;
    func_802A6274(&io, D_802C3804, 0x15F90, 1, 9, 1, 1, t7, s0, s2, s3, s4, 1);
    io.a3 = 1;
    func_802A6274(&io, D_802C3804, 0x15F90, 1, 9, 2, 1, t7, s0, s2, s3, s4, 1);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C7544.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803F7C44; /* phase 0 (odd = waiting for input 0 to be released) */
extern u8 D_803F7C45; /* timer 0 */
extern u8 D_803F7C46; /* phase 1 */
extern u8 D_803F7C47; /* timer 1 */
extern u8 D_80370C1A; /* input 0 */
extern u8 D_80370C1B; /* input 1 */

/* One step of a press/release tracker: when the timer has run out the phase
 * is reset; otherwise the phase advances (and the timer restarts at 4) once
 * the input reaches the state the phase is waiting for (pressed on even
 * phases, released on odd ones), and the timer counts down while it hasn't.
 * (A macro, not a function: the original is a single asm leaf.) */
#define PRESS_TRACKER_STEP(phase, timer, input)                     \
    {                                                               \
        if ((timer) == 0) {                                         \
            (phase) = 0;                                            \
            (timer) = 4;                                            \
        } else if (((phase) & 1) ? ((input) == 0) : ((input) != 0)) { \
            (timer) = 4;                                            \
            (phase)++;                                              \
        } else {                                                    \
            (timer)--;                                              \
        }                                                           \
    }

/* Update the two press/release trackers (D_803F7C44/45 on D_80370C1A and
 * D_803F7C46/47 on D_80370C1B).
 * Register note: the asm touches only at/v0/v1/a0; its caller func_802C61F0
 * keeps a1-a3, t6, t7, f12 and f14 live across the call (a mixed N64 build
 * would need a thunk, the native port does not). */
void func_802C770C(void) {
    PRESS_TRACKER_STEP(D_803F7C44, D_803F7C45, D_80370C1A);
    PRESS_TRACKER_STEP(D_803F7C46, D_803F7C47, D_80370C1B);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C770C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s16 D_803F7C34;
extern f32 D_8030D954;
extern f32 D_8030D958;
extern f32 D_8030D95C;
extern f32 D_8030D960;
extern f32 D_8030D964;
f32 func_802C7BC0(void);
f32 func_802C7C1C(s32 up);

/* Moves f towards 0.5 by step without passing it (NaN goes the "above"
 * way, as the asm's compares do). */
#define CENTER_STEP(f, step)          \
    {                                 \
        if ((f) < 0.5f) {             \
            (f) += (step);            \
            if (!((f) <= 0.5f)) {     \
                (f) = 0.5f;           \
            }                         \
        } else {                      \
            (f) -= (step);            \
            if ((f) < 0.5f) {         \
                (f) = 0.5f;           \
            }                         \
        }                             \
    }

/* Steering / tilt animation (0.5 = centred). D_803F7C2C: while steering
 * (D_80370C35 set or |D_803F7C34| large: >= 4000 or < -3999) with
 * D_80370C2C >= 31 it rises by D_8030D958 up to func_802C7C1C(1), with
 * D_80370C2C < -30 it falls by D_8030D954 down to func_802C7C1C(0) (the up
 * flag is left in *up, the asm's s2); otherwise it steps back to 0.5 by
 * D_8030D95C. Channel 4 of D_803F7850 then gets 1 - it as (2x, 1) below
 * 0.5 or (2(x - 0.5), 2) above. D_803F7C28 likewise with step
 * D_8030D960 * (1 - 2|x - 0.5|): D_80370C2D > 0 raises it up to
 * 1 - func_802C7BC0() (if not already above), < 0 lowers it down to
 * func_802C7BC0() (if not already below), else (or when outside) it centres
 * by D_8030D964; channel 3 gets it in the same two-range form.
 * The asm also computes round(100 * (1 - D_803F7C2C)) into t2 and drops it,
 * clobbers s5, and keeps func_802C7C1C's s3 = 270 (conventions.txt:
 * clobbers s3, s5). Asm caller func_802C61F0 keeps a1-a3, t6, t7, f12, f14
 * live (a mixed N64 build would need a thunk; the native port won't). */
void func_802C7864(s32 *up) {
    f32 f = D_803F7C2C;
    f32 lim;
    f32 step;
    f32 w;
    s32 s;

    if (D_80370C35 != 0 || D_803F7C34 >= 0xFA0 || D_803F7C34 < -0xF9F) {
        s = D_80370C2C;
        if (s >= 0x1F) {
            *up = 1;
            f += D_8030D958;
            lim = func_802C7C1C(1);
            if (!(f <= lim)) {
                f = lim;
            }
            goto store1;
        }
        if (s < -0x1E) {
            *up = 0;
            f -= D_8030D954;
            lim = func_802C7C1C(0);
            if (f < lim) {
                f = lim;
            }
            goto store1;
        }
    }
    step = D_8030D95C;
    CENTER_STEP(f, step);
store1:
    D_803F7C2C = f;
    f = 1.0f - f;
    if (f <= 0.5f) {
        func_802A0360(f * 2.0f, D_803F7850, 4, 1);
    } else {
        func_802A0360((f - 0.5f) * 2.0f, D_803F7850, 4, 2);
    }

    f = D_803F7C28;
    if (f <= 0.5f) {
        w = 0.5f - (0.5f - f);
    } else {
        w = 0.5f - (f - 0.5f);
    }
    step = D_8030D960 * (w * 2.0f);
    s = D_80370C2D;
    if (s > 0) {
        lim = 1.0f - func_802C7BC0();
        if (f <= lim) {
            f += step;
            if (!(f <= lim)) {
                f = lim;
            }
            goto store2;
        }
    } else if (s < 0) {
        lim = func_802C7BC0();
        if (!(f < lim)) {
            f -= step;
            if (f < lim) {
                f = lim;
            }
            goto store2;
        }
    }
    step = D_8030D964;
    CENTER_STEP(f, step);
store2:
    D_803F7C28 = f;
    if (f <= 0.5f) {
        func_802A0360(f * 2.0f, D_803F7850, 3, 1);
    } else {
        func_802A0360((f - 0.5f) * 2.0f, D_803F7850, 3, 2);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C7864.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* (1 - |D_80370C2D| / 80) / 2. The asm returns it in f4 (and changes f6);
 * asm caller func_802C7864 keeps f2 live (a mixed N64 build would need a
 * thunk, the native port doesn't). */
f32 func_802C7BC0(void) {
    s32 v = D_80370C2D;

    if (v < 0) {
        v = -v;
    }
    return (1.0f - (f32) v / 80.0f) / 2.0f;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C7BC0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* f = |+0x76 of the vehicle block| / 270 * 0.5, then 0.5 + f if `up`
 * else 0.5 - f, capped at D_8030D968 (-> D_8030D96C) and floored at
 * D_8030D970 (-> D_8030D974). The asm takes `up` in s2 and returns the value
 * in f2; it also leaves s3 = 270 (scratch, not read by its caller
 * func_802C7864). */
f32 func_802C7C1C(s32 up) {
    s32 v = VEH_S16(0x76);
    f32 f;

    if (v < 0) {
        v = -v;
    }
    f = (f32) v / 270.0f * 0.5f;
    if (up != 0) {
        f = f + 0.5f;
    } else {
        f = 0.5f - f;
    }
    if (!(f <= D_8030D968)) {
        f = D_8030D96C;
    }
    if (f < D_8030D970) {
        f = D_8030D974;
    }
    return f;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C7C1C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef MTX_CHAIN_REGS_DEFINED
#define MTX_CHAIN_REGS_DEFINED
/* func_802AA890's extra registers (62740.c), as func_8029C454 and
 * func_802ABBEC pass them through. */
typedef struct {
    s32 v1; /* out */
    s32 a0; /* out */
    s32 a3; /* in/out */
    s32 s1; /* in/out */
    s32 s2; /* in/out */
    s32 s0; /* in */
} MtxChainRegs;
#endif
extern u8 *D_803F7C04; /* this vehicle's model: +0 / +4 / +8 offsets of the point lists, +0x18 the matrix */
extern u8 D_8035805C;  /* which of the save copy pair is current */
extern s16 D_803ED390[]; /* rotation angles x, y, z for func_802AA764 */
void func_8029C454(s32 x, s32 y, s32 z, s32 tag, u8 *p, u8 *end, u8 *base, MtxChainRegs *regs);
void func_802AA764(s32 x, s32 y, s32 z, s32 scale, s32 *m);
void func_802ABBEC(s32 id, s16 *p, s16 *end, u8 *base, MtxChainRegs *regs);

/* Place this vehicle's model (vehicle 9): builds its matrix (scale 0x4268,
 * angles (0, +0x4C of the block, 0)) at the model's +0x18 entry inside the
 * current copy (D_8035805C ? D_803F7C08 : D_803F7C0C) with func_802AA764,
 * then, except in mode 5 (+0xA1), places the parts (func_8029C454(x, y, z,
 * 9, model + [4], model + [8], copy)), and always the points
 * (func_802ABBEC(9, model + [0], model + [4], copy)).
 * Register convention (conventions.txt): s0 and s1 come in for the
 * func_802AA890 chain, whose registers (v1, a0, a3, s1, s2) go out; regs
 * carries them. func_802AA764 leaves a3 = 0 (func_802ACCCC's loop counter)
 * and s2 = the matrix, which the asm then hands on: modelled explicitly.
 * v1/a0 going into func_802ABBEC are y and z (func_8029C454 restores them).
 * The asm reads the block through $gp (= D_803F7B50, set by every caller)
 * and leaves t0 = 9 and t2 = the point list end. Its cos/sin f12/f14
 * temporaries (survey: read by func_802C5AFC) are not modelled (see
 * func_802AA764). Asm caller func_802C5AFC keeps t6, t7 live (a mixed N64
 * build would need a thunk; the native port won't). */
void func_802C7CB0(MtxChainRegs *regs) {
    u8 *mdl = D_803F7C04;
    u8 *base;
    s32 *m;
    s32 x;
    s32 y;
    s32 z;

    m = (s32 *) (*(u32 *) (mdl + *(s32 *) (mdl + 0x18) + 4) +
                 (u32) (D_8035805C != 0 ? (u8 *) D_803F7C08 : (u8 *) D_803F7C0C));
    D_803ED390[0] = 0;
    D_803ED390[2] = 0;
    D_803ED390[1] = *(u16 *) (D_803F7B50 + 0x4C);
    func_802AA764(D_803F7BF8[0], D_803F7BF8[1], D_803F7BF8[2], 0x4268, m);
    base = D_8035805C != 0 ? (u8 *) D_803F7C08 : (u8 *) D_803F7C0C;
    x = D_803F7BF8[0];
    y = D_803F7BF8[1];
    z = D_803F7BF8[2];
    regs->a3 = 0;
    regs->s2 = (s32) m;
    if (VEH_U8(0xA1) != 5) {
        mdl = D_803F7C04;
        func_8029C454(x, y, z, 9, mdl + *(s32 *) (mdl + 4), mdl + *(s32 *) (mdl + 8), base, regs);
    }
    regs->v1 = y;
    regs->a0 = z;
    mdl = D_803F7C04;
    func_802ABBEC(9, (s16 *) (mdl + *(s32 *) (mdl + 0)), (s16 *) (mdl + *(s32 *) (mdl + 4)), base, regs);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C7CB0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Modes 0 and 5 (+0xA1): returns round(+0x76 / 1.5) (cvt.w.s, nearest).
 * Modes 1-4: if |D_80370C2C| is large (>= 51 or < -50) D_803F7C3C climbs by
 * 2 up to 100, else it is reset to 20; returns the new D_803F7C3C. Any other
 * mode hits a `syscall` in the asm (a deliberate crash) and, if that
 * returned, would continue as modes 1-4, which is what this does.
 * The asm returns the value in s3; asm caller func_802C5AFC keeps a0-a3
 * live (a mixed N64 build would need a thunk, the native port doesn't). */
s32 func_802C7DFC(void) {
    s32 mode = VEH_U8(0xA1);
    s32 v;
    s32 r;

    if (mode == 0 || mode == 5) {
        CVT_W_S(r, (f32) VEH_S16(0x76) / 1.5f);
        return r;
    }
    v = D_80370C2C;
    if (v >= 0x33 || v < -0x32) {
        r = D_803F7C3C + 2;
        if (r >= 0x65) {
            r = 100;
        }
        D_803F7C3C = r;
        return r;
    }
    D_803F7C3C = 20;
    return 20;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C7DFC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* D_803EBBF4 = D_803EBBF0, times 4 in mode 2. Asm callers keep registers
 * live across the call (func_802C5A14 t3, t4, f12, f14; func_802C5AFC
 * a0-a3, t6, f12, f14): a mixed N64 build would need a thunk. */
void func_802C7ECC(void) {
    f32 k;

    if (VEH_U8(0xA1) == 2) {
        k = 4.0f;
    } else {
        k = 1.0f;
    }
    D_803EBBF4 = D_803EBBF0 * k;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C7ECC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Sets D_803ED3F6 / D_803ED3F7 to 0xFF and loads the 15 s16 tuning values at
 * +0x78 of the vehicle block plus D_803F7C48 for the mode (+0xA1 nonzero or
 * zero). Asm callers keep registers live across the call (func_802C5A14 t3,
 * t4, f12, f14; func_802C5AFC a0-a3): a mixed N64 build would need a thunk. */
void func_802C7F28(void) {
    static const s16 sActive[15] = { -200, 0, 2, 0, 250, 2, 0, 250, 2, 0, 250, 2, 0, 250, 2 };
    static const s16 sIdle[15] = { -100, 0, 4, 0, 150, 4, 0, 150, 4, 0, 150, 4, 0, 150, 4 };
    const s16 *src;
    s32 i;

    D_803ED3F6 = 0xFF;
    D_803ED3F7 = 0xFF;
    if (VEH_U8(0xA1) != 0) {
        src = sActive;
    } else {
        src = sIdle;
    }
    for (i = 0; i < 15; i++) {
        VEH_S16(0x78 + i * 2) = src[i];
    }
    D_803F7C48 = (VEH_U8(0xA1) != 0) ? 4 : 12;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C7F28.s")
#endif

/* func_802C8074: two-address trampoline into func_802AC7DC, same
 * confirmed-unreachable-from-C 8-byte sd-$ra frame as func_802AC284
 * (hd_code/679E0.c). Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803F7B50[];  /* this vehicle's state block: 0xA6 bytes are (de)serialized */
extern u32 D_803F7BF8[]; /* plus these three words */
s32 func_802AC7DC(u8 *dst, u8 *src, u32 *words);
void func_802AC85C(u8 *src, u8 *dst, u32 *words);

/* Serialize this vehicle's state (D_803F7B50[0..0xA5] plus the three words
 * D_803F7BF8[0..2]) into `dst` via func_802AC7DC. Returns the callee's result
 * (0xB2, the byte count): the asm leaves the callee's v0 untouched. */
s32 func_802C8074(u8 *dst) {
    return func_802AC7DC(dst, D_803F7B50, D_803F7BF8);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C8074.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Inverse of func_802C8074: restore this vehicle's state from `src` via
 * func_802AC85C. Called from 17210.c as `void func_802C80A0(void *)`. */
void func_802C80A0(void *src) {
    func_802AC85C(src, D_803F7B50, D_803F7BF8);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C80A0.s")
#endif
