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
 */
#if defined(NON_MATCHING) && defined(PORT_HOST)
void port_spin(void);
void port_fe_loaded(void);
#define PORT_SPIN() port_spin()
#define PORT_FE_LOADED() port_fe_loaded()
#else
#define PORT_SPIN()
#define PORT_FE_LOADED()
#endif

#endif
