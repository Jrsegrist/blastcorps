#ifndef GAME_PORT_H
#define GAME_PORT_H

/*
 * Hooks for the native (Windows) port, which builds the NON_MATCHING C with
 * PORT_HOST defined (port/Makefile). They expand to nothing in every N64
 * build, so the matching build is unaffected.
 *
 *   PORT_SPIN()       inside a busy-wait loop that waits for another thread
 *                     or an interrupt (the VI counter): the port's
 *                     cooperative scheduler lets time pass there. It is a
 *                     call, so the waited-on global is re-read each time.
 *   PORT_FE_LOADED()  the front-end overlay has just been inflated over
 *                     0x801E7000: the port restores the overlay's initial
 *                     (host byte order) .data/.bss.
 *   PORT_SAVE_BEGIN(p, kind, size) / PORT_SAVE_END(p, kind, size)
 *                     around a save routine's CRC and EEPROM/pak transfer:
 *                     p is pointed at a copy of the record in N64 byte order,
 *                     then back at the record, which gets the copy's contents
 *                     in host order (port/src/platform/save.c), so save files
 *                     are byte-identical to the N64's.
 *   PORT_SAVE_BE64(x) a u64 (the save semaphore) to or from N64 byte order.
 *   PORT_SAVE_SEMBLOCK(p)  the 0x20 bytes the pak semaphore write sends from
 *                     &sem (a u64 parameter): the semaphore, then the 24 bytes
 *                     of stack that follow it on the N64 (as observed), not
 *                     the host's stack.
 *   PORT_SAVE_SWAP(p, kind, size)  in-place conversion (an involution), for
 *                     the status block, whose only multi-byte value is its
 *                     "no status" marker.
 */
/*   PORT_HALF(i)      index of halfword i of an array of 32-bit words viewed
 *                     as u16s (the N64 Mtx's elements: integer halves, then
 *                     fraction halves, two per word). The port keeps such
 *                     words in host order (as ultralib's gu writes them), so
 *                     on the little-endian host the two halves of each word
 *                     trade places: i ^ 1. */
#if defined(NON_MATCHING) && defined(PORT_HOST)
#define PORT_HALF(i) ((i) ^ 1)
#else
#define PORT_HALF(i) (i)
#endif
/*   PORT_SHAMT(n)     a variable shift amount as MIPS sllv/srlv/srav use it
 *                     (low 5 bits); in C a shift by 32 or more is undefined
 *                     and gcc folds it away. */
#if defined(NON_MATCHING) && defined(PORT_HOST)
#define PORT_SHAMT(n) ((n) & 31)
#else
#define PORT_SHAMT(n) (n)
#endif
/*   PORT_WRAP_MUL(a, b), PORT_WRAP_ADD(a, b)   a signed 32-bit multiply or add
 *                     that overflows at run time (the N64 wraps; in C it is
 *                     undefined, and MSVC has no -fwrapv): done on u32 in the
 *                     port.  The sites are the ones a signed-overflow census of
 *                     the attract mode and all levels finds (`make -C port
 *                     overflow-census`). */
#if defined(NON_MATCHING) && defined(PORT_HOST)
#define PORT_WRAP_MUL(a, b) ((s32) ((u32) (a) * (u32) (b)))
#define PORT_WRAP_ADD(a, b) ((s32) ((u32) (a) + (u32) (b)))
#else
#define PORT_WRAP_MUL(a, b) ((a) * (b))
#define PORT_WRAP_ADD(a, b) ((a) + (b))
#endif
/*   PORT_GVI(fn, v)   v, a read of the game's retrace counter D_803156C4 in
 *                     function fn after code the native port runs in no time;
 *                     when following an emulator, the value the emulator
 *                     read there (port/tools/cmp_spec.py GVI_FUNCS). */
/*   PORT_EMU(site, v) v, a value whose timing against the retrace the native
 *                     port can't reproduce (another thread writes it after CPU
 *                     time the native code doesn't take); when following an
 *                     emulator, the value the emulator read at SITE in the same
 *                     retrace (port/tools/cmp_spec.py EMU_SITES). */
