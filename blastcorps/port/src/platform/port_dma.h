/* The one hook between the platform layer's PI emulation and the byte-order
 * layer (port/src/load/).  The platform calls it after every ROM -> RDRAM
 * copy (osPiStartDma / osEPiStartDma, once the bytes are in place):
 * DST and ROM are the RDRAM address and the ROM (cartridge) offset, LEN the
 * byte count.  port/src/load/port_dma.c implements it. */
#ifndef PORT_DMA_H
#define PORT_DMA_H

#include <stdint.h>

void port_on_dma(uint32_t dst, uint32_t rom, uint32_t len);

#endif
