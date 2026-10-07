#ifndef GAME_FUNCTIONS_H
#define GAME_FUNCTIONS_H

/*
 * Prototypes of every game function used outside the file that defines it,
 * with the types of its definition (the NON_MATCHING definition where the
 * matching build still uses the original asm). Generated from the sources
 * (scratchpad types_genhdr.py), grouped by defining file.
 *
 * LEGACY_func_X: a file whose matched code IDO compiled against an older
 * prototype defines LEGACY_func_X and keeps that prototype for the matching
 * build only (see the file's "matching build" block). The NON_MATCHING
 * build always sees these.
 */

/* Types that only the defining files complete. */
struct BcScClient;
struct BcSched;
struct BoxSpawn;
struct Dyn;
struct Dyn48D00;
struct Dyn4B5E0;
struct DynBuf;
struct PlayerSelDyn;
struct SpriteVtxBuf;
struct TrailBuf;
struct Unk802BD99CModel;
struct Unk802C1DD0Entry;
struct Unk803B9890;
struct VtxBuf;

/* hd_code/00000.c */
Gfx *func_8024C404(Gfx *arg0, DynamicBuf *arg1, s32 *arg2);
void func_8024FC2C(Gfx **gfx, u8 lod);
void func_80255DC8(void);
void func_80256A34(s32 arg0);
void func_80257490(s32 *arg0, s32 arg1);
f32 func_802574F0(f32 arg0);
f32 func_80257514(f32 arg0);

/* hd_code/12D80.c */
Gfx *func_80257540(Gfx *gfx);
Gfx *func_802575F4(Gfx *gfx, Vtx *vtx, void *timg, s16 fmtsiz, s32 width, s32 height, s32 zbuf);

/* hd_code/13A70.c */
void func_80258230(u8 id, s32 arg1, s16 arg2, s16 arg3);
void func_802582C4(u8 id, s32 x, s32 y, s32 z, s32 arg4, s32 arg5, s32 arg6, s32 arg7);
s32 func_802584BC(u8 id);
s32 func_80258500(u8 id);
void func_80258544(void *cimg, s32 x, s32 y, s32 z, f32 dist, Gfx *dl, void *seg6, void *seg7);
void func_80258B78(Gfx **gdlp, struct VtxBuf *buf);

/* hd_code/14B30.c */
void func_802592F0(void);
void func_80259450(void);
void func_802595E0(u8 *base, s32 n, s32 size, s32 (*cmp)(void *, void *));
void func_80259BD4(Gfx **gdlp, s32 arg1);
void func_80259C24(Gfx **gdlp, Mtx *arg1);
void func_80259CCC(Gfx * N64P *gfxp, u8 *str, u16 *wstr, u8 align, s32 fit, s32 x, s32 y, s32 w, s32 h, u8 forward, u8 r, u8 g, u8 b, u8 a);
void func_80259DC8(Gfx * N64P *gfxp, u8 *str, u16 *wstr, u8 align, s32 fit, s32 x, s32 y, s32 w, s32 h, u8 forward, u8 r0, u8 g0, u8 b0, u8 a0, u8 r1, u8 g1, u8 b1, u8 a1);

/* hd_code/168B0.c */
void func_8025B070(void);
u8 *func_8025B0B8(u16 arg0);
void func_8025B2B8(void);
s32 func_8025B300(u8 *arg0);
s32 func_8025B370(u16 *arg0);
s32 func_8025B3F0(u8 *a, u8 *b);
s16 func_8025B498(s16 x, u16 scale, u8 *str, s32 arg3);
u8 *func_8025B558(u16 *arg0);

/* hd_code/17210.c */
void func_8025B9D0(s32 arg0, s32 *arg1);
void func_8025BB38(void);
void func_8025BB50(void);
void func_8025BBE8(u16 buttons, s8 x, s8 y);
void func_8025BD98(void);
void func_8025BEF8(void);

/* hd_code/17A70.c */
void func_8025C230(u8 * N64P *arg0, u8 * N64P *arg1, s32 arg2);

/* hd_code/17E10.c */
void func_8025C5D0(void);
Gfx *func_8025C878(Gfx *arg0, s32 arg1, u8 arg2, s32 *arg3);
void func_8025D184(void);
void func_8025E2CC(Gfx **arg0, s32 arg1, s32 arg2);
void func_8025E67C(Gfx **arg0, s32 arg1, u8 arg2);

/* hd_code/1A630.c */
void func_8025EDF0(SndConfig *c);
u8 func_80260634(void *arg0);
void *func_80260650(void *arg0, s16 arg1, void *arg2);
void func_802608C8(void *arg0);
void func_802609D0(void);
void func_802609F0(void);
void func_80260A10(void);
void func_80260A30(u8 arg0);
void func_80260AB8(void *arg0, s16 arg1, s32 arg2);
void func_80260B40(u8 arg0, u16 arg1);

/* hd_code/1C460.c */
void func_80260C20(u8 tune, f32 vol);
void func_80260D7C(f32 vol);
f32 func_80260DF0(void);
void func_80260DFC(void);
void func_80260E2C(void);
void func_80260E80(void);
void func_80260EE0(u8 tune);
void func_8026101C(void);
void func_80261040(void);
void func_80261068(void);
void func_802611F0(void);
void func_80261284(void);
void func_802613C8(void);
void func_80261528(void);
void func_80261570(f32 arg0);
void func_80261588(void);
void func_802619D0(u32 id);
u8 func_80261A44(u64 event);
void func_80261E9C(u64 event);
void func_80261FB0(u8 tune);
void func_80262008(u8 tune, f32 vol);
s32 func_8026205C(s32 arg0);

/* hd_code/1D990.c */
void func_80262150(u8 arg0);
void func_802621DC(u8 arg0);
void func_80262238(u8 arg0);
void func_80262320(u8 arg0);
void func_80262BF4(void);
Gfx *func_802639B4(Gfx *gdl, Gfx * N64P *arg1, u32 *arg2);
void func_8026420C(void);
void func_80264A34(char *buf, u16 t, s32 arg2);
void func_80264AEC(void);
u8 func_80264BA4(u8 arg0);

/* hd_code/20460.c */
void func_80264C20(s32 arg0);
void func_80264CB4(s16 arg0, s16 arg1, s16 arg2, s16 arg3, u8 arg4, s32 arg5);
void func_8026510C(void);
void func_802661EC(void);
void func_80266248(Gfx **gdlp, struct DynBuf *buf);

