#include "common.h"
#include <ultra64.h>

extern u8 D_8036C360;
extern u32 D_803156C4;

Gfx *func_80272ED8(Gfx *gdl, u8 arg1, s16 arg2, s16 arg3, s32 arg4, s32 arg5, f32 arg6);

void func_80272C50(void) {
    D_8036C360 = 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/2E490/func_80272C5C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/2E490/func_80272ED8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/2E490/func_802742D8.s")

Gfx *func_80274868(Gfx *gfx) {
    Gfx *gdl = gfx;

    gDPPipeSync(gdl++);
    gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
    gSPTexture(gdl++, 0, 0, 0, G_TX_RENDERTILE, G_OFF);
    gDPSetTexturePersp(gdl++, G_TP_NONE);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    gDPSetRenderMode(gdl++, G_RM_PASS, G_RM_OPA_SURF2);
    gDPSetTextureFilter(gdl++, G_TF_BILERP);
    return gdl;
}

Gfx *func_80274998(Gfx *gfx) {
    Gfx *gdl = gfx;

    gDPPipeSync(gdl++);
    gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
    gSPSetGeometryMode(gdl++, G_SHADE | G_SHADING_SMOOTH);
    gSPTexture(gdl++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    gDPSetRenderMode(gdl++, G_RM_PASS, G_RM_OPA_SURF2);
    return gdl;
}

Gfx *func_80274AA4(Gfx *gfx) {
    Gfx *gdl = gfx;

    gDPPipeSync(gdl++);
    gDPSetTexturePersp(gdl++, G_TP_PERSP);
    return gdl;
}

Gfx *func_80274B08(Gfx *gfx) {
    Gfx *gdl = gfx;

    gDPPipeSync(gdl++);
    return gdl;
}

void func_80274B40(Gfx **gfx, s32 arg1, u8 arg2, s16 arg3, s16 arg4) {
    Gfx *gdl = *gfx;

    if (D_803156C4 % 40 < 28) {
        gdl = func_80274868(gdl);
        gdl = func_80272ED8(gdl, arg2, arg3, arg4, 0xFF, 1, 1.0f);
        gdl = func_80274AA4(gdl);
    }
    *gfx = gdl;
}
