#include "common.h"
#include <ultra64.h>
#include <PR/sched.h>

/* RSP task submission: builds an OSScTask for one of the microcode slots,
 * hands it to the scheduler and waits for its done message. */

/* A scheduler task plus the message posted back on D_803153D8 when it's done
 * ((slot << 16) | id) */
typedef struct {
    OSScTask t;
    u32 reply;
    s32 pad;
} RspTask; /* 0x60 */

/* Per-framebuffer data written back before a task runs */
typedef struct {
    u8 data[0x21498];
} FrameData;

typedef struct {
    u16 px[320 * 240];
} FrameBuffer;

extern u8 D_80207090[]; /* microcode text/data pairs */
extern u8 D_80210690[];
extern u8 D_802E53F0[];
extern u8 D_8030E390[];
extern u8 D_802E6820[]; /* rspboot */
extern u8 D_802E68F0[];
extern u8 *D_8036E660[]; /* ucode text per slot */
extern u8 *D_8036E678[]; /* ucode data per slot */
extern u8 D_8036E68C[];  /* slot busy flags */
extern u64 *D_8036E694;  /* RDP output buffer (0xA000 bytes) */
extern RspTask D_8036E698[][2];
extern u8 D_8035805C; /* current framebuffer */
extern u64 D_80367750[]; /* dram stack */
extern u64 D_8036AFB0[]; /* yield buffer */
extern OSMesgQueue D_803153D8;
extern OSMesgQueue D_80315440;
extern OSMesgQueue D_803156D8;
extern FrameData D_803156F8[];
extern FrameBuffer D_80000400[];

void func_802D6710(void);
void func_8029A7E4(const char *, ...);

/* Fill in the microcode table */
void func_80284DB0(void) {
    D_8036E660[0] = D_80207090;
    D_8036E678[0] = D_80210690;
    D_8036E660[1] = D_802E53F0;
    D_8036E678[1] = D_8030E390;
    D_8036E660[2] = D_802E53F0;
    D_8036E678[2] = D_8030E390;
    D_8036E660[3] = D_802E53F0;
    D_8036E678[3] = D_8030E390;
}

/* Build and send an RSP task running microcode slot `slot` over the display
 * list `data` (`count` commands) */
void func_80284E54(u64 *data, s32 count, u8 slot, u8 yield, s32 id, u8 writebackAll) {
    RspTask *task;
    s32 size;

    size = count * 8;
    task = &D_8036E698[slot][D_8035805C];
    D_8036E68C[slot] = 1;
    task->t.list.t.type = M_GFXTASK;
    if (slot == 4) {
        task->t.list.t.flags = 2;
    } else {
        task->t.list.t.flags = 0;
    }
    task->t.list.t.ucode_boot = (u64 *) D_802E6820;
    task->t.list.t.ucode_boot_size = D_802E68F0 - D_802E6820;
    task->t.list.t.ucode = (u64 *) D_8036E660[slot];
    task->t.list.t.ucode_data = (u64 *) D_8036E678[slot];
    task->t.list.t.ucode_size = 0x1000;
    task->t.list.t.ucode_data_size = 0x800;
    task->t.list.t.dram_stack = D_80367750;
    task->t.list.t.dram_stack_size = 0x400;
    task->t.list.t.output_buff = D_8036E694;
    task->t.list.t.output_buff_size = (u64 *) ((u8 *) D_8036E694 + 0xA000);
    task->t.list.t.data_ptr = data;
    task->t.list.t.data_size = size;
    task->t.list.t.yield_data_ptr = D_8036AFB0;
    task->t.list.t.yield_data_size = 0x900;
    task->t.next = NULL;
    task->t.msg = (OSMesg) &D_803153D8;
    task->reply = (slot << 16) | id;
    task->t.flags = 3;
    if (yield) {
        task->t.flags |= 0x40;
    }
    task->t.framebuffer = &D_80000400[D_8035805C];
    task->t.msgQ = &D_803156D8;
    if (writebackAll) {
        func_802D6710();
    } else {
        osWritebackDCache(task, sizeof(RspTask));
        osWritebackDCache(data, size);
        osWritebackDCache(&D_803156F8[D_8035805C], sizeof(FrameData));
    }
    osSendMesg(&D_80315440, task, OS_MESG_BLOCK);
}

/* Wait for task `id` to finish, clearing the busy flag of every task that reports in */
void func_80285110(s32 id) {
    OSMesg msg;

    do {
        osRecvMesg(&D_803153D8, &msg, OS_MESG_BLOCK);
        D_8036E68C[(u32) msg >> 16] = 0;
        msg = (OSMesg) ((u32) msg & 0xFFFF);
        if ((s32) msg != id) {
            func_8029A7E4("Task %d received message %d\n", id, msg);
        }
    } while ((s32) msg != id);
}
