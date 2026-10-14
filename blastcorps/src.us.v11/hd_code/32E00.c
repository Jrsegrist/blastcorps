#include "common.h"
#include <ultra64.h>
#ifndef NON_MATCHING /* legacy declarations */
/* Matching build: IDO compiled the matched code here against older
 * declarations of these, which the file keeps; the NON_MATCHING build
 * uses game/game.h's. */
#define LEGACY_gDigTriggers
#define LEGACY_gDigTriggersEnd
#endif /* legacy declarations */
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_02000000 ((u8 *) D_02000000)
#ifdef NON_MATCHING
#define gDigTriggers (*(DigTrigger * N64P *) &gDigTriggers)
#define gDigTriggersEnd (*(DigTrigger * N64P *) &gDigTriggersEnd)
#endif
/* end of views */

/* The digger code (this file's .bss starts at 0x8036C8D0) */

/* 50-entry ring buffer of digger samples, indexed D_8036CB28..D_8036CB29 */
typedef struct {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ u8 unk4;   /* digger state when sampled */
    /* 0x08 */ s32 time;
} DigEntry;

/* Timed digger events, 8 bytes */
typedef struct {
    /* 0x00 */ s16 time;
    /* 0x02 */ u8 a;
    /* 0x03 */ u8 b;
    /* 0x04 */ u8 c;
    /* 0x05 */ u8 d;
    /* 0x06 */ u8 done;
} DigTrigger;

#ifndef NON_MATCHING
extern DigTrigger *gDigTriggers; /* first */
#ifndef NON_MATCHING
extern DigTrigger *gDigTriggersEnd; /* end */
#endif
#endif
extern u8 D_8036CB35;
extern u8 D_8036CB36;
extern u8 D_8036CB37;
extern u8 D_8036CB38;
extern u8 D_8036CB39;
extern u8 D_8036CB3A;
extern u8 D_8036CB3B;
extern u8 D_8036CB3C;
extern s16 * N64P D_8036CB40; /* event animation frame ids */
extern u8 D_8036CB44;
extern u8 D_8036CB50;
extern u8 D_8036CB51;      /* event panel alpha */
extern s16 D_802FBDD0[];
extern s16 D_802FBDEC[];
extern s16 D_802FBE18[];
extern s16 D_802FBE44[];
extern s16 D_802FBE80[];
extern Vtx D_802FBD50[8];

extern DigEntry gDigSamples[50];
extern u8 D_8036CB32;
extern void * N64P D_8036CB48[2];
extern u8 D_8036CB28;
extern u8 D_8036CB29;
extern u8 D_8036CB30;
extern u8 D_8036CB31;
extern u8 D_8036CB33;
extern u8 gPanelActive;

s32 dig_hit_is_weak(void);
s32 dig_part_hit_is_weak(void);
void dig_count_samples(void);
void dig_check_doing_well(void);
void dig_check_wrong_digger(void);
void dig_check_misuse(void);

/* The message panel's two 40x40 frame buffers (0x802775C0). */
void hud_panel_init(void) {
    gPanelActive = 0;
    D_8036CB48[0] = gHeapPtr;
    D_8036CB48[1] = gHeapPtr = (u8 *) gHeapPtr + 0xC80;
    gHeapPtr = (u8 *) gHeapPtr + 0xC80;
    D_8036CB28 = 0;
    D_8036CB29 = 0;
}

/* Samples digger hits, the coaching checks, the timed panel events (0x80277620). */
void dig_coach_update(s32 now) {
    u8 done = 0;
    DigTrigger *p = gDigTriggers;
    s16 limit;

    while (!done && D_8036CB28 != D_8036CB29) {
        if (now - gDigSamples[D_8036CB28].time > 1000) {
            if (++D_8036CB28 == 50) {
                D_8036CB28 = 0;
            }
        } else {
            done = 1;
        }
    }
    if (gDigHit) {
        if (D_8036CB29 + 1 != D_8036CB28 && !(D_8036CB29 == 49 && D_8036CB28 == 0)) {
            gDigSamples[D_8036CB29].unk0 = dig_hit_is_weak();
            gDigSamples[D_8036CB29].unk2 = dig_part_hit_is_weak();
            gDigSamples[D_8036CB29].unk4 = gDigHitVehicle;
            gDigSamples[D_8036CB29].time = now;
            if (++D_8036CB29 == 50) {
                D_8036CB29 = 0;
            }
        }
    }
    if (gDigHit) {
        dig_count_samples();
        dig_check_doing_well();
        dig_check_wrong_digger();
        dig_check_misuse();
    }
    if (now == 50 && !gCarrierHit) {
        hud_panel_start(2, 1, 2, snd_random_id(SND_GROUP_LEVEL_START));
    }
    if (gHaveCarrier) {
        limit = gCarrierZ >> 5;
        while (p < gDigTriggersEnd) {
            if (!p->done && limit > p->time) {
                hud_panel_start(p->a, p->b, p->c, p->d);
                p->done = 1;
            }
            p++;
        }
    }
    gDigHit = 0;
}

