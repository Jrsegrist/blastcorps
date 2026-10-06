/* Link check: every hd_code game file in one exe (make -C port linkall).
 * Maps and loads RDRAM, runs one pure function as a smoke test. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "rdram.h"

int32_t func_802AD7D4(int32_t sine);

void port_stub_hit(const char *name) {
    fprintf(stderr, "STUB called: %s\n", name);
    exit(3);
}

int main(int argc, char **argv) {
    if (rdram_map() || rdram_load(argc > 1 ? argv[1] : "baserom.us.v11.z64")) return 1;
    printf("asin(0x8000) = 0x%X, asin(0xFFFF) = 0x%X\n", (unsigned) func_802AD7D4(0x8000),
           (unsigned) func_802AD7D4(0xFFFF));
    return 0;
}
