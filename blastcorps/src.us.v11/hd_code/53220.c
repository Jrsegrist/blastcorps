#include "common.h"
#include <ultra64.h>
#ifndef NON_MATCHING /* legacy declarations */
/* Matching build: IDO compiled the matched code here against older
 * declarations of these, which the file keeps; the NON_MATCHING build
 * uses game/game.h's. */
#define LEGACY_D_8036BB24
#endif /* legacy declarations */
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_802F49F4 ((Font *) D_802F49F4)
#define D_80364AF0 ((Player *) D_80364AF0)
#ifdef NON_MATCHING
#define D_8036BB24 (*(MenuItem * *) &D_8036BB24)
#endif
/* end of views */

/*
 * Two parts: the academy/level-select yoshi window builder (func_802979E0
 * and helpers), then a second copy of the INFLATE decompressor that init/
 * carries (init/0050.c, init/0E30.c), compiled from the same source:
 *   func_80297FE0 huft_build, func_8029867C inflate_codes,
 *   func_80298A84 inflate_stored, func_80298C18 inflate_fixed,
 *   func_80298DC0 inflate_dynamic, func_802993AC inflate_block,
 *   func_802994F8 inflate.
 */

typedef struct {
    u8 pad0[0x90];
    u8 academy; /* 0x90 */
    u8 pad91[0x100 - 0x91];
} Player;

/* Yoshi window item, 0x1C bytes */
typedef struct {
    /* 0x00 */ u16 flags;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 w;
    /* 0x08 */ s16 h;
    /* 0x0A */ u8 padA[2];
    /* 0x0C */ u8 *text;
    /* 0x10 */ s32 jtext;
    /* 0x14 */ u8 font;
    /* 0x15 */ u8 pad15;
    /* 0x16 */ s16 unk16;
    /* 0x18 */ u8 unk18;
    /* 0x19 */ u8 unk19;
    /* 0x1A */ u8 unk1A;
    /* 0x1B */ u8 pad1B;
} MenuItem;

/* Yoshi window, 0x1C bytes */
typedef struct {
    /* 0x00 */ u8 unk0[0x10];
    /* 0x10 */ u16 count;
    /* 0x12 */ u8 unk12[6];
    /* 0x18 */ s16 cursor;
    /* 0x1A */ u8 unk1A[2];
} Window;

typedef struct {
    u8 *text;
    s32 jtext;
} MenuText;

/* 0x30 bytes */
typedef struct {
    /* 0x00 */ u8 unk0[4];
    /* 0x04 */ u8 unk4;
    /* 0x05 */ u8 unk5;
    /* 0x06 */ u8 ids[0x26];
    /* 0x2C */ u8 unk2C;
    /* 0x2D */ u8 unk2D;
    /* 0x2E */ u8 unk2E[2];
} Font;

extern u8 D_8039CAD0;
extern u8 D_802FF180[];
extern MenuText D_802FF188[][20];
extern u8 D_802FF5E8[];
#ifndef NON_MATCHING
extern MenuItem *D_8036BB24;
#endif


#define players D_80364AF0
#define playerNumber D_80364AE8

u8 func_80297F74(void);

/* Build the yoshi window that lists a group's levels, numbering the
 * "N left" text with how many levels remain */
