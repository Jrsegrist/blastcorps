/* .bss of front-end objects that define their own variables, at their
 * original addresses (NOLOAD). Listed before the splat linker script so it
 * takes precedence over its /DISCARD/ (as hd_code_bss.us.v11.ld). */
SECTIONS
{
    .bss_11530 0x8021A940 (NOLOAD) : { build/src.us.v11/hd_front_end/11530.c.o(.bss); }
}
