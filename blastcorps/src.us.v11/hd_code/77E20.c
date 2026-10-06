#include "common.h"
#include <ultra64.h>
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_803059F0 ((Unk803059F0 *) D_803059F0)
#define D_8039C800 ((Unk8039C800 *) D_8039C800)
#define D_803A6B30 ((Unk803A6B30 *) D_803A6B30)
#define D_803B9890 ((Unk803B9890 *) D_803B9890)
#define D_803EFED0 ((Unk803EFED0 *) D_803EFED0)
#define D_803F3FF8 (*(Unk803F3FF8 *) D_803F3FF8)
#define D_803F4030 ((Unk802C1DD0Entry *) D_803F4030)
#ifdef NON_MATCHING
#define D_803649E8 (*(u8 *) &D_803649E8)
#define D_803F7654 (*(Unk802C1DD0Entry * *) &D_803F7654)
#define D_803F7828 (*(s32 * *) &D_803F7828)
#endif
/* end of views */


#ifdef NON_MATCHING
/* Types, tables and helpers shared by the port-phase rewrites in this file.
 *
 * Several rewrites below call hand-written helpers that pass arguments and
 * results in t-/s-registers. Every such helper now has a C rewrite (and a
 * tools_port/conventions.txt line), so it is called normally through the
 * prototypes below: func_802ABCDC (62740), func_8029D210 (56040),
 * func_802ACA60 and func_802AC8CC (679E0), and this file's own helpers. */

/* A model/object header: mostly byte offsets (from the header) of its tables. */
typedef struct {
    /* 0x00 */ u16 unk0;  /* number of s16 entries at unk3C (func_802C1A28) */
    /* 0x02 */ u8 pad2[2];
    /* 0x04 */ u8 type;   /* 0xFF = ignored, 1 = not counted; divisor in func_802C18D4 */
    /* 0x05 */ u8 unk5;   /* scale of the debris records (func_802BF978, func_802BFDAC) */
    /* 0x06 */ u8 unk6;   /* passed to func_80264CB4 (func_802BF384) */
    /* 0x07 */ u8 unk7;   /* nonzero: no display list (func_802C09B8) */
    /* 0x08 */ s32 value; /* summed */
    /* 0x0C */ u8 padC;
    /* 0x0D */ u8 unkD;   /* 1-based index of the part func_802BF534 ignores */
    /* 0x0E */ u8 padE[0xE];
    /* 0x1C */ s32 unk1C; /* byte offset from this record (func_802BCE40) */
    /* 0x20 */ s32 unk20; /* offset of an s16[4] box x0, z0, x1, z1 (func_802C1214) */
    /* 0x24 */ s32 unk24; /* offsets of a table of 0x14-byte records (func_802BF1F0) */
    /* 0x28 */ s32 unk28; /*   and its end */
    /* 0x2C */ s32 unk2C; /* offset of 8-byte s16 x, y, z points (func_802BF978) */
    /* 0x30 */ s32 unk30; /* offsets of 0x38-byte records (func_802BF898) */
    /* 0x34 */ s32 unk34; /*   their end = start of more (func_802BFD1C) */
    /* 0x38 */ s32 unk38; /*   their end = start of the conditional DL records (func_802C09B8) */
    /* 0x3C */ s32 unk3C; /* offset of an s16 table (func_802C1A28); end of the DL records */
    /* 0x40 */ s32 unk40; /* offset of an s16 table indexed 1-based (func_802C038C) */
    /* 0x44 */ s32 unk44; /* offsets of variable-length records (func_802C038C) */
    /* 0x48 */ s32 unk48; /*   and their end */
} Unk802C1DD0Info;

