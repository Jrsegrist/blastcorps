/* .bss of libultra objects built from ultralib, at their original
 * addresses (past the end of the ROM image, so NOLOAD). Listed before the
 * splat linker script so it takes precedence over its /DISCARD/. */
SECTIONS
{
    .bss_ul_rotate 0x803FDF60 (NOLOAD) : { build/src.us.v11/hd_code/ul_rotate.c.o(.bss); }
    .bss_ul_seteventmesg 0x803FF180 (NOLOAD) : { build/src.us.v11/hd_code/ul_seteventmesg.c.o(.bss); }
    .bss_ul_piacs 0x803FF2D0 (NOLOAD) : { build/src.us.v11/hd_code/ul_piacs.c.o(.bss); }
    .bss_ul_timerintr 0x803FF2F0 (NOLOAD) : { build/src.us.v11/hd_code/ul_timerintr.c.o(.bss); }
    .bss_ul_siacs 0x803FF330 (NOLOAD) : { build/src.us.v11/hd_code/ul_siacs.c.o(.bss); }
    .bss_26570 0x8036BF00 (NOLOAD) : { build/src.us.v11/hd_code/26570.c.o(.bss); }
}
