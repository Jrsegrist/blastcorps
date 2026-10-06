#include "common.h"
#include <ultra64.h>
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#ifdef NON_MATCHING
#define D_80358070 (*(u32 *) &D_80358070)
#endif
/* end of views */

/* FILE-WIDE FINDING: this file's functions save $ra via the 64-bit `sd`/`ld`
 * doubleword form, not the normal 32-bit `sw`/`lw` pair - the same signature
 * documented in full in hd_code/77E20.c's top-of-file note and the project
 * skill file, confirming (with no exceptions found) that this file is part
 * of the same large hand-written-assembly module. Any pragma below without
 * a more specific comment follows this convention; a few have their own
 * more specific non-ABI explanation where one was already worked out. */
/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 *D_803FCD54;  /* model header */
extern u8 *D_803FCD58;  /* the two 0x800-byte save copies */
extern u8 *D_803FCD5C;
extern u8 D_803FCCA0[]; /* this vehicle's state block */
extern u8 D_803FC9A0[]; /* channel table */
extern void *D_803FCD64; /* sound state */
extern u8 D_803FCD72;
extern u8 D_803FCD73;
extern u8 D_803FCD74;
extern u8 D_803FCD76;
extern u8 D_803FCD77;
extern u8 D_803FCD78;
extern u8 D_803FCD79;
extern u8 D_803FCD7A;

/* Set up vehicle 0xFD (the level-object dispatcher func_802A30DC): model
 * header hdr -> D_803FCD54; two 0x800-byte save copies from the D_80358070
 * bump pointer (D_803FCD58/5C; trapping adds); func_802A1388(0xFD, 0,
 * copies, hdr); heading/target/speed of D_803FCCA0 and the event flags
 * D_803FCD72..7A (but 75) cleared; model channels D_803FC9A0 (func_8029F85C,
 * channel 0 = (100, 0, 0, 0.0, 1), run into both copies); channel 2's sound
 * part: mode D_803FCD75 1 -> func_802A0360(0.0, 2, 0), 0 -> start sound 0x6A
 * into D_803FCD64 at volume 0 (func_80260650, func_80260AB8(.., 8, 0)) and
 * func_802A0360(f0, 2, 1); then channels 2 = (0, 0, 1, -1) and 1 = (0, 0,
 * 0360 0.0, 1, -1); func_80258230(0xFD, 120, 45, 45), func_80268F54, one
 * update with +0x9A set (func_802D291C), and the model matrix copy (word +4
 * of the entry at hdr + hdr[6], D_803FCD5C -> D_803FCD58, func_802AA838).
 * Register notes: hdr comes in s2 (conventions.txt); the asm saves t0-t5
 * (asm caller func_802A30DC keeps t2), points $gp at D_803FCCA0 and leaves
 * it, and leaves the channel routines' s0-s5, fp, f20 and func_802D291C's
 * f12/f14 (survey: read by func_802A30DC; not modelled). In mode 0 the asm
 * hands func_802A0360 whatever f0 func_80260AB8 left (the mtc1 zero before
 * the sound calls; those IDO sound routines do no FP work): 0.0 here. Any
 * other mode hits the asm's `syscall` debug trap and then runs mode 1, as
 * here. */