typedef struct Unk802C1DD0Entry {
    /* 0x00 */ Unk802C1DD0Info *info;
    /* 0x04 */ void *unk4;  /* its collision triangles (Unk803B9890), func_802BF668 */
    /* 0x08 */ void *unk8;  /*   and their end */
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


typedef struct {
    /* 0x00 */ s32 pos[3];
    /* 0x0C */ u8 padC[0x1A];
    /* 0x26 */ u8 unk26;
    /* 0x27 */ u8 pad27;
} Unk8039C800; /* size 0x28 */


typedef struct {
    /* 0x00 */ s32 pos[3];
    /* 0x0C */ s8 ids[9]; /* -1 terminated (the scan isn't bounded), looked up in D_803B9890 */
    /* 0x15 */ s8 level;  /* -1 ends the table */
    /* 0x16 */ u8 unk16[0x1A];
} Unk80306480; /* size 0x30 */

extern Unk80306480 D_80306480[];

/* A collision triangle (func_802BD99C keeps its plane up to date). */
typedef struct Unk803B9890 {
    /* 0x00 */ s64 normal[3]; /* unnormalised: cross product of two edges */
    /* 0x18 */ s64 d;         /* -(normal . vertex 1) */
    /* 0x20 */ f32 len;       /* |normal| */
    /* 0x24 */ f32 lenSq;
    /* 0x28 */ s32 v[3][3];   /* vertices x, y, z */
    /* 0x4C */ u16 unk4C;  /* ring index (func_802BF264) */
    /* 0x4E */ u8 axis;       /* dominant normal axis: 0 = z, 1 = y, 2 = x */
    /* 0x4F */ u8 pad4F;
    /* 0x50 */ u8 id;
    /* 0x51 */ u8 unk51;
    /* 0x52 */ u16 unk52;
    /* 0x54 */ u8 pad54;
    /* 0x55 */ u8 unk55;
    /* 0x56 */ u8 unk56;
    /* 0x57 */ u8 unk57;
    /* 0x58 */ u8 unk58;
    /* 0x59 */ u8 pad59[7];
} Unk803B9890; /* size 0x60 */

/* What func_802BD85C/func_802BD99C get in v0: a model header and a range of
 * its triangles. */
typedef struct Unk802BD99CModel {
    /* 0x00 */ u8 *data;
    /* 0x04 */ Unk803B9890 *start;
    /* 0x08 */ Unk803B9890 *end;
} Unk802BD99CModel;


/* Callees in other files (C rewrites; see conventions.txt for their asm registers). */

/* Rewritten in this file (see conventions.txt for their asm registers). */
s32 func_802BC714(void);
s32 func_802BC7DC(void);
s32 func_802BD064(Unk802C1DD0Entry *next);
s32 func_802BFB50(s32 lo, s32 hi);
void func_802BFBF4(Unk802C1DD0Entry *e, s32 *obj);
void func_802C1214(u8 *base, Unk802C1DD0Entry *e);

/* The 0x38-byte debris record that func_802BF978, func_802BFDAC (and 8A2E0's
 * func_802CF3E0) fill in and hand to func_802C04F0. */
typedef struct {
    /* 0x00 */ s32 pos[3];
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 unk10[6];
    /* 0x28 */ u32 unk28;
    /* 0x2C */ u16 unk2C;
    /* 0x2E */ u8 unk2E;
    /* 0x2F */ u8 pad2F;
    /* 0x30 */ u8 unk30;
    /* 0x31 */ u8 unk31;
    /* 0x32 */ u8 pad32[2];
    /* 0x34 */ u8 unk34;
    /* 0x35 */ u8 unk35;
    /* 0x36 */ u8 pad36[2];
} Unk803F3FF8;


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

/* If D_803F7805 is set, clears it and, unless D_803643DB or D_80364AC1 is set
 * and the level isn't finished yet, marks it finished (D_803643DA and
 * D_802E8BD8 = 1). "Finished" means D_803F7806 is set, every D_8039C800 entry
 * has unk26 set (func_802BC7DC) and no D_80306480 entry of the current level
 * at or beyond the player's z has an id whose D_803B9890 unk51 is set
 * (func_802BC714). Then, if D_803643DB is clear: in game mode 0x40 it calls
 * func_80275390(0x40), otherwise it sets D_803649E8 = 1 and the next mode 8.
 * Declared void in hd.c; the asm leaves garbage in v0. */
void func_802BC5E0(void) {
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
        if (func_802BC714() == 0) {
            return;
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
#ifdef NON_MATCHING
/* Returns 0 if some D_80306480 entry of the current level whose z is at or
 * beyond the player's (D_803EF6E4, read once) lists an id (s8, sign-extended;
 * the list ends at -1) whose D_803B9890 unk51 is set (func_8029D210), else 1.
 * The table ends at a level of -1. The asm returns in t4 and saves t0, t1, t3,
 * t5, s0 and s6; asm caller func_802BDDB4 keeps a0-a3, t0-t2, f12 and f14
 * live across the call. */
s32 func_802BC714(void) {
    s32 z = D_803EF6E4;
    Unk80306480 *e;
    s8 *id;

    for (e = D_80306480; e->level != -1; e++) {
        if (e->level != D_802E8BDC || e->pos[2] < z) {
            continue;
        }
        for (id = e->ids; *id != -1; id++) {
            if (func_8029D210(*id) != 0) {
                return 0;
            }
        }
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BC714.s")
#endif

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
#ifdef NON_MATCHING

/* Delayed damage: for each of the 40 D_803F7690 entries {object, u16 amount,
 * u8 part, u8 timer} with a running timer, counts it down; on reaching 0 the
 * entry is spent (timer 0), the screen shakes (10, min(amount, 500)) and the
 * part's percentage += amount, capped at 100. A part reaching 100 collapses:
 * func_802C1438, func_802C09B8 unless D_803643D6 or D_803643D7 is set,
 * func_802C0E8C, func_802BF384 unless the game mode D_80364A90 is 0x200 or
 * 0x400, and the part's triangles off. Then func_802BF668, func_802BF534 and
 * func_802BF898(e, part, percentage).
 * ABI (00000.c calls it as void(void)); the asm saves s0-s7, gp, fp; the s1
 * the survey lists as an input is only saved. */
void func_802BCA2C(void) {
    u8 *p = D_803F7690[0];
    s32 n;

    for (n = 0x28; n != 0; n--, p += 8) {
        Unk802C1DD0Entry *e;
        s32 timer = p[7];
        s32 amount;
        s32 part;
        s32 level;
        u8 *pct;

        if (timer == 0) {
            continue;
        }
        timer--;
        if (timer != 0) {
            p[7] = timer;
            continue;
        }
        amount = *(u16 *) (p + 4);
        p[7] = 0;
        e = *(Unk802C1DD0Entry **) p;
        part = p[6];
        D_802E8BE4 = 10;
        if (amount < 0x1F5) {
            D_802E8BE8 = amount;
        } else {
            D_802E8BE8 = 500;
        }
        pct = (u8 *) e + 0xEC + part - 1;
        level = *pct + amount;
        if (level >= 100) {
            level = 100;
        }
        *pct = level;
        if (level == 100) {
            Unk803B9890 *t;
            Unk803B9890 *end;

            func_802C1438(e, part);
            if (D_803643D6 == 0 && D_803643D7 == 0) {
                func_802C09B8(part, e);
            }
            func_802C0E8C(part, e);
            if (D_80364A90 != 0x200 && D_80364A90 != 0x400) {
                func_802BF384(e);
            }
            end = e->unk8;
            for (t = e->unk4; t != end; t++) {
                if (t->unk52 == part) {
                    t->unk51 = 0;
                }
            }
        }
        func_802BF668(e);
        func_802BF534(e);
        func_802BF898(e, part, level);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCA2C.s")
#endif

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

/* If D_80358060 is set, copies buffer A (D_803F77D8 up to its write pointer)
 * into buffer B byte by byte, front to back; then sets B's write pointer to
 * just past the copied bytes (to B's start when nothing was copied). The asm
 * loops with `!=`, so A's write pointer must not be below D_803F77D8.
 * Register note: the asm saves/restores v0, v1, a0 and a1; asm caller
 * func_802BE77C keeps a0, a1, t6, f12 and f14 live across the call. */
void func_802BCC48(void) {
    u8 *dst = D_803F77E8;

    if (((s32) D_80358060) != 0) {
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
#ifdef NON_MATCHING
/* Appends value (its low byte) to buffer A unless buffer A already contains
 * it (func_802BCD80). The asm takes value in fp (a value, not a base pointer)
 * and saves v0 and v1, so it changes nothing; asm callers keep many registers
 * live across the call (func_8029C914: a2-t1, t3-t8, f12, f14; func_8029CB04:
 * a0-t1, t8, f12, f14; func_802BEBB0: a0, a1, t3, t8, t9, f12, f14). The
 * asm's `addi` on the write pointer can't trap for a RAM address. */
void func_802BCCD4(s32 value) {
    if (func_802BCD80(value) == 0) {
        *D_803F77D4++ = value;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCCD4.s")
#endif

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
    s32 i;
    u8 *ptr;

    for (e = D_803F4030; e != end; e++) {
        if (e->unkEB == 0 || e->unkEA != 0) {
            continue;
        }
        dist = func_802ABCDC(e->pos[0], e->pos[1], e->pos[2], D_803643E0, D_803643E4, D_803643E8);
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
        dist = func_802ABCDC(t->pos[0], t->pos[1], t->pos[2], D_803643E0, D_803643E4, D_803643E8);
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
        dist = func_802ABCDC(p->pos[0], p->pos[1], p->pos[2], D_803643E0, D_803643E4, D_803643E8);
        if (dist >= bestDist) {
            continue;
        }
        for (id = p->ids; *id != -1; id++) {
            if (func_8029D210(*id) != 0) {
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
    D_8036C790 = (s16 *) ptr;
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
    dist = func_802ABCDC(x, 0, z, D_803EF6DC, 0, D_803EF6E4);
    if (((u16) D_803EF6FC) != 0) {
        dist = (u32) dist / ((u16) D_803EF6FC);
    }
    D_8036C7C8 = dist;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BD10C.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern Mtx *D_803F7658; /* 2 matrices (func_802C1F30's falling parts) */
extern s32 D_803F765C;  /* Mtx pool cursor for func_802933A0 */
extern s32 D_803F24D0[]; /* matrix pools of the two frames */
extern s32 D_803F2ED0[];
extern s32 *D_803F3964;
void func_802BD85C(Unk802BD99CModel *model, s32 id);
void func_802BDDB4(Unk802C1DD0Entry *obj, u8 *rec, u8 *end);
u32 *func_802BE228(u32 *src, u32 *srcEnd, u32 *dst, u8 *tbl, u8 *tblEnd);
u32 *func_802BE3C8(u32 *src, u32 *srcEnd, u32 *dst, u8 *tbl, u8 *tblEnd);
s32 func_802BE574(Unk802C1DD0Entry *e, s32 off, s32 dy);
void func_802BFD1C(Unk802C1DD0Entry *e, s32 index);
u32 *func_802C08C4(u32 *gfx);
u32 *func_802C0CBC(u32 *gfx);
void func_802C12E0(u32 **gfxA, u32 **gfxB);

/* Copies the 8-byte display-list commands [src, end) to dst; returns the new dst. */
#define COPY_DL_802BD1F8(dst, src, end)      \
    {                                        \
        u64 *_s = (u64 *) (src);             \
        u64 *_e = (u64 *) (end);             \
        u64 *_d = (u64 *) (dst);             \
                                             \
        while (_s != _e) {                   \
            *_d++ = *_s++;                   \
        }                                    \
        (dst) = (u32 *) _d;                  \
    }

/* Draws the D_803F4030 objects (up to D_803F7654, read once, walked with
 * `!=`) into four display lists: gA / gB (the two halves of the scene list),
 * gC / gD (per-object lists the scene calls). D_803F7658/765C = mtxA/mtxB,
 * func_802C08C4(gE) and func_802C0CBC(gF) (their new ends are dropped), the
 * matrix pool is this frame's (D_803F24D0 or D_803F2ED0 by D_8035805C).
 * Per object whose unkE8 is in the D_803C30A8 list (-1 ends it) and that
 * isn't a 0x38 object while func_802BD8C8 is set:
 * - with a pose (info u16 +0xE != 0): func_802933A0 builds its matrix
 *   (words +0x1C..+0x24, the pose count, the next D_803F765C Mtx (advanced by
 *   0x40), state at +0x38, gA, gB, word +0x34, the position) and both scene
 *   lists step 8; the object moves to D_803F7664/68/6C (func_802BD99C by the
 *   difference);
 * - animated textures (func_802BDDB4 with info's table unk28..unk2C), then
 *   gA/gB get a segment-6 command for info + 0x50 and a G_DL to gC/gD
 *   (physical), and the commands info+unk10..unk18 are copied to gC and gD;
 * - the part records info+unk38..unk3C: {word n, n x {u16 part, u8 mode, u8
 *   limit}, 4 words}, the u16 at +4 being the record's (1-based) part p too.
 *   A record whose conditions all hold (part's percentage < limit fails for
 *   mode != 0, > limit for mode 0) is drawn: with a moving part
 *   (e->unk48[p - 1]), its counter unk68[p - 1] advances, dy = c * c * -30000;
 *   if dy <= -(table u16 at unk3C + 2(p - 1) << 16) the part drops
 *   (unk48 = 0, func_802BFD1C, shake 10/300, percentage 100,
 *   func_802BD85C); its matrix (func_802BE574) is pushed in gC; the
 *   record's gC and gD command ranges (words 0-1 / 2-3, offsets from info)
 *   are copied (through func_802BE228 / func_802BE3C8 when the texture table
 *   is non-empty), with G_POPMTX in gC and the matrix push / G_POPMTX in gD
 *   for a moving part. If the first record holds, it is the only one drawn;
 *   otherwise every holding record is.
 * Every object (listed or not) then ends gC and gD (G_ENDDL) and, with a
 * pose, pops the matrix in gA and gB. Finally func_802C12E0(&gA, &gB) and
 * G_ENDDL in both (not advanced).
 * ABI (00000.c passes eight pointers); the asm saves s0-s7, gp, fp and uses
 * gp as scratch. Its trapping add/sub stay in game range. */
void func_802BD1F8(u32 *gA, u32 *gB, u32 *gC, u32 *gD, s32 mtxA, s32 mtxB, u32 *gE, u32 *gF) {
    Unk802C1DD0Entry *end;
    Unk802C1DD0Entry *o;

    D_803F7658 = (Mtx *) mtxA;
    D_803F765C = mtxB;
    func_802C08C4(gE);
    func_802C0CBC(gF);
    if (D_8035805C != 0) {
        D_803F3964 = D_803F24D0;
    } else {
        D_803F3964 = D_803F2ED0;
    }
    end = D_803F7654;
    for (o = D_803F4030; o != end; o++) {
        u8 *ob = (u8 *) o;
        u8 *info;
        u8 *tbl;
        u8 *tblEnd;
        u8 *rec;
        u8 *recEnd;
        s16 *s;
        s32 count;
        s32 first;

        for (s = (s16 *) D_803C30A8;; s++) {
            if (*s == -1) {
                goto tail;
            }
            if (*s == o->unkE8) {
                break;
            }
        }
        if (o->unk30 == 0x38 && func_802BD8C8() != 0) {
            goto tail;
        }
        info = (u8 *) o->info;
        count = *(u16 *) (info + 0xE);
        if (count != 0) {
            s32 mtx = D_803F765C;
            s32 ox;
            s32 oy;
            s32 oz;

            D_803F765C = mtx + 0x40;
            func_802933A0(*(s32 *) (ob + 0x1C), *(s32 *) (ob + 0x20), *(s32 *) (ob + 0x24), count, (Mtx *) mtx,
                          ob + 0x38, (Gfx *) gA, (Gfx *) gB, *(s32 *) (ob + 0x34), o->pos[0], o->pos[1],
                          o->pos[2]);
            gA += 2;
            gB += 2;
            ox = o->pos[0];
            oy = o->pos[1];
            oz = o->pos[2];
            o->pos[0] = D_803F7664;
            o->pos[1] = D_803F7668;
            o->pos[2] = D_803F766C;
            func_802BD99C((Unk802BD99CModel *) o, o->pos[0] - ox, o->pos[1] - oy, o->pos[2] - oz);
        }
        info = (u8 *) o->info;
        tbl = info + *(s32 *) (info + 0x28);
        tblEnd = info + *(s32 *) (info + 0x2C);
        rec = info + *(s32 *) (info + 0x38);
        recEnd = info + *(s32 *) (info + 0x3C);
        func_802BDDB4(o, tbl, tblEnd);
        gA[0] = gB[0] = 0xBC002406;
        gA[1] = gB[1] = (u32) (info + 0x50) - 0x80000000;
        gA[2] = gB[2] = 0x06000000;
        gA[3] = (u32) gC - 0x80000000;
        gB[3] = (u32) gD - 0x80000000;
        gA += 4;
        gB += 4;
        {
            u64 *p = (u64 *) (info + *(s32 *) (info + 0x10));
            u64 *pe = (u64 *) (info + *(s32 *) (info + 0x18));

            while (p != pe) {
                u64 w = *p++;

                *(u64 *) gC = w;
                gC += 2;
                *(u64 *) gD = w;
                gD += 2;
            }
        }
        first = 1;
        while (rec != recEnd) {
            s32 n = *(s32 *) rec;
            s32 off = (*(u16 *) (rec + 4) - 1) * 2;
            s32 moving;
#ifdef NON_MATCHING
            s32 mtxAddr = 0; /* set and read only when moving != 0 */
#else
            s32 mtxAddr;
#endif
            u32 *src;
            u32 *srcEnd;

            rec += 4;
            while (n != 0) {
                s32 part = *(u16 *) rec;
                s32 mode = rec[2];
                s32 limit = rec[3];
                s32 pct = ob[0xEC + part - 1];

                n--;
                rec += 4;
                if (mode != 0 ? (pct < limit) : (limit < pct)) {
                    rec += n * 4 + 0x10;
                    goto next;
                }
            }
            moving = *(u16 *) (ob + 0x48 + off);
            if (moving != 0) {
                u16 *ctr = (u16 *) (ob + 0x68 + off);
                u32 c = *ctr + 1;
                s32 dy;
                s32 floor;

                *ctr = c;
                dy = c * c * (u32) -30000;
                floor = -(*(u16 *) (info + *(s32 *) (info + 0x3C) + off) << 16);
                if (floor >= dy) {
                    u32 idx = (u32) off >> 1;

                    *(u16 *) (ob + 0x48 + off) = 0;
                    func_802BFD1C(o, idx);
                    D_802E8BE4 = 10;
                    D_802E8BE8 = 300;
                    ob[idx + 0xEC] = 100;
                    func_802BD85C((Unk802BD99CModel *) o, idx);
                }
                mtxAddr = func_802BE574(o, off, dy);
                gC[0] = 0x01040040;
                gC[1] = mtxAddr;
                gC += 2;
            }
            src = (u32 *) (info + ((s32 *) rec)[0]);
            srcEnd = (u32 *) (info + ((s32 *) rec)[1]);
            if (tbl != tblEnd) {
                gC = func_802BE228(src, srcEnd, gC, tbl, tblEnd);
            } else {
                COPY_DL_802BD1F8(gC, src, srcEnd);
            }
            if (moving != 0) {
                gC[0] = 0xBD000000;
                gC[1] = 0;
                gC += 2;
                gD[0] = 0x01040040;
                gD[1] = mtxAddr;
                gD += 2;
            }
            src = (u32 *) (info + ((s32 *) rec)[2]);
            srcEnd = (u32 *) (info + ((s32 *) rec)[3]);
            if (tbl != tblEnd) {
                gD = func_802BE3C8(src, srcEnd, gD, tbl, tblEnd);
            } else {
                COPY_DL_802BD1F8(gD, src, srcEnd);
            }
            if (moving != 0) {
                gD[0] = 0xBD000000;
                gD[1] = 0;
                gD += 2;
            }
            rec += 0x10;
            if (first) {
                break;
            }
        next:
            first = 0;
        }
    tail:
        gC[0] = 0xB8000000;
        gC[1] = 0;
        gD[0] = 0xB8000000;
        gD[1] = 0;
        gC += 2;
        gD += 2;
        if (*(u16 *) ((u8 *) o->info + 0xE) != 0) {
            gA[0] = 0xBD000000;
            gA[1] = 0;
            gB[0] = 0xBD000000;
            gB[1] = 0;
            gA += 2;
            gB += 2;
        }
    }
    func_802C12E0(&gA, &gB);
    gA[0] = 0xB8000000;
    gA[1] = 0;
    gB[0] = 0xB8000000;
    gB[1] = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BD1F8.s")
#endif

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
#ifdef NON_MATCHING

/* An animation channel record (0xC bytes, then n words). */
#define ANIM_N(r) ((r)[4])                     /* frame count */
#define ANIM_FRAME(r) ((r)[5])                 /* current frame */
#define ANIM_TYPE(r) ((r)[6])                  /* 0 timed, 1 global clock, else player heading */
#define ANIM_FRAC(r) (*(u32 *) ((r) + 8))       /* 0..0xFE blend */
#define ANIM_LO(r) (*(u16 *) ((r) + 0xC))       /* period / heading range start */
#define ANIM_HI(r) (*(u16 *) ((r) + 0xE))       /* tick counter / heading range end */

/* Quarter-turn heading (0..0x3FF) from the sine num / d: arcsine >> 4. */
static s32 heading_802BDDB4(s32 num, f32 d) {
    s32 sine;

    CVT_W_S(sine, 65536.0f * ((f32) num / d));
    return (u32) func_802AD7FC(sine) >> 4;
}

/* Steps the animation channel records rec..end (each 0xC + n * 4 bytes) of
 * object `obj`. Objects with unk30 == 0x38 are skipped while D_803643DB or
 * D_80364AC1 is set unless D_803F7806, func_802BC7DC() and func_802BC714()
 * all say otherwise. Type 0: the u16 tick counter (+0xE) counts up to the
 * period (+0xC), then restarts at 0 and the frame (+5) advances (wrapping at
 * n); the blend word (+8) = tick * 255 / period. Type 1: when (low word of
 * D_803649D8) >> 4 is a multiple of the period, frame = (that >> 8) % n.
 * Other types: the heading from the object (pos[0], pos[2]) to the player
 * (D_803643F8 / D_80364400 >> 11, logical), 0..0xFFF by quadrant, selects
 * the frame within the u16 range lo (+0xC) .. hi (+0xE), wrapping through
 * 0xFFF when hi < lo (the wrap uses 0xFFF - lo, one short of a full turn,
 * as the asm): frame = v / 255, blend = v % 255 with v = offset * (n - 1) *
 * 255 / span (u32), frame 0 below the range and n - 1 above it.
 * Register convention: obj v0, rec t1, end t2 (conventions.txt); the asm
 * saves everything it uses. Its asm caller func_802BD1F8 keeps a0-a2 and
 * t0-t5 live (a mixed N64 build would need a thunk; the native port won't).
 * Divisors (period, n, the spans) of 0 trap (break 7); the object can't sit
 * exactly at the player (0 / 0). */
void func_802BDDB4(Unk802C1DD0Entry *obj, u8 *rec, u8 *end) {
    if (obj->unk30 == 0x38 && (D_803643DB != 0 || D_80364AC1 != 0)) {
        if (D_803F7806 == 0 || func_802BC7DC() == 0 || func_802BC714() == 0) {
            return;
        }
    }
    for (; rec != end; rec += 0xC + ANIM_N(rec) * 4) {
        if (ANIM_TYPE(rec) == 0) {
            u32 period = ANIM_LO(rec);
            u32 tick = ANIM_HI(rec) + 1;

            if (tick == period) {
                u32 frame = ANIM_FRAME(rec) + 1;

                if (frame == ANIM_N(rec)) {
                    frame = 0;
                }
                ANIM_FRAME(rec) = frame;
                tick = 0;
            }
            ANIM_HI(rec) = tick;
            ANIM_FRAC(rec) = tick * 0xFF / period;
        } else if (ANIM_TYPE(rec) == 1) {
            u32 t = (u32) D_803649D8 >> 4;

            if (t % ANIM_LO(rec) == 0) {
                ANIM_FRAME(rec) = (t >> 8) % ANIM_N(rec);
            }
        } else {
            s32 px = (u32) D_803643F8 >> 11;
            s32 pz = (u32) D_80364400 >> 11;
            s32 ox = obj->pos[0];
            s32 oz = obj->pos[2];
            f32 dx = (f32) (s32) (px - ox);
            f32 dz = (f32) (s32) (pz - oz);
            f32 d = sqrtf(dx * dx + dz * dz);
            s32 ang;
            s32 lo = ANIM_LO(rec);
            s32 hi = ANIM_HI(rec);
            u32 v;

            if (px < ox) {
                if (pz < oz) {
                    ang = heading_802BDDB4(ox - px, d) + 0x800;
                } else {
                    ang = heading_802BDDB4(pz - oz, d) + 0xC00;
                }
            } else if (pz < oz) {
                ang = heading_802BDDB4(oz - pz, d) + 0x400;
            } else {
                ang = heading_802BDDB4(px - ox, d);
            }
            if (hi < lo) {
                if (ang < lo && hi < ang) {
                    ANIM_FRAME(rec) = 0;
                    continue;
                }
                v = (ang < lo) ? (u32) (0xFFF - lo) + ang : (u32) (ang - lo);
                v = v * (u32) (ANIM_N(rec) - 1) * 0xFF / ((u32) (0xFFF - lo) + hi);
            } else if (ang < lo) {
                ANIM_FRAME(rec) = 0;
                continue;
            } else if (hi < ang) {
                ANIM_FRAME(rec) = ANIM_N(rec) - 1;
                continue;
            } else {
                v = (u32) (ang - lo) * (u32) (ANIM_N(rec) - 1) * 0xFF / (u32) (hi - lo);
            }
            ANIM_FRAME(rec) = v / 0xFF;
            ANIM_FRAC(rec) = v % 0xFF;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BDDB4.s")
#endif

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
#ifdef NON_MATCHING
extern s32 *D_803F3964; /* matrix pool cursor (0x40-byte 16.16 matrices) */
extern s32 D_803F38D0[]; /* scratch 16.16 matrix */

/* Builds the matrix of one moving part of object e in the next pool slot
 * (D_803F3964, advanced by 0x40) and returns its address - 0x80000000. off
 * is twice the part index: the halfwords at e + 0x68 + off (scale, u16) and
 * +0x88 (s16) / +0xA8 / +0xC8 (u16) give the angles scale * value (32-bit
 * product, + 0xFFF when negative) about x, y and z; the pivot is the u16
 * x, y, z (<< 16) at info + info->unk2C + off * 4. m = translate(-pivot),
 * then m *= rotation x, y, z, translate(pivot), translate(0, dy, 0) (each
 * built in D_803F38D0, func_802ACCCC), converted to the Mtx layout
 * (func_802AC8CC).
 * Register convention: e in v0, off in a3, dy in s6, result in s5. The asm
 * saves a0-t5, t7, t8, s0-s2, s4, s7 (t6 is untouched); asm caller
 * func_802BD1F8 keeps a0-a2 and t0-t6 live across the call. */
s32 func_802BE574(Unk802C1DD0Entry *e, s32 off, s32 dy) {
    s32 *m = D_803F3964;
    u8 *b = (u8 *) e + off;
    u32 scale;
    s32 ax;
    s32 ay;
    s32 az;
    u16 *pivot;
    s32 px;
    s32 py;
    s32 pz;

    D_803F3964 = m + 0x10;
    scale = *(u16 *) (b + 0x68);
    ax = scale * *(s16 *) (b + 0x88);
    if (ax < 0) {
        ax += 0xFFF;
    }
    ay = scale * *(u16 *) (b + 0xA8);
    if (ay < 0) {
        ay += 0xFFF;
    }
    az = scale * *(u16 *) (b + 0xC8);
    if (az < 0) {
        az += 0xFFF;
    }
    pivot = (u16 *) ((u8 *) e->info + e->info->unk2C + off * 4);
    px = pivot[0] << 16;
    py = pivot[1] << 16;
    pz = pivot[2] << 16;
    func_802ACA60(-px, -py, -pz, m);
    func_802ACBDC(ax, D_803F38D0);
    func_802ACCCC(D_803F38D0, m);
    func_802ACAC4(ay, D_803F38D0);
    func_802ACCCC(D_803F38D0, m);
    func_802ACB50(az, D_803F38D0);
    func_802ACCCC(D_803F38D0, m);
    func_802ACA60(px, py, pz, D_803F38D0);
    func_802ACCCC(D_803F38D0, m);
    func_802ACA60(0, dy, 0, D_803F38D0);
    func_802ACCCC(D_803F38D0, m);
    func_802AC8CC((u16 *) m);
    return (u32) m - 0x80000000;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BE574.s")
#endif

#ifdef NON_MATCHING
/* Callees of func_802BE77C / func_802BEADC / func_802BEBB0 (C rewrites; see
 * conventions.txt for their asm registers). */
void func_802BCC48(void);
s32 func_802BCDE0(s32 value);
s32 func_802BE944(Unk802C1DD0Entry *e, s32 bit);
void func_802BE9F8(void);
void func_802BEA30(s32 a, s32 b);
s32 func_802BEA70(s32 a, s32 b);
void func_802BEADC(s32 x, s32 y, s32 z, s32 r, Unk802C1DD0Entry *e, s32 id);
void func_802BEBB0(s32 id, Unk802C1DD0Entry *e, Unk803B9890 *tri);
void func_802BEF9C(Unk802C1DD0Entry *e);
void func_802BEFF4(void);
void func_802BFEE4(u8 *vehicle);
s32 func_802BFF6C(s32 id, s32 value, s32 bit, Unk802C1DD0Entry *e, s32 flag, s32 key);
extern u8 D_803F7802;
extern u8 D_803F7803;

/* The sphere-vs-triangle test the asm inlines as a call chain (see 56040.c,
 * func_8029BD0C): plane distance (func_8029C160), the projected
 * point-in-triangle test, the edge test, the vertex test; 1 on the first hit. */
static s32 sphere_hits_tri_77E20(s32 x, s32 y, s32 z, s32 r, Unk803B9890 *tri) {
    s32 hit[3];
    s32 uv[8];

    if (func_8029C160(x, y, z, r, (u8 *) tri, hit) == 0) {
        return 0;
    }
    func_8029C0DC((u8 *) tri, hit[0], hit[1], hit[2], uv);
    if (func_8029BF64(uv[2], uv[3], uv[4], uv[5], uv[0], uv[1], uv[6], uv[7]) != 0) {
        return 1;
    }
    if (func_8029BD0C(x, y, z, r, (u8 *) tri) != 0) {
        return 1;
    }
    return func_8029BEE4(x, y, z, r, (u8 *) tri) != 0;
}
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_803F77F4;
extern u8 D_803F7807;
extern u8 D_803F780F;

/* Collision pass of vehicle id (record `vehicle`): clears D_803F7802/7803/7807
 * and the once-per-frame latch D_803F780F, copies D_803F77F8 to D_803F77F4,
 * func_802BFEE4(vehicle), resets the hit list (func_802BE9F8), then finds the
 * vehicle's D_803A7300 sphere (first entry whose byte 0x10 is id; the scan
 * isn't bounded). If its flag byte 0x11 is set, every object (D_803F4030 up
 * to D_803F7654, read once, walked with `!=`) within range of the sphere
 * (func_8029CFA4), not a 0x38 object while func_802BD8C8 is set, and not
 * excluded for this vehicle (func_802BE944), gets the triangle pass
 * func_802BEADC. Then buffer A is copied to B (func_802BCC48) unless id is
 * 0xFF, D_803A7425 is set unless the ring span is 0/0xFFF, and vehicle[0x9C],
 * [0x9D], [0x9E] = D_803F7802, D_803F7803, D_803F7807.
 * Register convention: id in t8, vehicle in gp (an argument here: callers pass
 * different records). The asm saves a2-t5, t7, s0, s2 and t8 and leaves v0-a1
 * changed, s1/s3/s4 and f20-f28 changed by func_802BEADC's helpers. Asm callers
 * keep a3 (func_802B327C, func_802B49AC, func_802B6294, func_802B7A88,
 * func_802CBEF0, func_802CD068, func_802CFDE8), t7 (func_802BA9A0) or a3, t0,
 * t1 and t8 (func_802BB274) live across the call. The other leftovers the
 * survey lists as read by callers (v1, a0, t6, s1, s3, s4, f12-f26) are dead
 * there (overwritten, or saved and restored around the call). */
void func_802BE77C(s32 id, u8 *vehicle) {
    u8 *s;

    D_803F7802 = 0;
    D_803F7803 = 0;
    D_803F7807 = 0;
    D_803F77F4 = D_803F77F8;
    D_803F780F = 0;
    func_802BFEE4(vehicle);
    func_802BE9F8();
    for (s = D_803A7300; s[0x10] != id; s += 0x14) {
    }
    if ((s8) s[0x11] != 0) {
        s32 x = ((s32 *) s)[0];
        s32 y = ((s32 *) s)[1];
        s32 z = ((s32 *) s)[2];
        s32 r = ((s32 *) s)[3];
        Unk802C1DD0Entry *end = D_803F7654;
        Unk802C1DD0Entry *o;

        for (o = D_803F4030; o != end; o++) {
            if (func_8029CFA4(z, r, o->pos[0], o->pos[1], x, y, o->pos[2], o->unkC) == 0) {
                continue;
            }
            if (o->unk30 == 0x38 && func_802BD8C8() != 0) {
                continue;
            }
            if (func_802BE944(o, id) != 0) {
                continue;
            }
            func_802BEADC(x, y, z, r, o, id);
        }
    }
    if (id != 0xFF) {
        func_802BCC48();
    }
    if (((u16) D_803A7410) != 0 || ((u16) D_803A7412) != 0xFFF) {
        D_803A7425 = 1;
    }
    vehicle[0x9C] = D_803F7802;
    vehicle[0x9D] = D_803F7803;
    vehicle[0x9E] = D_803F7807;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BE77C.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
typedef struct {
    /* 0x00 */ s32 level; /* 0xFFFF = any level */
    /* 0x04 */ s32 key;
    /* 0x08 */ u32 mask;  /* 0 ends the table */
} Unk803059F0;            /* size 0xC */


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
    s32 *p = (s32 *) D_803F3960;

    p[0] = a;
    p[1] = b;
    D_803F3960 = (u8 *) (p + 2);
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
    s32 *end = (s32 *) D_803F3960;

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
#ifdef NON_MATCHING
/* For every collision triangle of e (e->unk4 up to e->unk8, read once; walked
 * with `!=`) whose unk51 is set, runs the sphere-vs-triangle test with the
 * sphere (x, y, z, r) >> 2 (arithmetic shifts) and, on a hit, func_802BEBB0
 * (id, e, triangle).
 * Register convention: x, y, z, r in v0, v1, a0, a1, e in t3, id in t8. The
 * asm saves v0-a2, t3 and t4 and leaves t5/t6 = z >> 2 / r >> 2, t9 = e, s0/s1
 * = the triangle range (and s2-s4 and f20-f28 changed by the test helpers);
 * asm caller func_802BE77C keeps a0, a1, t3, t4 and t8 live across the call.
 * The leftover registers the survey lists as read by func_802BE77C's callers
 * (t6, s1, s3, s4, f12-f26) are dead there: overwritten or only saved and
 * restored. */
void func_802BEADC(s32 x, s32 y, s32 z, s32 r, Unk802C1DD0Entry *e, s32 id) {
    Unk803B9890 *t = e->unk4;
    Unk803B9890 *end = e->unk8;

    x >>= 2;
    y >>= 2;
    z >>= 2;
    r >>= 2;
    for (; t != end; t++) {
        if (t->unk51 == 0) {
            continue;
        }
        if (sphere_hits_tri_77E20(x, y, z, r, t)) {
            func_802BEBB0(id, e, t);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BEADC.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
typedef struct {
    /* 0x00 */ s32 pos[3];
    /* 0x0C */ s32 r;
    /* 0x10 */ u16 value;
    /* 0x12 */ u8 id;
    /* 0x13 */ s8 flag; /* -1 ends the table, 0 = unused */
} Unk803A6B30;           /* size 0x14 */

extern u8 D_803F7800;
extern u16 D_803F77FE;

/* Damage from the vehicle's hit spheres to triangle tri of object e. Walks the
 * D_803A6B30 spheres (until flag -1) that are in use and belong to vehicle id,
 * counting them; the first whose sphere ((x, y, z, r) >> 2) hits tri wins
 * (none: returns). Then: stamps D_803F77F8 and records e in D_8036B974 /
 * D_8036B971 (id != 0xFF; e countable, unkEB clear, unk30 != 0x38, new),
 * func_802BEF9C, and unless D_803BE738 is set: adds the count to buffer A
 * (func_802BCCD4); if (e, part = tri->unk52) isn't in the hit list yet (and,
 * for a vehicle with D_803F7800 clear, buffer B has the count or D_803F7812 is
 * clear and func_802BEFF4 didn't set D_803F780C), it is added, the damage
 * (0 for unk30 0x38, which also sets D_803F7805 for vehicle 0; else
 * func_802BFF6C) goes into the screen shake (10, min(damage, 500)), the part's
 * percentage (capped at 100) and D_803F77FE, func_802BF898 updates the part,
 * and a part reaching 100 collapses (func_802BF1F0, func_802C1438,
 * func_802C09B8, func_802C0E8C, func_802BF384, shake 15/400, the part's
 * triangles off and the triangles whose unk57 is the part on unless their own
 * part is done). Finally D_803F7803 = 1, and unless the count is in the
 * D_803A7408 list: D_803A7424 = 1 and func_802BF264(tri) while the part isn't
 * done, else D_803F7802 = 1; then func_802BF668 and func_802BF534.
 * Register convention: id in t8, e in t9, tri in s0. The asm saves t3-t7,
 * s0-s4, gp and fp and leaves a1 = 0 when no sphere hits (dead: its only
 * caller func_802BEADC restores a1); f20-f28 are changed by the test helpers.
 * Asm caller func_802BEADC keeps t3-t6, t8 and t9 live across the call. */
void func_802BEBB0(s32 id, Unk802C1DD0Entry *e, Unk803B9890 *tri) {
    Unk803A6B30 *p;
    u8 *pcts = (u8 *) e + 0xEC;
    s32 count = 0;
    s32 part;
    s32 value;
    s32 level;
    s8 *s;

    for (p = D_803A6B30;; p++) {
        if (p->flag == -1) {
            return;
        }
        if (p->flag == 0 || p->id != id) {
            continue;
        }
        count++;
        if (sphere_hits_tri_77E20(p->pos[0] >> 2, p->pos[1] >> 2, p->pos[2] >> 2, p->r >> 2, tri)) {
            break;
        }
    }

    if (id != 0xFF) {
        D_803F77F8 = D_80358068;
    }
    if (id != 0xFF && e->info->type != 1 && e->unkEB == 0 && e->unk30 != 0x38 && D_8036B974 != e) {
        D_8036B971 = 1;
        D_8036B974 = e;
    }
    func_802BEF9C(e);
    if (D_803BE738 != 0) {
        return;
    }
    part = tri->unk52;
    func_802BCCD4(count);
    if (func_802BEA70((s32) e, part) != 0) {
        goto done;
    }
    if (D_803F7800 == 0 && id != 0xFF) {
        if (D_803F7812 == 0) {
            func_802BEFF4();
            if (D_803F780C != 0) {
                goto add;
            }
        }
        if (func_802BCDE0(count) != 0) {
            goto done;
        }
    }
add:
    func_802BEA30((s32) e, part);
    if (e->unk30 == 0x38) {
        if (id == 0) {
            D_803F7805 = 1;
        }
        value = 0;
    } else {
        value = func_802BFF6C(part, p->value, id, e, e->info->type, count);
    }
    D_802E8BE4 = 10;
    if (value < 0x1F5) {
        D_802E8BE8 = value;
    } else {
        D_802E8BE8 = 500;
    }
    level = pcts[part - 1] + value;
    if (level >= 100) {
        level = 100;
    }
    pcts[part - 1] = level;
    D_803F77FE = value;
    func_802BF898(e, part, level);
    if (level == 100) {
        Unk803B9890 *t;
        Unk803B9890 *end;

        func_802BF1F0(e, part);
        func_802C1438(e, part);
        func_802C09B8(part, e);
        func_802C0E8C(part, e);
        func_802BF384(e);
        D_802E8BE4 = 15;
        D_802E8BE8 = 400;
        end = e->unk8;
        for (t = e->unk4; t != end; t++) {
            if (t->unk52 == part) {
                t->unk51 = 0;
            }
            if (t->unk57 == part && pcts[t->unk52 - 1] != 100) {
                t->unk51 = 1;
            }
        }
    }
done:
    D_803F7803 = 1;
    for (s = D_803A7408;; s++) {
        if (*s == count) {
            goto tail;
        }
        if (*s < 0) {
            break;
        }
    }
    if (pcts[part - 1] != 100) {
        D_803A7424 = 1;
        func_802BF264(tri);
    } else {
        D_803F7802 = 1;
    }
tail:
    func_802BF668(e);
    func_802BF534(e);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BEBB0.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

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
extern s32 D_803F77F4;  /* frame stamp: run counting only if it is D_80358068 - 1 */
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

    if (D_803F77F4 + 1 == D_80358068 && (sign == 0 || ((s8) D_803F780A) != sign)) {
        count = D_803F780B;
        flag = (((u8) D_80370C2C) != 0) ? 1 : 0;
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
#ifdef NON_MATCHING

/* For a collision triangle t with unk58 set: s = t's plane function at the
 * point D_803A73F0/F4/F8 >> 2 (normal . p + d, 64-bit, wrapping where the
 * asm's dadd traps). If s has the same sign as d (s > 0 with d > 0, or s < 0
 * with d <= 0), func_8029B7CC(h - 0x400, h + 0x400) when t->unk55 is 1 or
 * t->unk56 is set; otherwise func_8029B7CC(h + 0x400, h - 0x400) when
 * t->unk55 is 1 or t->unk56 is clear (h = t->unk4C). The asm takes t in s0
 * and saves v0-a3 and t0; asm callers keep a1, t9, f12, f14 (func_802BEBB0)
 * or a1, t3-t6, t9 (func_802CDC7C) live across the call. */
void func_802BF264(Unk803B9890 *t) {
    s64 s;
    s32 h;

    if (t->unk58 == 0) {
        return;
    }
    s = t->normal[0] * (D_803A73F0 >> 2) + t->normal[1] * (D_803A73F4 >> 2) + t->normal[2] * (D_803A73F8 >> 2) +
        t->d;
    h = t->unk4C;
    if ((t->d > 0) ? (s > 0) : (s < 0)) {
        /* same side as d */
        if (t->unk55 == 1 || t->unk56 != 0) {
            func_8029B7CC(h - 0x400, h + 0x400);
        }
    } else {
        if (t->unk55 == 1 || t->unk56 == 0) {
            func_8029B7CC(h + 0x400, h - 0x400);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF264.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* 20460.c defines it with s16 coordinates and a u8; the asm passes the
 * unsliced shifted words, so it is declared with full words here. */

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
#ifdef NON_MATCHING

/* The u16 at byte offset `off` of entry e, for part index i (0-based; a part
 * byte of 0 gives -1, as in the asm). Four tables of 16: 0x48 = "fallen"
 * flag, 0x88/0xA8/0xC8 = x, y, z. */
#define ENTRY_U16(e, off, i) (*(u16 *) ((u8 *) (e) + (off) + (i) * 2))

/* Makes parts of e fall. Each record of info's unk44..unk48 list is
 * {u8 part; u8 threshold; u8 n; n x {u8 support; u8 weight}} (parts 1-based).
 * A part that hasn't fallen (u16 flag at 0x48 clear) and isn't done (unkEC
 * byte != 100) falls when the weights of its supports that are neither done
 * nor fallen sum to less than the threshold: func_802BF1F0(e, part), its flag
 * is set to 1 and D_803649E0/E2/E4 are stored as its x, y, z; then each of
 * e's triangles (unk4..unk8) with unk52 == part has unk51 cleared, and one
 * with unk57 == part gets unk51 = 1 unless the part its unk52 names is done.
 * Repeats the whole pass until nothing falls. All walks stop with `!=`. The
 * asm takes e in t9 and saves t3-t7 and s0-s7; asm callers keep a0, t3, t5,
 * t9, f12, f14 (func_802BCA2C), a1, t9, f12, f14 (func_802BEBB0) or t9
 * (func_802CDD74) live across the call. */
void func_802BF668(Unk802C1DD0Entry *e) {
    u8 *b = (u8 *) e;
    Unk802C1DD0Info *info;
    u8 *rec;
    u8 *end;
    u8 *p;
    Unk803B9890 *r;
    Unk803B9890 *rend;
    s32 changed;
    s32 part;
    s32 n;
    s32 sum;

    do {
        info = e->info;
        changed = 0;
        rec = (u8 *) info + info->unk44;
        end = (u8 *) info + info->unk48;
        while (rec != end) {
            part = rec[0] - 1;
            if (ENTRY_U16(e, 0x48, part) != 0 || b[0xEC + part] == 100) {
                rec += rec[2] * 2 + 3;
                continue;
            }
            sum = 0;
            for (n = rec[2], p = rec + 3; n != 0; n--, p += 2) {
                if (b[0xEB + p[0]] != 100 && ENTRY_U16(e, 0x48, p[0] - 1) == 0) {
                    sum += p[1];
                }
            }
            if (sum < rec[1]) {
                changed = 1;
                func_802BF1F0(e, rec[0]);
                part = rec[0] - 1;
                ENTRY_U16(e, 0x48, part) = 1;
                ENTRY_U16(e, 0x88, part) = ((u16) D_803649E0);
                ENTRY_U16(e, 0xA8, part) = ((u16) D_803649E2);
                ENTRY_U16(e, 0xC8, part) = ((u16) D_803649E4);
                part++;
                for (r = e->unk4, rend = e->unk8; r != rend; r++) {
                    if (r->unk52 == part) {
                        r->unk51 = 0;
                    }
                    if (r->unk57 == part && b[0xEB + r->unk52] != 100) {
                        r->unk51 = 1;
                    }
                }
            }
            rec = p;
        }
    } while (changed);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF668.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u16 D_803F77FE;
void func_802BF978(Unk802C1DD0Entry *e, s32 index, s32 level, Unk802C1DD0Info *info, s32 *obj);

/* Breaks off part `part` (1-based) of e at damage level `level`. If e->info's
 * unk5 (debris scale) is nonzero, func_802BF978 makes generic debris; the asm
 * passes it info and, as its obj (v1), the unk5 byte itself. Otherwise, for
 * each of info's 0x38-byte records unk30..unk34 (walked with `!=`) that isn't
 * done yet (byte 0x34 == 0) and belongs to the part (byte 0x32): with byte
 * 0x33 == 0xFF it is spawned (func_802C04F0) unless D_803F77FE < 20, and
 * stays not done; otherwise, if byte 0x33 (the level needed) <= level
 * (signed), func_802BFBF4 + func_802C04F0 and it is marked done.
 * The asm takes e in t9, part in t3, level in t5 and preserves every
 * register; asm callers keep a1, t3, t5, t9, f12, f14 (func_802BEBB0) and
 * t3, t5, t9 (func_802CDD74) live (a mixed N64 build would need a thunk). */
void func_802BF898(Unk802C1DD0Entry *e, s32 part, s32 level) {
    Unk802C1DD0Info *info = e->info;
    u8 *r;
    u8 *end;

    if (info->unk5 != 0) {
        func_802BF978(e, part, level, info, (s32 *) (u32) info->unk5);
        return;
    }
    r = (u8 *) info + info->unk30;
    end = (u8 *) info + info->unk34;
    for (; r != end; r += 0x38) {
        if (r[0x34] != 0 || r[0x32] != part) {
            continue;
        }
        if (r[0x33] == 0xFF) {
            if (D_803F77FE >= 20) {
                func_802C04F0((u32 *) r);
            }
        } else if (!(level < r[0x33])) {
            func_802BFBF4(e, (s32 *) r);
            func_802C04F0((u32 *) r);
            r[0x34] = 1;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF898.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Debris kinds by a 0..100 roll: the first record whose threshold is above
 * the roll wins (scans aren't bounded; the ROM tables end at 100). */
typedef struct {
    /* 0x00 */ u16 threshold;
    /* 0x02 */ u16 kind;  /* stored as the record's byte 0x31 */
    /* 0x04 */ s32 lo;    /* range of the second roll */
    /* 0x08 */ s32 hi;
} Unk80306344;            /* size 0xC */

extern Unk80306344 D_80306344[];
extern Unk80306344 D_80306350[];
extern Unk80306344 D_803063D4[];
extern u8 D_803063E0[]; /* {u8 threshold; u8 count} pairs */
extern u16 D_803F77FE;

/* Spawns debris for part `index` (1-based) of e: the part's s16 point in
 * e->info's unk2C table (8-byte records) << 16 is the position. A 0..100 roll
 * (func_802BFB50) picks a count from D_803063E0 (first pair whose threshold
 * isn't below the roll); for each of them, with `level` >= 100 the kind table
 * is D_80306350, else D_80306344, but then nothing more happens (not even the
 * final call) if D_803F77FE < 20. Another roll picks the kind record, a third
 * roll in [lo, hi] times info->unk5 (doubled if D_803EF6FF) becomes the
 * record's unkC, and the record (D_803F3FF8; u16 at 0x2C = 10 for all but the
 * first) goes to func_802C04F0. Finally func_802BFBF4(e, obj), where obj is
 * D_803F3FF8 once a record was made, else the caller's obj (the asm's v1).
 * The asm takes e in t9, index in t3, level in t5, info in v0 (the caller
 * passes e->info) and obj in v1; it preserves every register. */
void func_802BF978(Unk802C1DD0Entry *e, s32 index, s32 level, Unk802C1DD0Info *info, s32 *obj) {
    Unk802C1DD0Info *ei = e->info;
    s16 *pt = (s16 *) ((u8 *) ei + ei->unk2C + (index - 1) * 8);
    s32 x = pt[0] << 16;
    s32 y = pt[1] << 16;
    s32 z = pt[2] << 16;
    Unk803F3FF8 *d = &D_803F3FF8;
    Unk80306344 *t;
    u8 *c = D_803063E0;
    s32 *last = obj; /* the asm's v1 (a local: don't write the stack argument) */
    s32 roll;
    s32 count;
    s32 i;
    u32 v;

    roll = func_802BFB50(0, 100);
    while (c[0] < roll) {
        c += 2;
    }
    count = c[1];
    for (i = 0; i != count; i++) {
        if (level >= 100) {
            t = D_80306350;
        } else {
            t = D_80306344;
            if (D_803F77FE < 20) {
                return;
            }
        }
        roll = func_802BFB50(0, 100);
        while (!(roll < t->threshold)) {
            t++;
        }
        d->pos[0] = x;
        d->pos[1] = y;
        d->pos[2] = z;
        roll = func_802BFB50(t->lo, t->hi);
        v = info->unk5 * roll;
        if (D_803EF6FF != 0) {
            v <<= 1;
        }
        d->unkC = v;
        d->unk10[0] = 0;
        d->unk10[1] = 0;
        d->unk10[2] = 0;
        d->unk10[3] = 0;
        d->unk10[4] = 0;
        d->unk10[5] = 0;
        d->unk28 = 0xFC180000;
        d->unk2C = (i != 0) ? 10 : 0;
        d->unk2E = 0;
        d->unk30 = 0;
        d->unk34 = 0;
        d->unk35 = 0;
        d->unk31 = t->kind;
        func_802C04F0((u32 *) d);
        last = (s32 *) d;
    }
    func_802BFBF4(e, last);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF978.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

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
#ifdef NON_MATCHING
void func_802BFDAC(Unk802C1DD0Entry *e, s32 index);

/* Spawns the debris of part index + 1 of e: with e->info's unk5 nonzero, one
 * generic record (func_802BFDAC); else every one of info's 0x38-byte records
 * unk34..unk38 (walked with `!=`) whose byte 0x32 is the part goes to
 * func_802C04F0. The asm takes e in v0 and index in a3 (addi: index + 1 must
 * not overflow) and saves v0, v1, a0, a1, a3; asm caller func_802BD1F8 keeps
 * a0-a3 and t0-t6 live (a mixed N64 build would need a thunk). */
void func_802BFD1C(Unk802C1DD0Entry *e, s32 index) {
    Unk802C1DD0Info *info = e->info;
    s32 part = index + 1;
    u8 *r;
    u8 *end;

    if (info->unk5 != 0) {
        func_802BFDAC(e, part);
        return;
    }
    r = (u8 *) info + info->unk34;
    end = (u8 *) info + info->unk38;
    for (; r != end; r += 0x38) {
        if (r[0x32] == part) {
            func_802C04F0((u32 *) r);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFD1C.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* One debris record for part `index` (1-based) of e (see func_802BF978): x, z
 * from the part's s16 point in e->info's unk2C table (<< 16), y = e->pos[1]
 * << 11; the kind record is picked from D_803063D4 by a 0..100 roll
 * (func_802BFB50), and a roll in its [lo, hi] times info->unk5 is the unkC.
 * The record (D_803F3FF8) goes to func_802C04F0. The asm takes e in v0 and
 * index in a3 (its caller passes the 0-based index + 1) and preserves every
 * register. */
void func_802BFDAC(Unk802C1DD0Entry *e, s32 index) {
    Unk802C1DD0Info *info = e->info;
    s16 *pt = (s16 *) ((u8 *) info + info->unk2C + (index - 1) * 8);
    s32 x = pt[0];
    s32 z = pt[2];
    s32 y = e->pos[1];
    Unk803F3FF8 *d = &D_803F3FF8;
    Unk80306344 *t = D_803063D4;
    s32 roll;

    roll = func_802BFB50(0, 100);
    while (!(roll < t->threshold)) {
        t++;
    }
    d->pos[0] = x << 16;
    d->pos[1] = y << 11;
    d->pos[2] = z << 16;
    roll = func_802BFB50(t->lo, t->hi);
    d->unk10[0] = 0;
    d->unk10[1] = 0;
    d->unk10[2] = 0;
    d->unk10[3] = 0;
    d->unk10[4] = 0;
    d->unk10[5] = 0;
    d->unk2C = 0;
    d->unk2E = 0;
    d->unk30 = 0;
    d->unkC = info->unk5 * roll;
    d->unk28 = 0xFC180000;
    d->unk34 = 0;
    d->unk35 = 0;
    d->unk31 = t->kind;
    func_802C04F0((u32 *) d);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFDAC.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803F7800;

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
#ifdef NON_MATCHING
extern u8 D_803F7807;
extern u8 D_80305E50[];   /* records {u8 bit; u8 key; u8 kind; u8 n; ...}, see below */
s32 func_802C0284(s32 bit, s32 flag, s32 *out);
s32 func_802C038C(s32 id, Unk802C1DD0Entry *e);

/* Damage amount for part `id` of e hit with `value` under rule (bit, key):
 *   bit 0xFF: 100 if flag is 1, else 0 (nothing stored);
 *   otherwise D_8036CB2F = 1, D_8036CB2E = bit, then: the part info->unkD is
 *   immune (D_803F7807 = 1, D_8036CB2F = 0, 0); func_802C0284(bit, flag)
 *   hit: 100; D_803F7800 set: 100 (D_8036CB2A = D_8036CB2C = 100);
 *   else the first D_80305E50 record for (bit, key) that applies gives r:
 *     0xFC (if D_803F7804) / 0xFB (if func_802C038C(id, e)): n * value;
 *     0xFF (if D_803F77FC >= 0) / 0xFE (if <= 0) / 0xFD (always):
 *       |D_803F77FC| * value * n >> 4 (logical);
 *     any other kind k: func_802A04BC(k, D_803F77D0) record; if its first
 *       field is nonzero, with o = its 7th field (s8) and lo, hi the bytes at
 *       rec + o + 4 and + 5: (round(f * (hi - lo)) + lo) * value, f its float;
 *       else the record (n extra bytes) is skipped.
 *   The table walk isn't bounded: (bit, key) must have a record that applies
 *   (the ROM table ends each key's list with an 0xFD record). Then
 *   D_8036CB2A = r, D_8036CB2C = r / flag (unsigned; the asm traps on 0) and
 *   r / flag is returned. Products are 32-bit (multu low word).
 * The asm takes id in t3, value in t7, bit in t8, e in t9, flag in gp and key
 * in fp (all values), returns in t7 and saves v0-t4; asm caller func_802BEBB0
 * keeps a0, a1, t3, t9, f12 and f14 live across the call.
 * Register carry-over kept: a regular-kind record passes its kind to
 * func_802A04BC in v0, which the asm doesn't restore, so a later 0xFB record
 * of the same key gets that kind as its id. Not reproduced: func_802C038C
 * leaves v0 doubled when it reaches its height test, which only a second 0xFB
 * record of the same key would see (the ROM table has none). */
s32 func_802BFF6C(s32 id, s32 value, s32 bit, Unk802C1DD0Entry *e, s32 flag, s32 key) {
    s32 out = value;
    s32 v0 = id;
    u8 *rec;
    s32 kind;
    s32 o[8];
    s32 m;
    u32 r;

    if (bit == 0xFF) {
        return (flag == 1) ? 100 : 0;
    }
    D_8036CB2F = 1;
    D_8036CB2E = bit;
    if (e->info->unkD == id) {
        D_803F7807 = 1;
        D_8036CB2F = 0;
        return 0;
    }
    if (func_802C0284(bit, flag, &out) != 0) {
        return out;
    }
    if (D_803F7800 != 0) {
        D_8036CB2A = 100;
        D_8036CB2C = 100;
        return 100;
    }
    rec = D_80305E50;
    for (;;) {
        kind = rec[2];
        if (rec[0] == bit && rec[1] == key) {
            if (kind == 0xFC) {
                if (D_803F7804 != 0) {
                    r = (u32) rec[3] * out;
                    break;
                }
            } else if (kind == 0xFB) {
                if (func_802C038C(v0, e) != 0) {
                    r = (u32) rec[3] * out;
                    break;
                }
            } else if (kind == 0xFF || kind == 0xFE || kind == 0xFD) {
                if ((kind == 0xFF && D_803F77FC < 0) || (kind == 0xFE && D_803F77FC > 0)) {
                    rec += 4;
                    continue;
                }
                m = D_803F77FC;
                if (m < 0) {
                    m = -m;
                }
                r = ((u32) m * out * rec[3]) >> 4;
                break;
            } else {
                v0 = kind;
                func_802A04BC(kind, (Unk8029DEA0Entry *) D_803F77D0, o);
                if (o[0] != 0) {
                    u8 *q = rec + o[6];
                    s32 lo = q[4];

                    CVT_W_S(m, ((f32 *) o)[7] * (f32) (q[5] - lo));
                    r = (u32) (m + lo) * out;
                    break;
                }
                rec += rec[3] + 4;
                continue;
            }
        }
        if (kind == 0xFC || kind == 0xFB || kind == 0xFD || kind == 0xFF || kind == 0xFE) {
            rec += 4;
        } else {
            rec += rec[3] + 4;
        }
    }
    D_8036CB2A = r;
    r = r / (u32) flag;
    D_8036CB2C = r;
    return r;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFF6C.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
typedef struct {
    /* 0x00 */ s32 level;
    /* 0x04 */ u32 mask; /* 0 ends the table */
} Unk80305E10;

extern Unk80305E10 D_80305E10[];

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
#ifdef NON_MATCHING


extern u8 *D_80306270[];  /* per kind: {u8 n; u8 defIndex[n]} */

/* Runs the 30 pending debris records of D_803F3968 (0x38 bytes each): a
 * record with byte 0x34 set is free; one whose u16 delay at 0x2C is nonzero
 * just counts it down. Otherwise the effect definition is picked from kind
 * r[0x31]'s list in D_80306270 by (low word of D_803649D8 >> 4) % n (n == 0
 * would trap) and spawned with func_802A6274 (type 0, the record's words as
 * position/data/fields, tag 1 for kind 0x15 or when D_803F7810 is set or
 * D_8036DCD4 is set with D_8036DCD7 == 1, else 0). If that succeeded and
 * neither the u64 D_80364A98 nor D_803643D6 is set, func_802619D0(kind)
 * (the asm saves every register around that call). The record is then
 * freed, and a nonzero u16 radius at 0x2E applies func_802C18D4 blast
 * damage at its position with amount r[0x30] << 4. Saves s0-s7. */
void func_802C0574(void) {
    u8 *r = D_803F3968[0];
    s32 n;

    for (n = 30; n != 0; n--, r += 0x38) {
        Io802A6274 io;
        u8 *list;
        u8 *def;
        s32 tag;

        if (r[0x34] != 0) {
            continue;
        }
        if (*(u16 *) (r + 0x2C) != 0) {
            *(u16 *) (r + 0x2C) -= 1;
            continue;
        }
        list = D_80306270[r[0x31]];
        def = D_802C3FFC[list[1 + ((u32) D_803649D8 >> 4) % list[0]]];
        if (r[0x31] == 0x15 || D_803F7810 != 0 || (D_8036DCD4 != 0 && D_8036DCD7 == 1)) {
            tag = 1;
        } else {
            tag = 0;
        }
        io.a3 = tag;
        io.t6 = *(s32 *) (r + 0x10);
        io.s1 = *(s32 *) (r + 0x1C);
        if (func_802A6274(&io, def, *(s32 *) (r + 0x0C), 0, *(s32 *) (r + 0x00), *(s32 *) (r + 0x04),
                          *(s32 *) (r + 0x08), *(s32 *) (r + 0x14), *(s32 *) (r + 0x18), *(s32 *) (r + 0x20),
                          *(s32 *) (r + 0x24), *(s32 *) (r + 0x28), r[0x35]) != 0
            && D_80364A98 == 0 && D_803643D6 == 0) {
            func_802619D0(r[0x31]);
        }
        r[0x34] = 1;
        if (*(u16 *) (r + 0x2E) != 0) {
            func_802C18D4(*(s32 *) (r + 0x00), *(s32 *) (r + 0x04), *(s32 *) (r + 0x08), *(u16 *) (r + 0x2E),
                          r[0x30] << 4);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C0574.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

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
#ifdef NON_MATCHING
u64 func_802C0C64(u64 cmd);

/* Shared by func_802C09B8 and func_802C0E8C: appends to *gfxp the display
 * lists of e->info's conditional records (unk38..unk3C, walked with `!=`):
 * {s32 n; n x {u16 part; u8 atLeast; u8 limit}; s32 a0, a1, b0, b1}. A record
 * counts only if its first condition's part (the u16 at +4, read even when
 * n is 0) is `id`; then every condition must hold, with v = 90 for part id
 * itself, else e's unkEC byte for that (1-based) part: v >= limit when
 * atLeast is set, else v <= limit. A record that holds has its two ranges of
 * 8-byte commands [info + a0, info + a1) and [info + b0, info + b1) copied,
 * through func_802C0C64 when `substitute` is set. Each command uses up one of
 * *budget; when it reaches 0 the copy stops and 0 is returned (the callers
 * then return at once). Returns 1 otherwise. */
static s32 dl_cond_copy_802C09B8(u32 **gfxp, Unk802C1DD0Entry *e, s32 id, s32 *budget, s32 substitute) {
    Unk802C1DD0Info *info = e->info;
    u8 *rec = (u8 *) info + info->unk38;
    u8 *end = (u8 *) info + info->unk3C;
    u64 *gfx = (u64 *) *gfxp;
    u64 *src;
    u64 *srcEnd;
    s32 n;
    s32 part;
    s32 atLeast;
    s32 limit;
    s32 v;
    s32 ok;
    s32 k;

    while (rec != end) {
        n = *(s32 *) rec;
        part = *(u16 *) (rec + 4);
        rec += 4;
        if (part == id) {
            ok = 1;
            while (n != 0) {
                part = *(u16 *) rec;
                atLeast = rec[2];
                limit = rec[3];
                n--;
                rec += 4;
                v = (part == id) ? 90 : ((u8 *) e)[0xEC + part - 1];
                if ((atLeast != 0) ? (v < limit) : (limit < v)) {
                    ok = 0;
                    break;
                }
            }
            if (ok) {
                for (k = 0; k < 2; k++) {
                    src = (u64 *) ((u8 *) info + ((s32 *) rec)[k * 2]);
                    srcEnd = (u64 *) ((u8 *) info + ((s32 *) rec)[k * 2 + 1]);
                    for (; src != srcEnd; src++) {
                        if (--*budget == 0) {
                            return 0;
                        }
                        *gfx++ = substitute ? func_802C0C64(*src) : *src;
                    }
                }
            }
        }
        rec += n * 4 + 0x10; /* remaining conditions (0 once all held) and the 4 offsets */
    }
    *gfxp = (u32 *) gfx;
    return 1;
}

/* Builds entry `id`'s debris display list in the first free D_803F0900 slot
 * (u16 at 0x4B0 and byte at 0x4B2 both zero; none free: nothing happens),
 * unless e->info->unk7 is set: a segment-9 command (0xBC002406) pointing at
 * info + 0x50 (physical), the conditional records' commands (see
 * dl_cond_copy_802C09B8, through func_802C0C64, at most 0x93), then
 * 0xB8000000 0 (end), and marks the slot used (u16 at 0x4B0 = 0xFF). If the
 * command budget runs out it returns right there, without the end command.
 * The asm takes id in t3 and e in t9 and preserves every register (all of
 * them saved); asm callers keep a0, t3, t5, t9, f12, f14 (func_802BCA2C) or
 * t3, t9, f12, f14 (func_802BEBB0, func_802CDD74) live across the call. */
void func_802C09B8(s32 id, Unk802C1DD0Entry *e) {
    Unk802C1DD0Info *info = e->info;
    u8 *slot = D_803F0900;
    u32 *gfx;
    s32 budget = 0x94;
    s32 i;

    if (info->unk7 != 0) {
        return;
    }
    for (i = 4;; i--, slot += 0x4B8) {
        if (i == 0) {
            return;
        }
        if (*(u16 *) (slot + 0x4B0) == 0 && slot[0x4B2] == 0) {
            break;
        }
    }
    gfx = (u32 *) slot;
    gfx[0] = 0xBC002406;
    gfx[1] = (u32) info + 0x50 - 0x80000000;
    gfx += 2;
    if (dl_cond_copy_802C09B8(&gfx, e, id, &budget, 1) == 0) {
        return;
    }
    *(u16 *) (slot + 0x4B0) = 0xFF;
    gfx[0] = 0xB8000000;
    gfx[1] = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C09B8.s")
#endif

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
#ifdef NON_MATCHING
/* One rising piece of debris (func_802C0E8C starts it, func_802C0CBC moves
 * and draws it). The display list func_802C0E8C builds sits at the start. */
typedef struct {
    /* 0x000 */ u8 dl[0x960];
    /* 0x960 */ s32 mtx[2][16]; /* 16.16 translation, then Mtx layout; one per D_8035805C buffer */
    /* 0x9E0 */ s16 vtx[4][8];  /* 4 Vtx (func_802C1214) */
    /* 0xA20 */ s32 unkA20;     /* height limit */
    /* 0xA24 */ s32 unkA24;     /* height */
    /* 0xA28 */ u16 unkA28;     /* speed */
    /* 0xA2A */ u8 unkA2A;      /* active */
    /* 0xA2B */ u8 unkA2B;      /* cool-down */
    /* 0xA2C */ u8 padA2C[4];
} Unk803EFED0;                  /* size 0xA30 */


/* Moves the D_803EFED0 entry (one) and draws it into gfx: an inactive entry
 * counts unkA2B down to 0. An active one speeds up by 16 and rises by its
 * speed; once that reaches unkA20 it deactivates (unkA2A = 0, unkA2B = 3).
 * Otherwise D_803F7810 = 1, its matrix (mtx[D_8035805C != 0]) becomes a
 * translation by (r1 << 12, -(height << 11), r2 << 12) with r1, r2 rolls in
 * -32..32 (func_802BFB50), converted by func_802AC8CC, and gfx gets
 * G_RDPPIPESYNC, a segment-0xC command (0xBC003006) to that matrix and a
 * G_DL call to the entry (physical addresses). D_803F7810 is cleared first.
 * Ends the list with 0xB8000000 0 and returns the new end. The asm takes and
 * returns gfx in v0 (its caller ignores it) and saves v1, a0-t4, s2 and s4;
 * asm caller func_802BD1F8 keeps a0-a3, f12 and f14 live across the call. */
u32 *func_802C0CBC(u32 *gfx) {
    Unk803EFED0 *p = D_803EFED0;
    s32 i;
    s32 speed;
    s32 h;
    s32 x;
    s32 z;

    D_803F7810 = 0;
    for (i = 1; i != 0; i--, p++) {
        if (p->unkA2A == 0) {
            if (p->unkA2B != 0) {
                p->unkA2B--;
            }
            continue;
        }
        speed = p->unkA28 + 16;
        h = p->unkA24 + speed;
        p->unkA28 = speed;
        if (!(h < p->unkA20)) {
            p->unkA2A = 0;
            p->unkA2B = 3;
            continue;
        }
        p->unkA24 = h;
        D_803F7810 = 1;
        x = func_802BFB50(-32, 32) << 12;
        z = func_802BFB50(-32, 32) << 12;
        func_802ACA60(x, -(h << 11), z, p->mtx[(D_8035805C != 0) ? 1 : 0]);
        func_802AC8CC((u16 *) p->mtx[(D_8035805C != 0) ? 1 : 0]);
        gfx[0] = 0xE7000000;
        gfx[1] = 0;
        gfx[2] = 0xBC003006;
        gfx[3] = (u32) p->mtx[(D_8035805C != 0) ? 1 : 0] - 0x80000000;
        gfx[4] = 0x06000000;
        gfx[5] = (u32) p - 0x80000000;
        gfx += 6;
    }
    gfx[0] = 0xB8000000;
    gfx[1] = 0;
    return gfx + 2;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C0CBC.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s16 D_80305E38[]; /* unk30 values that get a rising piece; ends at the first negative one */
extern u64 D_802F4780[6]; /* fixed render-state commands */

/* Starts the rising piece of debris for part `id` of e: unless the game mode
 * D_80364A90 has 0x440 set, e->unk30 is in the list D_80305E38 (an equal
 * value is found before the negative terminator) and the D_803EFED0 entry is
 * idle (unkA2A and unkA2B zero). Then func_802C1214 sets its 4 vertices, and
 * its display list is built in place: segment 9 (0xBC002406) = info + 0x50,
 * segment 0xB (0xBC002C06) = its vertices, the 6 commands of D_802F4780,
 * 0x01040040 0x0C000000, the conditional records' commands (see
 * dl_cond_copy_802C09B8, copied as they are, at most 0x120; if that runs out
 * it returns right there), 0xBD000000 0 and 0xB8000000 0. Finally
 * unkA24 = 0, unkA28 = 0, unkA2A = 1 and unkA20 = (info->unk40[id - 1] << 5)
 * - e->pos[1]. The asm takes id in t3 and e in t9 and preserves every
 * register (all of them saved); asm callers keep a0, t3, t5, t9, f12, f14
 * (func_802BCA2C) or t3, t9, f12, f14 (func_802BEBB0, func_802CDD74) live
 * across the call. */
void func_802C0E8C(s32 id, Unk802C1DD0Entry *e) {
    s16 *k;
    Unk803EFED0 *p;
    Unk802C1DD0Info *info;
    u32 *gfx;
    s32 budget = 0x121;
    s32 i;

    if ((u32) D_80364A90 & 0x440) {
        return;
    }
    for (k = D_80305E38; *k != e->unk30; k++) {
        if (*k < 0) {
            return;
        }
    }
    for (i = 1, p = D_803EFED0;; i--, p++) {
        if (i == 0) {
            return;
        }
        if (p->unkA2A == 0 && p->unkA2B == 0) {
            break;
        }
    }
    info = e->info;
    func_802C1214((u8 *) p, e);
    gfx = (u32 *) p->dl;
    gfx[0] = 0xBC002406;
    gfx[1] = (u32) info + 0x50 - 0x80000000;
    gfx[2] = 0xBC002C06;
    gfx[3] = (u32) p->vtx - 0x80000000;
    gfx += 4;
    for (i = 0; i < 6; i++) {
        ((u64 *) gfx)[i] = D_802F4780[i];
    }
    gfx += 12;
    gfx[0] = 0x01040040;
    gfx[1] = 0x0C000000;
    gfx += 2;
    if (dl_cond_copy_802C09B8(&gfx, e, id, &budget, 0) == 0) {
        return;
    }
    info = e->info;
    p->unkA24 = 0;
    p->unkA20 = (*(s16 *) ((u8 *) info + info->unk40 + (id - 1) * 2) << 5) - e->pos[1];
    p->unkA28 = 0;
    p->unkA2A = 1;
    gfx[0] = 0xBD000000;
    gfx[1] = 0;
    gfx[2] = 0xB8000000;
    gfx[3] = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C0E8C.s")
#endif

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
extern Mtx *D_803F7658; /* 2 matrices for them */
/* 23C20.c defines it with s16 parameters; the asm passes rx as a full
 * u16 + u16 sum and ry zero-extended, so it is declared with words here. */

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
#ifdef NON_MATCHING
s32 func_802C1A28(Unk802C1DD0Entry *e, s32 index);

/* Quarter-turn heading (0..0x3FF) from the sine num / d: arcsine >> 4. */
static s32 heading_802C1438(s32 num, f32 d) {
    s32 sine;

    CVT_W_S(sine, 65536.0f * ((f32) num / d));
    return (u32) func_802AD7FC(sine) >> 4;
}

/* Spawns a falling-part effect for part `index` (1-based) of e: only when
 * e's info byte 7 is set and func_802C1A28(e, index) is 0. The first
 * D_803F1BE0 slot (2 of 0x478 bytes) whose bytes 0x470 and 0x472 are both
 * clear gets: word 0x460 = info + 0x50; two display lists, A at the slot and
 * B at slot + 0x230, each starting with {0x01040040, 0x02000000 + slot *
 * 0x40}. Then for every conditional record of info (unk38 .. unk3C: u32
 * condition count, then count x {u16 part, u8 mode, u8 limit}, then 4
 * offsets; the record belongs to the part named by its first condition) of
 * this part whose conditions all hold (the state of the
 * named part, e->unkEC[part - 1], or 0x5A for this part itself, >= limit
 * when mode != 0, <= limit when mode == 0), the 8-byte commands
 * info + off0 .. off1 go to list A and off2 .. off3 to list B, at most 0x42
 * per list (running out returns at once, the slot left inactive). Both
 * lists end with {0xBD000000, 0}, {0xB8000000, 0}; the slot becomes active
 * (0x470 = 1, u16 0x46C = 0, u16 0x46E = 1, 0x471 = 0) at x, y, z
 * (s16 0x464..0x468: the part's point from info->unk2C, and e->pos[1] >> 5)
 * with s16 0x46A = (heading from the part (<< 5) to D_803A73F0/F8) - 0x400,
 * plus 0xFFF when negative (heading as func_802BDDB4's).
 * Register convention: e in t9, index in t3 (conventions.txt); the asm
 * saves and restores every register except f0/f2. Asm callers keep a0, t3,
 * t5, t9, f12 and f14 live (a mixed N64 build would need a thunk; the native
 * port won't). The part can't sit exactly at the point (0 / 0). */
void func_802C1438(Unk802C1DD0Entry *e, s32 index) {
    Unk802C1DD0Info *info = e->info;
    u8 *base = (u8 *) info;
    u8 *slot = D_803F1BE0;
    u32 seg = 0x02000000;
    s32 i;
    u32 *a;
    u32 *b;
    u8 *rec;
    u8 *recEnd;
    s32 budgetA;
    s32 budgetB;
    s16 *pt;
    s32 x;
    s32 z;
    s32 px;
    s32 pz;
    f32 d;
    s32 ang;

    if (((u8 *) info)[7] == 0) {
        return;
    }
    if (func_802C1A28(e, index) != 0) {
        return;
    }
    for (i = 2;; i--, seg += 0x40, slot += 0x478) {
        if (i == 0) {
            return;
        }
        if (slot[0x470] == 0 && slot[0x472] == 0) {
            break;
        }
    }
    *(u8 **) (slot + 0x460) = base + 0x50;
    a = (u32 *) slot;
    b = (u32 *) (slot + 0x230);
    a[0] = 0x01040040;
    a[1] = seg;
    b[1] = seg;
    b[0] = 0x01040040;
    a += 2;
    b += 2;
    budgetA = 0x43;
    budgetB = 0x43;
    rec = base + info->unk38;
    recEnd = base + info->unk3C;
    while (rec != recEnd) {
        u32 n = *(u32 *) rec;
        s32 ok = *(u16 *) (rec + 4) == index;

        rec += 4;
        while (ok && n != 0) {
            s32 part = *(u16 *) rec;
            s32 mode = rec[2];
            s32 limit = rec[3];
            s32 state;

            n--;
            rec += 4;
            state = (part == index) ? 0x5A : e->unkEC[part - 1];
            ok = (mode != 0) ? (state >= limit) : (state <= limit);
        }
        if (!ok) {
            rec += n * 4 + 0x10;
            continue;
        }
        {
            u32 *src = (u32 *) (base + ((s32 *) rec)[0]);
            u32 *srcEnd = (u32 *) (base + ((s32 *) rec)[1]);

            for (; src != srcEnd; src += 2, a += 2) {
                if (--budgetA == 0) {
                    return;
                }
                a[0] = src[0];
                a[1] = src[1];
            }
            src = (u32 *) (base + ((s32 *) rec)[2]);
            srcEnd = (u32 *) (base + ((s32 *) rec)[3]);
            for (; src != srcEnd; src += 2, b += 2) {
                if (--budgetB == 0) {
                    return;
                }
                b[0] = src[0];
                b[1] = src[1];
            }
        }
        rec += 0x10;
    }
    a[0] = 0xBD000000;
    a[1] = 0;
    a[2] = 0xB8000000;
    a[3] = 0;
    b[0] = 0xBD000000;
    b[1] = 0;
    b[2] = 0xB8000000;
    b[3] = 0;
    slot[0x470] = 1;
    *(u16 *) (slot + 0x46C) = 0;
    *(u16 *) (slot + 0x46E) = 1;
    slot[0x471] = 0;
    *(s16 *) (slot + 0x466) = e->pos[1] >> 5;
    pt = (s16 *) (base + info->unk2C + (index - 1) * 8);
    *(s16 *) (slot + 0x464) = pt[0];
    *(s16 *) (slot + 0x468) = pt[2];
    x = pt[0] << 5;
    z = pt[2] << 5;
    px = D_803A73F0;
    pz = D_803A73F8;
    {
        f32 dx = (f32) (s32) (px - x);
        f32 dz = (f32) (s32) (pz - z);

        d = sqrtf(dx * dx + dz * dz);
    }
    if (px < x) {
        if (pz < z) {
            ang = heading_802C1438(x - px, d) + 0x800;
        } else {
            ang = heading_802C1438(pz - z, d) + 0xC00;
        }
    } else if (pz < z) {
        ang = heading_802C1438(z - pz, d) + 0x400;
    } else {
        ang = heading_802C1438(px - x, d);
    }
    ang -= 0x400;
    if (ang < 0) {
        ang += 0xFFF;
    }
    *(s16 *) (slot + 0x46A) = ang;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1438.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
void func_802BC888(s32 kind, s32 id, s32 amount);

/* Blast damage: for every D_803F4030 entry (up to D_803F7654, walked with
 * `!=`) whose sphere (pos, unkC) overlaps the sphere at (x, y, z) >> 11
 * (logical shifts) with radius radius << 5 (func_8029CFA4), unless its unk30
 * is 0x38, every part that isn't done (unkEC byte != 100) gets
 * func_802BC888(part (1-based), e, amount / info->type (unsigned; the asm
 * traps on a zero type)). The asm takes x, y, z in t3, t4, t5, radius in a1
 * and amount in t6, and preserves every register it touches; asm callers
 * keep a0 (func_802AC1A0) or f12, f14 (func_802C0574) live across the call. */
void func_802C18D4(s32 x, s32 y, s32 z, s32 radius, s32 amount) {
    Unk802C1DD0Entry *end = D_803F7654;
    Unk802C1DD0Entry *e;
    s32 px = (u32) x >> 11;
    s32 py = (u32) y >> 11;
    s32 pz = (u32) z >> 11;
    s32 r = radius << 5;
    s32 n;
    s32 part;
    u8 *p;

    for (e = D_803F4030; e != end; e++) {
        if (func_8029CFA4(pz, r, e->pos[0], e->pos[1], px, py, e->pos[2], e->unkC) == 0) {
            continue;
        }
        if (e->unk30 == 0x38) {
            continue;
        }
        for (n = e->unkE9, p = e->unkEC, part = 1; n != 0; n--, p++, part++) {
            if (*p != 100) {
                func_802BC888(part, (s32) e, (u32) amount / e->info->type);
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C18D4.s")
#endif

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
    s32 i;

    for (e = D_803F4030; e != end; e++) {
        if (e->unkEB == 0 || e->unkEA != 0) {
            continue;
        }
        dist = func_802ABCDC(e->pos[0], e->pos[1], e->pos[2], D_803EF6DC, D_803EF6E0, D_803EF6E4);
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
        dist = func_802ABCDC(t->pos[0], t->pos[1], t->pos[2], D_803EF6DC, D_803EF6E0, D_803EF6E4);
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
        dist = func_802ABCDC(p->pos[0], p->pos[1], p->pos[2], D_803EF6DC, D_803EF6E0, D_803EF6E4);
        if (dist >= bestDist || p->pos[2] < D_803EF6E4) {
            continue;
        }
        for (id = p->ids; *id != -1; id++) {
            if (func_8029D210(*id) != 0) {
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

/* Tallies the entries D_803F4030..D_803F7654 (0xFC bytes each). With
 * onlyFlagged set, only entries whose unkEB is nonzero count, and each of
 * those stores its unk30 into D_803F7684 (so the last one wins). Of the
 * counted entries whose info->type isn't 0xFF: those with type != 1 are
 * counted into D_8036EB92; if the entry's unkEA or D_803F7688 is nonzero,
 * type != 1 entries are also counted into D_8036EA70.bd (Score) and every such
 * entry's info->value is summed into D_8036EA70.ip. Clears D_803F7688.
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
    D_8036EA70.bd = active; /* the active count */
    D_8036EA70.ip = total; /* the total */
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
    s32 n;
    s32 k;

    for (group--; group != 0; group--) {
        g = (Unk802C2054Group *) (chain + (u16) g->next);
    }
    off = *(u16 *) ((u8 *) g + 0x14 + (part - 1) * 2); /* g->mtxOffset[part - 1] */
    mtx = (u16 *) (((D_8035805C != 0) ? D_803F7824 : D_803F7820) + off);
    func_802ACA60(x, y, z, (s32 *) mtx);
    func_802AC8CC(mtx);

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