/* hd_code/22EE0.c */
void func_802676A0(ALSynConfig *c, OSPri pri);
void func_80267A74(void);

/* hd_code/23C20.c */
void func_802683E0(void);
void func_80268664(s32 id);
void func_802688C4(s32 path);
s32 func_80268EE8(s32 id);
void func_80268F54(void);
void func_80269258(void);
void func_8026A2E8(f32 ref, f32 *angle);
void func_8026A378(s32 n, u8 *buf);
void func_8026A454(s16 x, s16 y, s16 z, s16 rx, s16 ry, Mtx *m);
void func_8026A5CC(u64 *dst, u64 *src, s32 size);
s32 func_8026A610(s32 x1, s32 y1, s32 x2, s32 y2);
s32 func_8026A6F0(s32 x1, s32 y1, s32 z1, s32 x2, s32 y2, s32 z2);
s32 func_8026A828(s32 lo, s32 hi);
void func_8026A8BC(void);
s32 func_8026A8E0(s32 lo, s32 hi);
void func_8026A974(void);
void func_8026A988(void);
void func_8026A9B4(void);

/* hd_code/26570.c */
u8 func_8026AD30(s16 arg0);
void func_8026AF6C(u16 yd);
u16 func_8026B10C(void);
void func_8026B118(u8 arg0);
void func_8026B8F8(void);
#ifndef LEGACY_func_8026BBD0
Gfx *func_8026BBD0(Gfx *gfx, s32 arg1, s32 *count);
#endif
s32 func_8026F92C(u64 in);
u8 func_8026FA38(char * N64P *name, s32 *arg1);
void func_8026FBB0(s16 *pos, s16 *end);
u8 func_8026FE6C(s32 arg0);
void func_8026FE8C(s32 arg0);
void func_8026FEC4(void);
void func_802701A8(Gfx **gfx, s32 arg1);
void func_80270AE0(u8 *cmdline);

/* hd_code/2C560.c */
void func_80270D20(struct BcSched *sc, void *stack, OSPri priority, u8 mode, u8 numFields);
void func_80270E50(struct BcSched *sc, struct BcScClient *c, OSMesgQueue *msgQ, s32 arg3, s32 arg4);
void func_80270ECC(struct BcSched *sc, struct BcScClient *c);
void *func_80270F74(void *arg0);

/* hd_code/2D810.c */
Gfx *func_80271FD0(Gfx *gfx, u32 arg1, u16 level, s16 arg3, s16 arg4, s32 *height);
void func_802729F0(u16 type, u16 level);

