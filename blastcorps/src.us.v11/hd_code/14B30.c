#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* drawtext.c (from its assert): batched text quads. */

typedef struct {
    u16 key;
    s32 vtx;
    u8 *tex;
} TextQuad; /* size 0xC */

extern u64 D_80364A98;
extern s32 D_802E8BDC;
extern s32 D_80365350;
extern u8 *D_80358070;
extern Vtx *D_80365348[2];
extern TextQuad *D_80365340;
extern s32 D_802E8C70;
extern s32 D_802E8C74;
extern s32 D_802E8C78;
extern u8 D_8035805C;
extern f32 D_802E8C84[]; /* per-font advance */
extern u16 D_802E8C8C[]; /* per-font characters that draw nothing */
extern u16 D_802E8C90[];
extern u16 D_802E8C94[];

#define VTX(n) D_80365348[D_8035805C][n].v

void func_8025946C(Gfx **gdlp, s32 arg1);
void func_80259824(Gfx **gdlp, s32 arg1);
void func_802597D8(u8 *dst, u8 *src, s32 n);
s32 func_80259814(u16 *arg0, u16 *arg1);

void func_802592F0(void) {
    s32 i;

    if ((D_80364A98 & 0xC9FD8FE7DBFF8080) || D_80364A98 == 0x100000000000 || D_80364A98 == 2 ||
        D_802E8BDC == 0x28 || D_802E8BDC == 0x32) {
        D_80365350 = 0x200;
    } else if (D_80364A98 == 0x40) {
        D_80365350 = 0x9C;
    } else {
        D_80365350 = 0xAC;
    }
    for (i = 0; i < 2; i++) {
        D_80365348[i] = D_80358070;
        D_80358070 += D_80365350 * 16 * 4;
    }
    D_80365340 = D_80358070;
    D_80358070 += D_80365350 * 12;
    func_8025B070();
}

void func_80259450(void) {
    D_802E8C70 = 0;
    D_802E8C74 = 0;
    D_802E8C78 = 0;
}

