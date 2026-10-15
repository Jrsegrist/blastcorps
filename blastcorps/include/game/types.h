#ifndef GAME_TYPES_H
#define GAME_TYPES_H

/*
 * Game structures used by more than one file.
 *
 * Several are partial: fields nobody has identified yet are padding, and a
 * file whose code reads the same memory differently keeps its own view type
 * (documented where it is defined).
 */

/* One per frame buffer, 0x21498 bytes (gDynamicBufs[2]): the in-game layout
 * that hd.c builds each frame. The area below the display list is scratch
 * space that other subsystems lay out their own way (their views: VtxBuf
 * 13A70.c, DynBuf 20460.c, SpriteVtxBuf 30C70.c, TrailBuf 37530.c,
 * Dyn48D00, Dyn4B5E0, Dyn 4DA80.c), and the front end uses FeDyn. */
typedef struct DynamicBuf {
    u8 pad0[0x80];
    Mtx persp;    /* 0x80: guPerspective (gCamFovy, 4:3) */
    Mtx ortho;    /* 0xC0: 320x240 ortho */
    Mtx orthoHi;  /* 0x100: 1280x960 ortho */
    Mtx view;     /* 0x140: the camera (guLookAtReflect; the projection is persp * view) */
    Mtx cullView; /* 0x180: the visibility test's view (guLookAt, the eye pulled in) */
    Mtx identity; /* 0x1C0: the modelview */
    Mtx unk200;
    Mtx chopperShadowTrans; /* seg2 Mtx 9 */
    Mtx chopperShadowRot;   /* seg2 Mtx 10 */
    Mtx mtxPool[0x49]; /* [i]: vehicle shadow i (seg2 Mtx 11 + i); later slots: obj_draw_all, commpt_draw, other views */
    Mtx failSpinVehMtx;    /* the failing vehicle's spin (seg2 Mtx 0x54) */
    Mtx failSpinShadowMtx; /* its shadow (seg2 Mtx 0x55) */
    u8 pad1580[0x15C0 - 0x1580];
    Vtx shadowVtx[0x30]; /* vehicle shadow quads, 4 per shadow */
    Vtx chopperShadowVtx[4]; /* the chopper's shadow quad */
    u8 pad1900[0x3C00 - 0x1900];
    LookAt lookAt; /* 0x3C00: the camera's (reflection mapping) */
    u8 pad3C20[0x48B0 - 0x3C20];
    Gfx dl[0xB5E]; /* TOPLEVEL_DL_SIZE */
    u8 subDls[0x21498 - 0xA3A0]; /* sub-display lists (lvl_build_visible_dls, obj_draw_all, commpt_draw) */
} DynamicBuf;

/* The front end's view of the same per-frame buffer (DynamicBuf). The
 * player-select screen (01C40.c, PlayerSelDyn) and the globe (11530.c,
 * Dynamic) use layouts of their own. */
typedef struct FeDyn {
    u8 pad0[0x140];
    Mtx view; /* lookat view (as DynamicBuf.view) */
    u8 pad180[0x240 - 0x180];
    Mtx commPointPersp; /* perspective (0DE70.c) */
    u8 pad280[0x1240 - 0x280];
    Mtx persp; /* perspective */
    Mtx sceneTranslate;
    Mtx sceneRotate;
    Mtx sceneScale;
    u8 pad1340[0x1E00 - 0x1340];
    Vtx arrowVtx[0x1E0]; /* arrow quads */
    LookAt lookAt;
    u8 pad3C20[0x48B0 - 0x3C20];
    Gfx dl[0xB5E]; /* top-level display list */
    u8 unkA3A0[0x21498 - 0xA3A0];
} FeDyn;

/* 0x18-byte animation channel (the -1-terminated texture-animation table
 * gTexAnims and the 32-channel blocks built by anim_init_model; 56040.c).
 * The setters (anim_play .. anim_set_interp) show what the fields hold.  A
 * vehicle's 32 channels (gXxxAnim) sit right before its VehState. */
