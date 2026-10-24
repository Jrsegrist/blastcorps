/* Edited level files in the port (see port_mods.h). */
#include <windows.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rdram.h"
#include "load/port_load.h"
#include "load/port_mods.h"

static void log_stderr(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}
void (*port_mods_log_hook)(const char *fmt, ...) = log_stderr;
#define LOG port_mods_log_hook

/* The level files' ROM starts (lvl_load_file's romLagpRaw .. romLevel59Raw, tools_port/bcfiles romfs.py
 * LEVEL_ROM; US 1.0 and 1.1 share them): each is two gzip members back to back. */
static const uint32_t kLevelRom[60] = {
    0x4ACC10, 0x4A5660, 0x4B8960, 0x4BFD60, 0x4C3AC0, 0x4D5F90, 0x4E2F70, 0x4E4E80, 0x4E7C00, 0x4E8F70,
    0x4F5C10, 0x500520, 0x507E80, 0x511340, 0x523080, 0x52CD00, 0x532700, 0x53E9B0, 0x54A820, 0x552DE0,
    0x555000, 0x560E90, 0x5652D0, 0x56F3F0, 0x5721E0, 0x5736E0, 0x57A2C0, 0x580B60, 0x588CE0, 0x58BE80,
    0x597B80, 0x59B7D0, 0x5A5840, 0x5B0B10, 0x5B5A30, 0x5B8BB0, 0x5C4C80, 0x5CA9C0, 0x5CCF50, 0x5D1060,
    0x5DC830, 0x5E6EE0, 0x5EC800, 0x5F3A80, 0x6014B0, 0x60A710, 0x613AA0, 0x61DD70, 0x621AF0, 0x6269E0,
    0x630C30, 0x635700, 0x63CA10, 0x641F30, 0x644810, 0x646080, 0x647550, 0x654FC0, 0x660950, 0x665F80};
#define ROM_OBJ_TABLE 0x6EC4C0u   /* 512 BE u32 offsets of the buildings; an unused entry: the end */
#define MAX_LEVEL 0x100000u       /* (the largest inflated level is about 0x50000 bytes) */
#define MAX_GROWTH 0x10000u       /* an edited file may be at most this much larger than the ROM's */

typedef struct {
    uint8_t *orig;
    size_t origSize;
    uint8_t *edit;
    size_t editSize;
    char source[160];
    LmChange *modChanges;   /* the mod's changes (port_level_mod_changes) */
    int nModChanges;
    char modName[128];
    unsigned loads;
} LevelState;
static LevelState g_lv[60];

typedef struct {
    char name[128];
} ModInfo;
static ModInfo *g_mods;
static int g_nmods;

/* the hook's snapshot of what lies under the ROM file's bytes (PORT_LEVEL_PRE) */
static struct {
    int level;
    uint32_t dest, len;
    uint8_t *bytes;
    void *units;
} g_snap = {-1, 0, 0, NULL, NULL};

int port_level_original(int level, const uint8_t **file, size_t *size) {
    LevelState *s;
    size_t romSize = 0;
    uint8_t *rom = rom_bytes(&romSize);
    if (level < 0 || level >= 60 || rom == NULL) return -1;
    s = &g_lv[level];
    if (s->orig == NULL) {
        uint8_t *buf = (uint8_t *) malloc(MAX_LEVEL);
        uint32_t next = 0, next2 = 0;
        long a, b;
        if (buf == NULL) return -1;
        a = rom_gunzip_next(kLevelRom[level], buf, MAX_LEVEL, &next);
        b = a > 0 ? rom_gunzip_next(next, buf + a, MAX_LEVEL - (size_t) a, &next2) : -1;
        if (a <= 0 || b <= 0) {
            LOG("mods: level %d: the ROM has no level file at 0x%06X (a ROM with its files moved?)\n", level,
                (unsigned) kLevelRom[level]);
            free(buf);
            return -1;
        }
        s->origSize = (size_t) (a + b);
        s->orig = (uint8_t *) realloc(buf, s->origSize);
        if (s->orig == NULL) s->orig = buf;
    }
    if (file) *file = s->orig;
    if (size) *size = s->origSize;
    return 0;
}

