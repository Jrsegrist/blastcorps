#include "common.h"
#include <ultra64.h>
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define gFrameBuffers ((FrameBuffer *) gFrameBuffers)
#define gDynamicBufs ((FrameData *) gDynamicBufs)
/* end of views */
#include <PR/sched.h>

/* RSP task submission: builds an OSScTask for one of the microcode slots,
 * hands it to the scheduler and waits for its done message. */

/* A scheduler task plus the message posted back on gTaskDoneQueue when it's done
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
extern u8 * N64P gUcodeText[]; /* ucode text per slot */
extern u8 * N64P gUcodeData[]; /* ucode data per slot */
extern RspTask gRspTasks[][2];
extern u64 gDramStack[]; /* dram stack */


/* Fills in the microcode slots: slot 0 the front end's line ucode, 1-3 the
 * game's (0x80284DB0). */
void rsp_init_ucode_table(void) {
    gUcodeText[0] = D_80207090;
    gUcodeData[0] = D_80210690;
    gUcodeText[1] = D_802E53F0;
    gUcodeData[1] = D_8030E390;
    gUcodeText[2] = D_802E53F0;
    gUcodeData[2] = D_8030E390;
    gUcodeText[3] = D_802E53F0;
    gUcodeData[3] = D_8030E390;
}

/* Build and send an RSP task running microcode slot `slot` over the display
 * list `data` (`count` commands); `yield` makes it a frame task (flag 0x40:
 * swaps the frame buffer when done); its done message is (slot << 16) | id
 * on gTaskDoneQueue (0x80284E54). */
void rsp_send_task(u64 *data, s32 count, u8 slot, u8 yield, s32 id, u8 writebackAll) {
    RspTask *task;
    s32 size;

    size = count * 8;
    task = &gRspTasks[slot][gBufferIndex];
    gRspSlotBusy[slot] = 1;
    task->t.list.t.type = M_GFXTASK;
    if (slot == 4) {
        task->t.list.t.flags = 2;
    } else {
        task->t.list.t.flags = 0;
    }
    task->t.list.t.ucode_boot = (u64 *) D_802E6820;
    task->t.list.t.ucode_boot_size = D_802E68F0 - D_802E6820;
    task->t.list.t.ucode = (u64 *) gUcodeText[slot];
    task->t.list.t.ucode_data = (u64 *) gUcodeData[slot];
    task->t.list.t.ucode_size = 0x1000;
    task->t.list.t.ucode_data_size = 0x800;
    task->t.list.t.dram_stack = gDramStack;
    task->t.list.t.dram_stack_size = 0x400;
    task->t.list.t.output_buff = gRdpOutputBuffer;
    task->t.list.t.output_buff_size = (u64 *) ((u8 *) gRdpOutputBuffer + 0xA000);
    task->t.list.t.data_ptr = data;
    task->t.list.t.data_size = size;
    task->t.list.t.yield_data_ptr = gYieldBuffer;
    task->t.list.t.yield_data_size = 0x900;
    task->t.next = NULL;
    task->t.msg = (OSMesg) &gTaskDoneQueue;
    task->reply = (slot << 16) | id;
    task->t.flags = 3;
    if (yield) {
        task->t.flags |= 0x40;
    }
    task->t.framebuffer = &gFrameBuffers[gBufferIndex];
    task->t.msgQ = &gMainSchedClient;
    if (writebackAll) {
        osWritebackDCacheAll();
    } else {
        osWritebackDCache(task, sizeof(RspTask));
        osWritebackDCache(data, size);
        osWritebackDCache(&gDynamicBufs[gBufferIndex], sizeof(FrameData));
    }
    osSendMesg(&gScheduler, task, OS_MESG_BLOCK);
}

/* Wait for task `id` to finish, clearing the busy flag of every task that
 * reports in (0x80285110). */
void rsp_wait_task(s32 id) {
    OSMesg msg;

    do {
        osRecvMesg(&gTaskDoneQueue, &msg, OS_MESG_BLOCK);
        gRspSlotBusy[(u32) msg >> 16] = 0;
        msg = (OSMesg) ((u32) msg & 0xFFFF);
        if ((s32) msg != id) {
            debug_printf("Task %d received message %d\n", id, msg);
        }
    } while ((s32) msg != id);
}