#if defined(NON_MATCHING) && defined(PORT_HOST)
unsigned int port_gvi_read(const char *fn, unsigned int v);
unsigned int port_emu_read(const char *site, unsigned int v);
#define PORT_GVI(fn, v) port_gvi_read(fn, v)
#define PORT_EMU(site, v) port_emu_read(site, v)
#else
#define PORT_GVI(fn, v) (v)
#define PORT_EMU(site, v) (v)
#endif
#define PORT_SAVE_PLAYER 0  /* player record, 0x100 bytes */
#define PORT_SAVE_TIMES 1   /* best times, 0x20 bytes */
#define PORT_SAVE_EETIMES 2 /* EEPROM best-time words, 8 bytes */
#define PORT_SAVE_STATUS 3  /* level status block, 0x40 bytes (in place) */
#if defined(NON_MATCHING) && defined(PORT_HOST)
#include <stdarg.h>
void port_spin(void);
void port_fe_loaded(void);
void port_game_print(const char *fmt, va_list ap); /* the game's debug printf */
void *port_save_begin(void *p, int kind, int size);
void *port_save_end(void *p, int kind, int size);
unsigned long long port_save_be64(unsigned long long v);
void *port_save_semblock(const void *sem);
#define PORT_SPIN() port_spin()
#define PORT_FE_LOADED() port_fe_loaded()
#define PORT_SAVE_BEGIN(p, kind, size) ((p) = port_save_begin((p), (kind), (size)))
#define PORT_SAVE_END(p, kind, size) ((p) = port_save_end((p), (kind), (size)))
#define PORT_SAVE_BE64(x) ((x) = port_save_be64(x))
#define PORT_SAVE_SWAP(p, kind, size) ((void) port_save_begin((p), (kind), (size)))
#define PORT_SAVE_SEMBLOCK(p) ((u8 *) port_save_semblock(p))
#else
#define PORT_SAVE_SWAP(p, kind, size)
#define PORT_SAVE_SEMBLOCK(p) (p)
#define PORT_SPIN()
#define PORT_FE_LOADED()
#define PORT_SAVE_BEGIN(p, kind, size)
#define PORT_SAVE_END(p, kind, size)
#define PORT_SAVE_BE64(x)
#endif

/*   Frame interpolation tags (bc.exe's tagged interpolation, port/src/live/
 *   live_interp.cpp).  They only tell the host who draws what; nothing is
 *   written to the display list or anywhere else in RDRAM, and the game's
 *   state is never read back, so the game runs the same with or without them.
 *   PORT_ITAG(gdl, kind, index)  the commands written from gdl on (in that
 *                     display list, and the lists it calls) belong to the
 *                     object (kind, index), until the next PORT_ITAG on that
 *                     list (PORT_ITAG_NONE: nobody's).
 *   PORT_ISPAWN(kind, index)  slot `index` of `kind` starts a new object (not
 *                     blended with what the slot showed before).
 *   PORT_ICUT_IF(c)   if c: the game snaps its camera (no in-between pictures
 *                     into this frame). */
#define PORT_ITAG_NONE 0
#define PORT_ITAG_HUD 1       /* 2D and HUD: shown exactly as drawn */
#define PORT_ITAG_OBJ 2       /* gVehicles[] record (vehicles, the carrier): index | lod << 8 */
#define PORT_ITAG_SHADOW 3    /* vehicle shadow quad, gVehicleShadows[] index */
#define PORT_ITAG_SHADOWFE 4  /* the flying vehicle's shadow */
#define PORT_ITAG_BUILDING 5  /* D_803F4030[] building object (posed or not) */
#define PORT_ITAG_BPART 6     /* a building's moving part: object << 8 | part */
#define PORT_ITAG_FALL 7      /* falling part slot D_803F1BE0[] */
#define PORT_ITAG_RISE 8      /* rising piece D_803EFED0 */
#define PORT_ITAG_BILLBOARD 9 /* explosion / smoke record D_803C4B70[] */
#define PORT_ITAG_PARTICLE 10 /* particle D_8036EC38[] */
#define PORT_ITAG_PEOPLE 11   /* person sprite D_80367D60[] (world-space quad) */
#define PORT_ITAG_GSHADOW 12  /* ground shadow of a D_803F4030[] object (world-space quad) */
#define PORT_ITAG_MARKER 13   /* people's marker */
#define PORT_ITAG_RADAR 14    /* the radar's sweep and heading arrow (blended; then back to HUD) */
#define PORT_ITAG_CUT 30
#define PORT_ITAG_SPAWN 31
#if defined(NON_MATCHING) && defined(PORT_HOST)
void port_itag(const void *gdl, int kind, int index);
#define PORT_ITAG(gdl, kind, index) port_itag((const void *) (gdl), (kind), (int) (index))
#define PORT_ISPAWN(kind, index) port_itag(0, PORT_ITAG_SPAWN, ((kind) << 16) | (int) (index))
#define PORT_ICUT_IF(c) ((c) ? port_itag(0, PORT_ITAG_CUT, 0) : (void) 0)
#else
#define PORT_ITAG(gdl, kind, index)
#define PORT_ISPAWN(kind, index)
#define PORT_ICUT_IF(c)
#endif

#endif
