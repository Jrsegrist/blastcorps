#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* The file after sched.c. Its .rodata (two pairs of doubles at
 * 0x8030C4D0) and .bss (0x8036BFC0) each start on a fresh 16-byte
 * boundary, which is what left the 8 zero bytes after sched.c's jump
 * table. The .bss is placed by hd_code_bss.us.v11.ld. */
f32 D_8036BFC0; /* backdrop scroll speed */
u8 D_8036BFC4;   /* backdrop textures loaded */
u8 D_8036BFC5;   /* vertex double-buffer index */
f32 D_8036BFC8;
f32 D_8036BFCC;
f32 D_8036BFD0;

/* One backdrop texture, 0xC bytes; two per level */
typedef struct {
    /* 0x00 */ s16 id;
    /* 0x04 */ void *ptr;
    /* 0x08 */ u8 unk8; /* log2 of the s scale */
    /* 0x09 */ u8 unk9; /* log2 of the t scale */
    /* 0x0A */ u8 unkA; /* cms */
    /* 0x0B */ u8 unkB; /* cmt */
} FadeEntry;

typedef struct {
    u16 fmt[2];
} FmtPair;

extern FadeEntry D_802FA280[][2];
extern Vtx D_802FA820[2][4];
extern FmtPair D_802FA8A0;
extern u16 D_803BE720;
extern u16 D_803BE722;
extern u32 D_803BE718;
extern u32 D_803BE71C;
extern void *D_80358070;


/* Draws the two-layer scrolling backdrop and returns the new display
 * list end; *height gets the backdrop's visible height */
Gfx *func_80271FD0(Gfx *gfx, u32 arg1, u16 level, s16 arg3, s16 arg4, s32 *height) {
    Gfx *gdl = gfx;
    FmtPair fmts = D_802FA8A0;
    s32 i;
    FadeEntry *entry;
    f32 scroll;
    f32 s0;
    f32 t0;
    f32 s1;
    f32 t1;

    if (!D_8036BFC4) {
        *height = 0;
        return gdl;
    }
    scroll = 120.0 - (f32) arg4 * 0.22 + D_8036BFC0;
    D_8036BFCC = (0.0 > scroll) ? 0.0 : scroll;
    D_8036BFD0 = (0.0 > -scroll) ? 0.0 : -scroll;
    *height = D_8036BFCC;
    if (*height <= 0) {
        return gdl;
    }
    D_8036BFC8 = (arg3 - 2048.0) * 0.5;
    D_802FA820[D_8036BFC5][2].v.ob[1] = D_8036BFCC * 4.0 - 1.0;
    D_802FA820[D_8036BFC5][3].v.ob[1] = D_8036BFCC * 4.0 - 1.0;
    gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
    gSPSetGeometryMode(gdl++, G_SHADE | G_SHADING_SMOOTH);
    gSPTexture(gdl++, 0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON);
    gDPPipeSync(gdl++);
    gDPSetCombine(gdl++, 0xFC24A3FF, 0x1FFC9238);
    gDPSetCycleType(gdl++, G_CYC_2CYCLE);
    gDPSetRenderMode(gdl++, G_RM_PASS, G_RM_OPA_SURF2);
    gDPSetTextureFilter(gdl++, G_TF_BILERP);
    for (i = 0; i < 2; i++) {
        entry = &D_802FA280[level][i];
        gDPSetTextureImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, entry->ptr);
        gDPTileSync(gdl++);
        gDPSetTile(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, i * 256, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);
        gDPLoadSync(gdl++);
        gDPLoadBlock(gdl++, G_TX_LOADTILE, 0, 0, 1023, 256);
    }
    gSPMatrix(gdl++, arg1 + 0x80000100, G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gdl++, arg1 + 0x800001C0, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gDPTileSync(gdl++);
    gDPSetTextureLOD(gdl++, G_TL_TILE);
    for (i = 0; i < 2; i++) {
        entry = &D_802FA280[level][i];
        gDPSetTile(gdl++, fmts.fmt[i], G_IM_SIZ_16b, 8, i * 256, i, 0, entry->unkB & 3, 5, i, entry->unkA & 3, 5, i);
        gDPSetTileSize(gdl++, i, 0, 0, 31 << G_TEXTURE_IMAGE_FRAC, 31 << G_TEXTURE_IMAGE_FRAC);
    }
    s0 = (D_8036BFC8 + 16.0f) / (f32) (1 << entry->unk8);
    s1 = 319.0 / (f32) (1 << entry->unk8);
    D_802FA820[D_8036BFC5][0].v.tc[0] = s0 * 32.0;
    D_802FA820[D_8036BFC5][1].v.tc[0] = (s0 + s1) * 32.0;
    D_802FA820[D_8036BFC5][2].v.tc[0] = s0 * 32.0;
    D_802FA820[D_8036BFC5][3].v.tc[0] = (s0 + s1) * 32.0;
    t0 = (D_8036BFCC - D_8036BFC0 - D_8036BFD0) / (f32) (1 << entry->unk9);
    t1 = (D_8036BFCC - 1.0) / (f32) (1 << entry->unk9);
    D_802FA820[D_8036BFC5][0].v.tc[1] = t0 * 32.0;
    D_802FA820[D_8036BFC5][1].v.tc[1] = t0 * 32.0;
    D_802FA820[D_8036BFC5][2].v.tc[1] = (t0 - t1) * 32.0;
    D_802FA820[D_8036BFC5][3].v.tc[1] = (t0 - t1) * 32.0;
    gSPVertex(gdl++, D_802FA820[D_8036BFC5], 4, 0);
    gDPPipeSync(gdl++);
    gSP1Triangle(gdl++, 0, 1, 2, 0);
    gSP1Triangle(gdl++, 1, 2, 3, 0);
    D_8036BFC5 ^= 1;
    return gdl;
}

/* Sets up the backdrop for a level: scroll speed from the screen area
 * and the level type, and loads its two textures */
void func_802729F0(u16 type, u16 level) {
    s32 i;
    f32 area;
    u16 id;

    area = D_803BE720 * D_803BE718;
    area = (D_803BE722 * D_803BE71C + area) / 32.0f;
    D_8036BFC4 = 1;
    switch (type) {
        case 0:
        case 1:
        case 2:
        case 0x40:
        case 0x800:
        case 0x1000:
            D_8036BFC0 = 1200000.0 / area;
            break;
        case 4:
        case 0x100:
        case 0x2000:
            D_8036BFC0 = 600000.0 / area;
            break;
        default:
            D_8036BFC0 = 0.0f;
            break;
    }
    if (level == 0x34 || level == 0x3B || level == 0x26 || level == 0x11) {
        D_8036BFC0 += 32.0f;
    }
    for (i = 0; i < 2; i++) {
        D_802FA280[level][i].ptr = D_80358070;
        id = D_802FA280[level][i].id;
        if (id) {
            func_802A0B00(id, 0);
        } else {
            D_8036BFC4 = 0;
        }
    }
}

void func_80272C40(s32 arg0) {
}
