/* The one hook shared by the platform layer ("plat") and the load-time byte
 * swapper ("load"): called after every PI DMA from the ROM into RDRAM.
 *   dst  N64 (KSEG0) address the bytes were copied to
 *   rom  ROM offset they came from
 *   len  byte count
 * The bytes at dst are still the ROM's big-endian bytes when it is called.
 * The platform has an identity version (port_dma_stub.c), linked only while
 * port/src/load/ has no sources; the load layer supplies the real one. */
#ifndef PORT_DMA_H
#define PORT_DMA_H

#ifdef _ULTRATYPES_H_
void port_on_dma(u32 dst, u32 rom, u32 len);
#else
#include <stdint.h>
void port_on_dma(uint32_t dst, uint32_t rom, uint32_t len);
#endif

#endif