void func_802D2570(u8 *hdr) {
    u32 p;
    u8 *h;
    void *snd;

    D_803FCD54 = hdr;
    p = D_80358070;
    D_803FCD58 = (u8 *) p;
    D_803FCD5C = (u8 *) (p + 0x800);
    D_80358070 = p + 0x1000;
    func_802A1388(0xFD, 0, (s32) D_803FCD58, (s32) D_803FCD5C, hdr);
    *(s16 *) (D_803FCCA0 + 0x4C) = 0;
    *(s16 *) (D_803FCCA0 + 0x4E) = 0;
    *(s16 *) (D_803FCCA0 + 0x76) = 0;
    D_803FCD72 = 0;
    D_803FCD73 = 0;
    D_803FCD74 = 0;
    D_803FCD76 = 0;
    D_803FCD77 = 0;
    D_803FCD7A = 0;
    D_803FCD78 = 0;
    D_803FCD79 = 0;
    func_8029F85C((u32 *) D_803FCD5C, (u32 *) D_803FCD58, (Unk8029DEA0Entry *) D_803FC9A0, D_803FCD54);
    func_802A039C((Unk8029DEA0Entry *) D_803FC9A0, 0, 100);
    func_802A03D4((Unk8029DEA0Entry *) D_803FC9A0, 0, 0);
    func_802A040C((Unk8029DEA0Entry *) D_803FC9A0, 0, 0);
    func_802A0480(0.0f, (Unk8029DEA0Entry *) D_803FC9A0, 0, 0);
    func_802A0290((Unk8029DEA0Entry *) D_803FC9A0, 0, 1);
    func_8029E558(D_803FCD58, D_803FCD5C, (Unk8029DEA0Entry *) D_803FC9A0);
    func_802A0320(0, (Unk8029DEA0Entry *) D_803FC9A0);
    func_802A0290((Unk8029DEA0Entry *) D_803FC9A0, 0, 1);
    func_8029E558(D_803FCD5C, D_803FCD58, (Unk8029DEA0Entry *) D_803FC9A0);
    if (D_803FCD75 == 0) {
        snd = func_80260650(D_80367738, 0x6A, &D_803FCD64);
        func_80260AB8(snd, 8, 0);
        func_802A0360(0.0f, (Unk8029DEA0Entry *) D_803FC9A0, 2, 1);
    } else {
        func_802A0360(0.0f, (Unk8029DEA0Entry *) D_803FC9A0, 2, 0);
    }
    func_802A039C((Unk8029DEA0Entry *) D_803FC9A0, 2, 0);
    func_802A03D4((Unk8029DEA0Entry *) D_803FC9A0, 2, 0);
    func_802A040C((Unk8029DEA0Entry *) D_803FC9A0, 2, 1);
    func_802A0290((Unk8029DEA0Entry *) D_803FC9A0, 2, -1);
    func_802A039C((Unk8029DEA0Entry *) D_803FC9A0, 1, 0);
    func_802A03D4((Unk8029DEA0Entry *) D_803FC9A0, 1, 0);
    func_802A0360(0.0f, (Unk8029DEA0Entry *) D_803FC9A0, 1, 0);
    func_802A040C((Unk8029DEA0Entry *) D_803FC9A0, 1, 1);
    func_802A0290((Unk8029DEA0Entry *) D_803FC9A0, 1, -1);
    func_80258230(0xFD, 120, 45, 45);
    func_80268F54();
    D_803FCCA0[0x9A] = 1;
    func_802D291C();
    D_803FCCA0[0x9A] = 0;
    h = D_803FCD54;
    h += *(s32 *) (h + 0x18);
    func_802AA838(D_803FCD5C, D_803FCD58, *(s32 *) (h + 4));
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8DDB0/func_802D2570.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
void func_802D2A40(void);
void func_802D2A74(void);
void func_802D2C20(void);
void func_802D2FA4(void);
extern u8 *D_803FCD58; /* the two save copies */
extern u8 *D_803FCD5C;
extern u8 D_803FC9A0[]; /* channel table */
extern u8 D_803FCCA0[]; /* this vehicle's state block */

/* Per-frame update of this vehicle (D_803FCCA0, the asm's $gp): func_802D2A40;
 * unless byte +0x9A is set, the sound distance (func_802D2A74) and the events
 * (func_802D2C20); then the channels D_803FC9A0 (func_8029E558 with the
 * current save copy first: D_803FCD58 when D_8035805C is set, else
 * D_803FCD5C) and the model placement (func_802D2FA4). The asm saves every
 * callee-saved register. It leaves func_802D2FA4's f12/f14 (func_802AA764's
 * trig temporaries), which the survey lists as read by asm caller
 * func_802D2570 (not modelled; its C rewrite doesn't read them). */
void func_802D291C(void) {
    func_802D2A40();
    if (D_803FCCA0[0x9A] == 0) {
        func_802D2A74();
        func_802D2C20();
    }
    if (D_8035805C != 0) {
        func_8029E558(D_803FCD58, D_803FCD5C, (Unk8029DEA0Entry *) D_803FC9A0);
    } else {
        func_8029E558(D_803FCD5C, D_803FCD58, (Unk8029DEA0Entry *) D_803FC9A0);
    }
    func_802D2FA4();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8DDB0/func_802D291C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

/* Calls func_80269258, saving gp around it (the asm caller func_802D291C
 * uses gp as its base pointer). The survey lists a0, a2, a3, t6, t7, f12 and
 * f14 as read by that caller afterwards; they're whatever func_80269258
 * leaves, nothing this function sets. */
void func_802D2A40(void) {
    func_80269258();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8DDB0/func_802D2A40.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
double sqrt(double);
#pragma intrinsic(sqrt)

/* func_802ABCDC (62740; points in t3-t5 and t6,t7,s0, result in s1) can't be
 * called from C: this is its logic (same macro as in 77E20.c). The distance
 * between two points, rounded to nearest (ties to even) as cvt.l.d does; the
 * squares are summed as s64 and converted as hi * 2^32 + lo (one rounding,
 * like cvt.d.l; a plain cast would call __ll_to_d, which the harness can't run). */
#define DIST3_802ABCDC(out, ax, ay, az, bx, by, bz)                     \
    do {                                                                \
        s64 _dx = (s32) ((bx) - (ax));                                  \
        s64 _dy = (s32) ((by) - (ay));                                  \
        s64 _dz = (s32) ((bz) - (az));                                  \
        s64 _sq = _dx * _dx + _dy * _dy + _dz * _dz;                    \
        f64 _d = sqrt((f64) (s32) (_sq >> 32) * 4294967296.0 + (f64) (u32) _sq); \
        s32 _r = (s32) _d;                                              \
        f64 _f = _d - _r;                                               \
                                                                        \
        if (_f > 0.5 || (_f == 0.5 && (_r & 1))) {                      \
            _r++;                                                       \
        }                                                               \
        (out) = _r;                                                     \
    } while (0)

extern void *D_803FCD64; /* sound state, NULL = none */

/* Distance attenuation: if D_803FCD64 is set, sets its volume (parameter 8)
 * to 0x7FFF - max(d - 0x3200, 0) / 4, clamped at 0, where d is the distance
 * from (D_803643F8, D_803643FC, D_80364400) >> 11 to D_803FCD48/4C/50.
 * The asm's dead `sub` (x - D_803FCD48) and its `addi` trap on overflow; C
 * doesn't. Register note: the asm saves and restores every register; asm
 * caller func_802D291C keeps a0, a2, a3, t6, t7, f12 and f14 live across the
 * call (a mixed N64 build would need a thunk; the native port doesn't). */
void func_802D2A74(void) {
    s32 dist;
    s32 vol;

    if (D_803FCD64 == NULL) {
        return;
    }
    DIST3_802ABCDC(dist, D_803643F8 >> 11, D_803643FC >> 11, D_80364400 >> 11,
                   D_803FCD48, D_803FCD4C, D_803FCD50);
    dist -= 0x3200;
    if (dist < 0) {
        dist = 0;
    }
    vol = 0x7FFF - (dist >> 2);
    if (vol < 0) {
        vol = 0;
    }
    func_80260AB8(D_803FCD64, 8, vol);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8DDB0/func_802D2A74.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING


extern u8 D_803FC9A0[];
extern u8 D_803FCD72;
extern u8 D_803FCD73;
extern u8 D_803FCD74;
extern u8 D_803FCD76;
extern u8 D_803FCD77;
extern u8 D_803FCD78;
extern u8 D_803FCD79;
extern u8 D_803FCD7A;

/* Spawns effect def at D_803EBC10 record (0xFD, n) (func_802A6274 type 1 with
 * z = 1, tag 0, b35 0). For that type func_802A6274 ignores its other fields;
 * the asm passes whatever its caller left in t6, t7, s0-s4 there, the C 0. */
#define SPAWN_AT_FD(def, data, n)                                         \
    {                                                                     \
        Io802A6274 _io;                                                   \
                                                                          \
        _io.a3 = 0;                                                       \
        _io.t6 = 0;                                                       \
        _io.s1 = 0;                                                       \
        func_802A6274(&_io, (def), (data), 1, 0xFD, (n), 1, 0, 0, 0, 0, 0, 0); \
    }

/* Per-frame event handling for this vehicle (id 0xFD in D_803EBC10): event
 * D_803FCD70 2 sets D_803FC9A0 entry 1's fields (func_802A039C/03D4/0290)
 * and plays sound 0x66; 3 plays 0x78; 0xB sets D_803FCD77/7A/78 and plays
 * 0x83; 0xC plays 0x7F into D_803FCD64 and clears D_803FCD7A; 0xD clears
 * D_803FCD78. Unless D_8036B964, each of the three one-shot flags
 * D_803FCD72..74 still clear plays sound 6, 6, 8 (and gets set) once
 * D_803FCD60 (read once) reaches word +4 of D_803EBC10 record (0xFD, 1..3).
 * D_803FCD7A forces D_802E8BE4 = 20 and D_802E8BE8 = 400. D_803FCD78 and
 * D_803FCD77 each run a 5-frame countdown (D_803FCD79 / D_803FCD76) that
 * spawns effects at the vehicle's points 4 (D_802C28E4) or 4, 5, 6
 * (D_802C3804). Only caller func_802D291C (asm); the survey's t6/t7/s0/s1
 * "outputs" are what func_8029E558 sets after it, not read from here. */
void func_802D2C20(void) {
    u8 *rec;
    s32 level;

    switch (D_803FCD70) {
        case 2:
            func_802A039C((Unk8029DEA0Entry *) D_803FC9A0, 1, 1);
            func_802A03D4((Unk8029DEA0Entry *) D_803FC9A0, 1, 0);
            func_802A0290((Unk8029DEA0Entry *) D_803FC9A0, 1, 1);
            func_80260650(D_80367738, 0x66, NULL);
            break;
        case 3:
            func_80260650(D_80367738, 0x78, NULL);
            break;
        case 0xB:
            D_803FCD77 = 1;
            D_803FCD7A = 1;
            D_803FCD78 = 1;
            func_80260650(D_80367738, 0x83, NULL);
            break;
        case 0xC:
            func_80260650(D_80367738, 0x7F, &D_803FCD64);
            D_803FCD7A = 0;
            break;
        case 0xD:
            D_803FCD78 = 0;
            break;
    }

    if (D_8036B964 == 0) {
        level = D_803FCD60;
        if (D_803FCD72 == 0) {
            func_802ABC88(0xFD, 1, &rec);
            if (!(level < ((s32 *) rec)[1])) {
                func_80260650(D_80367738, 6, NULL);
                D_803FCD72 = 1;
            }
        }
        if (D_803FCD73 == 0) {
            func_802ABC88(0xFD, 2, &rec);
            if (!(level < ((s32 *) rec)[1])) {
                func_80260650(D_80367738, 6, NULL);
                D_803FCD73 = 1;
            }
        }
        if (D_803FCD74 == 0) {
            func_802ABC88(0xFD, 3, &rec);
            if (!(level < ((s32 *) rec)[1])) {
                func_80260650(D_80367738, 8, NULL);
                D_803FCD74 = 1;
            }
        }
    }

    if (D_803FCD7A != 0) {
        D_802E8BE4 = 20;
        D_802E8BE8 = 400;
    }
    if (D_803FCD78 != 0) {
        if (D_803FCD79 != 0) {
            D_803FCD79 -= 1;
        } else {
            D_803FCD79 = 4;
            SPAWN_AT_FD(D_802C28E4, 0x107AC0, 4);
        }
    }
    if (D_803FCD77 != 0) {
        if (D_803FCD76 != 0) {
            D_803FCD76 -= 1;
        } else {
            D_803FCD76 = 4;
            SPAWN_AT_FD(D_802C3804, 0x53020, 4);
            SPAWN_AT_FD(D_802C3804, 0x53020, 5);
            SPAWN_AT_FD(D_802C3804, 0x5CC60, 6);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8DDB0/func_802D2C20.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 *D_803FCD54;    /* model header: word offsets to the part lists */
extern u8 *D_803FCD58;    /* the two save copies */
extern u8 *D_803FCD5C;
extern u8 D_803FCCA0[];   /* this vehicle's state block (the asm caller's $gp) */

/* Place this vehicle's model: m = the word at +4 of the header entry at
 * hdr + hdr[6] (hdr = D_803FCD54), plus the current save copy (D_803FCD58
 * when D_8035805C is set, else D_803FCD5C); rotation D_803ED390/92/94 =
 * D_803FCD6A/6C/6E (6C also to the heading at +0x4C of D_803FCCA0 and to
 * D_803FCD68); func_802AA764(position D_803FCD48..50, scale 0x11558, m).
 * Then the points (func_802ABBEC, id 0xFD, hdr + hdr[0] .. hdr + hdr[1])
 * relative to the current save copy. The header offsets are added with
 * trapping adds.
 * Register notes: the asm's $gp (= D_803FCCA0, set by its only caller
 * func_802D291C) is read as the global. It hands func_802ABBEC whatever v1,
 * a0, a3, s0, s1 held (they only matter for a point entry with no matrices;
 * 0 here) and s2 = m, and leaves func_802AA764's f12/f14, which the survey
 * lists as read by func_802D291C's callers (not modelled). Clobbers s1, s2,
 * s4-s7 (conventions.txt). */
void func_802D2FA4(void) {
    MtxChainRegs regs;
    u8 *hdr = D_803FCD54;
    u8 *base;
    s32 *m;

    m = (s32 *) (*(s32 *) (hdr + *(s32 *) (hdr + 0x18) + 4) + (s32) (D_8035805C ? D_803FCD58 : D_803FCD5C));
    D_803ED390[0] = ((u16) D_803FCD6A);
    *(u16 *) (D_803FCCA0 + 0x4C) = ((u16) D_803FCD6C);
    D_803FCD68 = ((u16) D_803FCD6C);
    D_803ED390[1] = ((u16) D_803FCD6C);
    D_803ED390[2] = ((u16) D_803FCD6E);
    func_802AA764(D_803FCD48, D_803FCD4C, D_803FCD50, 0x11558, m);
    base = D_8035805C ? D_803FCD58 : D_803FCD5C;
    hdr = D_803FCD54;
    regs.v1 = 0;
    regs.a0 = 0;
    regs.a3 = 0;
    regs.s1 = 0;
    regs.s2 = (s32) m;
    regs.s0 = 0;
    func_802ABBEC(0xFD, (s16 *) (hdr + *(s32 *) hdr), (s16 *) (hdr + *(s32 *) (hdr + 4)), base, &regs);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8DDB0/func_802D2FA4.s")
#endif

#ifdef NON_MATCHING
/* The tables of the 8E910 bin (ROM 0x8E910-0x8F860, 0x802D30D0-0x802D4020),
 * right after this file's code: the per-level part lists picked by
 * func_802A1EC8 (5CB60.c) for 20 levels (s32 count + LevelPart records; the
 * entry words are filled in at level load by func_802A21AC).
 * The NON_MATCHING build defines it in C and links this file's .data (it has
 * nothing else) back at the table's original address (Makefile NM_PIN_HD_CODE,
 * nm_ldscript.py --pin-object); nm_symaudit.py checks the bytes against the
 * bin. Code reaches it through the absolute D_ symbols. */
typedef struct {
    /* 802D30D0 */ s32 lp802D30D0; /* D_802D30D0: count */
    /* 802D30D4 */ LevelPart lp802D30D0_parts[8];
    /* 802D3194 */ s32 lp802D3194; /* D_802D3194: count */
    /* 802D3198 */ LevelPart lp802D3194_parts[11];
    /* 802D32A0 */ s32 lp802D32A0; /* D_802D32A0: count */
    /* 802D32A4 */ LevelPart lp802D32A0_parts[5];
    /* 802D331C */ s32 lp802D331C; /* D_802D331C: count */
    /* 802D3320 */ LevelPart lp802D331C_parts[7];
    /* 802D33C8 */ s32 lp802D33C8; /* D_802D33C8: count */
    /* 802D33CC */ LevelPart lp802D33C8_parts[5];
    /* 802D3444 */ s32 lp802D3444; /* D_802D3444: count */
    /* 802D3448 */ LevelPart lp802D3444_parts[10];
    /* 802D3538 */ s32 lp802D3538; /* D_802D3538: count */
    /* 802D353C */ LevelPart lp802D3538_parts[9];
    /* 802D3614 */ s32 lp802D3614; /* D_802D3614: count */
    /* 802D3618 */ LevelPart lp802D3614_parts[7];
    /* 802D36C0 */ s32 lp802D36C0; /* D_802D36C0: count */
    /* 802D36C4 */ LevelPart lp802D36C0_parts[8];
    /* 802D3784 */ s32 lp802D3784; /* D_802D3784: count */
    /* 802D3788 */ LevelPart lp802D3784_parts[11];
    /* 802D3890 */ s32 lp802D3890; /* D_802D3890: count */
    /* 802D3894 */ LevelPart lp802D3890_parts[7];
    /* 802D393C */ s32 lp802D393C; /* D_802D393C: count */
    /* 802D3940 */ LevelPart lp802D393C_parts[8];
    /* 802D3A00 */ s32 lp802D3A00; /* D_802D3A00: count */
    /* 802D3A04 */ LevelPart lp802D3A00_parts[3];
    /* 802D3A4C */ s32 lp802D3A4C; /* D_802D3A4C: count */
    /* 802D3A50 */ LevelPart lp802D3A4C_parts[14];
    /* 802D3BA0 */ s32 lp802D3BA0; /* D_802D3BA0: count */
    /* 802D3BA4 */ LevelPart lp802D3BA0_parts[2];
    /* 802D3BD4 */ s32 lp802D3BD4; /* D_802D3BD4: count */
    /* 802D3BD8 */ LevelPart lp802D3BD4_parts[11];
    /* 802D3CE0 */ s32 lp802D3CE0; /* D_802D3CE0: count */
    /* 802D3CE4 */ LevelPart lp802D3CE0_parts[10];
    /* 802D3DD4 */ s32 lp802D3DD4; /* D_802D3DD4: count */
    /* 802D3DD8 */ LevelPart lp802D3DD4_parts[9];
    /* 802D3EB0 */ s32 lp802D3EB0; /* D_802D3EB0: count */
    /* 802D3EB4 */ LevelPart lp802D3EB0_parts[8];
    /* 802D3F74 */ s32 lp802D3F74; /* D_802D3F74: count */
    /* 802D3F78 */ LevelPart lp802D3F74_parts[7];
} TextData8E910; /* size 0xF50 */
/* the members must sit at their ROM offsets (with 4-byte pointers) */
typedef char TextData8E910_size_check[(sizeof(void *) != 4 || sizeof(TextData8E910) == 0xF50) ? 1 : -1];

TextData8E910 textdata_8E910 = {
    8,
    {
        { NULL, 4, { 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 9, { 3, 7, 2, 6, 0, 4, 8, 1, 5, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 11, { 3, 9, 5, 10, 2, 8, 0, 7, 6, 11, 1, 0, 0, 0, 0, 0 } },
        { NULL, 3, { 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 8, { 2, 6, 3, 7, 0, 1, 4, 5, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 4, { 0, 6, 3, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 8, { 0, 2, 4, 6, 1, 5, 3, 7, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    },
    11,
    {
        { NULL, 3, { 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 2, { 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 5, { 4, 5, 6, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 12, { 0, 6, 3, 9, 1, 7, 4, 10, 8, 2, 5, 11, 0, 0, 0, 0 } },
        { NULL, 8, { 0, 4, 2, 6, 1, 5, 3, 7, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 5, { 0, 6, 1, 2, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 4, { 0, 2, 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 6, { 0, 3, 1, 4, 2, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 6, { 0, 3, 1, 4, 2, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 4, { 3, 7, 2, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    },
    5,
    {
        { NULL, 2, { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 3, { 0, 3, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 8, { 3, 9, 5, 10, 2, 8, 6, 11, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 10, { 0, 6, 9, 3, 1, 7, 8, 2, 11, 5, 0, 0, 0, 0, 0, 0 } },
        { NULL, 6, { 6, 9, 7, 10, 8, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    },
    7,
    {
        { NULL, 3, { 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 5, { 3, 7, 4, 2, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 4, { 1, 2, 4, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 8, { 0, 2, 4, 6, 1, 3, 5, 7, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 5, { 4, 2, 5, 3, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 6, { 0, 4, 2, 1, 5, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 12, { 0, 3, 6, 1, 4, 7, 9, 2, 5, 8, 10, 11, 0, 0, 0, 0 } },
    },
    5,
    {
        { NULL, 3, { 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 4, { 2, 6, 3, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 8, { 0, 4, 2, 6, 1, 5, 3, 7, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 3, { 0, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 9, { 8, 2, 0, 4, 6, 1, 3, 5, 7, 0, 0, 0, 0, 0, 0, 0 } },
    },
    10,
    {
        { NULL, 5, { 4, 2, 0, 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 8, { 2, 6, 0, 4, 3, 7, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 9, { 0, 6, 3, 1, 7, 4, 2, 5, 8, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 9, { 0, 3, 6, 1, 4, 7, 5, 8, 2, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 8, { 0, 4, 1, 5, 3, 2, 7, 6, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 6, { 0, 4, 1, 5, 3, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 6, { 0, 4, 1, 5, 3, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 12, { 6, 9, 0, 3, 7, 10, 1, 4, 8, 11, 2, 5, 0, 0, 0, 0 } },
    },
    9,
    {
        { NULL, 2, { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 2, { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 6, { 6, 9, 7, 10, 8, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 5, { 0, 7, 2, 8, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 2, { 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 2, { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 2, { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 2, { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 6, { 6, 9, 7, 10, 8, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    },
    7,
    {
        { NULL, 2, { 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 5, { 4, 5, 6, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 12, { 0, 3, 6, 9, 1, 4, 7, 10, 2, 5, 8, 11, 0, 0, 0, 0 } },
        { NULL, 2, { 1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 4, { 0, 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 4, { 0, 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 5, { 0, 6, 1, 2, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    },
    8,
    {
        { NULL, 10, { 0, 5, 1, 6, 2, 7, 3, 8, 4, 9, 0, 0, 0, 0, 0, 0 } },
        { NULL, 10, { 0, 5, 1, 6, 2, 7, 3, 8, 4, 9, 0, 0, 0, 0, 0, 0 } },
        { NULL, 10, { 0, 5, 1, 6, 2, 7, 3, 8, 4, 9, 0, 0, 0, 0, 0, 0 } },
        { NULL, 10, { 0, 5, 1, 6, 2, 7, 3, 8, 4, 9, 0, 0, 0, 0, 0, 0 } },
        { NULL, 10, { 0, 5, 1, 6, 2, 7, 3, 8, 4, 9, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    },
    11,
    {
        { NULL, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    },
    7,
    {
        { NULL, 5, { 0, 9, 1, 2, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 5, { 0, 9, 1, 2, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 5, { 0, 9, 1, 2, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 8, { 3, 1, 5, 9, 8, 4, 2, 6, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 3, { 2, 5, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 7, { 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    },
    8,
    {
        { NULL, 4, { 0, 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 4, { 0, 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 4, { 0, 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 2, { 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 2, { 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 6, { 0, 1, 3, 4, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 4, { 0, 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    },
    3,
    {
        { NULL, 3, { 3, 4, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 2, { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 3, { 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    },
    14,
    {
        { NULL, 2, { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 2, { 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 2, { 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 4, { 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 2, { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 2, { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 2, { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 4, { 2, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 2, { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 2, { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 2, { 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    },
    2,
    {
        { NULL, 6, { 2, 3, 0, 4, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    },
    11,
    {
        { NULL, 3, { 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 2, { 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 5, { 4, 5, 6, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 12, { 0, 6, 3, 9, 7, 1, 4, 10, 2, 8, 5, 11, 0, 0, 0, 0 } },
        { NULL, 8, { 0, 4, 2, 6, 1, 5, 3, 7, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 6, { 0, 6, 1, 7, 2, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 4, { 0, 2, 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 6, { 0, 3, 1, 4, 2, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 6, { 0, 3, 1, 4, 2, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 4, { 3, 7, 2, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    },
    10,
    {
        { NULL, 1, { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 6, { 6, 9, 7, 10, 8, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 5, { 0, 7, 2, 8, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 2, { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 2, { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 2, { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 10, { 0, 6, 3, 9, 1, 7, 2, 8, 5, 11, 0, 0, 0, 0, 0, 0 } },
        { NULL, 2, { 8, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 2, { 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    },
    9,
    {
        { NULL, 3, { 4, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 6, { 0, 1, 2, 3, 4, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 6, { 6, 0, 7, 1, 8, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 4, { 0, 2, 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 4, { 1, 4, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 8, { 3, 7, 1, 5, 2, 6, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 4, { 2, 6, 3, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 6, { 6, 7, 9, 10, 8, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    },
    8,
    {
        { NULL, 4, { 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 9, { 3, 7, 2, 6, 0, 4, 8, 1, 5, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 11, { 3, 9, 5, 10, 2, 8, 0, 7, 6, 11, 1, 0, 0, 0, 0, 0 } },
        { NULL, 3, { 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 8, { 2, 6, 3, 7, 0, 1, 4, 5, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 4, { 0, 6, 3, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 8, { 0, 2, 4, 6, 1, 5, 3, 7, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    },
    7,
    {
        { NULL, 3, { 0, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 4, { 5, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 4, { 0, 6, 2, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 2, { 4, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 2, { 4, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 2, { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        { NULL, 3, { 6, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    },
};
#endif