int port_level_edited(int level, const uint8_t **file, size_t *size) {
    if (level < 0 || level >= 60 || g_lv[level].edit == NULL) return 0;
    if (file) *file = g_lv[level].edit;
    if (size) *size = g_lv[level].editSize;
    return 1;
}

const char *port_level_edit_source(int level) {
    return level >= 0 && level < 60 && g_lv[level].edit != NULL ? g_lv[level].source : NULL;
}

int port_level_set_edit(int level, const uint8_t *file, size_t size, const char *source) {
    LevelState *s;
    const uint8_t *orig;
    size_t origSize;
    if (level < 0 || level >= 60 || port_level_original(level, &orig, &origSize)) return -1;
    s = &g_lv[level];
    free(s->edit);
    s->edit = NULL;
    s->editSize = 0;
    s->source[0] = 0;
    if (file == NULL) {
        LOG("mods: level %d: the ROM's file again\n", level);
        return 0;
    }
    if (size > origSize + MAX_GROWTH) {
        LOG("mods: level %d: the edited file is 0x%X bytes, more than 0x%X over the ROM's: not used\n", level,
            (unsigned) size, MAX_GROWTH);
        return -1;
    }
    s->edit = (uint8_t *) malloc(size ? size : 1);
    if (s->edit == NULL) return -1;
    memcpy(s->edit, file, size);
    s->editSize = size;
    snprintf(s->source, sizeof s->source, "%s", source ? source : "an edit");
    {
        char sha[41];
        lm_sha1_hex(file, size, sha);
        LOG("mods: level %d: the game reads an edited file from its next load (%s; 0x%X bytes, the ROM's 0x%X; "
            "SHA-1 %s)\n", level, s->source, (unsigned) size, (unsigned) origSize, sha);
    }
    return 0;
}

int port_level_mod_changes(int level, const LmChange **changes, int *n, const char **modName) {
    if (level < 0 || level >= 60 || g_lv[level].modChanges == NULL) return 0;
    if (changes) *changes = g_lv[level].modChanges;
    if (n) *n = g_lv[level].nModChanges;
    if (modName) *modName = g_lv[level].modName;
    return 1;
}

unsigned port_level_edit_loads(int level) {
    return level >= 0 && level < 60 ? g_lv[level].loads : 0;
}

int port_level_check_objects(const uint8_t *file, size_t size, char *err, size_t errlen) {
    size_t romSize = 0;
    const uint8_t *rom = rom_bytes(&romSize);
    LmLevel *lv;
    uint32_t tab[512], end = 0;
    int i, n, r = 0;
    if (rom == NULL || romSize < ROM_OBJ_TABLE + 0x800) return 0;
    for (i = 0; i < 512; i++) {
        const uint8_t *p = rom + ROM_OBJ_TABLE + 4 * i;
        tab[i] = (uint32_t) p[0] << 24 | (uint32_t) p[1] << 16 | (uint32_t) p[2] << 8 | p[3];
        if (tab[i] > end) end = tab[i];
    }
    lv = lm_open(file, size, err, errlen);
    if (lv == NULL) return -1;
    n = lm_count(lv, LM_OBJECT);
    for (i = 0; i < n && r == 0; i++) {
        long t;
        lm_get(lv, LM_OBJECT, i, lm_field_index(LM_OBJECT, "type"), &t);
        if (t < 0 || t >= 512 || tab[t] >= end) {
            if (err) snprintf(err, errlen, "object %d: type 0x%lX has no building in the ROM's object table", i, t);
            r = -1;
        }
    }
    lm_free(lv);
    return r;
}

/* ---- mods ---- */
static FILE *open_utf8(const char *path, const char *mode) {
    wchar_t wp[4096], wm[8];
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wp, 4096) > 0 &&
        MultiByteToWideChar(CP_UTF8, 0, mode, -1, wm, 8) > 0)
        return _wfopen(wp, wm);
    return fopen(path, mode);
}

