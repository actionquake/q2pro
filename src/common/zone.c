/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "shared/shared.h"
#include "shared/list.h"
#include "common/common.h"
#include "common/zone.h"
#include "system/pthread.h"

#define Z_MAGIC     0x1d0d

typedef struct {
    uint16_t        magic;
    uint16_t        tag;        // for group free
    size_t          size;
    list_t          entry;
} zhead_t;

typedef struct {
    zhead_t     z;
    char        data[2];
} zstatic_t;

typedef struct {
    size_t      count;
    size_t      bytes;
} zstats_t;

static list_t       z_chain;
static zstats_t     z_stats[TAG_MAX];

// the render thread allocates too (lazy shader programs, scrap uploads);
// guards z_chain and z_stats. malloc and free run outside it, realloc
// inside, since the block is relinked.
static pthread_mutex_t  z_lock = PTHREAD_MUTEX_INITIALIZER;

#define S(d) \
    { .z = { .magic = Z_MAGIC, .tag = TAG_STATIC, .size = sizeof(zstatic_t) }, .data = d }

static const zstatic_t z_static[11] = {
    S("0"), S("1"), S("2"), S("3"), S("4"), S("5"), S("6"), S("7"), S("8"), S("9"), S("")
};

#undef S

static const char *const z_tagnames[TAG_MAX] = {
    "game",
    "static",
    "generic",
    "cmd",
    "cvar",
    "fs",
    "refresh",
    "ui",
    "server",
    "mvd",
    "sound",
    "cmodel"
};

#define TAG_INDEX(tag)  ((tag) < TAG_MAX ? (tag) : TAG_FREE)

static inline void Z_CountFree(const zhead_t *z)
{
    zstats_t *s = &z_stats[TAG_INDEX(z->tag)];
    s->count--;
    s->bytes -= z->size;
}

static inline void Z_CountAlloc(const zhead_t *z)
{
    zstats_t *s = &z_stats[TAG_INDEX(z->tag)];
    s->count++;
    s->bytes += z->size;
}

#define Z_Valid(z) \
    ((z)->magic == Z_MAGIC && (z)->tag != TAG_FREE)

#define Z_Validate(z) \
    Q_assert(Z_Valid(z))

// For walks under z_lock: a failed assert raises an error, and the error
// path frees memory, which would deadlock on the lock still held.
#define Z_ValidateLocked(z) \
    do { if (!Z_Valid(z)) { pthread_mutex_unlock(&z_lock); Z_Validate(z); } } while (0)

// walks (and validates) the whole chain
static size_t Z_CountTag(memtag_t tag, size_t *bytes)
{
    zhead_t *z;
    size_t count = 0;

    *bytes = 0;
    pthread_mutex_lock(&z_lock);
    LIST_FOR_EACH(zhead_t, z, &z_chain, entry) {
        Z_ValidateLocked(z);
        if (z->tag == tag || (tag == TAG_FREE && z->tag >= TAG_MAX)) {
            count++;
            *bytes += z->size;
        }
    }
    pthread_mutex_unlock(&z_lock);

    return count;
}

#if USE_TESTS
size_t Z_TagCount(memtag_t tag)
{
    size_t bytes;
    return Z_CountTag(tag, &bytes);
}
#endif

void Z_LeakTest(memtag_t tag)
{
    size_t numBytes, numLeaks = Z_CountTag(tag, &numBytes);

    if (numLeaks) {
        Com_WPrintf("************* Z_LeakTest *************\n"
                    "%s leaked %zu bytes of memory (%zu object%s)\n"
                    "**************************************\n",
                    z_tagnames[TAG_INDEX(tag)],
                    numBytes, numLeaks, numLeaks == 1 ? "" : "s");
    }
}

/*
========================
Z_Free
========================
*/
void Z_Free(void *ptr)
{
    zhead_t *z;

    if (!ptr) {
        return;
    }

    z = (zhead_t *)ptr - 1;

    Z_Validate(z);

    pthread_mutex_lock(&z_lock);
    Z_CountFree(z);
    if (z->tag != TAG_STATIC)
        List_Remove(&z->entry);
    pthread_mutex_unlock(&z_lock);

    if (z->tag != TAG_STATIC) {
        z->magic = 0xdead;
        z->tag = TAG_FREE;
        free(z);
    }
}

/*
========================
Z_Freep
========================
*/
void Z_Freep(void *ptr)
{
    void **p = ptr;

    Q_assert(p);
    if (*p) {
        Z_Free(*p);
        *p = NULL;
    }
}