void func_802979E0(u8 level) {
    Window *win = (Window *) (D_802F8BDC + 0x24C);
    MenuItem *item;
    Font *font;
    u8 left;
    u8 done;
    s32 i;
    s32 j;
    s32 nsel;
    s32 n;
    u8 stop;
    u8 *text;
    s32 jtext;

    left = func_80297F74();
    done = 0;
    if (left == 1 && !func_80297EF8(level)) {
        left--;
    }
    if (left == 0) {
        D_8039CAD0 = 6;
    } else {
        stop = 0;
        for (i = D_8039CAB6; i < D_8039CAB6 + 6 && !stop;) {
            if (!func_80297EF8(D_802FF180[i % 6]) && D_802FF180[i % 6] != level) {
                stop = 1;
            } else {
                i++;
            }
        }
        D_8039CAD0 = i % 6;
    }
    i = 0;
    D_8036BB24 = D_80358070;
    D_80358070 = (u8 *) D_80358070 + 0x24C;
    nsel = 0;
    n = 0;
    stop = 0;
    for (; i < 20 && !stop; i++) {
        text = D_802FF188[D_8039CAD0][i].text;
        jtext = D_802FF188[D_8039CAD0][i].jtext;
        item = &D_8036BB24[i];
        if (text != NULL) {
            n++;
            item->flags = 0x1020;
            if (D_802FF5E8[D_8039CAD0 * 5 + nsel] == i) {
                item->flags |= 1;
                nsel++;
            }
            item->x = i << 4;
            item->w = 0x10;
            item->h = 0x10;
            item->text = text;
            item->jtext = jtext;
            item->font = 0;
            item->unk16 = 0;
            item->unk18 = 7;
            item->unk19 = 7;
            item->unk1A = 0;
        } else {
            stop = 1;
        }
    }

    item = &D_8036BB24[n];
    item->text = NULL;
    item->jtext = 0;
    item->flags = 0x400;
    item->y = -0x20;
    item->x = 0x26;
    item->font = 0x18;
    item->unk16 = item->unk1A = 0;
    font = &D_802F49F4[item->font];
    item->unk1A = func_80272C5C(font->ids, 0, font->unk4, font->unk2C, font->unk2D | 4, 1.0f);
    win->count = n + 1;
    win->cursor = D_802FF5E8[D_8039CAD0 * 5];
    if (!func_80297EF8(level)) {
        left--;
    }
    for (i = 0; i < win->count && !done; i++) {
        item = &D_8036BB24[i];
        for (j = 0; j < func_8025B300(item->text) && !done; j++) {
            if (item->text[j] >= '0' && item->text[j] < '6') {
                item->text[j] = left + '0';
                done = 1;
            }
        }
    }
}

void func_80297ECC(void) {
    *(s16 *) (D_802F8BDC + 0x264) = D_802FF5E8[D_8039CAD0 * 5];
}

/* Is level completed in its academy group? */
u8 func_80297EF8(u8 level) {
    u8 idx;
    u8 found;

    found = func_8029766C(level, &idx);
    return (found && (players[playerNumber].academy & (1 << idx))) ? 1 : 0;
}

/* Number of academy groups not yet completed */
u8 func_80297F74(void) {
    s32 i;
    s32 n = 6;

    for (i = 0; i < 6; i++) {
        if (players[playerNumber].academy & (1 << i)) {
            n--;
        }
    }
    return n;
}

typedef struct Huft {
    u8 e;
    u8 b;
    u8 pad[2];
    union {
        u16 n;
        struct Huft *t;
    } v;
} Huft;

extern u32 D_8039CAE4;  /* bb */
extern u32 D_8039CAE8;  /* bk */
extern u32 D_8039CAEC;  /* hufts */

extern u8 D_802FF630[];  /* border */
extern u16 D_802FF644[]; /* cplens */
extern u8 D_802FF684[];  /* cplext */
extern u16 D_802FF6A4[]; /* cpdist */
extern u8 D_802FF6E0[];  /* cpdext */
extern u16 D_802FF700[]; /* mask_bits */
extern s32 D_802FF724;   /* lbits */
extern s32 D_802FF728;   /* dbits */

int func_80297FE0(s32 *b, u32 n, u32 s, u16 *d, u8 *e, Huft **t, s32 *m);
s32 func_8029867C(Huft *tl, Huft *td, u32 bl, u32 bd);
s32 func_80298DC0(void);
s32 func_80298A84(void);
s32 func_80298C18(void);

/* huft_build: build a Huffman decode table from code lengths b[0..n-1]
 * (classic inflate.c, bump-allocated from the D_8039CAE0 arena). It returns
 * `int`, not s32 (long): with s32, IDO puts the `&&` result in $s6 and
 * adds a move to $v0. */
#define BMAX 16
#define N_MAX 288