/* Most recent hits strong: "DOING REALLY WELL" (0x802778FC). */
void dig_check_doing_well(void) {
    switch (gCurVehicle) {
        case 5:
            if (D_8036CB31 >= 16 && D_8036CB30 >= 16) {
                hud_panel_start(3, 1, 1, SFX_COACH_WELL_BACKLASH);
                D_8036CB28 = 0;
                D_8036CB29 = 0;
                debug_printf("DOING REALLY WELL\n");
            }
            break;
        case 4:
            if (D_8036CB31 >= 41 && D_8036CB30 >= 41) {
                hud_panel_start(4, 1, 1, snd_random_id(SND_GROUP_DOING_WELL));
                D_8036CB28 = 0;
                D_8036CB29 = 0;
                debug_printf("DOING REALLY WELL\n");
            }
            break;
        case 3:
            if (D_8036CB31 >= 11 && D_8036CB30 >= 11) {
                hud_panel_start(0, 1, 3, SFX_COACH_WELL_SKYFALL);
                D_8036CB28 = 0;
                D_8036CB29 = 0;
                debug_printf("DOING REALLY WELL\n");
            }
            break;
        case 9:
            if (D_8036CB31 >= 26 && D_8036CB30 >= 26) {
                hud_panel_start(4, 1, 1, snd_random_id(SND_GROUP_DOING_WELL));
                D_8036CB28 = 0;
                D_8036CB29 = 0;
                debug_printf("DOING REALLY WELL\n");
            }
            break;
    }
}

/* Strong hits but weak part damage: "USING WRONG DIGGER" (0x80277AE0). */
void dig_check_wrong_digger(void) {
    switch (gCurVehicle) {
        case 3:
        case 4:
        case 5:
        case 9:
            if (D_8036CB31 >= 6 && D_8036CB30 < 2) {
                hud_panel_start(1, 1, 3, SFX_COACH_WRONG_DIGGER);
                D_8036CB28 = 0;
                D_8036CB29 = 0;
                debug_printf("USING WRONG DIGGER\n");
            }
            break;
    }
}

/* Most hits weak: "USING DIGGER INCORRECTLY" (0x80277B84). */
void dig_check_misuse(void) {
    switch (gCurVehicle) {
        case 3:
        case 4:
        case 5:
            if (D_8036CB33 >= 31 && D_8036CB30 < 6) {
                hud_panel_start(2, 1, 2, SFX_COACH_MISUSE);
                D_8036CB28 = 0;
                D_8036CB29 = 0;
                debug_printf("USING DIGGER INCORRECTLY\n");
            }
            break;
    }
}

/* Counts strong and weak hits in the sample ring (0x80277C20). */
void dig_count_samples(void) {
    u8 i;

    D_8036CB30 = 0;
    i = D_8036CB28;
    D_8036CB31 = 0;
    D_8036CB32 = 0;
    D_8036CB33 = 0;
    while (i != D_8036CB29) {
        if (!gDigSamples[i].unk2) {
            D_8036CB30++;
        } else {
            D_8036CB32++;
        }
        if (!gDigSamples[i].unk0) {
            D_8036CB31++;
        } else {
            D_8036CB33++;
        }
        if (++i == 50) {
            i = 0;
        }
    }
}

/* 1 if the hit's damage is under the vehicle's limit (0x80277D34). */
s32 dig_hit_is_weak(void) {
    switch (gDigHitVehicle) {
        case 5:
            if (gDigHitDamage > 400) {
                return 0;
            }
            return 1;
        case 4:
            if (gDigHitDamage > 400) {
                return 0;
            }
            return 1;
        case 9:
            if (gDigHitDamage >= 100) {
                return 0;
            }
            return 1;
        case 3:
            if (gDigHitDamage >= 100) {
                return 0;
            }
            return 1;
    }
    return 1;
}