typedef struct AnimChannel {
    /* 0x00 */ s32 id;        /* the track (or texture animation) data; -1 ends gTexAnims */
    /* 0x04 */ f32 frac;      /* fraction within the current keyframe */
    /* 0x08 */ f32 tension;   /* cardinal-spline tension (anim_set_interp) */
    /* 0x0C */ s16 loopCount; /* loops completed (anim_play and anim_set_loops reset it) */
    /* 0x0E */ s16 loopLimit; /* stop after this many loops; -1 = forever */
    /* 0x10 */ s8 playing;
    /* 0x11 */ s8 reverse;    /* 1 = playing backward (ping-pong flips it; anim_set_reverse) */
    /* 0x12 */ s8 loopMode;   /* 0 wrap, 1 ping-pong (anim_set_loop_mode) */
    /* 0x13 */ s8 frame;      /* the current keyframe */
    /* 0x14 */ u8 speed;      /* step multiplier, 100 = normal (anim_set_speed) */
    /* 0x15 */ u8 interp;     /* interpolation: 0 linear, 1 spline (anim_set_interp) */
    /* 0x16 */ u8 pad16[2];
} AnimChannel; /* size 0x18 */

/* A vehicle's state block: the 0xA6 bytes every vehicle update reads (the
 * asm reaches it through $gp; gDriverState, gSideswipeState ... gCarrierState).
 * veh_serialize_state / veh_snapshot copy exactly 0xA6 bytes.  Each type's
 * block sits between its AnimChannel[32] (gXxxAnim = state - 0x300) and a
 * per-type tail (x, y, z at +0xA8 for most types; PortVehPos, regs.h).  The
 * code reads it through per-file byte-offset macros (VEH_S16(v, VS_SPEED),
 * VEH5(VS_HEADING, u16) ...), with the VS_ offsets below; several fields
 * are read with either signedness (headings s16/u16; 0x96..0x9A, 0xA5 s8/u8),
 * which the macros keep as each site has it. */
typedef struct VehState {
    /* 0x00 */ f32 slope;              /* (slot 0's rise this frame) / speed (veh_slope) */
    /* 0x04 */ s32 slotY[3][3];        /* per wheel slot: the height now, last frame, the frame before */
    /* 0x28 */ s32 slotLaunchVel[3];   /* airborne slot: the vertical step when it left the ground */
    /* 0x34 */ s32 slotLaunchY[3];     /*   the height it left at */
    /* 0x40 */ s32 slotAirTime[3];     /*   frames airborne (from 2) */
    /* 0x4C */ u16 heading;            /* facing, 12-bit (the model's rotation) */
    /* 0x4E */ u16 moveHeading;        /* direction of travel this frame (veh_turn_heading's output) */
    /* 0x50 */ u8 surface;             /* average surface class under the slots (speed bands, turn rate) */
    /* 0x51 */ u8 pad51;
    /* 0x52 */ s16 wheelOffsets[3][2]; /* {x, z} of the three wheel slots, rotated by heading */
    /* 0x5E */ s16 standOffsets[3][2]; /* {x, z} of the stand-on probe points (veh_update_stand_on) */
    /* 0x6A */ s16 deckPos[2];         /* riding a moving deck: deck-local (u, w) of the position */
    /* 0x6E */ u16 deckHeading;        /*   the heading when attached */
    /* 0x70 */ s16 deckFront[2];       /*   deck-local (u, w) of wheel slot 0 */
    /* 0x74 */ u16 momentumHeading;    /* turns toward heading; moveHeading follows it while skidding */
    /* 0x76 */ s16 speed;
    /* 0x78 */ s16 bands[5][3];        /* {lo, hi, accel} speed bands; [0][0] reverse, [4][1] forward top */
    /* 0x96 */ u8 slotAirborne[3];
    /* 0x99 */ s8 skidding;            /* momentumHeading lags the facing by >= 0x156 */
    /* 0x9A */ u8 spawnUpdate;         /* set during the update run by *_spawn */
    /* 0x9B */ u8 onMovingPart;        /* a slot rests on a moving level part (-> VehRecord.onMovingPart) */
    /* 0x9C */ u8 hitPartDown;         /* obj_collide_vehicle's gObjHitPartDown */
    /* 0x9D */ u8 hitObject;           /*   gObjHit */
    /* 0x9E */ u8 hitImmunePart;       /*   gObjHitImmunePart */
    /* 0x9F */ u8 surfaceMax;          /* highest surface class under the slots (gSurfaceMax; 100 = water) */
    /* 0xA0 */ u8 pushDivisor;         /* -> gPushDivisor (veh_collision_setup, veh_prepare_escape) */
    /* 0xA1 */ u8 action;              /* per-type action state (J-Bomb mode, Thunderfist roll, crane L/R) */
    /* 0xA2 */ u8 actionAux;           /* per-type secondary state */
    /* 0xA3 */ u8 moving;              /* walkers (Thunderfist, Cyclone): moving last frame */
    /* 0xA4 */ u8 unkA4;               /* only cleared (veh_reset_state) */
    /* 0xA5 */ s8 lastDir;             /* the speed's sign at the last hit (1 / -1) */
} VehState; /* 0xA6 bytes used; sizeof 0xA8 */
#define VS_SLOPE 0x00
#define VS_SLOT_Y 0x04
#define VS_SLOT_LAUNCH_VEL 0x28
#define VS_SLOT_LAUNCH_Y 0x34
#define VS_SLOT_AIR_TIME 0x40
#define VS_HEADING 0x4C
#define VS_MOVE_HEADING 0x4E
#define VS_SURFACE 0x50
#define VS_WHEEL_OFFSETS 0x52
#define VS_STAND_OFFSETS 0x5E
#define VS_DECK_POS 0x6A
#define VS_DECK_HEADING 0x6E
#define VS_DECK_FRONT 0x70
#define VS_MOMENTUM_HEADING 0x74
#define VS_SPEED 0x76
#define VS_BANDS 0x78
#define VS_SLOT_AIRBORNE 0x96
#define VS_SKIDDING 0x99
#define VS_SPAWN_UPDATE 0x9A
#define VS_ON_MOVING_PART 0x9B
#define VS_HIT_PART_DOWN 0x9C
#define VS_HIT_OBJECT 0x9D
#define VS_HIT_IMMUNE_PART 0x9E
#define VS_SURFACE_MAX 0x9F
#define VS_PUSH_DIVISOR 0xA0
#define VS_ACTION 0xA1
#define VS_ACTION_AUX 0xA2
#define VS_MOVING 0xA3
#define VS_LAST_DIR 0xA5
#define VEH_STATE_SIZE 0xA6