/*
========================
Z_Realloc
========================
*/
void *Z_Realloc(void *ptr, size_t size)
{
    zhead_t *z;

    if (!ptr) {
        return Z_Malloc(size);
    }

    if (!size) {
        Z_Free(ptr);
        return NULL;
    }

    z = (zhead_t *)ptr - 1;

    Z_Validate(z);

    Q_assert(size <= INT_MAX);

    size += sizeof(*z);
    if (z->size == size) {
        return z + 1;
    }

    Q_assert(z->tag != TAG_STATIC);

    // list neighbours point at the old block until it is relinked
    pthread_mutex_lock(&z_lock);
    Z_CountFree(z);

    z = realloc(z, size);
    if (!z) {
        pthread_mutex_unlock(&z_lock);
        Com_Error(ERR_FATAL, "%s: couldn't realloc %zu bytes", __func__, size);
    }

    z->size = size;
    List_Relink(&z->entry);

    Z_CountAlloc(z);
    pthread_mutex_unlock(&z_lock);

    return z + 1;
}

void *Z_ReallocArray(void *ptr, size_t nmemb, size_t size, memtag_t tag)
{
    Q_assert(!size || nmemb <= INT_MAX / size);
    if (!ptr)
        return Z_TagMalloc(nmemb * size, tag);
    return Z_Realloc(ptr, nmemb * size);
}

/*
========================
Z_Stats_f
========================
*/
void Z_Stats_f(void)
{
    size_t bytes = 0, count = 0;
    zstats_t stats[TAG_MAX], *s;
    int i;

    pthread_mutex_lock(&z_lock);
    memcpy(stats, z_stats, sizeof(stats));
    pthread_mutex_unlock(&z_lock);

    Com_Printf("    bytes blocks name\n"
               "--------- ------ -------\n");

    for (i = 0, s = stats; i < TAG_MAX; i++, s++) {
        if (!s->count) {
            continue;
        }
        Com_Printf("%9zu %6zu %s\n", s->bytes, s->count, z_tagnames[i]);
        bytes += s->bytes;
        count += s->count;
    }

    Com_Printf("--------- ------ -------\n"
               "%9zu %6zu total\n",
               bytes, count);
}

/*
========================
Z_FreeTags
========================
*/
void Z_FreeTags(memtag_t tag)
{
    zhead_t *z, *n;
    list_t doomed;

    List_Init(&doomed);

    pthread_mutex_lock(&z_lock);
    LIST_FOR_EACH_SAFE(zhead_t, z, n, &z_chain, entry) {
        Z_ValidateLocked(z);
        if (z->tag == tag) {
            Z_CountFree(z);
            List_Remove(&z->entry);
            List_Append(&doomed, &z->entry);
        }
    }
    pthread_mutex_unlock(&z_lock);

    LIST_FOR_EACH_SAFE(zhead_t, z, n, &doomed, entry) {
        z->magic = 0xdead;
        z->tag = TAG_FREE;
        free(z);
    }
}

/*
========================
Z_TagMalloc
========================
*/
static void *Z_TagMallocInternal(size_t size, memtag_t tag, bool init)
{
    zhead_t *z;

    if (!size) {
        return NULL;
    }

    Q_assert(size <= INT_MAX);
    Q_assert(tag > TAG_FREE && tag <= UINT16_MAX);

    size += sizeof(*z);
    z = init ? calloc(1, size) : malloc(size);
    if (!z) {
        Com_Error(ERR_FATAL, "%s: couldn't allocate %zu bytes", __func__, size);
    }
    z->magic = Z_MAGIC;
    z->tag = tag;
    z->size = size;

#if USE_TESTS
    if (!init && z_perturb && z_perturb->integer) {
        memset(z + 1, z_perturb->integer, size - sizeof(*z));
    }
#endif

    pthread_mutex_lock(&z_lock);
    List_Insert(&z_chain, &z->entry);
    Z_CountAlloc(z);
    pthread_mutex_unlock(&z_lock);

    return z + 1;
}

void *Z_TagMalloc(size_t size, memtag_t tag)
{
    return Z_TagMallocInternal(size, tag, false);
}

void *Z_TagMallocz(size_t size, memtag_t tag)
{
    return Z_TagMallocInternal(size, tag, true);
}

void *Z_Malloc(size_t size)
{
    return Z_TagMalloc(size, TAG_GENERAL);
}

void *Z_Mallocz(size_t size)
{
    return Z_TagMallocz(size, TAG_GENERAL);
}

/*
========================
Z_Init
========================
*/
void Z_Init(void)
{
    List_Init(&z_chain);
}

/*
================
Z_TagCopyString
================
*/
char *Z_TagCopyString(const char *in, memtag_t tag)
{
    size_t len;

    if (!in) {
        return NULL;
    }

    len = strlen(in) + 1;
    return memcpy(Z_TagMalloc(len, tag), in, len);
}

/*
================
Z_CvarCopyString
================
*/
char *Z_CvarCopyString(const char *in)
{
    const zstatic_t *z;
    int i;

    if (!in) {
        return NULL;
    }

    if (!in[0]) {
        i = 10;
    } else if (!in[1] && Q_isdigit(in[0])) {
        i = in[0] - '0';
    } else {
        return Z_TagCopyString(in, TAG_CVAR);
    }

    // return static storage
    z = &z_static[i];
    Z_CountAlloc(&z->z);
    return (char *)z->data;
}
