#include "common.h"
#include <ultra64.h>

/* (not <PR/os_internal.h>: its __osSetHWIntrRoutine is the older
 * two-argument one; this is the three-argument variant with a stack) */
extern u32 __osDisableInt(void);
extern void __osRestoreInt(u32);

typedef struct {
    s32 unk0;
    s32 unk4;
} HandlerEntry;

extern HandlerEntry __osHwIntTable[];

/* libultra __osSetHWIntrRoutine (the three-argument variant): with interrupts off, stores the handler and its
 * stack in __osHwIntTable[interrupt]; no callers in init (0x80222400). */
void __osSetHWIntrRoutine(s32 arg0, s32 arg1, s32 arg2) {
    register u32 temp_v0;
    HandlerEntry *entry;

    temp_v0 = __osDisableInt();
    entry = &__osHwIntTable[arg0];
    entry->unk0 = arg1;
    entry->unk4 = arg2;
    __osRestoreInt(temp_v0);
}