int func_80297FE0(s32 *b, u32 n, u32 s, u16 *d, u8 *e, Huft **t, s32 *m) {
    u32 a;
    u32 c[BMAX + 1];
    u32 f;
    s32 g;
    s32 h;
    register u32 i;
    register u32 j;
    register s32 k;
    s32 l;
    register u32 *p;
    register Huft *q;
    Huft r;
    Huft *u[BMAX];
    u32 v[N_MAX];
    register s32 w;
    u32 x[BMAX + 1];
    u32 *xp;
    s32 y;
    u32 z;

    func_802DB7B0(c, sizeof(c));
    p = (u32 *) b;
    i = n;
    do {
        c[*p]++;
        p++;
    } while (--i);
    if (c[0] == n) {
        *t = NULL;
        *m = 0;
        return 0;
    }

    l = *m;
    for (j = 1; j <= BMAX; j++) {
        if (c[j]) {
            break;
        }
    }
    k = j;
    if ((u32) l < j) {
        l = j;
    }
    for (i = BMAX; i; i--) {
        if (c[i]) {
            break;
        }
    }
    g = i;
    if ((u32) l > i) {
        l = i;
    }
    *m = l;

    for (y = 1 << j; j < (u32) i; j++, y <<= 1) {
        y -= c[j];
    }
    y -= c[i];
    c[i] += y;

    x[1] = j = 0;
    p = c + 1, xp = x + 2;
    while (--i) {
        *xp++ = (j += *p++);
    }

    p = (u32 *) b;
    i = 0;
    do {
        if ((j = *p++) != 0) {
            v[x[j]++] = i;
        }
    } while (++i < n);

    x[0] = i = 0;
    p = v;
    h = -1;
    w = -l;
    u[0] = NULL;
    q = NULL;
    z = 0;

    for (; k <= g; k++) {
        a = c[k];
        while (a--) {
            while (k > w + l) {
                h++;
                w += l;

                z = (z = g - w) > (u32) l ? l : z;
                if ((f = 1 << (j = k - w)) > a + 1) {
                    f -= a + 1;
                    xp = c + k;
                    while (++j < z) {
                        if ((f <<= 1) <= *++xp) {
                            break;
                        }
                        f -= *xp;
                    }
                }
                z = 1 << j;

                q = (Huft *) (D_8039CAEC * 8 + D_8039CAE0);
                D_8039CAEC += z + 1;
                *t = q + 1;
                *(t = (Huft **) &(q->v.t)) = NULL;
                u[h] = ++q;

                if (h) {
                    x[h] = i;
                    r.b = (u8) l;
                    r.e = (u8) (16 + j);
                    r.v.t = q;
                    j = i >> (w - l);
                    u[h - 1][j] = r;
                }
            }

            r.b = (u8) (k - w);
            if (p >= v + n) {
                r.e = 99;
            } else if (*p < s) {
                r.e = (u8) (*p < 256 ? 16 : 15);
                r.v.n = (u16) (*p);
                p++;
            } else {
                r.e = e[*p - s];
                r.v.n = d[*p++ - s];
            }

            f = 1 << (k - w);
            for (j = i >> w; j < z; j += f) {
                q[j] = r;
            }

            for (j = 1 << (k - 1); i & j; j >>= 1) {
                i ^= j;
            }
            i ^= j;

            while ((i & ((1 << w) - 1)) != x[h]) {
                h--;
                w -= l;
            }
        }
    }

    return y != 0 && g != 1;
}