static char *read_text(const char *path, size_t *len) {
    FILE *f = open_utf8(path, "rb");
    long n;
    char *p;
    if (f == NULL) return NULL;
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (n < 0 || n > 0x1000000 || (p = (char *) malloc((size_t) n + 1)) == NULL) {
        fclose(f);
        return NULL;
    }
    if (fread(p, 1, (size_t) n, f) != (size_t) n) {
        fclose(f);
        free(p);
        return NULL;
    }
    fclose(f);
    p[n] = 0;
    *len = (size_t) n;
    return p;
}

int port_mods_add(const char *path, char *err, size_t errlen) {
    LmMod m;
    size_t len = 0;
    char *text = read_text(path, &len), e[512], name[128];
    uint8_t *out[60];
    size_t outSize[60];
    int i, k;
    if (text == NULL) {
        snprintf(err, errlen, "can't read %s", path);
        return -1;
    }
    if (lm_mod_parse(text, len, &m, e, sizeof e)) {
        free(text);
        snprintf(err, errlen, "%s: %s", path, e);
        return -1;
    }
    free(text);
    if (m.name[0]) {
        snprintf(name, sizeof name, "%s", m.name);
    } else {
        const char *b = path + strlen(path);
        while (b > path && b[-1] != '\\' && b[-1] != '/') b--;
        snprintf(name, sizeof name, "%s", b);
    }
    memset(out, 0, sizeof out);
    for (i = 0; i < m.nlevels; i++) {
        const LmModLevel *ml = &m.levels[i];
        const uint8_t *orig;
        size_t origSize;
        if (port_level_original(ml->level, &orig, &origSize)) {
            snprintf(err, errlen, "%s: level %d: the ROM's level file can't be read", name, ml->level);
            goto fail;
        }
        if (g_lv[ml->level].edit != NULL) {
            snprintf(err, errlen, "%s: level %d is already changed by %s (two mods can't change the same level)",
                     name, ml->level, g_lv[ml->level].source);
            goto fail;
        }
        if (lm_mod_apply(ml, orig, origSize, &out[ml->level], &outSize[ml->level], e, sizeof e)) {
            snprintf(err, errlen, "%s: %s", name, e);
            goto fail;
        }
        if (port_level_check_objects(out[ml->level], outSize[ml->level], e, sizeof e)) {
            snprintf(err, errlen, "%s: level %d: %s", name, ml->level, e);
            goto fail;
        }
        if (outSize[ml->level] > origSize + MAX_GROWTH) {
            snprintf(err, errlen, "%s: level %d grows by more than 0x%X bytes", name, ml->level, MAX_GROWTH);
            goto fail;
        }
    }
    /* all good: in place */
    for (i = 0; i < m.nlevels; i++) {
        const LmModLevel *ml = &m.levels[i];
        LevelState *s = &g_lv[ml->level];
        char src[200];
        snprintf(src, sizeof src, "mod %s", name);
        port_level_set_edit(ml->level, out[ml->level], outSize[ml->level], src);
        free(s->modChanges);
        s->modChanges = (LmChange *) malloc(sizeof(LmChange) * (size_t) (ml->nchanges ? ml->nchanges : 1));
        if (s->modChanges) memcpy(s->modChanges, ml->changes, sizeof(LmChange) * (size_t) ml->nchanges);
        s->nModChanges = s->modChanges ? ml->nchanges : 0;
        snprintf(s->modName, sizeof s->modName, "%s", name);
    }
    {
        ModInfo *q = (ModInfo *) realloc(g_mods, sizeof(ModInfo) * (size_t) (g_nmods + 1));
        if (q != NULL) {
            g_mods = q;
            snprintf(g_mods[g_nmods++].name, sizeof g_mods[0].name, "%s", name);
        }
    }
    LOG("mods: %s (%s): %d level%s\n", name, path, m.nlevels, m.nlevels == 1 ? "" : "s");
    for (k = 0; k < 60; k++) free(out[k]);
    lm_mod_free(&m);
    return 0;
fail:
    for (k = 0; k < 60; k++) free(out[k]);
    lm_mod_free(&m);
    return -1;
}