/* The same on the per-part damage (0x80277E08). */
s32 dig_part_hit_is_weak(void) {
    switch (gDigHitVehicle) {
        case 5:
            if (gDigHitDamagePerPart > 80) {
                return 0;
            }
            return 1;
        case 4:
            if (gDigHitDamagePerPart > 80) {
                return 0;
            }
            return 1;
        case 9:
            if (gDigHitDamagePerPart >= 100) {
                return 0;
            }
            return 1;
        case 3:
            if (gDigHitDamagePerPart >= 100) {
                return 0;
            }
            return 1;
    }
    return 1;
}

/* Starts message animation type 0..4 of the coaching panel, with its sound (0x80277EDC).
 * Starts a digger event. A K&R definition: callers pass plain ints, and the
 * u8 parameters read their low bytes */
#ifdef NON_MATCHING
void hud_panel_start(u8 type, u8 arg1, s32 arg2, u8 sound)
#else
void hud_panel_start(type, arg1, arg2, sound)
    u8 type;
    u8 arg1;
    s32 arg2;
    u8 sound;
#endif
{
#ifdef NON_MATCHING
    u8 pos = 0; /* types without a case (with gLevelKind == 1) leave it indeterminate in the original */
#else
    u8 pos;
#endif

    if ((gGameMode & (MODE_LEVEL | MODE_LEVEL_CLEAR | MODE_CARRIER_END | MODE_LEVEL_START | MODE_TIMED_END | MODE_YOSHI_PAGES)) && !gPanelActive) {
        if (sound) {
            snd_play(gSfxBank, sound, NULL);
        }
        gPanelActive = 1;
        D_8036CB35 = arg2;
        D_8036CB36 = 0;
        D_8036CB37 = 0;
        D_8036CB39 = 0;
        D_8036CB3A = 0;
        D_8036CB38 = arg1;
        D_8036CB44 = type;
        D_8036CB50 = 0;
        switch (type) {
            case 0:
                pos = 0;
                D_8036CB3C = 13;
                D_8036CB40 = D_802FBDD0;
                D_8036CB3B = 1;
                break;
            case 1:
                pos = 1;
                D_8036CB3C = 21;
                D_8036CB40 = D_802FBDEC;
                D_8036CB3B = 1;
                break;
            case 2:
                pos = 0;
                D_8036CB3C = 21;
                D_8036CB40 = D_802FBE18;
                D_8036CB3B = 2;
                break;
            case 3:
                pos = 1;
                D_8036CB3C = 30;
                D_8036CB40 = D_802FBE44;
                D_8036CB3B = 1;
                break;
            case 4:
                pos = 0;
                D_8036CB3C = 34;
                D_8036CB40 = D_802FBE80;
                D_8036CB3B = 1;
                break;
        }
        if (gLevelKind != 1) {
            pos = 1;
        }
        switch (pos) {
            case 0:
                D_802FBD50[0].v.ob[0] = 32, D_802FBD50[0].v.ob[1] = 68;
                D_802FBD50[1].v.ob[0] = 78, D_802FBD50[1].v.ob[1] = 68;
                D_802FBD50[2].v.ob[0] = 78, D_802FBD50[2].v.ob[1] = 23;
                D_802FBD50[3].v.ob[0] = 32, D_802FBD50[3].v.ob[1] = 23;
                D_802FBD50[4].v.ob[0] = 26, D_802FBD50[4].v.ob[1] = 73;
                D_802FBD50[5].v.ob[0] = 84, D_802FBD50[5].v.ob[1] = 73;
                D_802FBD50[6].v.ob[0] = 84, D_802FBD50[6].v.ob[1] = 13;
                D_802FBD50[7].v.ob[0] = 26, D_802FBD50[7].v.ob[1] = 13;
                break;
            case 1:
                D_802FBD50[0].v.ob[0] = 238, D_802FBD50[0].v.ob[1] = 222;
                D_802FBD50[1].v.ob[0] = 284, D_802FBD50[1].v.ob[1] = 222;
                D_802FBD50[2].v.ob[0] = 284, D_802FBD50[2].v.ob[1] = 177;
                D_802FBD50[3].v.ob[0] = 238, D_802FBD50[3].v.ob[1] = 177;
                D_802FBD50[4].v.ob[0] = 232, D_802FBD50[4].v.ob[1] = 227;
                D_802FBD50[5].v.ob[0] = 290, D_802FBD50[5].v.ob[1] = 227;
                D_802FBD50[6].v.ob[0] = 290, D_802FBD50[6].v.ob[1] = 167;
                D_802FBD50[7].v.ob[0] = 232, D_802FBD50[7].v.ob[1] = 167;
                break;
        }
    }
}

