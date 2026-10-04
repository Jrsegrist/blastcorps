#include "common.h"
#include <ultra64.h>

void alSynDelete(ALSynth *drvr) {
    drvr->head = NULL;
}
