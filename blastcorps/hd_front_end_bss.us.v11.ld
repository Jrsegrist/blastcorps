/* .bss of front-end libultra objects built from ultralib, at their original
 * addresses (NOLOAD). Listed before the splat linker script so it takes
 * precedence over its /DISCARD/ (see hd_code_bss.us.v11.ld). */
SECTIONS
{
    .bss_ul_conteepread 0x8021ABB0 (NOLOAD) : { build/src.us.v11/hd_front_end/ul_conteepread.c.o(.bss); }
    .bss_ul_pfsisplug 0x8021ABF0 (NOLOAD) : { build/src.us.v11/hd_front_end/ul_pfsisplug.c.o(.bss); }
}