int port_mods_count(void) {
    return g_nmods;
}

const char *port_mods_name(int i) {
    return i >= 0 && i < g_nmods ? g_mods[i].name : "";
}

int port_mods_write_levels(const char *dir) {
    int level, n = 0;
    for (level = 0; level < 60; level++) {
        char path[1024];
        FILE *f;
        if (g_lv[level].edit == NULL) continue;
        snprintf(path, sizeof path, "%s\\level_%02d.bin", dir, level);
        f = open_utf8(path, "wb");
        if (f == NULL || fwrite(g_lv[level].edit, 1, g_lv[level].editSize, f) != g_lv[level].editSize) {
            if (f) fclose(f);
            LOG("mods: can't write %s\n", path);
            return -1;
        }
        fclose(f);
        n++;
    }
    return n;
}

/* ---- the game's hook (game/port.h) ---- */
void port_level_pre(int level, unsigned int dest) {
    LevelState *s;
    if (level < 0 || level >= 60 || g_lv[level].edit == NULL) return;
    s = &g_lv[level];
    /* hd_rom_load writes the ROM's file (origSize bytes) here; an edited file that is shorter must leave the rest
     * as it was, as a ROM with the edited file in it would: keep it (and its byte-order units) */
    free(g_snap.bytes);
    free(g_snap.units);
    g_snap.level = level;
    g_snap.dest = dest;
    g_snap.len = (uint32_t) s->origSize;
    if (dest < RDRAM_BASE || dest >= RDRAM_BASE + RDRAM_SIZE) g_snap.len = 0;
    else if (g_snap.len > RDRAM_BASE + RDRAM_SIZE - dest) g_snap.len = RDRAM_BASE + RDRAM_SIZE - dest;
    g_snap.bytes = (uint8_t *) malloc(g_snap.len ? g_snap.len : 1);
    if (g_snap.bytes && g_snap.len) memcpy(g_snap.bytes, (void *) (uintptr_t) dest, g_snap.len);
    g_snap.units = g_snap.len ? port_unit_save(dest, g_snap.len) : NULL;
}

void port_level_post(int level, unsigned int rom, unsigned int dest, unsigned int *size) {
    LevelState *s;
    uint32_t n;
    if (level < 0 || level >= 60 || g_lv[level].edit == NULL) return;
    s = &g_lv[level];
    n = (uint32_t) s->editSize;
    if (dest < RDRAM_BASE || n > RDRAM_BASE + RDRAM_SIZE - dest) {
        LOG("mods: level %d: no room for the edited file at 0x%08X: the ROM's is used\n", level, dest);
        return;
    }
    if (*size != s->origSize)
        LOG("mods: level %d: the game inflated 0x%X bytes, the ROM's file is 0x%X\n", level, *size,
            (unsigned) s->origSize);
    port_gfx_forget(dest, *size > n ? *size : n);
    if (g_snap.level == level && g_snap.dest == dest && g_snap.bytes != NULL && n < g_snap.len) {
        memcpy((void *) (uintptr_t) (dest + n), g_snap.bytes + n, g_snap.len - n);
        if (g_snap.units != NULL) {
            /* (the units of the whole range back, then the edited file's own: port_on_load marks them) */
            port_unit_restore(dest, g_snap.units, g_snap.len);
        }
    }
    free(g_snap.bytes);
    free(g_snap.units);
    g_snap.bytes = NULL, g_snap.units = NULL, g_snap.level = -1;
    memcpy((void *) (uintptr_t) dest, s->edit, n);
    *size = n;
    port_on_load(rom, dest, n);   /* swapped as the ROM's file was */
    s->loads++;
    LOG("mods: level %d: the edited file (%s, 0x%X bytes) loaded at 0x%08X\n", level, s->source, n, dest);
}