/* The sound player's configuration (snd_init, 1A630.c; the ancestor of
 * GoldenEye's snd.c). */
typedef struct SndConfig {
    u32 maxStates;
    u32 maxEvents;
    s32 maxSounds;
    void * N64P heap;
    u16 slotCount;
} SndConfig;

/* A vehicle's shadow record, 0x1040 bytes (gVehicleShadows / gVehicleShadowsEnd; hd.c and
 * 13A70.c, which renders and draws them). */
typedef struct VehShadow {
    /* 0x0000 */ u8 tex[0x1000];  /* shadow texture, 64x64 IA8 (veh_render_shadow_tex) */
    /* 0x1000 */ f32 renderDist;  /* camera distance of the top-down render */
    /* 0x1004 */ s32 x;           /* position, 1/32 units */
    /* 0x1008 */ s32 floorY;      /* where the shadow is drawn (veh_shadow_floor_y) */
    /* 0x100C */ s32 z;
    /* 0x1010 */ s32 bodyY;       /* the vehicle's own y (the render eye) */
    /* 0x1014 */ s32 groundY;     /* the y given to veh_update_shadow (veh_shadow_src_y) */
    /* 0x1018 */ s16 halfWidth;   /* shadow half-width */
    /* 0x101A */ s16 halfDepth;   /* shadow half-depth */
    /* 0x101C */ s16 rotX;        /* ground tilt (veh_ground_tilt) */
    /* 0x101E */ s16 rotY;        /* the vehicle's heading */
    /* 0x1020 */ s16 rotZ;        /* ground tilt */
    /* 0x1022 */ u8 type;         /* the vehicle type */
    /* 0x1023 */ u8 layer;        /* 0 ground, 1 on an object, 2 water (veh_draw_shadows) */
    /* 0x1024 */ u8 pad1024[0x1C];
} VehShadow; /* 0x1040 bytes */

/* A level result (gScoreOld best, nss this run, gScoreRes / gScoreRs2). */
typedef struct Score {
    s32 ip;
    u32 tc; /* time */
    u8 bd;
    u8 cr;
    u8 coin; /* grade */
    u8 bdn;
    u16 rt;
} Score;

/* One entry per level on the front end's level-select globe (gGlobeLevels,
 * defined in hd_front_end/11530.c). */
typedef struct GlobeLevel {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1;
    /* 0x04 */ char * N64P name;
    /* 0x08 */ u16 * N64P jname; /* hd_code glyph string (0x0FFE-terminated) */
    /* 0x0C */ s32 unkC;
    /* 0x10 */ f32 lat; /* latitude, degrees */
    /* 0x14 */ f32 lon; /* longitude, degrees */
    /* 0x18 */ s8 commRoutes[4]; /* levels opened by comm point j, -1-terminated */
    /* 0x1C */ s8 routes[8]; /* levels opened when this one is done, -1-terminated */
    /* 0x24 */ f32 x; /* globe position (radius 250) from lat/lon */
    /* 0x28 */ f32 y;
    /* 0x2C */ f32 z;
} GlobeLevel; /* size 0x30 */