void func_8025946C(Gfx **gdlp, s32 arg1) {
    Gfx *gdl = *gdlp;

    gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
    gSPSetGeometryMode(gdl++, G_SHADE | G_SHADING_SMOOTH);
    gDPPipeSync(gdl++);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    gDPSetRenderMode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    gDPSetCombineMode(gdl++, G_CC_MODULATERGBA, G_CC_MODULATERGBA);
    gDPSetTextureFilter(gdl++, G_TF_BILERP);
    gSPTexture(gdl++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    *gdlp = gdl;
}

/* Shell sort (h = 3h + 1) of n elements of the given size. */
void func_802595E0(u8 *base, s32 n, s32 size, s32 (*cmp)(void *, void *)) {
    s32 i;
    s32 j;
    s32 h;
    u8 *b;
    u8 tmp[256];

    b = base;
    for (h = 1; h <= n / 9; h = 3 * h + 1) {
    }
    for (; h > 0; h /= 3) {
        for (i = h; i < n; i++) {
            func_802597D8(tmp, b + size * i, size);
            j = i;
            while (j >= h && cmp(b + (j - h) * size, tmp) > 0) {
                func_802597D8(b + size * j, b + (j - h) * size, size);
                j -= h;
            }
            func_802597D8(b + size * j, tmp, size);
        }
    }
}

void func_802597D8(u8 *dst, u8 *src, s32 n) {
    s32 i;

    for (i = 0; i < n; i++) {
        dst[i] = src[i];
    }
}

s32 func_80259814(u16 *arg0, u16 *arg1) {
    return *arg0 - *arg1;
}

void func_80259824(Gfx **gdlp, s32 arg1) {
    u8 *tex = NULL;
    Gfx *gdl = *gdlp;
    s32 i;

    func_802595E0((u8 *) &D_80365340[D_802E8C70], D_802E8C74 - D_802E8C70, sizeof(TextQuad), func_80259814);
    for (i = D_802E8C70; i < D_802E8C74; i++) {
        if (D_80365340[i].tex != tex) {
            tex = D_80365340[i].tex;
            if (i == D_802E8C70) {
                gDPLoadTextureBlock_4b(gdl++, OS_K0_TO_PHYSICAL(tex), G_IM_FMT_I, 32, 32, 0, G_TX_CLAMP, G_TX_CLAMP,
                                       G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            } else {
                gDPSetTextureImage(gdl++, G_IM_FMT_I, G_IM_SIZ_16b, 1, OS_K0_TO_PHYSICAL(tex));
                gDPLoadSync(gdl++);
                gDPLoadBlock(gdl++, G_TX_LOADTILE, 0, 0, 0xFF, 0x400);
            }
        }
        gSPVertex(gdl++, &D_80365348[D_8035805C][D_80365340[i].vtx * 4], 4, 0);
        gSP1Triangle(gdl++, 0, 1, 2, 0);
        gSP1Triangle(gdl++, 0, 2, 3, 0);
    }
    osWritebackDCache(&D_80365348[D_8035805C][D_802E8C70 * 4], (D_802E8C74 - D_802E8C70) * 64);
    D_802E8C70 = D_802E8C74;
    *gdlp = gdl;
}

void func_80259BD4(Gfx **gdlp, s32 arg1) {
    Gfx *gdl = *gdlp;

    func_8025946C(&gdl, arg1);
    func_80259824(&gdl, arg1);
    *gdlp = gdl;
}

void func_80259C24(Gfx **gdlp, Mtx *arg1) {
    Gfx *gdl = *gdlp;

    gSPMatrix(gdl++, &arg1[3], G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &arg1[7], G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    func_8025946C(&gdl, (s32) arg1);
    func_80259824(&gdl, (s32) arg1);
    *gdlp = gdl;
}

void func_80259EC4(Gfx **gfxp, u8 *str, u16 *wstr, u8 align, s32 fit, f32 x, s32 y, f32 w, s32 h, u8 forward,
                   u8 r0, u8 g0, u8 b0, u8 a0, u8 r1, u8 g1, u8 b1, u8 a1, u8 r2, u8 g2, u8 b2, u8 a2,
                   u8 r3, u8 g3, u8 b3, u8 a3);

/* Draws a string in one flat colour. */
void func_80259CCC(Gfx **gfxp, u8 *str, u16 *wstr, u8 align, s32 fit, s32 x, s32 y, s32 w, s32 h, u8 forward,
                   u8 r, u8 g, u8 b, u8 a) {
    func_80259EC4(gfxp, str, wstr, align, fit, x, y, w, h, forward, r, g, b, a, r, g, b, a, r, g, b, a, r, g, b, a);
}

/* Draws a string with a vertical gradient (top colour, bottom colour). */
void func_80259DC8(Gfx **gfxp, u8 *str, u16 *wstr, u8 align, s32 fit, s32 x, s32 y, s32 w, s32 h, u8 forward,
                   u8 r0, u8 g0, u8 b0, u8 a0, u8 r1, u8 g1, u8 b1, u8 a1) {
    func_80259EC4(gfxp, str, wstr, align, fit, x, y, w, h, forward, r0, g0, b0, a0, r0, g0, b0, a0, r1, g1, b1, a1,
                  r1, g1, b1, a1);
}

/* Queues one textured quad per visible character of str (or the 0xFFE-
 * terminated wide string wstr) into the current vertex buffer and the
 * TextQuad list; func_80259824 later sorts and draws them. Characters with
 * all four alphas zero, and the per-font space characters, only advance. */
void func_80259EC4(Gfx **gfxp, u8 *str, u16 *wstr, u8 align, s32 fit, f32 x, s32 y, f32 w, s32 h, u8 forward,
                   u8 r0, u8 g0, u8 b0, u8 a0, u8 r1, u8 g1, u8 b1, u8 a1, u8 r2, u8 g2, u8 b2, u8 a2,
                   u8 r3, u8 g3, u8 b3, u8 a3) {
    u16 glyph;
    u8 *p;
    u16 *wp;
    u8 done = 0;
    f32 adv;
    u8 wide = 0;
    u8 font = 0;
    s32 ch;

    if (str == NULL || *str == 0) {
        return;
    }
    p = str;
    wp = wstr;
    if (!forward) {
        if (wide) {
            while (*wp != 0xFFE) {
                wp++;
            }
            wp--;
        } else {
            while (*p != 0) {
                p++;
            }
            p--;
        }
    }
    if (fit) {
        x = func_8025B498(fit, w, str, wstr);
    }
    while (!done) {
        switch (font) {
            case 0:
                adv = 0.21f;
                switch (*p) {
                    case '0':
                        adv = 0.25f;
                        glyph = 0;
                        break;
                    case '1':
                        adv = 0.25f;
                        glyph = 1;
                        break;
                    case '2':
                        adv = 0.25f;
                        glyph = 2;
                        break;
                    case '3':
                        adv = 0.25f;
                        glyph = 3;
                        break;
                    case '4':
                        adv = 0.25f;
                        glyph = 4;
                        break;
                    case '5':
                        adv = 0.25f;
                        glyph = 5;
                        break;
                    case '6':
                        adv = 0.25f;
                        glyph = 6;
                        break;
                    case '7':
                        adv = 0.25f;
                        glyph = 7;
                        break;
                    case '8':
                        adv = 0.25f;
                        glyph = 8;
                        break;
                    case '9':
                        adv = 0.25f;
                        glyph = 9;
                        break;
                    case 'A':
                        glyph = 10;
                        break;
                    case 'B':
                        glyph = 11;
                        break;
                    case 'C':
                        glyph = 12;
                        break;
                    case 'D':
                        glyph = 13;
                        break;
                    case 0x7f:
                        adv = 0.05f;
                        glyph = 14;
                        break;
                    case 'E':
                        glyph = 15;
                        break;
                    case 'F':
                        glyph = 16;
                        break;
                    case 'G':
                        glyph = 17;
                        break;
                    case 'H':
                        glyph = 18;
                        break;
                    case 'I':
                        adv = 0.27f;
                        glyph = 19;
                        break;
                    case 'J':
                        glyph = 20;
                        break;
                    case 'K':
                        glyph = 21;
                        break;
                    case 'L':
                        glyph = 22;
                        break;
                    case 'M':
                        adv = 0.05f;
                        glyph = 23;
                        break;
                    case 'N':
                        glyph = 24;
                        break;
                    case 'O':
                        glyph = 25;
                        break;
                    case 'P':
                        glyph = 26;
                        break;
                    case 'Q':
                        glyph = 27;
                        break;
                    case 'R':
                        glyph = 28;
                        break;
                    case 'S':
                        glyph = 29;
                        break;
                    case 'T':
                        glyph = 30;
                        break;
                    case 'U':
                        glyph = 31;
                        break;
                    case 'V':
                        glyph = 32;
                        break;
                    case 'W':
                        adv = 0.05f;
                        glyph = 33;
                        break;
                    case 'X':
                        glyph = 34;
                        break;
                    case 'Y':
                        glyph = 35;
                        break;
                    case 'Z':
                        glyph = 36;
                        break;
                    case '\'':
                        adv = 0.25f;
                        glyph = 38;
                        break;
                    case ')':
                        adv = 0.25f;
                        glyph = 39;
                        break;
                    case ':':
                        adv = 0.36f;
                        glyph = 40;
                        break;
                    case ',':
                        adv = 0.25f;
                        glyph = 41;
                        break;
                    case '$':
                        glyph = 42;
                        break;
                    case '!':
                        adv = 0.25f;
                        glyph = 43;
                        break;
                    case '.':
                        adv = 0.3f;
                        glyph = 44;
                        break;
                    case '-':
                        adv = 0.25f;
                        glyph = 45;
                        break;
                    case '(':
                        adv = 0.25f;
                        glyph = 46;
                        break;
                    case '%':
                        glyph = 47;
                        break;
                    case '?':
                        glyph = 48;
                        break;
                    case '#':
                        glyph = 49;
                        break;
                    case '/':
                        glyph = 50;
                        break;
                    case ' ':
                    case '&':
                        adv = 0.3f;
                        glyph = 0;
                        break;
                    case 'a':
                    case 'b':
                    case 'd':
                    case 'e':
                    case 'k':
                    case 'm':
                        glyph = *p - 0x22C;
                        break;
                    default:
                        glyph = 0;
                        adv = 0.0f;
                        break;
                }
                glyph += 0xF4C;
                break;
            case 1:
                adv = 1.0f;
                if (*wp < 0x200) {
                    glyph = *wp + 0xD4C;
                } else if (*wp == 0x1001) {
                    glyph = 0xF7D;
                } else {
                    glyph = 0xD4C;
                }
                break;
        }
        if (align == 1) {
            if (forward) {
                x -= adv * w;
            } else {
                x += adv * w;
            }
        }
        if (wide) {
            ch = *wp;
        } else {
            ch = *p;
        }
        if (ch != D_802E8C94[font] && ch != D_802E8C90[font] && ch != D_802E8C8C[font] &&
            (a0 != 0 || a1 != 0 || a2 != 0 || a3 != 0)) {
            VTX(D_802E8C78).ob[0] = x;
            VTX(D_802E8C78).ob[1] = y;
            VTX(D_802E8C78).ob[2] = -10;
            VTX(D_802E8C78).flag = 0;
            VTX(D_802E8C78).tc[0] = 0;
            VTX(D_802E8C78).tc[1] = 0x3E0;
            VTX(D_802E8C78).cn[0] = r0;
            VTX(D_802E8C78).cn[1] = g0;
            VTX(D_802E8C78).cn[2] = b0;
            VTX(D_802E8C78).cn[3] = a0;
            D_802E8C78++;
            VTX(D_802E8C78).ob[0] = x + w;
            VTX(D_802E8C78).ob[1] = y;
            VTX(D_802E8C78).ob[2] = -10;
            VTX(D_802E8C78).flag = 0;
            VTX(D_802E8C78).tc[0] = 0x3E0;
            VTX(D_802E8C78).tc[1] = 0x3E0;
            VTX(D_802E8C78).cn[0] = r1;
            VTX(D_802E8C78).cn[1] = g1;
            VTX(D_802E8C78).cn[2] = b1;
            VTX(D_802E8C78).cn[3] = a1;
            D_802E8C78++;
            VTX(D_802E8C78).ob[0] = x + w;
            VTX(D_802E8C78).ob[1] = y + h;
            VTX(D_802E8C78).ob[2] = -10;
            VTX(D_802E8C78).flag = 0;
            VTX(D_802E8C78).tc[0] = 0x3E0;
            VTX(D_802E8C78).tc[1] = 0;
            VTX(D_802E8C78).cn[0] = r2;
            VTX(D_802E8C78).cn[1] = g2;
            VTX(D_802E8C78).cn[2] = b2;
            VTX(D_802E8C78).cn[3] = a2;
            D_802E8C78++;
            VTX(D_802E8C78).ob[0] = x;
            VTX(D_802E8C78).ob[1] = y + h;
            VTX(D_802E8C78).ob[2] = -10;
            VTX(D_802E8C78).flag = 0;
            VTX(D_802E8C78).tc[0] = 0;
            VTX(D_802E8C78).tc[1] = 0;
            VTX(D_802E8C78).cn[0] = r3;
            VTX(D_802E8C78).cn[1] = g3;
            VTX(D_802E8C78).cn[2] = b3;
            VTX(D_802E8C78).cn[3] = a3;
            D_802E8C78++;
            D_80365340[D_802E8C74].key = ch;
            if (r0 != 0 || b0 != 0 || g0 != 0) {
                D_80365340[D_802E8C74].key += 0x8000;
            }
            D_80365340[D_802E8C74].vtx = D_802E8C74;
            D_80365340[D_802E8C74].tex = func_8025B0B8(glyph);
            D_802E8C74++;
            if (!(D_802E8C74 < D_80365350)) {
                func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", "index<maxCharacters", "drawtext.c", 435);
            }
            if (D_802E8C74 >= D_80365350) {
                func_8029A7E4("%d %d\n", D_802E8C74, D_80365350);
            }
        }
        if (align == 1) {
            if (forward) {
                x += w - adv * w;
            } else {
                x -= w - adv * w;
            }
        } else if (ch != D_802E8C8C[font]) {
            if (forward) {
                x += w * D_802E8C84[font];
            }
            if (!forward) {
                x -= w * D_802E8C84[font];
            }
        }
        if (forward) {
            if (wide) {
                wp++;
                if (*wp == 0xFFE) {
                    done = 1;
                }
            } else {
                p++;
                if (*p == 0) {
                    done = 1;
                }
            }
        } else {
            if (wide) {
                if (wp == wstr) {
                    done = 1;
                }
                wp--;
            } else {
                if (p == str) {
                    done = 1;
                }
                p--;
            }
        }
    }
}
