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
    .bss_00000 0x80310D80 (NOLOAD) : { build/src.us.v11/hd_code/00000.c.o(.bss); }
    .bss_2C560 0x8036BEF0 (NOLOAD) : { build/src.us.v11/hd_code/2C560.c.o(.bss); }
    .bss_2D810 0x8036BFC0 (NOLOAD) : { build/src.us.v11/hd_code/2D810.c.o(.bss); }
    .bss_30430 0x8036C770 (NOLOAD) : { build/src.us.v11/hd_code/30430.c.o(.bss); }
    .bss_54E30 0x803A6AF0 (NOLOAD) : { build/src.us.v11/hd_code/54E30.c.o(.bss); }
}
