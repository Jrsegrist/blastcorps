#include "common.h"
#include <ultra64.h>
#include "game/game.h"

void func_8029A7D0(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
}

/* The debug printf (asserts and messages), compiled out: an empty variadic
 * function (IDO homes a0-a3 as for any varargs function). The native port
 * can print the messages (bc_headless.exe --print). */
void func_8029A7E4(const char *fmt, ...) {
#if defined(NON_MATCHING) && defined(PORT_HOST)
    __builtin_va_list ap;

    __builtin_va_start(ap, fmt);
    port_game_print(fmt, ap);
    __builtin_va_end(ap);
#endif
}
