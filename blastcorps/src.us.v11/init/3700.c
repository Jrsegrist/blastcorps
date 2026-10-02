#include "common.h"
#include <ultra64.h>
#include <PR/os_internal.h>

typedef struct {
    s32 unk0;
    s32 unk4;
} HandlerEntry;

extern HandlerEntry D_80224D00[];

void func_80222400(s32 arg0, s32 arg1, s32 arg2) {
    register u32 temp_v0;
    HandlerEntry *entry;

    temp_v0 = __osDisableInt();
    entry = &D_80224D00[arg0];
    entry->unk0 = arg1;
    entry->unk4 = arg2;
    __osRestoreInt(temp_v0);
}
