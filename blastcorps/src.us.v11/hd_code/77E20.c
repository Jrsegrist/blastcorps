#include "common.h"
#include <ultra64.h>

extern void *D_80358074;

#ifdef NON_MATCHING
/* Types, tables and helpers shared by the port-phase rewrites in this file.
 *
 * Several rewrites below call hand-written helpers that pass arguments and
 * results in t-/s-registers. Helpers of this file that have a C rewrite (and a
 * tools_port/conventions.txt line) are called normally: func_802BD064 and
 * func_802BC7DC. The others can't be called from C yet, so their logic is
 * written inline as macros or loops (each comment names the helper it stands
 * for): func_802ABCDC (62740), func_8029D210 (56040), func_802BC714 (this
 * file, still asm: it calls func_8029D210), func_802ACA60 and func_802AC8CC
 * (679E0). The native port can turn them into real calls once those have C
 * rewrites. */

/* A model/object header: mostly byte offsets (from the header) of its tables. */
typedef struct {
    /* 0x00 */ u16 unk0;  /* number of s16 entries at unk3C (func_802C1A28) */
    /* 0x02 */ u8 pad2[2];
    /* 0x04 */ u8 type;   /* 0xFF = ignored, 1 = not counted */
    /* 0x05 */ u8 pad5;
    /* 0x06 */ u8 unk6;   /* passed to func_80264CB4 (func_802BF384) */
    /* 0x07 */ u8 pad7;
    /* 0x08 */ s32 value; /* summed */
    /* 0x0C */ u8 padC;
    /* 0x0D */ u8 unkD;   /* 1-based index of the part func_802BF534 ignores */
    /* 0x0E */ u8 padE[0xE];
    /* 0x1C */ s32 unk1C; /* byte offset from this record (func_802BCE40) */
    /* 0x20 */ s32 unk20; /* offset of an s16[4] box x0, z0, x1, z1 (func_802C1214) */
    /* 0x24 */ s32 unk24; /* offsets of a table of 0x14-byte records (func_802BF1F0) */
    /* 0x28 */ s32 unk28; /*   and its end */
    /* 0x2C */ u8 pad2C[0x10];
    /* 0x3C */ s32 unk3C; /* offset of an s16 table (func_802C1A28) */
    /* 0x40 */ s32 unk40; /* offset of an s16 table indexed 1-based (func_802C038C) */
    /* 0x44 */ s32 unk44; /* offsets of variable-length records (func_802C038C) */
    /* 0x48 */ s32 unk48; /*   and their end */
} Unk802C1DD0Info;

typedef struct {
    /* 0x00 */ Unk802C1DD0Info *info;
    /* 0x04 */ u8 pad4[8];
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 pos[3];
    /* 0x1C */ u8 pad1C[0xC];
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ u8 pad34[0x14];
    /* 0x48 */ u16 unk48[0x50]; /* one per part (func_802BF534) */
    /* 0xE8 */ u8 unkE8;
    /* 0xE9 */ u8 unkE9;        /* number of parts */
    /* 0xEA */ u8 unkEA;
    /* 0xEB */ u8 unkEB;
    /* 0xEC */ u8 unkEC[0x10]; /* per part; 100 = done. Indexed by D_803BE708's byte lists (0..0xFF) */
} Unk802C1DD0Entry; /* size 0xFC */

extern Unk802C1DD0Entry D_803F4030[];
extern Unk802C1DD0Entry *D_803F7654; /* end of the used part of D_803F4030 */

typedef struct {
    /* 0x00 */ s32 pos[3];
    /* 0x0C */ u8 padC[0x1A];
    /* 0x26 */ u8 unk26;
    /* 0x27 */ u8 pad27;
} Unk8039C800; /* size 0x28 */

extern Unk8039C800 D_8039C800[];
extern u8 D_8039C940; /* number of D_8039C800 entries */

typedef struct {
    /* 0x00 */ s32 pos[3];
    /* 0x0C */ s8 ids[9]; /* -1 terminated (the scan isn't bounded), looked up in D_803B9890 */
    /* 0x15 */ s8 level;  /* -1 ends the table */
    /* 0x16 */ u8 unk16[0x1A];
} Unk80306480; /* size 0x30 */

extern Unk80306480 D_80306480[];

/* A collision triangle (func_802BD99C keeps its plane up to date). */
typedef struct {
    /* 0x00 */ s64 normal[3]; /* unnormalised: cross product of two edges */
    /* 0x18 */ s64 d;         /* -(normal . vertex 1) */
    /* 0x20 */ f32 len;       /* |normal| */
    /* 0x24 */ f32 lenSq;
    /* 0x28 */ s32 v[3][3];   /* vertices x, y, z */
    /* 0x4C */ u8 pad4C[2];
    /* 0x4E */ u8 axis;       /* dominant normal axis: 0 = z, 1 = y, 2 = x */
    /* 0x4F */ u8 pad4F;
    /* 0x50 */ u8 id;
    /* 0x51 */ u8 unk51;
    /* 0x52 */ u16 unk52;
    /* 0x54 */ u8 pad54[3];
    /* 0x57 */ u8 unk57;
    /* 0x58 */ u8 pad58[8];
} Unk803B9890; /* size 0x60 */

/* What func_802BD85C/func_802BD99C get in v0: a model header and a range of
 * its triangles. */
typedef struct {
    /* 0x00 */ u8 *data;
    /* 0x04 */ Unk803B9890 *start;
    /* 0x08 */ Unk803B9890 *end;
} Unk802BD99CModel;

extern Unk803B9890 D_803B9890[];
extern u8 *D_803BE708;
extern s32 D_802E8BDC; /* current level */
extern s32 D_803EF6DC; /* player x, y, z */
extern s32 D_803EF6E0;
extern s32 D_803EF6E4;

double sqrt(double);
#pragma intrinsic(sqrt)

/* func_802ABCDC (62740; points in t3-t5 and t6,t7,s0, result in s1): the
 * distance between two points, rounded to the nearest integer. The asm wraps
 * each difference to 32 bits, sums the squares as s64 (dmult/daddu), converts
 * with cvt.d.l, takes sqrt.d and rounds with cvt.l.d (FCSR default: nearest,
 * ties to even). A sum past 2^63 (all three differences near 2^31) is NaN
 * there; keep coordinates in range. The sum goes to f64 as hi * 2^32 + lo,
 * which rounds once like cvt.d.l (a plain cast would call __ll_to_d, which
 * the equivalence harness can't run: cvt.d.l needs Status.FR=1 there). */
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

/* func_8029D210 (56040; key in t4, result in t5): unk51 of the first
 * D_803B9890 entry whose id equals key. The search is unbounded, so the key
 * must be present; callers pass sign-extended s8 ids, so a negative one never
 * matches. */
#define LOOKUP_8029D210(out, key)                                       \
    do {                                                                \
        Unk803B9890 *_e = D_803B9890;                                   \
                                                                        \
        while (_e->id != (key)) {                                       \
            _e++;                                                       \
        }                                                               \
        (out) = _e->unk51;                                              \
    } while (0)

/* Rewritten in this file (see conventions.txt for their asm registers). */
s32 func_802BC7DC(void);
s32 func_802BD064(Unk802C1DD0Entry *next);

typedef struct {
    /* 0x00 */ s16 next;  /* chain offset of the next group; -1 = last group */
    /* 0x02 */ u8 pad2[0x12];
    /* 0x14 */ u16 mtxOffset[10]; /* 1-based part index -> offset into the matrix buffer */
    /* 0x28 */ s32 count; /* records following this header */
} Unk802C2054Group;       /* size 0x2C */

typedef struct {
    /* 0x00 */ u16 index[9];
    /* 0x12 */ u16 pad12;
    struct {
        /* 0x00 */ s32 unk0;    /* matrix offset this part follows */
        /* 0x04 */ s32 base[3];
    } /* 0x14 */ part[3];
} Unk802C2054Record;      /* size 0x44 */
#endif

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
#ifdef NON_MATCHING
extern u8 D_803F7805;  /* request flag, consumed here */
extern u8 D_803F7806;
extern u8 D_803643DB;
extern u8 D_80364AC1;
extern u8 D_803643DA;
extern u8 D_802E8BD8;
extern u8 D_803649E8;
extern u64 D_80364A90; /* game mode */
extern u64 D_80364A98; /* next game mode */
void func_80275390(u64);