/* Stops the message panel (0x80278318). */
void hud_panel_stop(void) {
    gPanelActive = 0;
}

/* Inflates the frame, animates, fades and draws the panel (0x80278324).
 * Animates and draws the digger event panel */
void hud_panel_draw(Gfx **gfx, s32 arg1, u8 buf) {
    Gfx *gdl = *gfx;

    if (gPanelActive) {
        tex_queue_load(D_8036CB40[D_8036CB39], D_8036CB48[buf], 0);
        switch (D_8036CB38) {
            case 1:
                if (!D_8036CB37) {
                    D_8036CB3A++;
                    if (D_8036CB3A == D_8036CB3B) {
                        D_8036CB3A = 0;
                        D_8036CB39++;
                        if (D_8036CB39 + 1 == D_8036CB3C) {
                            D_8036CB37 = 1;
                            D_8036CB36++;
                        }
                    }
                } else {
                    D_8036CB3A++;
                    if (D_8036CB3A == D_8036CB3B) {
                        D_8036CB3A = 0;
                        D_8036CB39--;
                        if (D_8036CB39 == 0) {
                            D_8036CB37 = 0;
                            D_8036CB36++;
                        }
                    }
                }
                break;
            case 0:
                D_8036CB3A++;
                if (D_8036CB3A == D_8036CB3B) {
                    D_8036CB3A = 0;
                    D_8036CB39++;
                    if (D_8036CB39 == D_8036CB3C) {
                        D_8036CB39 = 0;
                        D_8036CB36++;
                    }
                }
                break;
        }
        if (D_8036CB36 == D_8036CB35) {
            gPanelActive = 0;
        }
        if (D_8036CB36 == 0 && D_8036CB39 < 5) {
            if (D_8036CB39 == 0) {
                snd_play(gSfxBank, SFX_STATIC, NULL);
            }
            D_8036CB51 = 0x50;
        } else if (D_8036CB50) {
            D_8036CB51 -= 15;
            D_8036CB50--;
        } else if (!util_rand_range(0, 20)) {
            D_8036CB50 = 5;
            D_8036CB51 = 0x50;
            snd_play(gSfxBank, SFX_STATIC, NULL);
        } else {
            D_8036CB51 = 0;
        }
        gSPMatrix(gdl++, (u32) D_02000000 + 0xC0, G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
        gSPMatrix(gdl++, (u32) D_02000000 + 0x1C0, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
        gDPPipeSync(gdl++);
        gDPSetCycleType(gdl++, G_CYC_2CYCLE);
        gDPSetRenderMode(gdl++, G_RM_PASS, G_RM_OPA_SURF2);
        gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
        gSPSetGeometryMode(gdl++, G_SHADE | G_SHADING_SMOOTH);
        gSPTexture(gdl++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
        gDPSetCombine(gdl++, 0xFC757E44, 0xFFFFF83C);
        gDPSetPrimColor(gdl++, 0, 0, 0, 0, 0, D_8036CB51);
        gDPLoadTextureBlock(gdl++, (u32) D_8036CB48[buf] - 0x80000000, G_IM_FMT_RGBA, G_IM_SIZ_16b, 40, 40, 0,
                            G_TX_MIRROR | G_TX_CLAMP, G_TX_MIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                            G_TX_NOLOD);
        gSPVertex(gdl++, (u32) D_802FBD50 - 0x80000000, 8, 0);
        gSP1Triangle(gdl++, 0, 1, 2, 0);
        gSP1Triangle(gdl++, 0, 2, 3, 0);
        gDPPipeSync(gdl++);
        gDPSetCycleType(gdl++, G_CYC_1CYCLE);
        gDPSetRenderMode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gDPSetCombineMode(gdl++, G_CC_MODULATERGBA, G_CC_MODULATERGBA);
        gDPLoadTextureBlock(gdl++, (u32) D_802FAD50 - 0x80000000, G_IM_FMT_RGBA, G_IM_SIZ_32b, 32, 32, 0,
                            G_TX_MIRROR | G_TX_CLAMP, G_TX_MIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                            G_TX_NOLOD);
        gSP1Triangle(gdl++, 4, 5, 6, 0);
        gSP1Triangle(gdl++, 4, 6, 7, 0);
        gDPPipeSync(gdl++);
    }
    *gfx = gdl;
}
