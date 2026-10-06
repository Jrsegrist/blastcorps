#include "common.h"

/*
 * Hand-written asm (sd $ra frames that save s0-s7, gp and fp): o32 entry
 * points through which the front end's C calls hd_code functions that take
 * their arguments in other registers (model loading, display-list setup and
 * the engine-sound channel setters of 56040.c).
 *
 * The NON_MATCHING versions call those functions' C rewrites directly.  The
 * asm compares and passes whole registers, so the parameters are s32 here
 * where 00000.c / 0DE70.c declare u8 (the callers pass zero-extended values).
 */

#ifdef NON_MATCHING
#include <ultra64.h>
#include "game/regs.h"

typedef struct Unk8029DEA0Entry Unk8029DEA0Entry;


extern u8 *D_80358070; /* hd_code heap pointer */
/* engine-sound channel tables in hd_code's 7D9D0 text blob */
extern u8 D_802C2190[];
extern u8 D_802C21A4[];
extern u8 D_802C21B8[];
extern u8 D_802C2208[];
extern u8 D_802C226C[];

void func_802A396C(s32 type, Out802A396C *out);
void func_802A1388(s32 a0Val, s32 a1Val, s32 v0Val, s32 v1Val, u8 *hdr);
void func_8029E558(u8 *base, u8 *other, Unk8029DEA0Entry *ch);
s32 func_8029F85C(u32 *bufA, u32 *bufB, Unk8029DEA0Entry *ch, u8 *hdr);
void func_802A039C(Unk8029DEA0Entry *base, s32 idx, s32 val);
void func_802A03D4(Unk8029DEA0Entry *base, s32 idx, s32 val);
void func_802A040C(Unk8029DEA0Entry *base, s32 idx, s32 val);
void func_802A0480(f32 f, Unk8029DEA0Entry *base, s32 idx, s32 val);
void func_802A0290(Unk8029DEA0Entry *base, s32 idx, s32 val);
void func_802A05A4(f32 f, s32 key, s32 val);
void func_802A05D0(s32 key, s32 val);
void func_802A05F8(s32 key, s32 val);
void func_802A0620(s32 key, s32 val);
void func_802A0508(s32 key, s32 val);

/*
 * Load model `type` (func_802A396C), reserve two 6000-byte heap buffers
 * (heap[0], heap[1]) and set up the two display-list ranges in dl[0..3]
 * (start in the model data at its header offsets 0x1C / 0x2C, write position
 * in the second buffer), then func_802A1388(type, 0, heap[0], heap[1], model).
 * The loads and stores keep the asm's order (the last call's arguments are
 * read back from memory).
 */
void func_80202100(s32 type, u8 **model, u8 **heap, u8 **dl) {
    Out802A396C o;
    u8 *data;
    u8 *h;
    s32 off1C;
    s32 off2C;

    func_802A396C(type, &o);
    data = o.s2;
    *model = data;
    h = D_80358070;
    heap[0] = h;
    h += 0x1770;
    heap[1] = h;
    h += 0x1770;
    D_80358070 = h;
    off1C = *(s32 *) (data + 0x1C);
    dl[0] = data + off1C;
    dl[1] = h;
    off2C = *(s32 *) (data + 0x2C);
    dl[2] = data + off2C;
    dl[3] = h + off2C - off1C;
    func_802A1388(type, 0, (s32) heap[0], (s32) heap[1], *model);
}

/* func_8029E558 with the arguments rotated (asm: v0 = a1, v1 = a2, t0 = a0).
 * The asm saves only integer registers, so it leaves f20/f30 as the asm
 * func_8029E558 leaves them (conventions.txt: clobbers f20,f30). */
void func_802021FC(void *ch, void *base, void *other) {
    func_8029E558(base, other, ch);
}

/* func_8029F85C(bufs[1], bufs[0], ch, hdr); its result is dropped */
void func_80202270(u8 *hdr, u32 **bufs, void *ch) {
    func_8029F85C(bufs[1], bufs[0], ch, hdr);
}

/* the five setters of channel `idx` of table `base` */
void func_802022EC(void *base, s32 idx, s32 v040C, s32 v0480, f32 f, s32 v039C, s32 v03D4) {
    func_802A039C(base, idx, v039C);
    func_802A03D4(base, idx, v03D4);
    func_802A040C(base, idx, v040C);
    func_802A0480(f, base, idx, v0480);
    func_802A0290(base, idx, -1);
}

#define RESET_CHANNEL(tbl)         \
    func_802A05D0((s32) (tbl), 0); \
    func_802A05F8((s32) (tbl), 0); \
    func_802A0620((s32) (tbl), 0); \
    func_802A0508((s32) (tbl), -1)

/* reset the engine-sound channels of vehicle kind 5 or 4.  (After the kind-4
 * block the asm has a third, unreachable one that would set up D_802C2324 and
 * D_802C2348 with 100.) */
void func_80202380(s32 kind) {
    if (kind == 5) {
        RESET_CHANNEL(D_802C2208);
        RESET_CHANNEL(D_802C226C);
    } else if (kind == 4) {
        RESET_CHANNEL(D_802C2190);
        RESET_CHANNEL(D_802C21A4);
        RESET_CHANNEL(D_802C21B8);
    }
}

/* engine-sound pitch from `val` for vehicle kind 5 or 4 */
void func_802025D0(s32 kind, s32 val) {
    if (kind == 5) {
        if (val >= 0x800) {
            val -= 0x800;
        }
        val = (u32) val / 0x55;
        func_802A05A4(0.0f, (s32) D_802C2208, val);
        func_802A05A4(0.0f, (s32) D_802C226C, val);
    } else if (kind == 4) {
        if (val >= 0x355 && val < 0x8AB) {
            val -= 0x355;
        } else {
            val = 0;
        }
        val = (u32) val / 0x4C;
        func_802A05A4(0.0f, (s32) D_802C21B8, val);
        func_802A05D0((s32) D_802C2190, 0x32);
        func_802A05D0((s32) D_802C21A4, 0x32);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/1B100/func_80202100.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/1B100/func_802021FC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/1B100/func_80202270.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/1B100/func_802022EC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/1B100/func_80202380.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/1B100/func_802025D0.s")
#endif