/* One save slot's progress: players[4] (0x80364AF0), Rare's "playerInfo"
 * (0E7B0.c's sizeof(playerInfo) assert), saved whole to the EEPROM or the
 * Controller Pak (port/src/platform/save.c).  Files that read it differently
 * keep a view (26570.c PlayerHintView: the hint counts by window id; 409D0.c
 * PlayerRec; 45BB0.c PlayerCtlView at +0xF0; 17E10.c: bytes). */
typedef struct PlayerInfo {
    /* 0x00 */ char name[8];          /* 7 characters + NUL; "NEW GAME" when fresh */
    /* 0x08 */ u8 levelno;            /* the level last played (the academy's assert names it) */
    /* 0x09 */ u8 pad9;
    /* 0x0A */ u16 units;             /* stats.c's "units": medal points (gold 3, silver 2, bronze 1), max 360 */
    /* 0x0C */ u8 title;              /* the rank title, index into gRankTitles (units / 12; promotions) */
    /* 0x0D */ u8 padD[3];
    /* 0x10 */ u32 vehiclesUnlocked;  /* bit per vehicle type; 0x1063E when new */
    /* 0x14 */ s32 money;             /* the last mission's money ($) */
    /* 0x18 */ u8 medal[60];          /* per level: 0 locked, 1-4 bronze..platinum, 5 done, 8 open */
    /* 0x54 */ u8 commPts[60];        /* per level: bit n = communication point n taken (opens routes) */
    /* 0x90 */ u8 academy;            /* bit per academy marker found (6) */
    /* 0x91 */ u8 gameState;          /* academy.c's progress state, 0-13 */
    /* 0x92 */ u8 bestVehicle[60];    /* per level: the vehicle of the best result (gVehicleTimeSlot -> gBestTimes slot) */
    /* 0xCE */ u8 hintShows[0x12];    /* times each hint window 0x46-0x57 was shown (yoshi_show_hint) */
    /* 0xE0 */ u8 padE0[0xE];
    /* 0xEE */ u8 demosSeen;          /* bits: the vehicle demo sequences 55, 28, 53, 7, 19 seen */
    /* 0xEF */ u8 padEF;
    /* 0xF0 */ u32 controlModes;      /* bit per vehicle type: the other controller mode chosen */
    /* 0xF4 */ u8 padF4[8];
    /* 0xFC */ u32 crc;               /* the save record's CRC */
} PlayerInfo; /* 0x100 bytes */

/* Per-level setup and objectives: gLevelInfo[60] (0x802E8F94, ROM data;
 * gCurLevelInfo points at the current one).  01C40.c reads the medal times
 * through its own view (LevelTimesView). */
typedef struct LevelInfo {
    /* 0x00 */ u8 kind;               /* gLevelKind: 1 mission, 2 race, 4 targets, 0x20 path buildings,
                                       * 0x40 power-pill maze, 0x80 carrier path, ... */
    /* 0x01 */ u8 gameState;          /* the players[].gameState it opens at / is due at */
    /* 0x02 */ u16 raceArea[4];       /* race track bounds x0, z0, x1, z1 (1/32 units) */
    /* 0x0A */ u16 raceStartBox[4];   /* race start/finish line box */
    /* 0x12 */ u8 raceQuads[4];       /* race: track quadrants in lap order */
    /* 0x16 */ u8 pad16[2];
    /* 0x18 */ u32 target;            /* laps, targets, damage ($) or RDUs to get */
    /* 0x1C */ s16 jbombLiftFadeY;    /* gJbombLiftFadeY >> 5 */
    /* 0x1E */ s16 jbombMaxY;         /* gJbombMaxY >> 5 */
    /* 0x20 */ u8 unk20[4];
    /* 0x24 */ s16 waterY;            /* below it the vehicle fails (levels without water) */
    /* 0x26 */ s16 chopperPos[3];     /* the BCT chopper's start, >> 5 */
    /* 0x2C */ u32 vehicles;          /* bit per vehicle type on the level */
    /* 0x30 */ u16 medalTimes[4];     /* platinum, gold, silver, bronze; [3] is also the time limit (1/10 s) */
    /* 0x38 */ u8 camLift;            /* the camera rises out of the geometry */
    /* 0x39 */ u8 pad39[3];
    /* 0x3C */ s32 camLiftMax;        /* >> 5 */
    /* 0x40 */ u16 camLiftStep;
    /* 0x42 */ u16 camLiftRadius;     /* >> 5 */
} LevelInfo; /* 0x44 bytes */