/* If D_803F7805 is set, clears it and, unless D_803643DB or D_80364AC1 is set
 * and the level isn't finished yet, marks it finished (D_803643DA and
 * D_802E8BD8 = 1). "Finished" means D_803F7806 is set, every D_8039C800 entry
 * has unk26 set (func_802BC7DC) and no D_80306480 entry of the current level
 * at or beyond the player's z has an id whose D_803B9890 unk51 is set
 * (func_802BC714). Then, if D_803643DB is clear: in game mode 0x40 it calls
 * func_80275390(0x40), otherwise it sets D_803649E8 = 1 and the next mode 8.
 * Declared void in hd.c; the asm leaves garbage in v0. */
void func_802BC5E0(void) {
    s32 v;
    s8 *id;
    Unk80306480 *e;

    if (D_803F7805 == 0) {
        return;
    }
    D_803F7805 = 0;
    if (D_803643DB != 0 || D_80364AC1 != 0) {
        if (D_803F7806 == 0) {
            return;
        }
        if (func_802BC7DC() == 0) {
            return;
        }
        /* func_802BC714: no wanted id left among the current level's entries
         * at or beyond the player's z */
        for (e = D_80306480; e->level != -1; e++) {
            if (e->level == D_802E8BDC && e->pos[2] >= D_803EF6E4) {
                for (id = e->ids; *id != -1; id++) {
                    LOOKUP_8029D210(v, *id);
                    if (v != 0) {
                        return;
                    }
                }
            }
        }
    }
    D_803643DA = 1;
    D_802E8BD8 = 1;
    if (D_803643DB != 0) {
        return;
    }
    if (D_80364A90 == 0x40) {
        func_80275390(0x40);
    } else {
        D_803649E8 = 1;
        D_80364A98 = 8;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BC5E0.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BC714.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Returns 1 if every D_8039C800 entry has unk26 set, else 0. The asm returns
 * it in t4 and saves t0, t1 and t3. Asm caller func_802BDDB4 keeps a0-a3,
 * t0-t2, f12 and f14 live across the call. */
s32 func_802BC7DC(void) {
    s32 i;
    Unk8039C800 *t;

    for (i = D_8039C940, t = D_8039C800; i != 0; i--, t++) {
        if (t->unk26 == 0) {
            return 0;
        }
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BC7DC.s")
#endif

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
 * attempt a C translation for any function flagged with this comment.
 * (Port phase: functions of this family with an `#ifdef NON_MATCHING` C
 * rewrite below are functional, non-matching equivalents for the native port.) */
#ifdef NON_MATCHING
extern u8 D_803F7690[40][8];

/* Clears byte 7 of each of the 40 8-byte entries of D_803F7690.
 * Register note: the asm saves/restores v0 and v1; asm caller func_802A1674
 * keeps t0, f12 and f14 live across the call (a mixed N64 build would need a
 * thunk; the native port doesn't). */
void func_802BC840(void) {
    s32 i;

    for (i = 0; i < 40; i++) {
        D_803F7690[i][7] = 0;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BC840.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
s32 func_8026A8E0(s32 lo, s32 hi);

/* D_803F7690 holds 40 8-byte counters {s32 id; u16 amount; u8 kind; u8 timer},
 * a nonzero timer marking a slot in use. Adds `amount` to the used slot with
 * this id and kind; if there is none, fills the first free slot with id, kind
 * and amount and a timer of func_8026A8E0(1, 20). Does nothing when all 40 are
 * in use. kind is compared with the byte as a full word.
 * The asm takes kind in t3, id in t9 and amount in a2 and preserves every
 * register (it saves all of them, $sp included); asm caller func_802C18D4
 * keeps a0, a1, t3, t4, t6, t7 and t9 live across the call. */
void func_802BC888(s32 kind, s32 id, s32 amount) {
    u8 *e;
    s32 i;

    for (i = 40, e = D_803F7690[0]; i != 0; i--, e += 8) {
        if (e[7] != 0 && *(s32 *) e == id && e[6] == kind) {
            *(u16 *) (e + 4) += amount;
            return;
        }
    }
    for (i = 40, e = D_803F7690[0]; i != 0; i--, e += 8) {
        if (e[7] == 0) {
            e[6] = kind;
            *(u16 *) (e + 4) = amount;
            *(s32 *) e = id;
            e[7] = func_8026A8E0(1, 20);
            return;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BC888.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCA2C.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Two small byte buffers, each with a write pointer stored just before it:
 * buffer A is D_803F77D8 (write pointer D_803F77D4), buffer B is D_803F77E8
 * (write pointer D_803F77E4). Used by func_802BCBD8/802BCC10/802BCC48. */
extern u8 *D_803F77D4;
extern u8 D_803F77D8[];
extern u8 *D_803F77E4;
extern u8 D_803F77E8[];

/* Resets buffer A: its write pointer goes back to the buffer start.
 * Register note: the asm saves/restores v0 and v1 and touches nothing else. */
void func_802BCBD8(void) {
    D_803F77D4 = D_803F77D8;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCBD8.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Resets buffer B: its write pointer goes back to the buffer start.
 * Register note: the asm saves/restores v0 and v1; asm callers keep
 * registers live across the call (func_8029A800: a2, t8; func_802B49AC: a0,
 * a3, t6, f12, f14), so a mixed N64 build would need a thunk. */
void func_802BCC10(void) {
    D_803F77E4 = D_803F77E8;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCC10.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_80358060;

/* If D_80358060 is set, copies buffer A (D_803F77D8 up to its write pointer)
 * into buffer B byte by byte, front to back; then sets B's write pointer to
 * just past the copied bytes (to B's start when nothing was copied). The asm
 * loops with `!=`, so A's write pointer must not be below D_803F77D8.
 * Register note: the asm saves/restores v0, v1, a0 and a1; asm caller
 * func_802BE77C keeps a0, a1, t6, f12 and f14 live across the call. */
void func_802BCC48(void) {
    u8 *dst = D_803F77E8;

    if (D_80358060 != 0) {
        u8 *src = D_803F77D8;
        u8 *end = D_803F77D4;

        while (src != end) {
            *dst++ = *src++;
        }
    }
    D_803F77E4 = dst;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCC48.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCCD4.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Returns 1 if some byte of buffer A (D_803F77D8 up to its write pointer)
 * differs from value (compared as a full word), else 0. The asm takes value
 * in v0 and returns in v1 (it has no callers); it saves a0, a1 and a2. The
 * scan stops at the write pointer with `!=`, which must not be below the
 * buffer start. */
s32 func_802BCD20(s32 value) {
    u8 *p = D_803F77D8;
    u8 *end = D_803F77D4;

    while (p != end) {
        if (*p++ != value) {
            return 1;
        }
    }
    return 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCD20.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Returns 1 if buffer A (D_803F77D8 up to its write pointer) contains value
 * (compared as a full word), else 0. The asm takes value in v0, returns in v1
 * and saves a0, a1 and a2; asm callers keep a3 (func_802B0AAC) or a0, a3,
 * f12 and f14 (func_802BBEB8) live across the call. */
s32 func_802BCD80(s32 value) {
    u8 *p = D_803F77D8;
    u8 *end = D_803F77D4;

    while (p != end) {
        if (*p++ == value) {
            return 1;
        }
    }
    return 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCD80.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* func_802BCD80 for buffer B (D_803F77E8 up to its write pointer D_803F77E4).
 * The asm takes value in fp (as a value, not a base pointer), returns in v1
 * and saves a0, a1 and a2; asm caller func_802BEBB0 keeps a0, a1, t3, t8, t9,
 * f12 and f14 live across the call. */
s32 func_802BCDE0(s32 value) {
    u8 *p = D_803F77E8;
    u8 *end = D_803F77E4;

    while (p != end) {
        if (*p++ == value) {
            return 1;
        }
    }
    return 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCDE0.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_803643E0; /* reference point x, y, z */
extern s32 D_803643E4;
extern s32 D_803643E8;
extern void *D_8036C790;
extern u8 D_803F7808;

/* Finds the closest target to the point D_803643E0/E4/E8, starting from a
 * distance limit of 9999999, over three tables (a later one wins only when
 * strictly closer):
 *   1: D_803F4030 entries with unkEB set, unkEA clear and not yet done
 *      (func_802BD064);
 *   2: D_8039C800 entries with unk26 clear;
 *   3: D_80306480 entries of the current level with an id whose D_803B9890
 *      unk51 is set.
 * Stores the kind (0 = none) in D_803F7808 and a pointer for it in D_8036C790
 * (NULL; info + info->unk1C; &entry + 0xC; &entry + 0x16), and returns the
 * entry (0 if none). The asm loops over D_803F4030 with `!=`, so D_803F7654
 * must be D_803F4030 + n entries. */
s32 func_802BCE40(void) {
    Unk802C1DD0Entry *end = D_803F7654;
    Unk802C1DD0Entry *e;
    Unk8039C800 *t;
    Unk80306480 *p;
    s8 *id;
    void *best = NULL;
    s32 bestDist = 9999999;
    s32 kind = 0;
    s32 dist;
    s32 v;
    s32 i;
    u8 *ptr;

    for (e = D_803F4030; e != end; e++) {
        if (e->unkEB == 0 || e->unkEA != 0) {
            continue;
        }
        DIST3_802ABCDC(dist, e->pos[0], e->pos[1], e->pos[2], D_803643E0, D_803643E4, D_803643E8);
        if (dist >= bestDist) {
            continue;
        }
        if (func_802BD064(e + 1) != 0) {
            continue;
        }
        bestDist = dist;
        best = e;
        kind = 1;
    }
    for (i = D_8039C940, t = D_8039C800; i != 0; i--, t++) {
        if (t->unk26 != 0) {
            continue;
        }
        DIST3_802ABCDC(dist, t->pos[0], t->pos[1], t->pos[2], D_803643E0, D_803643E4, D_803643E8);
        if (dist < bestDist) {
            kind = 2;
            best = t;
            bestDist = dist;
        }
    }
    for (p = D_80306480; p->level != -1; p++) {
        if (p->level != D_802E8BDC) {
            continue;
        }
        DIST3_802ABCDC(dist, p->pos[0], p->pos[1], p->pos[2], D_803643E0, D_803643E4, D_803643E8);
        if (dist >= bestDist) {
            continue;
        }
        for (id = p->ids; *id != -1; id++) {
            LOOKUP_8029D210(v, *id);
            if (v != 0) {
                kind = 3;
                best = p;
                bestDist = dist;
                break;
            }
        }
    }
    if (kind == 0) {
        ptr = NULL;
    } else if (kind == 1) {
        Unk802C1DD0Info *info = ((Unk802C1DD0Entry *) best)->info;

        ptr = (u8 *) info + info->unk1C;
    } else if (kind == 3) {
        ptr = (u8 *) best + 0x16;
    } else {
        ptr = (u8 *) best + 0xC;
    }
    D_8036C790 = ptr;
    D_803F7808 = kind;
    return (s32) best;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCE40.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Returns 1 if the D_803F4030 entry just before `next` is in D_803BE708's
 * table and every unkEC byte its record lists is 100, else 0 (also 0 when
 * D_803BE708 is NULL). The asm callers pass their loop pointer already
 * advanced past the entry (t0 = entry + 0xFC), so this takes that pointer too.
 * Table: s32 count, then count 0x18-byte records {Unk802C1DD0Entry *entry;
 * s32 n; u8 idx[16]}; the first record for the entry decides. Both counts are
 * counted down with `!=`. The asm returns in v0 and saves t0, t1, t2 and t4;
 * asm callers keep t0, t1 and t4-t7 live across the call. */
s32 func_802BD064(Unk802C1DD0Entry *next) {
    Unk802C1DD0Entry *e = next - 1;
    u8 *t = D_803BE708;
    s32 n;
    s32 k;
    u8 *b;

    if (t == NULL) {
        return 0;
    }
    for (n = *(s32 *) t, t += 4; n != 0; n--, t += 0x18) {
        if (*(Unk802C1DD0Entry **) t == e) {
            for (k = *(s32 *) (t + 4), b = t + 8; k != 0; k--, b++) {
                if (((u8 *) e)[0xEC + *b] != 100) {
                    return 0;
                }
            }
            return 1;
        }
    }
    return 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BD064.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803F7809;  /* kind of the target (as D_803F7808: 2 and 3 have pos at +0) */
extern u16 D_803EF6FC; /* divisor, 0 = none */
extern s32 D_8036C7C8;

/* D_8036C7C8 = the x/z distance from the player (D_803EF6DC, D_803EF6E4) to
 * the target, divided (unsigned) by D_803EF6FC when that is nonzero. The
 * target's x/z words are at +0x0/+0x8 for kinds 2 and 3, else +0x10/+0x18 (a
 * D_803F4030 entry). The C callers pass the target pointer as an s32. */
void func_802BD10C(s32 arg0) {
    s32 *target = (s32 *) arg0;
    s32 x;
    s32 z;
    s32 dist;

    if (D_803F7809 == 2 || D_803F7809 == 3) {
        x = target[0];
        z = target[2];
    } else {
        x = target[4];
        z = target[6];
    }
    DIST3_802ABCDC(dist, x, 0, z, D_803EF6DC, 0, D_803EF6E4);
    if (D_803EF6FC != 0) {
        dist = (u32) dist / D_803EF6FC;
    }
    D_8036C7C8 = dist;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BD10C.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BD1F8.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Clears unk51 of every triangle in [model->start, model->end) whose unk52
 * equals id + 1 and whose unk57 is set. The end is read once and the walk
 * stops with `!=`. The asm takes model in v0 and id in a3, and saves v1, a0,
 * a1 and a3 (so it changes nothing); asm caller func_802BD1F8 keeps a0-a3,
 * t0-t5, f12 and f14 live across the call. The asm's `addi` on id traps on
 * 0x7FFFFFFF; the C wraps instead. */
void func_802BD85C(Unk802BD99CModel *model, s32 id) {
    Unk803B9890 *r = model->start;
    Unk803B9890 *end = model->end;

    id++;
    for (; r != end; r++) {
        if (r->unk52 == id && r->unk57 != 0) {
            r->unk51 = 0;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BD85C.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* 409D0.c defines it as u8; declared s32 here because the asm tests the whole
 * of v0 (the u8 callee leaves the upper bits clear anyway). */
s32 func_80286090(s32 level);

/* Returns 1 on level 0x32 when func_80286090(level) returns 0, else 0. The
 * asm returns in t1 and preserves every other register it touches; asm
 * callers keep many registers live across the call (func_8029AB88 and
 * func_8029B02C: a0-a3, t3-t6, t8, t9, f12, f14). */
s32 func_802BD8C8(void) {
    if (D_802E8BDC == 0x32 && func_80286090(D_802E8BDC) == 0) {
        return 1;
    }
    return 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BD8C8.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
float sqrtf(float);
#pragma intrinsic(sqrtf)

/* Moves a model by (dx, dy, dz) world units and refreshes its triangles. The
 * model header's tables (offsets from the header) hold coordinates at two
 * scales: s16 tables at >> 5 (unk40..unk44: y only; the box at unk20: x, z,
 * x, z; 0x14-byte records unk24..unk28: three x, y, z points; 4 points at
 * unk1C; 8-byte records unk2C..unk30: one point) and s32 words at << 11
 * (0x38-byte records unk30..unk34 and unk34..unk38: x, y, z at +0 and y again
 * at +0x28). Each triangle in [model->start, model->end) moves by
 * (dx, dy, dz) >> 2 and gets its plane recomputed: the s64 cross product of
 * its edges, its squared length and length as floats (cvt.s.l, single
 * precision), d = -(normal . vertex 1) (64-bit, wrapping) and the dominant
 * normal axis. Every range is walked with `!=`. The asm's trapping 32-bit
 * `sub`s wrap here.
 * The asm takes model in v0 and (dx, dy, dz) in a2, a3, t0; it changes s0-s7
 * and fp (its callers save them) and leaves junk in a1-a3; asm callers keep
 * f12 and f14 live across the call. */
void func_802BD99C(Unk802BD99CModel *model, s32 dx, s32 dy, s32 dz) {
    u8 *d = model->data;
    s32 hx = dx >> 5, hy = dy >> 5, hz = dz >> 5;
    s32 wx = dx << 11, wy = dy << 11, wz = dz << 11;
    s32 sx, sy, sz;
    s16 *h;
    s16 *hend;
    s32 *w;
    s32 *wend;
    s32 i;
    Unk803B9890 *r;
    Unk803B9890 *rend;

#define MODEL_TABLE(o) (d + *(s32 *) (d + (o)))
    for (h = (s16 *) MODEL_TABLE(0x40), hend = (s16 *) MODEL_TABLE(0x44); h != hend; h++) {
        h[0] += hy;
    }
    h = (s16 *) MODEL_TABLE(0x20);
    h[0] += hx;
    h[1] += hz;
    h[2] += hx;
    h[3] += hz;
    for (h = (s16 *) MODEL_TABLE(0x24), hend = (s16 *) MODEL_TABLE(0x28); h != hend; h += 10) {
        for (i = 0; i < 9; i += 3) {
            h[i + 0] += hx;
            h[i + 1] += hy;
            h[i + 2] += hz;
        }
    }
    for (h = (s16 *) MODEL_TABLE(0x1C), i = 4; i != 0; i--, h += 3) {
        h[0] += hx;
        h[1] += hy;
        h[2] += hz;
    }
    for (w = (s32 *) MODEL_TABLE(0x30), wend = (s32 *) MODEL_TABLE(0x34); w != wend; w += 14) {
        w[0] += wx;
        w[1] += wy;
        w[2] += wz;
        w[10] += wy;
    }
    for (w = (s32 *) MODEL_TABLE(0x34), wend = (s32 *) MODEL_TABLE(0x38); w != wend; w += 14) {
        w[0] += wx;
        w[1] += wy;
        w[2] += wz;
        w[10] += wy;
    }
    for (h = (s16 *) MODEL_TABLE(0x2C), hend = (s16 *) MODEL_TABLE(0x30); h != hend; h += 4) {
        h[0] += hx;
        h[1] += hy;
        h[2] += hz;
    }
#undef MODEL_TABLE

    sx = dx >> 2;
    sy = dy >> 2;
    sz = dz >> 2;
    for (r = model->start, rend = model->end; r != rend; r++) {
        s32 x0 = r->v[0][0] + sx, y0 = r->v[0][1] + sy, z0 = r->v[0][2] + sz;
        s32 x1 = r->v[1][0] + sx, y1 = r->v[1][1] + sy, z1 = r->v[1][2] + sz;
        s32 x2 = r->v[2][0] + sx, y2 = r->v[2][1] + sy, z2 = r->v[2][2] + sz;
        s32 ex1 = x0 - x1, ey1 = y0 - y1, ez1 = z0 - z1;
        s32 ex2 = x0 - x2, ey2 = y0 - y2, ez2 = z0 - z2;
        s64 nx, ny, nz;
        s64 ax, ay, az;
        f32 fx, fy, fz;
        f32 sq;

        r->v[0][0] = x0, r->v[0][1] = y0, r->v[0][2] = z0;
        r->v[1][0] = x1, r->v[1][1] = y1, r->v[1][2] = z1;
        r->v[2][0] = x2, r->v[2][1] = y2, r->v[2][2] = z2;
        nx = (s64) ey1 * ez2 - (s64) ez1 * ey2;
        ny = (s64) ez1 * ex2 - (s64) ex1 * ez2;
        nz = (s64) ex1 * ey2 - (s64) ey1 * ex2;
        r->normal[0] = nx;
        r->normal[1] = ny;
        r->normal[2] = nz;
        fx = nx;
        fy = ny;
        fz = nz;
        sq = fx * fx + fy * fy + fz * fz;
        r->lenSq = sq;
        r->d = -(nx * x1 + ny * y1 + nz * z1);
        r->len = sqrtf(sq);

        ax = (nx < 0) ? -nx : nx;
        ay = (ny < 0) ? -ny : ny;
        az = (nz < 0) ? -nz : nz;
        if (az < ax || az < ay) {
            r->axis = (ay < ax || ay < az) ? 2 : 1;
        } else {
            r->axis = 0;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BD99C.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BDDB4.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Copies the display list [src, srcEnd) (8-byte commands) to dst and returns
 * the new end of dst, animating textures on the way: a G_SETTIMG (0xFD) whose
 * image address is a key in the table [tbl, tblEnd) gets one of that record's
 * images. Records are {u32 key; u8 n; u8 cur; u8 pad; u8 animate; u32 primOr;
 * u32 image[n]} (0xC + 4n bytes). With animate set, the first SETTIMG gets
 * image[cur] (its own address when cur is 0), the commands up to and
 * including the next SETTIMG are copied with that one getting image[cur + 1]
 * (cur + 1 wrapping to 0 at n; 0 again means "unchanged"), and the commands up
 * to the next G_SETPRIMCOLOR (0xFA) are copied with primOr ORed into its first
 * word. Without animate, the SETTIMG gets image[cur]. Those two inner scans
 * aren't bounded by srcEnd: the list must contain the markers. The record's
 * cur is never written here. Shared by func_802BE228 and func_802BE3C8, which
 * differ only in the register that holds dst. */
static u32 *dl_anim_copy_802BE228(u32 *src, u32 *srcEnd, u32 *dst, u8 *tbl, u8 *tblEnd) {
    u32 w0;
    u32 w1;
    u8 *rec;
    s32 next;

    while (src != srcEnd) {
        w0 = src[0];
        dst[0] = w0;
        w1 = src[1];
        if ((w0 & 0xFF000000) >> 24 == 0xFD) {
            for (rec = tbl; rec != tblEnd; rec += rec[4] * 4 + 0xC) {
                if (*(u32 *) rec == w1) {
                    break;
                }
            }
            if (rec != tblEnd) {
                if (rec[7] != 0) {
                    next = rec[5];
                    if (next != 0) {
                        w1 = *(u32 *) (rec + 0xC + next * 4);
                    }
                    dst[1] = w1;
                    next++;
                    if (rec[4] == next) {
                        next = 0;
                    }
                    src += 2;
                    dst += 2;
                    for (;;) {
                        w0 = src[0];
                        dst[0] = w0;
                        w1 = src[1];
                        if ((w0 & 0xFF000000) >> 24 == 0xFD) {
                            break;
                        }
                        dst[1] = w1;
                        src += 2;
                        dst += 2;
                    }
                    if (next != 0) {
                        w1 = *(u32 *) (rec + 0xC + next * 4);
                    }
                    dst[1] = w1;
                    src += 2;
                    dst += 2;
                    for (;;) {
                        w0 = src[0];
                        if ((w0 & 0xFF000000) >> 24 == 0xFA) {
                            break;
                        }
                        dst[0] = w0;
                        dst[1] = src[1];
                        src += 2;
                        dst += 2;
                    }
                    dst[0] = w0 | *(u32 *) (rec + 8);
                    dst[1] = src[1];
                    src += 2;
                    dst += 2;
                    continue;
                }
                next = rec[5];
                if (next != 0) {
                    w1 = *(u32 *) (rec + 0xC + next * 4);
                }
            }
        }
        dst[1] = w1;
        src += 2;
        dst += 2;
    }
    return dst;
}

/* See dl_anim_copy_802BE228. The asm takes src in t7, srcEnd in s0, dst in a2
 * and the table in t1..t2, returns the new dst in a2 (and leaves t7 at
 * srcEnd); it saves v0, v1, a0, a3, t3 and t4. Asm caller func_802BD1F8 keeps
 * a0, a1, t0-t6, f12 and f14 live across the call. */
u32 *func_802BE228(u32 *src, u32 *srcEnd, u32 *dst, u8 *tbl, u8 *tblEnd) {
    return dl_anim_copy_802BE228(src, srcEnd, dst, tbl, tblEnd);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BE228.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* func_802BE228 with dst in t0 (in and out) instead of a2. The asm saves v0,
 * v1, a0, a2, a3, t3 and t4; asm caller func_802BD1F8 keeps a0-a2, t1-t6, f12
 * and f14 live across the call. */
u32 *func_802BE3C8(u32 *src, u32 *srcEnd, u32 *dst, u8 *tbl, u8 *tblEnd) {
    return dl_anim_copy_802BE228(src, srcEnd, dst, tbl, tblEnd);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BE3C8.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BE574.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BE77C.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
typedef struct {
    /* 0x00 */ s32 level; /* 0xFFFF = any level */
    /* 0x04 */ s32 key;
    /* 0x08 */ u32 mask;  /* 0 ends the table */
} Unk803059F0;            /* size 0xC */

extern Unk803059F0 D_803059F0[];

/* Returns 1 if D_803059F0 has an entry for the current level (or any level)
 * whose key is e->unk30 and whose mask has bit `bit` (taken mod 32, as sllv
 * does), else 0; always 0 when unk30 is 0x38. The asm takes e in t3 and bit
 * in t8, returns in t9 and saves v0, v1 and a0-a2; asm caller func_802BE77C
 * keeps a0, a1, t3, t4, t6, t8, f12 and f14 live across the call. */
s32 func_802BE944(Unk802C1DD0Entry *e, s32 bit) {
    s32 key = e->unk30;
    s32 level = D_802E8BDC;
    Unk803059F0 *p;

    if (key == 0x38) {
        return 0;
    }
    for (p = D_803059F0; p->mask != 0; p++) {
        if ((p->level == 0xFFFF || p->level == level) && p->key == key && (p->mask & (1 << (bit & 0x1F)))) {
            return 1;
        }
    }
    return 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BE944.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern void *D_803F3960;
extern u8 D_803F3910[];

/* Resets the pointer D_803F3960 to the start of D_803F3910.
 * Register note: the asm saves/restores v0 and v1; asm caller func_802BE77C
 * keeps a0, a1, t6, t8, f12 and f14 live across the call. */
void func_802BE9F8(void) {
    D_803F3960 = D_803F3910;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BE9F8.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Appends the pair (a, b) to the list at D_803F3910 (write pointer
 * D_803F3960). The asm takes a in t9 and b in t3 and saves v0 and v1; asm
 * caller func_802BEBB0 keeps a0, a1, t3, t8, t9, f12 and f14 live. */
void func_802BEA30(s32 a, s32 b) {
    s32 *p = D_803F3960;

    p[0] = a;
    p[1] = b;
    D_803F3960 = p + 2;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BEA30.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Returns 1 if the pair (a, b) is in the list D_803F3910..D_803F3960 (see
 * func_802BEA30), else 0. The asm takes a in t9 and b in t3, returns in t7 and
 * saves v0, v1 and a0; asm caller func_802BEBB0 keeps a0, a1, t3, t8, t9, f12
 * and f14 live. The walk stops at the write pointer with `!=`. */
s32 func_802BEA70(s32 a, s32 b) {
    s32 *p = (s32 *) D_803F3910;
    s32 *end = D_803F3960;

    while (p != end) {
        p += 2;
        if (p[-2] == a && p[-1] == b) {
            return 1;
        }
    }
    return 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BEA70.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BEADC.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BEBB0.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803BE738;

/* On level 0x2D, sets D_803BE738 when e->unk30 is 0xE7..0xE9 (signed). The asm
 * takes e in t9 and saves v0; asm caller func_802BEBB0 keeps a0, a1, t8, t9,
 * f12 and f14 live. */
void func_802BEF9C(Unk802C1DD0Entry *e) {
    if (D_802E8BDC == 0x2D && e->unk30 >= 0xE7 && e->unk30 < 0xEA) {
        D_803BE738 = 1;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BEF9C.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_80364456;   /* mode: the update is skipped in modes 0,1,2,6,7,9,0xB,0x10..0x12 */
extern s16 D_803F77FC;  /* signed input whose sign is tracked */
extern s32 D_803F77F4;  /* frame stamp: run counting only if it is D_80358068 - 1 */
extern s32 D_80358068;
extern u8 D_80370C2C;   /* flag sampled each counted frame */
extern s8 D_803F780A;   /* last sign of D_803F77FC (-1, 0, 1) */
extern u8 D_803F780B;   /* run length, 0..0x14 */
extern u8 D_803F780C;   /* result: set for one call when a run of 0x14 completes with a change */
extern u8 D_803F780D;   /* "the flag changed during this run" */
extern u8 D_803F780E;   /* flag value at the start of the run */
extern u8 D_803F780F;   /* once-per-frame latch, set here */

/* Once-per-frame run detector. Unless the mode excludes it or it already ran
 * (latch D_803F780F), it counts consecutive frames (D_803F77F4 + 1 ==
 * D_80358068) in which D_803F77FC's sign doesn't repeat a nonzero previous
 * sign, recording whether D_80370C2C's truth value changed since the run
 * started. After 0x14 such frames the count restarts and D_803F780C is set if
 * a change was seen. Any other frame resets the run. The asm's `addi` on
 * D_803F77F4 traps on 0x7FFFFFFF; the C wraps instead.
 * Register note: the asm saves/restores v0, v1, a0 and a1 (and clobbers at);
 * asm caller func_802BEBB0 keeps a0, a1, t3, t8, t9, f12 and f14 live. */
void func_802BEFF4(void) {
    u8 mode = D_80364456;
    s32 sign;
    s32 flag;
    s32 count;

    if (mode == 0 || mode == 1 || mode == 2 || mode == 6 || mode == 7 || mode == 9 || mode == 0xB ||
        mode == 0x10 || mode == 0x11 || mode == 0x12) {
        return;
    }
    if (D_803F77FC < 0) {
        sign = -1;
    } else if (D_803F77FC != 0) {
        sign = 1;
    } else {
        sign = 0;
    }
    if (D_803F780F != 0) {
        return;
    }
    D_803F780F = 1;

    if (D_803F77F4 + 1 == D_80358068 && (sign == 0 || D_803F780A != sign)) {
        count = D_803F780B;
        flag = (D_80370C2C != 0) ? 1 : 0;
        if (count == 0) {
            D_803F780E = flag;
            D_803F780D = 0;
        } else if (flag == 0 || flag != D_803F780E) {
            D_803F780D = 1;
        }
        count++; /* full width: 0xFF + 1 stores 0 but is compared as 0x100 */
        D_803F780B = count;
        D_803F780A = sign;
        D_803F780C = 0;
        if (count == 0x14) {
            D_803F780B = 0;
            if (D_803F780D != 0) {
                D_803F780C = 1;
            }
        }
    } else {
        D_803F780B = 0;
        D_803F780A = sign;
        D_803F780C = 0;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BEFF4.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Sets byte 0x13 of every 0x14-byte record in e->info's unk24..unk28 table
 * whose byte 0x12 equals id (compared as a full word). The walk stops with
 * `!=`. The asm takes e in t9 and id in t3 and saves v0, v1 and a0-a2; asm
 * callers keep t3 and t9 (func_802BEBB0, func_802CDD74: also f12, f14),
 * t5-t7 and t9 (func_802BF668) or a1, t0-t2, t4, t5 and t7 (func_802C4BF0)
 * live across the call. */
void func_802BF1F0(Unk802C1DD0Entry *e, s32 id) {
    Unk802C1DD0Info *info = e->info;
    u8 *p = (u8 *) info + info->unk24;
    u8 *end = (u8 *) info + info->unk28;

    for (; p != end; p += 0x14) {
        if (p[0x12] == id) {
            p[0x13] = 1;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF1F0.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF264.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* 20460.c defines it with s16 coordinates and a u8; the asm passes the
 * unsliced shifted words, so it is declared with full words here. */
void func_80264CB4(s32 x, s32 y, s32 z, s32 size, s32 arg4, s32 arg5);

/* If exactly one of e's unkE9 parts is done (unkEC byte == 100), calls
 * func_80264CB4(x >> 5, y >> 5, z >> 5, unkC >> 5, unkE8, info->unk6), the
 * shifts logical. The asm takes e in t9 and preserves every register (all of
 * them saved around the call); asm callers keep a0, t3, t5, t9, f12 and f14
 * (func_802BCA2C), t3, t9, f12 and f14 (func_802BEBB0, func_802CDD74) live. */
void func_802BF384(Unk802C1DD0Entry *e) {
    s32 n;
    s32 count = 0;
    u8 *p = e->unkEC;

    for (n = e->unkE9; n != 0; n--) {
        if (*p++ == 100) {
            count++;
        }
    }
    if (count == 1) {
        func_80264CB4((u32) e->pos[0] >> 5, (u32) e->pos[1] >> 5, (u32) e->pos[2] >> 5, (u32) e->unkC >> 5,
                      e->unkE8, e->info->unk6);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF384.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_80364A40; /* message to post */
extern u32 D_803649F0; /* score */
extern u8 *D_803BDFD4; /* end of the 0x24-byte records at D_803BDFD8 */
extern u8 D_803BDFD8[];

/* Unless e is already finished (unkEA), finishes it when every part other
 * than the 1-based part info->unkD is either done (unkEC byte == 100) or has
 * a nonzero unk48 halfword: sets unkEA, posts info->value as message
 * D_80364A40 and adds it to D_803649F0, and clears byte 0x12 of every 0x24-byte
 * record at D_803BDFD8 (up to D_803BDFD4, walked with `!=`) whose byte 0x11 is
 * e's 1-based index in D_803F4030. The asm takes e in t9 and saves t4-t7, s0
 * and s1; asm callers keep a0, t3, t5, t9, f12 and f14 (func_802BCA2C) or a1,
 * f12 and f14 (func_802BEBB0) live. */
void func_802BF534(Unk802C1DD0Entry *e) {
    s32 skip;
    s32 part;
    s32 n;
    u8 *p;
    u16 *q;
    s32 value;
    u32 index;
    u8 *r;
    u8 *end;

    if (e->unkEA != 0) {
        return;
    }
    skip = e->info->unkD;
    p = e->unkEC;
    q = e->unk48;
    for (part = 0, n = e->unkE9; n != 0; n--, p++, q++) {
        part++;
        if (part != skip && *p != 100 && *q == 0) {
            return;
        }
    }
    e->unkEA = 1;
    value = e->info->value;
    D_80364A40 = value;
    D_803649F0 += value;
    index = (u32) ((u8 *) e - (u8 *) D_803F4030) / 0xFC + 1;
    end = D_803BDFD4;
    for (r = D_803BDFD8; r != end; r += 0x24) {
        if (r[0x11] == index) {
            r[0x12] = 0;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF534.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF668.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF898.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF978.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
s32 func_8026A828(s32 lo, s32 hi);

/* func_8026A828(lo, hi) (a random number in a range) for asm callers: the asm
 * passes a0/a1 through, returns the result in s4 and preserves every other
 * register; asm callers keep t5-t7 and t9 (func_802BF978), t7
 * (func_802BFDAC) or t0, t1 and t4 (func_802C0CBC) live. */
s32 func_802BFB50(s32 lo, s32 hi) {
    return func_8026A828(lo, hi);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFB50.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s8 D_8030633C[]; /* -1 terminated */
extern u64 D_803649D8;  /* frame time */
s32 func_80288284(u8 type, s32 x, s32 y, s32 z, s32 floor);

/* Unless e's info type is 1, and if obj's byte 0x31 is in the list
 * D_8030633C (s8 entries compared with the u8, so 0x80..0xFE never match),
 * calls func_80288284((low word of D_803649D8 >> 8) % 4, obj x, y, z >> 11
 * (logical), e->pos[1]). The asm takes e in t9 and obj in v1 and preserves
 * every register; asm caller func_802BF898 keeps a0, t3, t5 and t9 live. */
void func_802BFBF4(Unk802C1DD0Entry *e, s32 *obj) {
    s8 *p;
    s32 key;

    if (e->info->type == 1) {
        return;
    }
    key = ((u8 *) obj)[0x31];
    for (p = D_8030633C; *p != -1; p++) {
        if (*p == key) {
            func_80288284(((u32) D_803649D8 >> 8) % 4, (u32) obj[0] >> 11, (u32) obj[1] >> 11,
                          (u32) obj[2] >> 11, e->pos[1]);
            return;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFBF4.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFD1C.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFDAC.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803F7800;
extern u8 D_803F7801;

/* D_803F7800 = 0 if D_803F7801 is 1; else 1 if D_803F7801 is 2 or any of the
 * vehicle's bytes 0x96..0x98 is 1; else 0. The asm takes the vehicle in gp
 * (the vehicle update functions point gp at their own D_803EE6C0 /
 * D_803EEA90 block, so it is an argument here, not a fixed base) and saves v0
 * and v1; asm caller func_802BE77C keeps a0, a1, t6, t8, f12 and f14 live. */
void func_802BFEE4(u8 *vehicle) {
    if (D_803F7801 == 1) {
        D_803F7800 = 0;
    } else if (D_803F7801 == 2 || vehicle[0x96] == 1 || vehicle[0x97] == 1 || vehicle[0x98] == 1) {
        D_803F7800 = 1;
    } else {
        D_803F7800 = 0;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFEE4.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFF6C.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
typedef struct {
    /* 0x00 */ s32 level;
    /* 0x04 */ u32 mask; /* 0 ends the table */
} Unk80305E10;

extern Unk80305E10 D_80305E10[];
extern s32 D_803A740C; /* frame stamp */

/* Returns 1, and sets *out = 100, when any of these holds (else returns 0 and
 * leaves *out alone): D_803F780C is set (it is cleared, and nothing else is
 * checked); in the frame after D_803A740C (D_803A740C + 1 == D_80358068), a
 * D_80305E10 entry of the current level has bit `bit` (mod 32) in its mask;
 * or flag is 1 and bit is 1, 10 or 16. The asm takes bit in t8 and flag in gp
 * (a value here), returns in t2 and t7 (100, written only with a 1) and saves
 * v0, v1 and a0; asm caller func_802BFF6C keeps t8 and t9 live. The asm's
 * `addi` on D_803A740C traps on 0x7FFFFFFF; the C wraps instead. */
s32 func_802C0284(s32 bit, s32 flag, s32 *out) {
    Unk80305E10 *p;
    s32 ret = 0;

    if (D_803F780C != 0) {
        *out = 100;
        D_803F780C = 0;
        return 1;
    }
    if (D_803A740C + 1 == D_80358068) {
        for (p = D_80305E10; p->mask != 0; p++) {
            if ((p->mask & (1 << (bit & 0x1F))) && p->level == D_802E8BDC) {
                ret = 1;
                *out = 100;
                break;
            }
        }
    }
    if (bit == 1 && flag == 1) {
        ret = 1;
        *out = 100;
    }
    if (bit == 10 && flag == 1) {
        ret = 1;
        *out = 100;
    }
    if (bit == 16 && flag == 1) {
        ret = 1;
        *out = 100;
    }
    return ret;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C0284.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_803A73F4;

/* Returns 0 if part `id` of e is blocked, else 1. Blocked: a record in
 * e->info's unk44..unk48 list ({u8 part; u8 pad; u8 n; n x {u8 id; u8 pad}})
 * lists id (compared as a full word) while e's unkEC byte for its 1-based
 * part isn't 100 (part 0 reads unkEB); or the s16 height
 * info->unk40[id - 1] << 5 is above D_803A73F4 + 0x578. A record whose part is
 * done skips the rest of its ids. The record walk stops with `!=`. The asm
 * takes id in v0 and e in t9, returns in t3 (and leaves v0 doubled when it
 * gets to the height test); it saves v1, a0-a3, t0-t2 and t4. Asm caller
 * func_802BFF6C keeps t2, t4, t7, t8 and t9 live. The asm's `addi` on
 * D_803A73F4 traps on overflow; the C wraps. */
s32 func_802C038C(s32 id, Unk802C1DD0Entry *e) {
    Unk802C1DD0Info *info = e->info;
    u8 *rec = (u8 *) info + info->unk44;
    u8 *end = (u8 *) info + info->unk48;
    s32 n;
    s32 k;
    u8 *p;

    while (rec != end) {
        n = rec[2];
        for (k = n, p = rec + 3; k != 0; k--, p += 2) {
            if (*p == id) {
                if (((u8 *) e)[0xEB + rec[0]] == 100) {
                    break;
                }
                return 0;
            }
        }
        rec += n * 2 + 3;
    }
    if (D_803A73F4 + 0x578 < *(s16 *) ((u8 *) info + info->unk40 + id * 2 - 2) << 5) {
        return 0;
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C038C.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803F3968[30][0x38];

/* Sets byte 0x34 of each of the 30 0x38-byte entries of D_803F3968 to 1.
 * Register note: the asm saves/restores v0, v1 and a0; asm callers keep
 * registers live across the call (func_802A1674: a2, a3, t0, t1, t2, f12,
 * f14; func_802BA9A0: a0, a2, a3, t6, t7, f12, f14; func_802CF3E0: t1). */
void func_802C049C(void) {
    s32 i;

    for (i = 0; i < 30; i++) {
        D_803F3968[i][0x34] = 1;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C049C.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Copies the 0x38-byte record src (word by word) into the first of the 30
 * D_803F3968 entries whose byte 0x34 is set (see func_802C049C); does nothing
 * if there is none. The asm takes src in v1 and saves v1 and a0-a2; asm
 * callers keep registers live across the call (func_802BF898: a0, t3, t5,
 * t9; func_802BF978: t5-t7, t9; func_802BFD1C: a0, a3; func_802CF3E0:
 * a1-a3). */
void func_802C04F0(u32 *src) {
    u8 *e = D_803F3968[0];
    u32 *dst;
    s32 i;
    s32 k;

    for (i = 30; i != 0; i--, e += 0x38) {
        if (e[0x34] != 0) {
            for (dst = (u32 *) e, k = 0x38; k != 0; k -= 4) {
                *dst++ = *src++;
            }
            return;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C04F0.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C0574.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803F0900[]; /* 4 entries of 0x4B8 bytes */

/* For each of the 4 D_803F0900 entries: with its s16 at 0x4B0 zero, counts
 * its byte at 0x4B2 down to 0; otherwise lowers the s16 by 20 and, if that
 * leaves it <= 0, clears it and sets the byte to 3, else draws the entry
 * (G_RDPPIPESYNC, G_SETENVCOLOR with the new value as its colour word, and a
 * G_DL call to the entry itself as a physical address). Ends the list with 0xB8 (end display list) and returns the new
 * end. The asm takes and returns gfx in v0 and saves v1 and a0-a3; asm caller
 * func_802BD1F8 keeps a0-a3, f12 and f14 live. */
u32 *func_802C08C4(u32 *gfx) {
    u8 *e = D_803F0900;
    s32 i;
    s32 h;

    for (i = 4; i != 0; i--, e += 0x4B8) {
        h = *(s16 *) (e + 0x4B0);
        if (h == 0) {
            if (e[0x4B2] != 0) {
                e[0x4B2]--;
            }
            continue;
        }
        h -= 20;
        if (h <= 0) {
            *(s16 *) (e + 0x4B0) = 0;
            e[0x4B2] = 3;
            continue;
        }
        *(s16 *) (e + 0x4B0) = h;
        gfx[0] = 0xE7000000;
        gfx[1] = 0;
        gfx[2] = 0xFB000000;
        gfx[3] = h;
        gfx[4] = 0x06000000;
        gfx[5] = (u32) e - 0x80000000;
        gfx += 6;
    }
    gfx[0] = 0xB8000000;
    gfx[1] = 0;
    return gfx + 2;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C08C4.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C09B8.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
typedef struct {
    /* 0x00 */ u64 from;
    /* 0x08 */ u64 to;
} Unk802F46C0;

extern Unk802F46C0 D_802F46C0[12]; /* display-list command substitutions */

/* Returns the replacement for a 64-bit display-list command: the `to` of the
 * first of the 12 D_802F46C0 entries whose `from` equals cmd, else cmd
 * itself. The asm takes and returns cmd as one 64-bit value in t4 (ld/sd by
 * its caller func_802C09B8) and saves v0, v1 and a0; func_802C09B8 keeps
 * a0-a3, t1-t3 and t9 live. The harness only maps the low word (t4 <-> a1 /
 * v1), see conventions.txt. */
u64 func_802C0C64(u64 cmd) {
    s32 i;

    for (i = 0; i < 12; i++) {
        if (D_802F46C0[i].from == cmd) {
            return D_802F46C0[i].to;
        }
    }
    return cmd;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C0C64.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C0CBC.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C0E8C.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Writes the positions of 4 Vtx at base + 0x9E0: the corners of e's info box
 * (s16 x0, z0, x1, z1 at info->unk20) grown by 100 on every side, at height
 * e->pos[1] >> 5 (arithmetic), in the order (x0, z0), (x1, z0), (x0, z1),
 * (x1, z1); each value is stored as its low 16 bits. The asm takes base in v1
 * and e in t9 and saves a0-a3 and t0-t3; asm caller func_802C0E8C keeps t3
 * and t9 live. */
void func_802C1214(u8 *base, Unk802C1DD0Entry *e) {
    Unk802C1DD0Info *info = e->info;
    s16 *box = (s16 *) ((u8 *) info + info->unk20);
    s32 y = e->pos[1] >> 5;
    s32 x0 = box[0] - 100;
    s32 z0 = box[1] - 100;
    s32 x1 = box[2] + 100;
    s32 z1 = box[3] + 100;
    s16 *v = (s16 *) (base + 0x9E0);

    v[0] = x0, v[1] = y, v[2] = z0;
    v[8] = x1, v[9] = y, v[10] = z0;
    v[16] = x0, v[17] = y, v[18] = z1;
    v[24] = x1, v[25] = y, v[26] = z1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1214.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803F1BE0[]; /* 2 entries of 0x478 bytes */
extern Mtx *D_803F7658; /* 2 matrices for them */
/* 23C20.c defines it with s16 parameters; the asm passes rx as a full
 * u16 + u16 sum and ry zero-extended, so it is declared with words here. */
void func_8026A454(s32 x, s32 y, s32 z, s32 rx, s32 ry, Mtx *m);

/* Updates and draws the 2 D_803F1BE0 entries into two display lists. An
 * inactive entry (byte 0x470 clear) counts its byte 0x472 down to 0. An
 * active one advances its u16 angle 0x46C by its u16 speed 0x46E while the
 * sum stays below 0x385 (the speed then grows by 15); once it doesn't, it
 * counts byte 0x471 up and, at 6, deactivates (0x470 = 0, 0x472 = 3) without
 * drawing. A drawn entry gets func_8026A454(x, y, z at 0x464..0x468, the sum,
 * u16 0x46A, its matrix D_803F7658[i]), then both lists get gSPSegment(9,
 * physical address in word 0x460), and a G_DL call to the entry (list A) or
 * to entry + 0x230 (list B), as physical addresses. The asm takes and returns
 * the two list pointers in a0/a1 (here through gfxA/gfxB) and changes s0-s5
 * (its caller func_802BD1F8 saves them). */
void func_802C12E0(u32 **gfxA, u32 **gfxB) {
    u32 *a = *gfxA;
    u32 *b = *gfxB;
    u8 *e = D_803F1BE0;
    Mtx *m = D_803F7658;
    s32 i;
    s32 sum;
    s32 c;
    u32 seg;

    for (i = 2; i != 0; i--, e += 0x478, m++) {
        if (e[0x470] == 0) {
            if (e[0x472] != 0) {
                e[0x472]--;
            }
            continue;
        }
        sum = *(u16 *) (e + 0x46C) + *(u16 *) (e + 0x46E);
        if (sum < 0x385) {
            *(u16 *) (e + 0x46E) += 15;
            *(u16 *) (e + 0x46C) = sum;
        } else {
            c = e[0x471] + 1;
            e[0x471] = c;
            if (c >= 6) {
                e[0x470] = 0;
                e[0x472] = 3;
                continue;
            }
        }
        func_8026A454(*(s16 *) (e + 0x464), *(s16 *) (e + 0x466), *(s16 *) (e + 0x468), sum,
                      *(u16 *) (e + 0x46A), m);
        seg = *(u32 *) (e + 0x460) - 0x80000000;
        a[0] = 0xBC002406;
        a[1] = seg;
        b[0] = 0xBC002406;
        b[1] = seg;
        a[2] = 0x06000000;
        a[3] = (u32) e - 0x80000000;
        b[2] = 0x06000000;
        b[3] = (u32) (e + 0x230) - 0x80000000;
        a += 4;
        b += 4;
    }
    *gfxA = a;
    *gfxB = b;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C12E0.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1438.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C18D4.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* With tbl the s16 table at e->info->unk3C and n = info->unk0 entries,
 * returns n minus the number of leading entries >= tbl[index - 1] (so 0 if
 * all n are). The asm takes e in t9 and index in t3, returns in a2 and saves
 * v0, a0, a1 and a3; asm caller func_802C1438 keeps t3, t9, f12 and f14
 * live. */
s32 func_802C1A28(Unk802C1DD0Entry *e, s32 index) {
    Unk802C1DD0Info *info = e->info;
    s16 *tbl = (s16 *) ((u8 *) info + info->unk3C);
    s32 key = tbl[index - 1];
    s32 n = info->unk0;

    while (n != 0 && *tbl >= key) {
        tbl++;
        n--;
    }
    return n;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1A28.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Returns 1 if every D_803F4030 entry with unkEB set is done (func_802BD064),
 * else 0. The asm loops with `!=`, so D_803F7654 must be D_803F4030 + n
 * entries. The asm saves and restores v1, t0, t1 and t3. */
s32 func_802C1AA0(void) {
    Unk802C1DD0Entry *end = D_803F7654;
    Unk802C1DD0Entry *e;

    for (e = D_803F4030; e != end; e++) {
        if (e->unkEB == 0) {
            continue;
        }
        if (func_802BD064(e + 1) == 0) {
            return 0;
        }
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1AA0.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Counts the D_803F4030 entries with unkEB set that are done (func_802BD064).
 * 1D990.c declares it u8, but the asm returns the full 32-bit count. The asm
 * loops with `!=`, so D_803F7654 must be D_803F4030 + n entries. The asm saves
 * and restores v1, t0, t1 and t3. */
s32 func_802C1B1C(void) {
    Unk802C1DD0Entry *end = D_803F7654;
    Unk802C1DD0Entry *e;
    s32 count = 0;

    for (e = D_803F4030; e != end; e++) {
        if (e->unkEB == 0) {
            continue;
        }
        if (func_802BD064(e + 1) != 0) {
            count++;
        }
    }
    return count;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1B1C.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_803F7660;
extern s32 D_803F7670;
extern s32 D_803F7674;
extern s32 D_803F7678;

/* Finds the distance from the player (D_803EF6DC/E0/E4) to the closest
 * target, starting from a limit of 9999999, over the same three tables as
 * func_802BCE40 (a later candidate wins only when strictly closer):
 *   1: D_803F4030 entries with unkEB set, unkEA clear and not yet done
 *      (func_802BD064), skipping one whose z is below the player's when its
 *      unkC is below the distance; records unk28, y, unk2C;
 *   2: D_8039C800 entries with unk26 clear; records x, z;
 *   3: D_80306480 entries of the current level whose z isn't below the
 *      player's, with an id whose D_803B9890 unk51 is set; records x, z.
 * The recorded values go to D_803F7670/74/78 (74 only for kind 1), the
 * distance to D_803F7660 and the return value. The asm loops over D_803F4030
 * with `!=`, so D_803F7654 must be D_803F4030 + n entries. */
u32 func_802C1B9C(void) {
    Unk802C1DD0Entry *end = D_803F7654;
    Unk802C1DD0Entry *e;
    Unk8039C800 *t;
    Unk80306480 *p;
    s8 *id;
    s32 bestDist = 9999999;
    s32 dist;
    s32 v;
    s32 i;

    for (e = D_803F4030; e != end; e++) {
        if (e->unkEB == 0 || e->unkEA != 0) {
            continue;
        }
        DIST3_802ABCDC(dist, e->pos[0], e->pos[1], e->pos[2], D_803EF6DC, D_803EF6E0, D_803EF6E4);
        if (dist >= bestDist) {
            continue;
        }
        if (func_802BD064(e + 1) != 0) {
            continue;
        }
        if (e->pos[2] < D_803EF6E4 && e->unkC < dist) {
            continue;
        }
        D_803F7670 = e->unk28;
        bestDist = dist;
        D_803F7674 = e->pos[1];
        D_803F7678 = e->unk2C;
    }
    for (i = D_8039C940, t = D_8039C800; i != 0; i--, t++) {
        if (t->unk26 != 0) {
            continue;
        }
        DIST3_802ABCDC(dist, t->pos[0], t->pos[1], t->pos[2], D_803EF6DC, D_803EF6E0, D_803EF6E4);
        if (dist < bestDist) {
            D_803F7670 = t->pos[0];
            D_803F7678 = t->pos[2];
            bestDist = dist;
        }
    }
    for (p = D_80306480; p->level != -1; p++) {
        if (p->level != D_802E8BDC) {
            continue;
        }
        DIST3_802ABCDC(dist, p->pos[0], p->pos[1], p->pos[2], D_803EF6DC, D_803EF6E0, D_803EF6E4);
        if (dist >= bestDist || p->pos[2] < D_803EF6E4) {
            continue;
        }
        for (id = p->ids; *id != -1; id++) {
            LOOKUP_8029D210(v, *id);
            if (v != 0) {
                D_803F7670 = p->pos[0];
                D_803F7678 = p->pos[2];
                bestDist = dist;
                break;
            }
        }
    }
    D_803F7660 = bestDist;
    return bestDist;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1B9C.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Unk802C1DD0Info/Entry and D_803F4030/D_803F7654 are declared at the top of the file. */
extern s32 D_803F7684;
extern s32 D_803F7688;
extern u8 D_8036EB92;
extern struct {
    /* 0x00 */ s32 total;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ u8 active;
} D_8036EA70;

/* Tallies the entries D_803F4030..D_803F7654 (0xFC bytes each). With
 * onlyFlagged set, only entries whose unkEB is nonzero count, and each of
 * those stores its unk30 into D_803F7684 (so the last one wins). Of the
 * counted entries whose info->type isn't 0xFF: those with type != 1 are
 * counted into D_8036EB92; if the entry's unkEA or D_803F7688 is nonzero,
 * type != 1 entries are also counted into D_8036EA70.active and every such
 * entry's info->value is summed into D_8036EA70.total. Clears D_803F7688.
 * The asm loops with `!=`, so D_803F7654 must be D_803F4030 + n entries.
 * The asm keeps its counters in s0-s2 but saves and restores s0-s7. */
void func_802C1DD0(s32 onlyFlagged) {
    Unk802C1DD0Entry *e;
    Unk802C1DD0Entry *end = D_803F7654;
    s32 counted = 0;
    s32 active = 0;
    s32 total = 0;

    for (e = D_803F4030; e != end; e++) {
        Unk802C1DD0Info *info = e->info;

        if (onlyFlagged != 0) {
            if (e->unkEB == 0) {
                continue;
            }
            D_803F7684 = e->unk30;
        }
        if (info->type == 0xFF) {
            continue;
        }
        if (info->type != 1) {
            counted++;
        }
        if ((e->unkEA | D_803F7688) == 0) {
            continue;
        }
        if (info->type != 1) {
            active++;
        }
        total += info->value;
    }
    D_8036EB92 = counted;
    D_8036EA70.active = active;
    D_8036EA70.total = total;
    D_803F7688 = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1DD0.s")
#endif

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
#ifdef NON_MATCHING
/* Returns record `index` (1-based) of a chain of variable-length records.
 * The chain starts at level + *(s32 *)(level + 0x74), where level is
 * D_80358074. Each record is {u16 offsetOfNextRecord, s16 data[]}, with
 * offsets relative to the chain start, and the first record sits at chain + 4.
 * The result points at the record's data, just past its link. index 0 makes
 * the asm loop ~2^32 times, so callers must pass index >= 1. */
s16 *func_802C1EE0(s32 index) {
    u8 *chain = (u8 *) D_80358074 + *(s32 *) ((u8 *) D_80358074 + 0x74);
    u16 *record = (u16 *) (chain + 4);

    for (index--; index != 0; index--) {
        record = (u16 *) (chain + *record);
    }
    return (s16 *) (record + 1);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1EE0.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_8035805C;  /* selects the matrix buffer */
extern u8 *D_803F7820; /* matrix buffers */
extern u8 *D_803F7824;

/* Moves part `part` (1-based) of group `group` (1-based) of the level's group
 * chain (see func_802C2054) to (x, y, z). The group's mtxOffset[part - 1]
 * locates a matrix in D_803F7824 (if D_8035805C) or D_803F7820; it becomes a
 * translation by (x, y, z) in s15.16 (as func_802ACA60 builds it), stored in
 * the N64 Mtx layout (func_802AC8CC splits the words into integer and
 * fraction halves). Every part of the group's records that follows the same
 * offset gets base = (x, y, z) >> 11. Groups are found through their `next`
 * offsets; group 0 makes the asm loop ~2^32 times, so callers pass >= 1. */
void func_802C1F30(s32 group, s32 part, s32 x, s32 y, s32 z) {
    u8 *level = D_80358074;
    u8 *chain = level + *(s32 *) (level + 0x74);
    Unk802C2054Group *g = (Unk802C2054Group *) (chain + 4);
    Unk802C2054Record *r;
    u16 off;
    u16 *mtx;
    s32 m[16];
    s32 n;
    s32 i;
    s32 k;

    for (group--; group != 0; group--) {
        g = (Unk802C2054Group *) (chain + (u16) g->next);
    }
    off = *(u16 *) ((u8 *) g + 0x14 + (part - 1) * 2); /* g->mtxOffset[part - 1] */
    mtx = (u16 *) (((D_8035805C != 0) ? D_803F7824 : D_803F7820) + off);

    for (i = 0; i < 16; i++) {
        m[i] = (i % 5 == 0) ? 0x10000 : 0;
    }
    m[12] = x;
    m[13] = y;
    m[14] = z;
    for (i = 0; i < 16; i++) {
        mtx[i] = (u32) m[i] >> 16;
        mtx[16 + i] = m[i];
    }

    x >>= 11;
    y >>= 11;
    z >>= 11;
    r = (Unk802C2054Record *) (g + 1);
    for (n = g->count; n != 0; n--, r++) {
        for (k = 0; k < 3; k++) {
            if (r->part[k].unk0 == off) {
                r->part[k].base[0] = x;
                r->part[k].base[1] = y;
                r->part[k].base[2] = z;
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1F30.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Unk802C2054Group/Record are declared at the top of the file. */
extern s32 *D_803F7828;

/* Walks the level's group list (at D_80358074 + *(s32 *)(D_80358074 + 0x74);
 * an initial zero word means "empty") and, for each record, writes nine words
 * (index[k] << 5) + part[k / 3].base[k % 3] to the output array D_803F7828,
 * 10 words (0x28 bytes) per record; word 9 of each output slot is left
 * untouched. Groups are stored back to back: a 0x2C-byte header, then
 * `count` 0x44-byte records, then the next header. The walk stops after a
 * group whose header's `next` is -1. The asm also computes chain + next but
 * never uses it (dead), so `next` acts only as an end marker here.
 * `count` is counted down with `!=` and must not be negative. */
void func_802C2054(void) {
    u8 *level = D_80358074;
    u8 *chain = level + *(s32 *) (level + 0x74);
    s32 *out = D_803F7828;
    u8 *p;
    s16 next;
    s32 n;
    s32 k;
    Unk802C2054Record *r;

    if (*(s32 *) chain == 0) {
        return;
    }
    p = chain + 4;
    do {
        next = ((Unk802C2054Group *) p)->next;
        n = ((Unk802C2054Group *) p)->count;
        r = (Unk802C2054Record *) (p + sizeof(Unk802C2054Group));
        while (n != 0) {
            n--;
            for (k = 0; k < 9; k++) {
                out[k] = (r->index[k] << 5) + r->part[k / 3].base[k % 3];
            }
            out += 10;
            r++;
        }
        p = (u8 *) r;
    } while (next != -1);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C2054.s")
#endif