s32 func_8029867C(Huft *tl, Huft *td, u32 bl, u32 bd) {
    register u32 e;
    u32 n;
    u32 d;
    u32 w;
    Huft *t;
    u32 ml;
    u32 md;
    register u32 k;
    register u32 b;

    b = D_8039CAE4;
    k = D_8039CAE8;
    w = D_803669F0;

    ml = D_802FF700[bl];
    md = D_802FF700[bd];

    for (;;) {
        while (k < bl) {
            b |= (u32) D_803669C0[D_803669EC++] << k;
            k += 8;
        }
        t = tl + (b & ml);
        e = t->e;
        if (e > 16) {
            do {
                b >>= t->b;
                k -= t->b;
                e -= 16;
                while (k < e) {
                    b |= (u32) D_803669C0[D_803669EC++] << k;
                    k += 8;
                }
                t = t->v.t + (b & D_802FF700[e]);
                e = t->e;
            } while (e > 16);
        }
        b >>= t->b;
        k -= t->b;
        if (e == 16) {
            D_803669C4[w++] = (u8) t->v.n;
            continue;
        }
        if (e == 15) {
            break;
        }

        while (k < e) {
            b |= (u32) D_803669C0[D_803669EC++] << k;
            k += 8;
        }
        n = t->v.n + (b & D_802FF700[e]);
        k -= e;
        b >>= e;

        while (k < bd) {
            b |= (u32) D_803669C0[D_803669EC++] << k;
            k += 8;
        }
        t = td + (b & md);
        e = t->e;
        if (e > 16) {
            do {
                b >>= t->b;
                k -= t->b;
                e -= 16;
                while (k < e) {
                    b |= (u32) D_803669C0[D_803669EC++] << k;
                    k += 8;
                }
                t = t->v.t + (b & D_802FF700[e]);
                e = t->e;
            } while (e > 16);
        }
        b >>= t->b;
        k -= t->b;

        while (k < e) {
            b |= (u32) D_803669C0[D_803669EC++] << k;
            k += 8;
        }
        d = (w - t->v.n) - (b & D_802FF700[e]);
        b >>= e;
        k -= e;

        do {
            n -= (e = n);
            do {
                D_803669C4[w++] = D_803669C4[d++];
            } while (--e);
        } while (n);
    }

    D_803669F0 = w;
    D_8039CAE4 = b;
    D_8039CAE8 = k;
    return 0;
}

s32 func_80298A84(void) {
    u32 n;
    u32 w;
    register u32 k;
    register u32 b;

    k = D_8039CAE8;
    b = D_8039CAE4;
    w = D_803669F0;

    n = k & 7;
    b >>= n;
    k -= n;

    while (k < 16) {
        b |= (u32) D_803669C0[D_803669EC++] << k;
        k += 8;
    }
    n = b & 0xffff;
    b >>= 16;
    k -= 16;

    while (k < 16) {
        b |= (u32) D_803669C0[D_803669EC++] << k;
        k += 8;
    }
    b >>= 16;
    k -= 16;

    while (n--) {
        while (k < 8) {
            b |= (u32) D_803669C0[D_803669EC++] << k;
            k += 8;
        }
        D_803669C4[w++] = (u8) b;
        b >>= 8;
        k -= 8;
    }

    D_803669F0 = w;
    D_8039CAE4 = b;
    D_8039CAE8 = k;
    return 0;
}

s32 func_80298C18(void) {
    s32 i;
    Huft *tl;
    Huft *td;
    s32 bl;
    s32 bd;
    s32 l[288];

    for (i = 0; i < 144; i++) {
        l[i] = 8;
    }
    for (; i < 256; i++) {
        l[i] = 9;
    }
    for (; i < 280; i++) {
        l[i] = 7;
    }
    for (; i < 288; i++) {
        l[i] = 8;
    }
    bl = 7;
    func_80297FE0(l, 288, 257, D_802FF644, D_802FF684, &tl, &bl);

    for (i = 0; i < 30; i++) {
        l[i] = 5;
    }
    bd = 5;
    func_80297FE0(l, 30, 0, D_802FF6A4, D_802FF6E0, &td, &bd);

    func_8029867C(tl, td, bl, bd);
    return 0;
}

#define LL_SIZE (286 + 30)