/* A rank title (gRankTitles, defined in hd_front_end/01C40.c). The u16
 * pointers are hd_code data (glyph strings and portraits). */
typedef struct RankTitle {
    char * N64P title;
    u16 * N64P glyphs;
} RankTitle;

/* Tables the original keeps inside hd_code's .text (bins 7D9D0, 800E0,
 * 8E910); the NON_MATCHING build defines them in C (7F8B0.c, 7FB50.c,
 * 8DDB0.c). Each field has the width its readers use, so the tables read
 * the same on either byte order. */

/* Header of a billboard effect definition (fx_spawn / fx_draw_all,
 * listed at gFxDefsByKind), followed by cols * rows * frames u16 texture ids. */
typedef struct EffectDef {
    /* 0x0 */ s16 id;     /* texture-group id, -1 = none */
    /* 0x2 */ u8 cols;
    /* 0x3 */ u8 rows;
    /* 0x4 */ u16 fmtsiz; /* texture format/size */
    /* 0x6 */ u8 r;
    /* 0x7 */ u8 g;
    /* 0x8 */ u8 b;
    /* 0x9 */ u8 a;
    /* 0xA */ u16 w;
    /* 0xC */ u16 h;
    /* 0xE */ u16 frames;
} EffectDef; /* size 0x10 */

/* A particle emitter definition (43A60.c, list gParticleDefs). */
typedef struct ParticleDef {
    /* 0x00 */ s16 * N64P seqBirth; /* texture ids; [0] doubles as the frame count */
    /* 0x04 */ s16 * N64P seqLoop;
    /* 0x08 */ s16 * N64P seqDeath;
    /* 0x0C */ s32 size;
    /* 0x10 */ s16 speed;
    /* 0x12 */ s16 speedVar;
    /* 0x14 */ u8 life;
    /* 0x15 */ u8 lifeVar;
    /* 0x16 */ u8 texW;
    /* 0x17 */ u8 texH;
    /* 0x18 */ s8 gravity;
    /* 0x19 */ u8 count;  /* particles per burst */
    /* 0x1A */ u8 bursts;
    /* 0x1B */ u8 spread; /* max tilt from vertical, degrees */
    /* 0x1C */ u8 bounce; /* bounce damping divisor */
    /* 0x1D */ u8 r;
    /* 0x1E */ u8 g;
    /* 0x1F */ u8 b;
    /* 0x20 */ u8 a;
    /* 0x21 */ u8 fmt; /* G_IM_FMT_* */
    /* 0x22 */ u8 siz; /* G_IM_SIZ_* */
} ParticleDef; /* size 0x24 */

/* One record of a level's part list (lvl_pick_object_list picks the list by level:
 * s32 count, then count of these). `entry` is filled in at level load
 * (lvl_add_object); obj_parts_destroyed checks the listed parts. */
typedef struct LevelPart {
    /* 0x00 */ void * N64P entry; /* the level object's 0xFC-byte record */
    /* 0x04 */ s32 n;       /* number of part indices used */
    /* 0x08 */ u8 idx[16];  /* indices into the record's +0xEC bytes */
} LevelPart; /* size 0x18 */

/* Layout checks: these records are read from the N64 image and by hand-written asm at fixed
 * offsets, so their sizes must stay the original ones in every build (a mismatch makes the
 * array size negative, a compile error).  No code comes from a typedef. */
#define STATIC_ASSERT_SIZE(T, n) typedef char static_assert_size_##T[(sizeof(T) == (n)) ? 1 : -1]
STATIC_ASSERT_SIZE(DynamicBuf, 0x21498);
STATIC_ASSERT_SIZE(AnimChannel, 0x18);
STATIC_ASSERT_SIZE(VehState, 0xA8);
STATIC_ASSERT_SIZE(VehShadow, 0x1040);
STATIC_ASSERT_SIZE(Score, 0x10);
STATIC_ASSERT_SIZE(GlobeLevel, 0x30);
STATIC_ASSERT_SIZE(PlayerInfo, 0x100);
STATIC_ASSERT_SIZE(LevelInfo, 0x44);
STATIC_ASSERT_SIZE(EffectDef, 0x10);
STATIC_ASSERT_SIZE(ParticleDef, 0x24);
STATIC_ASSERT_SIZE(LevelPart, 0x18);

#endif
