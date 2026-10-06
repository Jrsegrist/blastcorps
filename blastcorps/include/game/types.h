#ifndef GAME_TYPES_H
#define GAME_TYPES_H

/*
 * Game structures used by more than one file.
 *
 * Several are partial: fields nobody has identified yet are padding, and a
 * file whose code reads the same memory differently keeps its own view type
 * (documented where it is defined).
 */

/* One per frame buffer, 0x21498 bytes (D_803156F8[2]): the in-game layout
 * that hd.c builds each frame. The area below the display list is scratch
 * space that other subsystems lay out their own way (their views: VtxBuf
 * 13A70.c, DynBuf 20460.c, SpriteVtxBuf 30C70.c, TrailBuf 37530.c,
 * Dyn48D00, Dyn4B5E0, Dyn 4DA80.c), and the front end uses FeDyn. */
typedef struct DynamicBuf {
    u8 pad0[0x80];
    Mtx unk80;  /* perspective */
    Mtx unkC0;  /* 320x240 ortho */
    Mtx unk100; /* 1280x960 ortho */
    Mtx unk140;
    Mtx unk180;
    Mtx unk1C0; /* identity */
    Mtx unk200;
    Mtx unk240;
    Mtx unk280;
    Mtx unk2C0[0x49]; /* one per vehicle */
    Mtx unk1500;
    Mtx unk1540;
    u8 pad1580[0x15C0 - 0x1580];
    Vtx unk15C0[0x30]; /* vehicle shadow quads */
    Vtx unk18C0[4]; /* shadow quad */
    u8 pad1900[0x3C00 - 0x1900];
    LookAt unk3C00;
    u8 pad3C20[0x48B0 - 0x3C20];
    Gfx dl[0xB5E]; /* TOPLEVEL_DL_SIZE */
    u8 unkA3A0[0x21498 - 0xA3A0];
} DynamicBuf;

/* The front end's view of the same per-frame buffer (DynamicBuf). The
 * player-select screen (01C40.c, PlayerSelDyn) and the globe (11530.c,
 * Dynamic) use layouts of their own. */
typedef struct FeDyn {
    u8 pad0[0x140];
    Mtx unk140; /* lookat view */
    u8 pad180[0x240 - 0x180];
    Mtx unk240; /* perspective (0DE70.c) */
    u8 pad280[0x1240 - 0x280];
    Mtx unk1240; /* perspective */
    Mtx unk1280;
    Mtx unk12C0;
    Mtx unk1300; /* scene scale */
    u8 pad1340[0x1E00 - 0x1340];
    Vtx unk1E00[0x1E0]; /* arrow quads */
    LookAt unk3C00;
    u8 pad3C20[0x48B0 - 0x3C20];
    Gfx dl[0xB5E]; /* top-level display list */
    u8 unkA3A0[0x21498 - 0xA3A0];
} FeDyn;

/* 0x18-byte animation-channel entry (the -1-terminated table D_803B35F8 and
 * the 0x20-entry blocks built by func_8029F85C; 56040.c). */
typedef struct Unk8029DEA0Entry {
    /* 0x00 */ s32 id;
    /* 0x04 */ f32 unk4;
    /* 0x08 */ f32 unk8;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ s8 unk10;
    /* 0x11 */ s8 unk11;
    /* 0x12 */ s8 unk12;
    /* 0x13 */ s8 unk13;
    /* 0x14 */ u8 unk14;
    /* 0x15 */ u8 unk15;
    /* 0x16 */ u8 pad16[2];
} Unk8029DEA0Entry; /* size 0x18 */

/* The sound player's configuration (func_8025EDF0, 1A630.c; the ancestor of
 * GoldenEye's snd.c). */
typedef struct SndConfig {
    u32 maxStates;
    u32 maxEvents;
    s32 maxSounds;
    void *heap;
    u16 slotCount;
} SndConfig;

/* A vehicle's shadow record, 0x1040 bytes (D_803643C8 / D_803643CC; hd.c;
 * 13A70.c reads it as its Struct13A70). */
typedef struct Vehicle {
    u8 unk0[0x1000]; /* shadow texture, 64x64 IA8 */
    f32 unk1000;
    s32 unk1004; /* position, 1/32 units */
    s32 unk1008;
    s32 unk100C;
    s32 unk1010;
    u8 pad1014[4];
    s16 unk1018; /* shadow half-width */
    s16 unk101A; /* shadow half-depth */
    s16 unk101C; /* rotation x */
    s16 unk101E; /* rotation y */
    s16 unk1020; /* rotation z */
    u8 unk1022; /* id */
    u8 unk1023; /* draw layer */
    u8 pad1024[0x1C];
} Vehicle; /* 0x1040 bytes */

/* A level result (D_8036EA60 best, D_8036EA70 this run, D_8036EA80/90). */
typedef struct Score {
    s32 ip;
    u32 tc; /* time */
    u8 bd;
    u8 cr;
    u8 coin; /* grade */
    u8 bdn;
    u16 rt;
} Score;

/* One entry per level on the front end's level-select globe (D_8020D810,
 * defined in hd_front_end/11530.c). */
typedef struct GlobeLevel {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1;
    /* 0x04 */ char *name;
    /* 0x08 */ u16 *jname; /* hd_code glyph string (0x0FFE-terminated) */
    /* 0x0C */ s32 unkC;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ f32 unk14;
    /* 0x18 */ s8 unk18[4]; /* level ids, -1-terminated */
    /* 0x1C */ s8 unk1C[8]; /* level ids, -1-terminated */
    /* 0x24 */ f32 unk24;
    /* 0x28 */ f32 unk28;
    /* 0x2C */ f32 unk2C;
} GlobeLevel; /* size 0x30 */

/* A rank title (D_802081C0, defined in hd_front_end/01C40.c). The u16
 * pointers are hd_code data (glyph strings and portraits). */
typedef struct RankTitle {
    char *title;
    u16 *glyphs;
} RankTitle;

/* Tables the original keeps inside hd_code's .text (bins 7D9D0, 800E0,
 * 8E910); the NON_MATCHING build defines them in C (7F8B0.c, 7FB50.c,
 * 8DDB0.c). Each field has the width its readers use, so the tables read
 * the same on either byte order. */

/* Header of a billboard effect definition (func_802A6274 / func_802A64A4,
 * listed at D_802C3FFC), followed by cols * rows * frames u16 texture ids. */
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

/* A particle emitter definition (43A60.c, list D_802C4A20). */
typedef struct ParticleDef {
    /* 0x00 */ s16 *seqBirth; /* texture ids; [0] doubles as the frame count */
    /* 0x04 */ s16 *seqLoop;
    /* 0x08 */ s16 *seqDeath;
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

/* One record of a level's part list (func_802A1EC8 picks the list by level:
 * s32 count, then count of these). `entry` is filled in at level load
 * (func_802A21AC); func_802BD064 checks the listed parts. */
typedef struct LevelPart {
    /* 0x00 */ void *entry; /* the level object's 0xFC-byte record */
    /* 0x04 */ s32 n;       /* number of part indices used */
    /* 0x08 */ u8 idx[16];  /* indices into the record's +0xEC bytes */
} LevelPart; /* size 0x18 */

#endif