/* hd_code/2E490.c */
void func_80272C50(void);
u8 func_80272C5C(u16 *ids, u16 *palIds, u8 count, u8 frames, u8 flags, f32 scale);
Gfx *func_80272ED8(Gfx *arg0, u8 slot, s16 x, s16 y, u8 alpha, u8 mode, f32 scale);
Gfx *func_80274868(Gfx *gfx);
Gfx *func_80274998(Gfx *gfx);
Gfx *func_80274AA4(Gfx *gfx);
Gfx *func_80274B08(Gfx *gfx);
void func_80274B40(Gfx **gfx, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* hd_code/30430.c */
Gfx *func_80274BF0(s32 arg0, Gfx *gfx);
void func_80275270(u64 next, f32 speed);
void func_80275390(u64 next);
s32 func_802753C0(void);
s32 func_802753F8(void);

/* hd_code/30C70.c */
void func_80275430(void);
void func_80275478(struct SpriteVtxBuf *arg0, Gfx **gfxp, u8 arg2);
Gfx *func_80275DA4(Gfx *gfx, u8 arg1);
s32 func_80276080(struct SpriteVtxBuf *arg0, u8 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, u8 r, u8 g, u8 b, u8 a);
s32 func_80276130(struct SpriteVtxBuf *arg0, u8 arg1, s32 arg2, s32 x, s32 y, s32 w, s32 h, u8 r0, u8 g0, u8 b0, u8 a0, u8 r1, u8 g1, u8 b1, u8 a1, u8 r2, u8 g2, u8 b2, u8 a2, u8 r3, u8 g3, u8 b3, u8 a3);
void func_8027690C(void *arg0, f32 x, f32 y, f32 z, s16 *sx, s16 *sy, Mtx *arg6, Mtx *arg7, Mtx *arg8, f32 arg9);
void func_80276E50(Gfx **gfxp, void *view, u8 idx, s32 x, s32 y, s32 z);

/* hd_code/32E00.c */
void func_802775C0(void);
void func_80277620(s32 now);
#ifdef NON_MATCHING
void func_80277EDC(u8 type, u8 arg1, s32 arg2, u8 sound);
#else
void func_80277EDC(); /* K&R definition in the matching build */
#endif
void func_80278318(void);
void func_80278324(Gfx **gfx, s32 arg1, u8 buf);

/* hd_code/34430.c */
void func_80278BF0(Gfx *src, Gfx *end, Gfx * N64P *dstp);
void func_80278E3C(void);
void func_80278EB0(s32 n, f32 scale, s32 arg2);
void func_802794A4(void);
void func_802794E4(void);
u8 func_802794F0(void);
void func_80279514(s32 x, s32 y, s32 z, s32 a, s32 b, s32 c);
void func_80279778(s32 x, s32 y, s32 z, s32 a, s32 b, s32 c, void *dl, void *seg6, void *seg7, s32 alpha);
void func_80279EE8(Gfx **gfxp, s32 arg1, u8 slot);

/* hd_code/37530.c */
void func_8027BE4C(void);
void func_8027BE7C(u8 period, s32 y, s16 x1, s16 z1, s16 x2, s16 z2, s32 x, s32 z, s16 yaw, u8 halfw, u8 a, u8 b, u8 d);
void func_8027C4C8(Gfx **gfxp, struct TrailBuf *buf);

/* hd_code/39050.c */
void func_8027D810(s32 arg0);
void func_8027E344(s32 id);
void func_8027E9B8(u8 buf);
s32 func_8027EED8(s16 x, s16 z, s16 *y);
void func_8027F1F8(Gfx **gfx, u8 buf, u8 layer);
void func_802807D8(u8 id);
void func_80280F34(Gfx **gfx, u8 buf);
void func_80281A70(s32 arg0);
void func_80281CE4(void);
void func_80281E44(Gfx **gfx);
void func_802821D0(void);
void func_80282224(Gfx **gfx, u8 arg1);
void func_80282728(void);
void func_8028273C(Gfx **gfx, u8 arg1);

/* hd_code/3E4C0.c */
void func_80282C80(Gfx **gfxp, Mtx *mtx, s32 x, s32 y, s32 z, s32 x2, s32 y2, s32 z2);
void func_8028376C(Gfx **gfxp, Mtx *mtx, u8 idx, s32 x, s32 z, s32 x2, s32 z2);

/* hd_code/405F0.c */
void func_80284DB0(void);
void func_80284E54(u64 *data, s32 count, u8 slot, u8 yield, s32 id, u8 writebackAll);
void func_80285110(s32 id);

/* hd_code/409D0.c */
void func_80285190(void);
u32 func_802852EC(void);
u8 func_80285814(void);
void func_80285A78(u8 *src, u8 *dst);
void func_80285AB0(u8 bit);
s32 func_80285B10(u8 bit);
void func_80285B68(s32 arg0);
void func_80285CA0(void);
void func_80285CC0(void);
void func_80285EF4(s32 start);
s32 func_80286038(u16 arg0);
u16 func_8028604C(u32 frames);
u8 func_80286090(s32 level);

/* hd_code/41930.c */
void func_802860F0(void);
void func_802862DC(void);
void func_80286330(void);
u8 func_8028653C(void);

/* hd_code/42240.c */
void func_80286A00(void);
void func_80286C60(Gfx **gfxp, s32 arg1, u8 frame, u8 arg3);
void func_802873AC(void);
void func_80287530(Gfx **gfxp, Gfx * N64P *gfxp2, u8 frame, u8 arg3);
void func_80287AE4(void);
void func_80287C68(Gfx **gfxp, Gfx * N64P *gfxp2, u8 frame, u8 arg3);

/* hd_code/43A60.c */
void func_80288220(void);
s32 func_80288284(u8 type, s32 x, s32 y, s32 z, s32 floor);
void func_802886A0(void);
void func_80288DF0(Gfx **gdl, u8 buf);

/* hd_code/45BB0.c */
u8 func_8028A370(void);
void func_8028A3E4(void);
void func_8028A42C(void);
void func_8028A470(void);
void func_8028AE88(void);
void func_8028B240(void);

/* hd_code/46C20.c */
void func_8028B3E0(void);
void func_8028B4C4(u32 devAddr, u32 dest, u32 *size, u8 arg3, u8 arg4, u8 arg5);

/* hd_code/46F60.c */
void func_8028B720(void);
void func_8028B734(s8 *arg0, s8 *arg1, u8 arg2);
f32 func_8028BBF4(s16 arg0, s16 arg1, s16 arg2, s16 arg3);

/* hd_code/479D0.c */
void func_8028C190(struct BoxSpawn *arg0, struct BoxSpawn *arg1);
void func_8028C874(u8 arg0);
void func_8028CB30(Gfx **arg0, s32 arg1);

/* hd_code/48D00.c */
void func_8028D4C0(u8 *arg0, u8 *arg1);
void func_8028DF14(u8 arg0);
void func_8028E9E4(Gfx **gdl, struct Dyn48D00 *dyn);
void func_8028F6B4(u8 arg0);
void func_8028F794(u8 arg0);
void func_8028F93C(void);
void func_8028F994(s32 arg0, s32 arg1, s32 arg2);
void func_8028FAC0(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8028FC10(void);
u8 func_8028FCD4(void *arg0, u8 *arg1);

/* hd_code/4B5E0.c */
#ifdef NON_MATCHING
void func_8028FDA0(u8 *arg0, u8 *arg1, s32 s1);
#else
void func_8028FDA0(u8 *arg0, u8 *arg1);
#endif
void func_802906C0(u8 arg0);
void func_802917B0(Gfx **gdl, struct Dyn4B5E0 *dyn);
void func_80291ED8(u8 arg0);
void func_80291FAC(u8 arg0);
void func_80292084(void);
void func_802920DC(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* hd_code/4DA80.c */
void func_80292240(void);
s32 func_80292288(s16 speed, s32 x0, s32 y0, s32 z0, s32 x1, s32 y1, s32 z1, u8 type, s16 arg8);
void func_80292830(void);
void func_80292EB8(Gfx **gdl, struct Dyn *dyn);

/* hd_code/4EBE0.c */
void func_802933A0(s32 x, s32 y, s32 z, s32 type, Mtx *mtx, void *state, Gfx *gfx1, Gfx *gfx2, s32 idx, s32 arg9, s32 arg10, s32 arg11);

/* hd_code/50670.c */
void func_80294E30(void);
void func_80294E88(void);
void func_80294EB8(void);
void func_80294F00(void);
void func_80295120(Gfx **gfxp, Mtx *mtx);
void func_80295A20(u32 arg0);
void func_80295AE0(Gfx *gdl, Gfx *end);
void func_80295C70(u8 id, s32 px, s32 pz);

/* hd_code/51690.c */
void func_80295E50(void);
Gfx *func_80295EFC(s32 arg0, Gfx *arg1, s16 x, s16 y, u8 alpha);

/* hd_code/52D70.c */
#ifdef NON_MATCHING
void func_80297530(u8 arg0);
#else
void func_80297530(); /* K&R definition in the matching build */
#endif
u8 func_8029766C(u8 arg0, u8 *arg1);
void func_802976E8(Gfx **arg0);
void func_80297804(s32 x, s32 y, s32 z);
void func_80297960(void);

/* hd_code/53220.c */
void func_802979E0(u8 level);
void func_80297ECC(void);
u8 func_80297EF8(u8 level);
s32 func_802994F8(void);

/* hd_code/54E30.c */
void func_802995F0(s32 seq);
void func_80299C0C(void);
void func_80299C20(void);
void func_80299E10(s32 arg0);
u64 func_80299FE8(u8 level);

/* hd_code/55970.c */
void func_8029A130(void);
Gfx *func_8029A1A8(s32 arg0, Gfx *arg1);

/* hd_code/55D40.c */
void func_8029A500(void);
Gfx *func_8029A518(Gfx * N64P *gfxp, Gfx *gfx);

/* hd_code/56010.c */
void func_8029A7E4(const char *fmt, ...);

/* hd_code/56040.c */
void func_8029A800(s32 z, s32 a1, s32 b2, s32 b3, s32 x, s32 y, s32 b0, s32 h1, s32 h2, s32 b4, s32 b8, u8 *veh);
void func_8029A914(u8 *veh);
void func_8029AA10(s32 kind);
void func_8029B02C(s32 x, s32 y, s32 z, s32 r, s32 kind, s32 id);
void func_8029B7CC(s32 a, s32 b);
s32 func_8029B930(void);
s32 func_8029BD0C(s32 x, s32 y, s32 z, s32 r, u8 *tri);
s32 func_8029BEE4(s32 x, s32 y, s32 z, s32 r, u8 *tri);
s32 func_8029BF64(s32 bu, s32 bv, s32 cu, s32 cv, s32 au, s32 av, s32 pu, s32 pv);
void func_8029C0DC(u8 *tri, s32 px, s32 py, s32 pz, s32 *out);
s32 func_8029C160(s32 x, s32 y, s32 z, s32 r, u8 *tri, s32 *hit);
void func_8029C354(s32 tag, u8 *p, u8 *end, u32 scale);
void func_8029C454(s32 x, s32 y, s32 z, s32 tag, u8 *p, u8 *end, u8 *base, MtxChainRegs *regs);
void func_8029C52C(s32 tag, u8 *veh);
s32 func_8029C5EC(s32 s0, s32 s1, s32 s2, s32 s3);
void func_8029C6E4(Unk8029C6E4Out *o);
s32 func_8029CFA4(s32 pz, s32 r1, s32 qx, s32 qy, s32 px, s32 py, s32 qz, s32 r2);
void func_8029D040(s32 val, Unk8029DEA0Entry *tbl, s32 x, s32 z, s32 id, u8 *model, u8 *mtxBase);
s32 func_8029D210(s32 sub);
s32 func_8029DBF0(s32 id);
s32 func_8029DC14(s32 id);
void func_8029DC80(void);
void func_8029DDC8(void);
void func_8029DEA0(void);
void func_8029DF78(u8 *dl, u8 *dlEnd, s32 key);
#ifdef NON_MATCHING
void func_8029E0AC(u8 *param);
#else
void func_8029E0AC(); /* hd.c passes nothing: the asm reads leaked registers */
#endif
void func_8029E558(u8 *base, u8 *other, Unk8029DEA0Entry *ch);
s32 func_8029F85C(u32 *bufA, u32 *bufB, Unk8029DEA0Entry *ch, u8 *hdr);
void func_8029F9D4(s32 a, s32 b, Unk8029DEA0Entry *base);
void func_8029FC74(s32 a, s32 b, Unk8029DEA0Entry *base);
void func_802A0290(Unk8029DEA0Entry *base, s32 idx, s32 val);
void func_802A02E4(s32 idx, Unk8029DEA0Entry *base);
void func_802A0320(s32 idx, Unk8029DEA0Entry *base);
void func_802A0360(f32 f, Unk8029DEA0Entry *base, s32 idx, s32 val);
void func_802A039C(Unk8029DEA0Entry *base, s32 idx, s32 val);
void func_802A03D4(Unk8029DEA0Entry *base, s32 idx, s32 val);
void func_802A040C(Unk8029DEA0Entry *base, s32 idx, s32 val);
void func_802A0480(f32 f, Unk8029DEA0Entry *base, s32 idx, s32 val);
void func_802A04BC(s32 idx, Unk8029DEA0Entry *base, s32 *out);
void func_802A0508(s32 key, s32 val);
void func_802A05A4(f32 f, s32 key, s32 val);
void func_802A05D0(s32 key, s32 val);
void func_802A05F8(s32 key, s32 val);
void func_802A0620(s32 key, s32 val);

/* hd_code/5BF40.c */
void func_802A0700(void);
void func_802A08B4(u32 *dl, u32 *end);
void func_802A08E4(u32 *dl, u32 *end, Unk802A08E4Regs *r);
void func_802A0B00(s32 id, u8 *param);
u32 func_802A0CC8(s32 id, u8 *param);
u32 func_802A0CFC(s32 id, u8 *param);
void func_802A0EE0(s32 id, void *dest);
void func_802A1040(s32 id, u8 *dest, u8 *param);
void func_802A1074(s32 id, u8 *dest, u8 *param);
void func_802A11C4(s32 id, void *dest);

/* hd_code/5CB60.c */
u32 func_802A1320(void);
void func_802A133C(s32 a0Val, s32 id, s32 v0Val, s32 v1Val, u8 *obj);
void func_802A1388(s32 a0Val, s32 a1Val, s32 v0Val, s32 v1Val, u8 *hdr);
void func_802A1674(u8 *obj, s32 arg1);
void func_802A396C(s32 type, Out802A396C *out);
u8 *func_802A41B0(u8 *rec, u8 *v, s32 id, s32 h52, s32 b57, s32 b56, s32 *s1io, s32 b4F, s32 b55);

/* hd_code/5FD50.c */
void func_802A4510(void);
void func_802A45D4(s32 arg0);
void func_802A467C(s32 arg0, Gfx *arg1, Vtx *arg2, s32 arg3);
void func_802A4CDC(u32 *dl0, u32 *dl1, u32 *dl2, u32 *dl3, u32 *gfx);

/* hd_code/60D50.c */
void func_802A5510(u8 *level);
void func_802A5604(u8 *level);
s32 func_802A56C4(void);

/* hd_code/60F60.c */
void func_802A5720(void);
void func_802A5764(s32 a, s32 b, s32 c, s32 d);
void func_802A57AC(void);
s32 func_802A57DC(u8 *rec);
void func_802A5E60(void);
s32 func_802A5ED0(void);
void func_802A5F30(void);
void func_802A5FA8(void);
s32 func_802A6274(Io802A6274 *io, u8 *def, s32 data, s32 type, s32 x, s32 y, s32 z, s32 w24, s32 w28, s32 w18, s32 w1C, s32 w2C, s32 b35);
void func_802A64A4(void);

/* hd_code/62740.c */
void func_802A6F00(u8 *veh);
s32 func_802A6F6C(void);
void func_802A6FE4(u8 *veh, s32 limit);
void func_802A7070(u8 *veh, s16 *angle);
void func_802A70D8(u8 *veh);
s32 func_802A71DC(u8 *veh, s32 cur, s32 target, s32 *curOut, f32 scale);
s32 func_802A746C(u8 *veh, s32 delta, s32 v1, s32 *targetOut);
void func_802A754C(u8 *veh);
void func_802A75DC(u8 *veh, u8 *src, s32 *w0, s32 *w1, s32 *w2);
u64 *func_802A768C(u8 *veh, u8 *dst, s32 *w0, s32 *w1, s32 *w2, u64 *src, u64 *dst2, s32 size, u64 **dst2End);
void func_802A7764(u64 *a, u64 *b, s32 size);
void func_802A77D0(u8 *veh);
void func_802A7834(s32 rate, u16 *angle, u8 *veh, s16 *speed, s32 mode, u8 *flags, s16 *bands, s32 delta);
void func_802A785C(u8 *veh, s16 *speed, s32 mode, u8 *flags, s16 *bands, s32 delta);
s32 func_802A7CB0(u8 *veh, s32 range);
void func_802A7E70(s32 rate, u16 *angle);
void func_802A7FD8(u8 *veh, u16 *heading, s32 rate, s16 *speedp, u16 *target, u16 *out, s8 *flag, s32 sound);
f32 func_802A83B8(s16 *div, u8 *f, s32 *p, f32 *out);
void func_802A843C(u8 *veh, s16 *speed, s32 kind, s8 *f, s32 *p, s32 clamp, f32 div);
s32 func_802A860C(f32 f, s32 angle, s16 *len, s32 *px, s32 *pz, Out802A860C *out);
void func_802A8768(u8 *veh, s32 id, s32 *px, s32 *py, s32 *pz, s32 x, s32 z, s32 divB, s32 divA, s16 *angle, u8 *flags, s16 *tbl, s32 *a, s32 *b, s32 *c, s32 *ys, Regs802A8768 *r, TriSideOut *f);
s32 func_802A92C8(s32 x, s32 z, s16 *tbl, s16 *angle, s32 *ys, s32 key, u8 *veh, s32 fpIn, TriSideOut *f);
s32 func_802A94A4(s32 index, s16 *tbl, s16 *angle, s32 *dz);
s32 *func_802A992C(s16 *tbl, s32 y, s32 x, s32 z, s32 *dst, s32 *mid, s16 *angle, s32 key, s32 fpIn, u8 *veh, TriSideOut *f);
s32 func_802A9A60(s16 *tbl, s32 y, s32 x, s32 z, s32 *dst, s32 *mid, s16 *angle, s32 key, s32 fp, u8 *veh, TriSideOut *f, Out802A9A60 *out);
s32 func_802A9B1C(s32 index, s32 x, s32 z, s32 y, s32 skip, u8 *veh, s32 fpIn, TriSideOut *f);
s32 func_802A9DC0(s32 x, s32 z, s32 y, TriSideOut *f, s32 *fpOut);
s32 func_802A9F24(s32 x, s32 z, s32 y, s32 skip, TriSideOut *f, s32 *idOut, s32 *a2Out);
s32 func_802AA094(s32 x, s32 z, s32 y, TriSideOut *f, s32 *t3io, s32 *fpio);
s32 func_802AA460(s32 x, s32 z, s32 x0, s32 z0, s32 x1, s32 z1, s32 x2, s32 z2, TriSideOut *out);
s32 func_802AA5E0(s32 x, s32 z, s32 x0, s32 z0, s32 x1, s32 z1, s32 x2, s32 z2);
void func_802AA6D0(s32 x, s32 y, s32 z, s16 rx, s16 ry, s16 rz, s32 scale, Mtx *m);
void func_802AA764(s32 x, s32 y, s32 z, s32 scale, s32 *m);
void func_802AA838(u8 *src, u8 *dst, s32 off);
s32 func_802AA890(s32 count, s32 *offsets, u8 *base, s32 x, s32 y, s32 z, MtxChainRegs *regs);
s32 func_802AABE4(s32 id, u16 *desc, u8 *base, MtxChainRegs *regs, s16 **vertsOut);
void func_802AACD4(s32 id, s32 x, s32 z, s16 *uOut, s16 *wOut);
void func_802AAD0C(s32 id, s32 x, s32 z, InterpRegs *r);
void func_802AAE1C(s32 id, s32 x, s32 z, s32 *uOut, s32 *wOut);
void func_802AAE54(s32 id, s32 x, s32 z, InterpRegs *r);
s32 func_802AB3C0(s32 id);
s32 func_802AB41C(s32 key0, s32 key1);
void func_802AB478(s32 id);
void func_802AB670(s32 id);
s32 func_802AB878(s32 id);
s32 func_802AB9A4(s16 *tbl, s32 x1, u16 *angle, s32 id, s32 z1, s32 x2, s32 z2, s32 *s3Out, InterpRegs *r);
s32 func_802ABB1C(s32 ax, s32 az, s32 dx, s32 dz, s32 bx, s32 bz);
void func_802ABBEC(s32 id, s16 *p, s16 *end, u8 *base, MtxChainRegs *regs);
s32 func_802ABC88(s32 id, s32 n, u8 **recOut);
s32 func_802ABCDC(s32 ax, s32 ay, s32 az, s32 bx, s32 by, s32 bz);
s32 func_802ABD54(s32 id, s32 x, s32 y, s32 z, ZoneScanRegs *r);
s32 func_802ABEDC(s32 x, s32 y, s32 z);
s32 func_802AC0BC(s32 x, s32 z, s32 y, TriSideOut *f, TriScanRegs *r);

/* hd_code/679E0.c */
void func_802AC1A0(s32 radius);
void func_802AC284(s32 *px, s32 *py, s32 *pz);
void func_802AC2A4(s32 z, s32 a1, s32 x, s32 y, s32 b0, s32 id, u8 *vehicle);
s32 func_802AC4C4(s32 px, s32 pz, s32 x0, s32 z0, s32 x1, s32 z1, s32 x2, s32 z2);
void func_802AC544(s32 x, s32 y, s32 z);
void func_802AC61C(s32 x, s32 y, s32 z, s32 kind, s32 data);
void func_802AC6FC(s32 x, s32 y, s32 z, s32 kind, s32 data);
s32 func_802AC7DC(u8 *dst, u8 *src, u32 *words);
void func_802AC85C(u8 *src, u8 *dst, u32 *words);
void func_802AC8CC(u16 *m);
void func_802ACA60(s32 x, s32 y, s32 z, s32 *m);
void func_802ACAC4(s32 angle, s32 *m);
void func_802ACB50(s32 angle, s32 *m);
void func_802ACBDC(s32 angle, s32 *m);
void func_802ACC68(s32 x, s32 y, s32 z, s32 *m);
void func_802ACCCC(s32 *b, s32 *m);
s32 func_802ACE38(s32 x, s32 z, s32 angle, s32 *t1out);
s32 func_802ACF3C(s32 x);
s32 func_802ACF64(u32 x);

/* hd_code/69BB0.c */
void func_802AE370(u8 *hdr, s32 x, s32 y, s32 z, s32 heading, s32 fp, TriSideOut *f);
void func_802AE860(void);
s32 func_802AE888(s32 dist);
void func_802AEEC8(void);
void func_802AFC28(void *src);

/* hd_code/6B4A0.c */
void func_802AFC60(u8 *model, s32 x, s32 y, s32 z, s32 heading, s32 fp);
void func_802AFFD4(void);
s32 func_802B01DC(void);
void func_802B0254(void);
#ifdef NON_MATCHING
void func_802B02A0(s32 fp);
#else
void func_802B02A0(); /* hd.c passes nothing: the asm reads leaked registers */
#endif
#ifdef NON_MATCHING
void func_802B03F4(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4);
#else
void func_802B03F4(); /* hd.c passes nothing: the asm reads leaked registers */
#endif
void func_802B0D70(void *src);

/* hd_code/6C5E0.c */
void func_802B0DA0(u8 *model, s32 x, s32 y, s32 z, s32 heading, s32 fpIn, s32 t6, TriSideOut *f);
s32 func_802B1150(void);
void func_802B11B8(void);
void func_802B1228(void);
#ifdef NON_MATCHING
void func_802B152C(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4, f32 f14, f32 f20, f32 f22, f32 f24, f32 f26);
#else
void func_802B152C(); /* hd.c passes nothing: the asm reads leaked registers */
#endif
void func_802B2988(void *src);

/* hd_code/6E200.c */
void func_802B29C0(u8 *model, s32 x, s32 y, s32 z, s32 heading, s32 fp);
void func_802B2D7C(void);
s32 func_802B2EF8(void);
void func_802B2F54(void);
#ifdef NON_MATCHING
void func_802B2FA0(s32 fp);
#else
void func_802B2FA0(); /* hd.c passes nothing: the asm reads leaked registers */
#endif
s32 func_802B30F4(s32 id, InterpRegs *r);
s32 func_802B3180(s32 id, InterpRegs *r);
#ifdef NON_MATCHING
void func_802B327C(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4);
#else
void func_802B327C(); /* hd.c passes nothing: the asm reads leaked registers */
#endif
void func_802B40D4(void *src);
void func_802B4100(u8 *model, s32 x, s32 y, s32 z, s32 heading, s32 fp);
void func_802B448C(void);
s32 func_802B45FC(void);
void func_802B4658(void);
#ifdef NON_MATCHING
void func_802B46C4(s32 fp);
#else
void func_802B46C4(); /* hd.c passes nothing: the asm reads leaked registers */
#endif
s32 func_802B4818(s32 id, InterpRegs *r);
s32 func_802B48A4(s32 id, InterpRegs *r);
#ifdef NON_MATCHING
void func_802B49AC(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4);
#else
void func_802B49AC(); /* hd.c passes nothing: the asm reads leaked registers */
#endif
void func_802B58C8(void *src);

/* hd_code/71140.c */
void func_802B5900(u8 *model, s32 x, s32 y, s32 z, s32 heading, s32 fp);
void func_802B5CD8(void);
s32 func_802B5F04(void);
void func_802B5F60(void);
void func_802B5FAC(void);
void func_802B6100(s32 id, InterpRegs *r);
s32 func_802B618C(s32 id, TriSideOut *f);
void func_802B6294(void);
void func_802B7308(void *src);

/* hd_code/72B80.c */
void func_802B7340(u8 *hdr, s32 x, s32 y, s32 z, s32 heading, s32 fp, TriSideOut *f);
#ifdef NON_MATCHING
void func_802B76AC(s32 arg0);
#else
void func_802B76AC(); /* hd.c passes nothing: the asm reads leaked registers */
#endif
s32 func_802B76F8(void);
void func_802B7754(void);
void func_802B77A0(void);
void func_802B78F4(s32 id, InterpRegs *r);
void func_802B7980(s32 id, f32 *f24);
void func_802B7A88(void);
void func_802B8480(s32 a1Val, u8 *hdr);
void func_802B8794(void);
void func_802B899C(void);
void func_802B8AE4(void);

/* hd_code/75490.c */
void func_802B9C50(u8 *model, s32 x, s32 z, s32 speed, s32 limit, s32 heading, s32 fp);
void func_802BA148(void);
void func_802BA354(void);
void func_802BAD80(u8 *model, s32 x, s32 y, s32 z, s32 heading, s32 fp);
void func_802BB054(void);
s32 func_802BB170(void);
void func_802BB1A0(void);
void func_802BB274(void);

/* hd_code/772A0.c */
void func_802BBA60(s32 x, s32 y, s32 z, s32 angle, u8 *data, s32 fp);
#ifdef NON_MATCHING
void func_802BBDC8(s32 arg0);
#else
void func_802BBDC8(); /* hd.c passes nothing: the asm reads leaked registers */
#endif
s32 func_802BBE10(void);
void func_802BBE2C(void);
void func_802BBEB8(void);

/* hd_code/77E20.c */
void func_802BC5E0(void);
void func_802BC840(void);
void func_802BCA2C(void);
void func_802BCBD8(void);
void func_802BCC10(void);
void func_802BCCD4(s32 value);
s32 func_802BCD80(s32 value);
s32 func_802BCE40(void);
void func_802BD10C(s32 arg0);
void func_802BD1F8(u32 *gA, u32 *gB, u32 *gC, u32 *gD, s32 mtxA, s32 mtxB, u32 *gE, u32 *gF);
s32 func_802BD8C8(void);
void func_802BD99C(struct Unk802BD99CModel *model, s32 dx, s32 dy, s32 dz);
void func_802BE77C(s32 id, u8 *vehicle);
void func_802BF1F0(struct Unk802C1DD0Entry *e, s32 id);
void func_802BF264(struct Unk803B9890 *t);
void func_802BF384(struct Unk802C1DD0Entry *e);
void func_802BF534(struct Unk802C1DD0Entry *e);
void func_802BF668(struct Unk802C1DD0Entry *e);
void func_802BF898(struct Unk802C1DD0Entry *e, s32 part, s32 level);
void func_802C049C(void);
void func_802C04F0(u32 *src);
void func_802C0574(void);
void func_802C09B8(s32 id, struct Unk802C1DD0Entry *e);
void func_802C0E8C(s32 id, struct Unk802C1DD0Entry *e);
void func_802C1438(struct Unk802C1DD0Entry *e, s32 index);
void func_802C18D4(s32 x, s32 y, s32 z, s32 radius, s32 amount);
s32 func_802C1AA0(void);
#ifndef LEGACY_func_802C1B1C
s32 func_802C1B1C(void);
#endif
u32 func_802C1B9C(void);
void func_802C1DD0(s32 onlyFlagged);
s16 *func_802C1EE0(s32 index);
void func_802C1F30(s32 group, s32 part, s32 x, s32 y, s32 z);
void func_802C2054(void);

/* hd_code/7F8B0.c */
void func_802C4070(u32 *src, u32 *dst, u32 work, u8 type);
void func_802C4108(u8 **src, u8 **dst, s32 work);
void func_802C4310(s32 arg0, s32 arg1);
void func_802C444C(void);
void func_802C4584(s32 level);
void func_802C4724(s32 sfx);

/* hd_code/7FB50.c */
s32 func_802C4A40(u8 *out);
void func_802C4BF0(void *in);
u32 func_802C4E58(void *outp, u8 level);
void func_802C5120(s32 x, s32 y, s32 z, s32 heading, u8 *model, s32 fp);
s32 func_802C5508(void);
void func_802C5688(void);
void func_802C5714(void);
#ifdef NON_MATCHING
void func_802C5860(s32 fp);
#else
void func_802C5860(); /* hd.c passes nothing: the asm reads leaked registers */
#endif
void func_802C59B4(s32 id, InterpRegs *r);
void func_802C5A14(s32 id, s32 *s3, InterpRegs *r);
void func_802C5AFC(void);
void func_802C80A0(void *src);

/* hd_code/83910.c */
void func_802C80D0(s32 type, s32 x, s32 y, s32 z, s32 angle, u8 *data, s32 fp);
#ifdef NON_MATCHING
void func_802C8AB0(s32 arg0);
#else
void func_802C8AB0(); /* hd.c passes nothing: the asm reads leaked registers */
#endif
s32 func_802C8AF0(void);
void func_802C8B0C(s32 type);
void func_802C8BB8(s32 type);

/* hd_code/853D0.c */
void func_802C9B90(s32 x, s32 y, s32 z, s32 heading, u8 *model, s32 fp);
void func_802C9F54(void);
s32 func_802CA140(void);
void func_802CA1AC(void);
void func_802CA34C(s32 id, InterpRegs *r);
void func_802CA3D8(s32 id, s32 *s3, InterpRegs *r);
void func_802CA4E0(void);
void func_802CB660(void *src);

/* hd_code/86ED0.c */
void func_802CB690(u8 *state);

/* hd_code/86F60.c */
void func_802CB720(u8 *hdr, s32 x, s32 y, s32 z, s32 heading, s32 fp, TriSideOut *f);
#ifdef NON_MATCHING
void func_802CBA94(s32 arg0);
#else
void func_802CBA94(); /* hd.c passes nothing: the asm reads leaked registers */
#endif
s32 func_802CBB60(void);
void func_802CBBBC(void);
void func_802CBC08(void);
s32 func_802CBD5C(s32 id, InterpRegs *r);
s32 func_802CBDE8(s32 id, TriSideOut *f);
void func_802CBEF0(void);

/* hd_code/88160.c */
void func_802CC920(u8 *hdr, s32 x, s32 y, s32 z, s32 heading, s32 fp, TriSideOut *f);
#ifdef NON_MATCHING
void func_802CCC8C(s32 arg0);
#else
void func_802CCC8C(); /* hd.c passes nothing: the asm reads leaked registers */
#endif
s32 func_802CCCD8(void);
void func_802CCD34(void);
void func_802CCD80(void);
s32 func_802CCED4(s32 id, InterpRegs *r);
s32 func_802CCF60(s32 id, TriSideOut *f);
void func_802CD068(void);

/* hd_code/89250.c */
void func_802CDA10(s32 x, s32 y, s32 z);
void func_802CDAE8(s32 amount, s32 radius);
s32 func_802CDB70(s32 r, s32 amount);
s32 func_802CDF94(s32 r);
void func_802CE204(s32 x1, s32 z1, s32 x2, s32 z2);
s32 func_802CE3B8(s32 idx);
void func_802CE4F0(s32 x, s32 y, s32 z);
void func_802CE5BC(s32 x, s32 y, s32 z, s32 r, s32 kind, s32 tag);
void func_802CE65C(s32 x, s32 z, s32 len, s32 angle);
s32 func_802CE6F8(s32 x, s32 z, s32 y);

/* hd_code/8A080.c */
void func_802CE840(void);
void func_802CE880(s32 id, s32 x, s32 y, s32 z, s32 size);
void func_802CE90C(s32 id);
s32 func_802CE958(s32 id);
void func_802CE9A4(void);
#ifdef NON_MATCHING
void func_802CE9C8(u8 *items, s32 n, s32 h52, s32 id, s32 b57, s32 b4F, s32 s1);
#else
void func_802CE9C8(); /* hd.c passes nothing: the asm reads leaked registers */
#endif
void func_802CEA68(u8 *start, u8 *end);

/* hd_code/8A2E0.c */
void func_802CEAA0(u8 *obj);
Gfx *func_802CEEFC(Gfx *gdl, s32 cur, Gfx *dl2, Mtx *mtx);
void func_802CF1A4(void);
void func_802CF5B0(void);
void func_802CF628(void);

/* hd_code/8AEE0.c */
void func_802CF6A0(u8 *model, s32 x, s32 y, s32 z, s32 heading, s32 fpIn, s32 t6, TriSideOut *f);
#ifdef NON_MATCHING
void func_802CFA0C(s32 arg0);
#else
void func_802CFA0C(); /* hd.c passes nothing: the asm reads leaked registers */
#endif
s32 func_802CFA58(void);
void func_802CFAB4(void);
#ifdef NON_MATCHING
void func_802CFB00(s32 fpIn, s32 a3, f32 f12, f32 f14, f32 f20, f32 f22, f32 f24, f32 f26);
#else
void func_802CFB00(); /* hd.c passes nothing: the asm reads leaked registers */
#endif
s32 func_802CFC54(s32 id, InterpRegs *r);
s32 func_802CFCE0(s32 id, InterpRegs *r);
#ifdef NON_MATCHING
void func_802CFDE8(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4, f32 f14, f32 f20, f32 f22, f32 f24, f32 f26);
#else
void func_802CFDE8(); /* hd.c passes nothing: the asm reads leaked registers */
#endif
void func_802D07E0(u8 *model, s32 x, s32 y, s32 z, s32 heading, s32 fpIn, s32 t6, TriSideOut *f);
s32 func_802D0B90(void);
void func_802D0BF8(void);
void func_802D0C68(void);
#ifdef NON_MATCHING
void func_802D0F98(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4, f32 f14, f32 f20, f32 f22, f32 f24, f32 f26);
#else
void func_802D0F98(); /* hd.c passes nothing: the asm reads leaked registers */
#endif
void func_802D2524(void *src);

/* hd_code/8DDB0.c */
void func_802D2570(u8 *hdr);
void func_802D291C(void);

/* hd_code/8FD90.c */
void func_802D4550(u32 arg0);

/* hd_code/93B50.c */
s16 func_802D8310(void *arg0);

/* hd_code/96300.c */
u32 func_802DAAC0(void);

/* hd_front_end/00000.c */
s32 func_801E7000(void);
void func_801E7598(void);

/* hd_front_end/01C40.c */
void func_801E8C40(u8 arg0);
void func_801E8DCC(u8 arg0);
void func_801E8EB8(u8 slot, u8 arg1);
s32 func_801E96F8(void);
Gfx *func_801E9718(Gfx *arg0, struct PlayerSelDyn *dyn, s32 arg2);
void func_801EA108(u8 slot, u8 send, u8 newGame);
void func_801EA278(void);
void func_801EA4B8(void);
void func_801EA6E8(void);
void func_801EA93C(char *title, u16 *glyphs, u8 arg2, u8 width, char *buf);
Gfx *func_801EAA7C(Gfx *arg0, struct PlayerSelDyn *dyn, s32 *count);
void func_801EC288(u8 arg0);
void func_801EC30C(u8 arg0);
Gfx *func_801EC770(Gfx *start, void *gfxp, s32 *count);
u64 func_801ECA50(u8 level);
void func_801ECB18(void);
void func_801ECC8C(void);
void func_801ECE9C(void);
void func_801ECF5C(void);
void func_801ED4B8(void);

/* hd_front_end/06790.c */
void func_801ED790(void);
Gfx *func_801ED800(Gfx *arg0, FeDyn *dyn, u8 arg2, s32 *arg3);

/* hd_front_end/07390.c */
void func_801EE390(void);
void func_801EE398(s32 arg0);

/* hd_front_end/07800.c */
u8 func_801EE800(u8 *arg0, u8 arg1, u8 arg2);
s8 func_801EF1E0(void);
#ifdef NON_MATCHING
u8 func_801EF2BC(u16 arg0, u8 arg1, u8 arg2);
#else
u8 func_801EF2BC(); /* K&R definition in the matching build */
#endif
void func_801EF380(s32 arg0);
void func_801EF4AC(void);

/* hd_front_end/09570.c */
Gfx *func_801F1568(void);
Gfx *func_801F2000(void);
Gfx *func_801F2428(void);
Gfx *func_801F2E20(void);

/* hd_front_end/0C450.c */
Gfx *func_801F3450(Gfx *gdl, u8 *dyn);

/* hd_front_end/0DE70.c */
void func_801F4E70(u8 arg0);
Gfx *func_801F4FBC(FeDyn *dyn, Gfx *arg1);
Gfx *func_801F51C8(FeDyn *dyn, Gfx *arg1);
void func_801F55D8(void);

/* hd_front_end/0E7B0.c */
void func_801F57B0(void);
s32 func_801F6F18(void);
s32 func_801F73FC(void);

/* hd_front_end/10850.c */
void func_801F7850(void);
void func_801F803C(void);
void func_801F8228(void);
void func_801F8354(u8 pn);
Gfx *func_801F8440(s32 arg0, Gfx *gfx);

/* hd_front_end/11530.c */
void func_801F8530(s32 level);
void func_801F8980(void);
void func_801FCE74(Vtx *v, u8 level, f32 dlat, f32 dlon, u8 w, u8 h, f32 scale, u8 flip);
void func_801FD484(f32 *lat, f32 *lon, f32 *x, f32 *y, f32 *z, f32 r);
void func_801FDCA4(Vtx *v, s32 idx, s32 z);
void func_801FDE50(void);
void func_801FE018(u8 rank);
Gfx *func_801FE238(Gfx *arg0, s32 arg1);
#ifdef NON_MATCHING
s32 func_801FE760(u8 level);
#else
s32 func_801FE760(); /* K&R definition in the matching build */
#endif

/* hd_front_end/17990.c */
void func_801FE990(void);

/* hd_front_end/196F0.c */
void func_80200714(u8 mode);
Gfx *func_80200BE0(Gfx *gfx, s32 arg1, s32 *count);

/* hd_front_end/1A240.c */
void func_80201240(s32 arg0);
Gfx *func_80201364(s32 arg0, Gfx *arg1);

/* hd_front_end/1AE80.c */
s32 func_80201E80(void);

/* hd_front_end/1B100.c */
void func_80202100(s32 type, u8 * N64P *model, u8 * N64P *heap, u8 * N64P *dl);
void func_802021FC(void *ch, void *base, void *other);
void func_80202270(u8 *hdr, u32 * N64P *bufs, void *ch);
void func_802022EC(void *base, s32 idx, s32 v040C, s32 v0480, f32 f, s32 v039C, s32 v03D4);
void func_80202380(s32 kind);
#ifndef LEGACY_func_802025D0
void func_802025D0(s32 kind, s32 val);
#endif

/* (hd_code/69000.c, 69930.c: fixed-point trig in the asm blobs) */
s32 func_802AD7D4(s32 sine);
s32 func_802AD7FC(u32 sine);
s32 func_802AE104(s32 angle);
s32 func_802AE160(s32 angle);

/* (libultra functions the game calls by address names; aliased in undefined_syms.hd_code.us.v11.txt) */
#define func_802D4020 osInitialize
#define func_802D4560 osCreatePiManager
#define func_802D4E10 alCSPGetState
#define func_802D6710 osWritebackDCacheAll
#define func_802D67F0 osDestroyThread
#define func_802D6A60 sprintf
#define func_802D9B60 osAiSetNextBuffer
#define func_802D9C10 osAiGetLength
#define func_802D9D68 alAudioFrame
#define func_802DA2F0 osPiStartDma
#define func_802DA610 osCreateViManager
#define func_802DAE1C osSpTaskLoad
#define func_802DB0A0 osContInit
#define func_802DB4D0 osContStartReadData
#define func_802DB594 osContGetReadData
#define func_802DB730 osInvalICache
#define func_802DB7B0 bzero

#endif