s32 func_80298DC0(void) {
    s32 i;
    u32 j;
    u32 l;
    u32 m;
    u32 n;
    Huft *tl;
    Huft *td;
    s32 bl;
    s32 bd;
    u32 nb;
    u32 nl;
    u32 nd;
    register u32 k;
    register u32 b;
    s32 ll[LL_SIZE];

    k = D_8039CAE8;
    b = D_8039CAE4;

    while (k < 5) {
        b |= (u32) D_803669C0[D_803669EC++] << k;
        k += 8;
    }
    nl = 257 + (b & 0x1F);
    b >>= 5;
    k -= 5;

    while (k < 5) {
        b |= (u32) D_803669C0[D_803669EC++] << k;
        k += 8;
    }
    nd = 1 + (b & 0x1F);
    b >>= 5;
    k -= 5;

    while (k < 4) {
        b |= (u32) D_803669C0[D_803669EC++] << k;
        k += 8;
    }
    nb = 4 + (b & 0xF);
    b >>= 4;
    k -= 4;

    for (j = 0; j < nb; j++) {
        while (k < 3) {
            b |= (u32) D_803669C0[D_803669EC++] << k;
            k += 8;
        }
        ll[D_802FF630[j]] = b & 7;
        b >>= 3;
        k -= 3;
    }
    for (; j < 19; j++) {
        ll[D_802FF630[j]] = 0;
    }

    bl = 7;
    func_80297FE0(ll, 19, 19, NULL, NULL, &tl, &bl);

    n = nl + nd;
    m = D_802FF700[bl];
    i = l = 0;
    while ((u32) i < n) {
        while (k < (u32) bl) {
            b |= (u32) D_803669C0[D_803669EC++] << k;
            k += 8;
        }
        td = tl + (b & m);
        j = td->b;
        b >>= j;
        k -= j;
        j = td->v.n;
        if (j < 16) {
            ll[i++] = l = j;
        } else if (j == 16) {
            while (k < 2) {
                b |= (u32) D_803669C0[D_803669EC++] << k;
                k += 8;
            }
            j = 3 + (b & 3);
            b >>= 2;
            k -= 2;
            while (j--) {
                ll[i++] = l;
            }
        } else if (j == 17) {
            while (k < 3) {
                b |= (u32) D_803669C0[D_803669EC++] << k;
                k += 8;
            }
            j = 3 + (b & 7);
            b >>= 3;
            k -= 3;
            while (j--) {
                ll[i++] = 0;
            }
            l = 0;
        } else {
            while (k < 7) {
                b |= (u32) D_803669C0[D_803669EC++] << k;
                k += 8;
            }
            j = 11 + (b & 0x7F);
            b >>= 7;
            k -= 7;
            while (j--) {
                ll[i++] = 0;
            }
            l = 0;
        }
    }

    D_8039CAE4 = b;
    D_8039CAE8 = k;

    bl = D_802FF724;
    func_80297FE0(ll, nl, 257, D_802FF644, D_802FF684, &tl, &bl);
    bd = D_802FF728;
    func_80297FE0(ll + nl, nd, 0, D_802FF6A4, D_802FF6E0, &td, &bd);
    func_8029867C(tl, td, bl, bd);
    return 0;
}

s32 func_802993AC(s32 *arg0) {
    s32 type;
    register u32 k;
    register u32 b;

    k = D_8039CAE8;
    b = D_8039CAE4;

    while (k == 0) {
        b |= (u32) D_803669C0[D_803669EC++] << k;
        k += 8;
    }
    *arg0 = b & 1;
    b = b >> 1;
    k -= 1;

    while (k < 2) {
        b |= (u32) D_803669C0[D_803669EC++] << k;
        k += 8;
    }
    type = b & 3;
    b = b >> 2;
    k -= 2;

    D_8039CAE4 = b;
    D_8039CAE8 = k;

    if (type == 2) {
        return func_80298DC0();
    }
    if (type == 0) {
        return func_80298A84();
    }
    if (type == 1) {
        return func_80298C18();
    }
    return 2;
}

s32 func_802994F8(void) {
    s32 sp24;
    s32 sp20;
    u32 sp1C;

    D_803669F0 = 0;
    D_8039CAE8 = 0;
    D_8039CAE4 = 0;
    sp1C = 0;
    for (;;) {
        D_8039CAEC = 0;
        sp20 = func_802993AC(&sp24);
        if (sp20 != 0) {
            return sp20;
        }
        if (D_8039CAEC > sp1C) {
            sp1C = D_8039CAEC;
        }
        if (sp24 != 0) {
            break;
        }
    }
    if (D_8039CAE8 >= 8) {
        do {
            D_8039CAE8 -= 8;
            D_803669EC -= 1;
        } while (D_8039CAE8 >= 8);
    }
    D_803669EC += 8;
    return 0;
}
