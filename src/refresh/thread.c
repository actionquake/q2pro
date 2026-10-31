/*
Copyright (C) 2026

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

/*
Render thread (r_thread 1).

Presenting a frame blocks: the swap waits for a free drawable, and how
long depends on the compositor and on everything else using the GPU. On
one thread that stall delays reading input, reading packets and sending
commands. Here the main thread records each frame's draw calls into a
command buffer and the render thread replays them and swaps, so the main
thread never waits on the GPU.

- The render thread only works on what the main thread posts, so once
  RT_Wait or RT_Acquire returns the main thread has the renderer to
  itself until its next submit.
- At most one frame is in flight. The client's main loop starts a frame
  only when the render thread is idle (R_FrameReady), so frames are
  built as late as possible; other callers (loading screens, timedemo)
  wait for the previous frame in rt_submit.
- Everything that is not per-frame drawing (registration, cvar callbacks,
  console commands) runs on the main thread after RT_Acquire has moved
  the GL context over, even in the middle of recording a frame; the
  context moves back at the next submit.
- Refdef arrays are deep-copied: the client rewrites them while the
  previous frame is still drawing, and the renderer writes dlights.
- The render thread must not write shared state: it prints and raises
  errors through the worker queue (Com_EnterWorker) and gets cvar clamps
  without the write.
*/

#define RT_IMPLEMENTATION
#include "shared/shared.h"
#include "common/cmodel.h"
#include "common/common.h"
#include "common/cvar.h"
#include "common/net/net.h"
#include "common/zone.h"
#include "client/client.h"
#include "client/video.h"
#include "system/pthread.h"
#include "thread.h"

#ifdef __APPLE__
#include <pthread/qos.h>
#endif

unsigned r_eventtime;

typedef enum {
    RC_BEGIN_FRAME,
    RC_END_FRAME,
    RC_RENDER_FRAME,
    RC_CLEAR_COLOR,
    RC_SET_ALPHA,
    RC_SET_COLOR,
    RC_SET_CLIP,
    RC_SET_SCALE,
    RC_DRAW_CHAR,
    RC_DRAW_STRING,
    RC_DRAW_PIC,
    RC_DRAW_STRETCH_PIC,
    RC_DRAW_KEEP_ASPECT_PIC,
    RC_DRAW_STRETCH_RAW,
    RC_TILE_CLEAR,
    RC_DRAW_FILL8,
    RC_DRAW_FILL32,
} rcmd_type_t;

typedef struct {
    uint32_t    type;
    uint32_t    size;       // whole command including payload, aligned
    int         x, y, w, h;
    int         i;          // flags, char, palette index or string length
    uint32_t    u;          // color, event time, clip present
    float       f;
    qhandle_t   pic;
    clipRect_t  clip;
} rcmd_t;

#define RCMD_ALIGN      16
#define RCMD_HEADER     Q_ALIGN(sizeof(rcmd_t), RCMD_ALIGN)

typedef struct {
    byte    *data;
    size_t  size, cursize;
} arena_t;

typedef enum {
    JOB_FRAME,
    JOB_RELEASE,    // give up the GL context
    JOB_QUIT,
} job_t;

typedef enum {
    OWNER_MAIN,
    OWNER_NONE,
    OWNER_RENDER,
} owner_t;

static struct {
    bool            running;
    pthread_t       thread;
    pthread_mutex_t lock;
    pthread_cond_t  work;
    pthread_cond_t  idle;

    // under lock
    bool            busy;
    job_t           job;
    int             exec;       // arena the render thread replays
    vec3_t          light;      // R_LightPoint at the last world view

    // the render thread writes these only while busy, the main thread
    // only while it is idle (busy/idle protocol)
    owner_t         owner;
    bool            failed;     // render thread couldn't take the context

    // main thread only
    arena_t         arenas[2];
    int             rec;        // arena the main thread records into
    int             depth;      // R_BeginFrame nesting
    unsigned        framenum;   // com_framenum when the outer frame began
    unsigned        finished;   // outer frames ended (tests)
} rt = {
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .work = PTHREAD_COND_INITIALIZER,
    .idle = PTHREAD_COND_INITIALIZER,
};

static q_thread_local bool rt_self;

static cvar_t *r_thread;

/*
===============================================================================

COMMAND BUFFER

===============================================================================
*/

static rcmd_t *rt_cmd(rcmd_type_t type, size_t extra)
{
    arena_t *a = &rt.arenas[rt.rec];
    size_t size = RCMD_HEADER + Q_ALIGN(extra, RCMD_ALIGN);
    rcmd_t *c;

    if (a->cursize + size > a->size) {
        size_t newsize = max(a->size * 2, a->cursize + size);
        a->data = Z_Realloc(a->data, max(newsize, 0x10000));
        a->size = max(newsize, 0x10000);
    }

    c = (rcmd_t *)(a->data + a->cursize);
    a->cursize += size;

    memset(c, 0, sizeof(*c));
    c->type = type;
    c->size = size;
    return c;
}

static inline byte *rt_payload(const rcmd_t *c)
{
    return (byte *)c + RCMD_HEADER;
}

static void rt_record_refdef(const refdef_t *fd, const void *dbg)
{
    size_t ents = Q_ALIGN(fd->num_entities * sizeof(entity_t), RCMD_ALIGN);
    size_t dls = Q_ALIGN(fd->num_dlights * sizeof(dlight_t), RCMD_ALIGN);
    size_t parts = Q_ALIGN(fd->num_particles * sizeof(particle_t), RCMD_ALIGN);
    size_t styles = fd->lightstyles ? Q_ALIGN(sizeof(lightstyle_t) * MAX_LIGHTSTYLES, RCMD_ALIGN) : 0;
    size_t areas = fd->areabits ? Q_ALIGN(MAX_MAP_AREA_BYTES, RCMD_ALIGN) : 0;
    size_t head = Q_ALIGN(sizeof(refdef_t), RCMD_ALIGN);
#if DEBUG_DRAWING
    const debug_frame_t *df = dbg;
    size_t dhead = Q_ALIGN(sizeof(*df), RCMD_ALIGN);
    size_t dboxes = Q_ALIGN(df->num_boxes * sizeof(draw_boxes_t), RCMD_ALIGN);
    size_t darrows = Q_ALIGN(df->num_arrows * sizeof(draw_arrows_t), RCMD_ALIGN);
#else
    size_t dhead = 0, dboxes = 0, darrows = 0;
#endif
    rcmd_t *c = rt_cmd(RC_RENDER_FRAME, head + ents + dls + parts + styles + areas +
                       dhead + dboxes + darrows);
    byte *p = rt_payload(c);

    c->u = com_eventTime;

    // array pointers are rebuilt at replay; non-NULL marks presence
    memcpy(p, fd, sizeof(*fd));
    p += head;
    memcpy(p, fd->entities, fd->num_entities * sizeof(entity_t));
    p += ents;
    memcpy(p, fd->dlights, fd->num_dlights * sizeof(dlight_t));
    p += dls;
    memcpy(p, fd->particles, fd->num_particles * sizeof(particle_t));
    p += parts;
    if (styles) {
        memcpy(p, fd->lightstyles, sizeof(lightstyle_t) * MAX_LIGHTSTYLES);
        p += styles;
    }
    if (areas) {
        memcpy(p, fd->areabits, MAX_MAP_AREA_BYTES);
        p += areas;
    }
#if DEBUG_DRAWING
    memcpy(p, df, sizeof(*df));
    p += dhead;
    memcpy(p, df->boxes, df->num_boxes * sizeof(draw_boxes_t));
    p += dboxes;
    memcpy(p, df->arrows, df->num_arrows * sizeof(draw_arrows_t));
#endif
}

static void rt_replay_refdef(const rcmd_t *c)
{
    const byte *p = rt_payload(c);
    refdef_t fd;
    vec3_t light;

    memcpy(&fd, p, sizeof(fd));
    p += Q_ALIGN(sizeof(refdef_t), RCMD_ALIGN);

    fd.entities = fd.num_entities ? (entity_t *)p : NULL;
    p += Q_ALIGN(fd.num_entities * sizeof(entity_t), RCMD_ALIGN);
    fd.dlights = fd.num_dlights ? (dlight_t *)p : NULL;
    p += Q_ALIGN(fd.num_dlights * sizeof(dlight_t), RCMD_ALIGN);
    fd.particles = fd.num_particles ? (particle_t *)p : NULL;
    p += Q_ALIGN(fd.num_particles * sizeof(particle_t), RCMD_ALIGN);
    if (fd.lightstyles) {
        fd.lightstyles = (lightstyle_t *)p;
        p += Q_ALIGN(sizeof(lightstyle_t) * MAX_LIGHTSTYLES, RCMD_ALIGN);
    }
    if (fd.areabits) {
        fd.areabits = (byte *)p;
        p += Q_ALIGN(MAX_MAP_AREA_BYTES, RCMD_ALIGN);
    }
#if DEBUG_DRAWING
    memcpy(&r_debugframe, p, sizeof(r_debugframe));
    p += Q_ALIGN(sizeof(r_debugframe), RCMD_ALIGN);
    r_debugframe.boxes = (draw_boxes_t *)p;
    p += Q_ALIGN(r_debugframe.num_boxes * sizeof(draw_boxes_t), RCMD_ALIGN);
    r_debugframe.arrows = (draw_arrows_t *)p;
#endif

    r_eventtime = c->u;
    RB_RenderFrame(&fd);

    if (!(fd.rdflags & RDF_NOWORLDMODEL)) {
        RB_LightPoint(fd.vieworg, light);
        pthread_mutex_lock(&rt.lock);
        VectorCopy(light, rt.light);
        pthread_mutex_unlock(&rt.lock);
    }
}

static void rt_replay(const arena_t *a)
{
    const byte *p = a->data, *end = a->data + a->cursize;

    while (p < end) {
        const rcmd_t *c = (const rcmd_t *)p;

        switch (c->type) {
        case RC_BEGIN_FRAME:
            RB_BeginFrame();
            break;
        case RC_END_FRAME:
            RB_EndFrame();
            break;
        case RC_RENDER_FRAME:
            rt_replay_refdef(c);
            break;
        case RC_CLEAR_COLOR:
            RB_ClearColor();
            break;
        case RC_SET_ALPHA:
            RB_SetAlpha(c->f);
            break;
        case RC_SET_COLOR:
            RB_SetColor(c->u);
            break;
        case RC_SET_CLIP:
            RB_SetClipRect(c->u ? &c->clip : NULL);
            break;
        case RC_SET_SCALE:
            RB_SetScale(c->f);
            break;
        case RC_DRAW_CHAR:
            RB_DrawChar(c->x, c->y, c->w, c->i, c->pic);
            break;
        case RC_DRAW_STRING:
            RB_DrawString(c->x, c->y, c->w, c->i, (const char *)rt_payload(c), c->pic);
            break;
        case RC_DRAW_PIC:
            RB_DrawPic(c->x, c->y, c->pic);
            break;
        case RC_DRAW_STRETCH_PIC:
            RB_DrawStretchPic(c->x, c->y, c->w, c->h, c->pic);
            break;
        case RC_DRAW_KEEP_ASPECT_PIC:
            RB_DrawKeepAspectPic(c->x, c->y, c->w, c->h, c->pic);
            break;
        case RC_DRAW_STRETCH_RAW:
            RB_DrawStretchRaw(c->x, c->y, c->w, c->h);
            break;
        case RC_TILE_CLEAR:
            RB_TileClear(c->x, c->y, c->w, c->h, c->pic);
            break;
        case RC_DRAW_FILL8:
            RB_DrawFill8(c->x, c->y, c->w, c->h, c->i);
            break;
        case RC_DRAW_FILL32:
            RB_DrawFill32(c->x, c->y, c->w, c->h, c->u);
            break;
        default:
            Q_assert(!"bad render command");
        }

        p += c->size;
    }
}

/*
===============================================================================

THREAD

===============================================================================
*/

// owner only changes when make_current succeeds
static bool rt_take_context(owner_t self)
{
    if (rt.owner == self)
        return true;
    if (!vid->make_current(true))
        return false;
    rt.owner = self;
    return true;
}

static void rt_drop_context(owner_t self)
{
    if (rt.owner == self && vid->make_current(false))
        rt.owner = OWNER_NONE;
}

static void rt_run(job_t job)
{
    switch (job) {
    case JOB_FRAME:
        // without the context, drop the frame; the main thread stops
        // the render thread at the next frame (rt_update)
        if (!rt_take_context(OWNER_RENDER)) {
            rt.failed = true;
            break;
        }
        rt_replay(&rt.arenas[rt.exec]);
        break;
    case JOB_RELEASE:
    case JOB_QUIT:
        rt_drop_context(OWNER_RENDER);
        break;
    }
}

static void *rt_main(void *arg)
{
    jmp_buf abort_point;
    job_t job;

    rt_self = true;
    Com_EnterWorker(&abort_point);

    pthread_mutex_lock(&rt.lock);
    while (1) {
        while (!rt.busy)
            pthread_cond_wait(&rt.work, &rt.lock);
        job = rt.job;
        pthread_mutex_unlock(&rt.lock);

        // A Com_Error lands here: the rest of the frame is dropped
        // without a swap, GL state is left as it was, and the main
        // thread re-raises the error at its next Qcommon_Frame.
        if (!setjmp(abort_point))
            rt_run(job);

        pthread_mutex_lock(&rt.lock);
        rt.busy = false;
        pthread_cond_signal(&rt.idle);    // only the main thread waits
        // a frame may be waiting for this one; don't let the main loop
        // sleep through it (see CL_ProcessEvents)
        if (job == JOB_FRAME)
            NET_Wake();
        if (job == JOB_QUIT)
            break;
    }
    pthread_mutex_unlock(&rt.lock);

    return NULL;
}

static void rt_wait_locked(void)
{
    while (rt.busy)
        pthread_cond_wait(&rt.idle, &rt.lock);
}

// render thread must be idle
static void rt_post_locked(job_t job)
{
    rt.job = job;
    rt.busy = true;
    pthread_cond_signal(&rt.work);
}

void RT_Wait(void)
{
    if (!rt.running || rt_self)
        return;

    pthread_mutex_lock(&rt.lock);
    rt_wait_locked();
    pthread_mutex_unlock(&rt.lock);
}

void RT_Acquire(void)
{
    // only the main thread sets or clears OWNER_MAIN, so this unlocked
    // read can't race when it matters
    if (!rt.running || rt_self || rt.owner == OWNER_MAIN)
        return;

    pthread_mutex_lock(&rt.lock);
    rt_wait_locked();
    if (rt.owner == OWNER_RENDER) {
        rt_post_locked(JOB_RELEASE);
        rt_wait_locked();
    }
    pthread_mutex_unlock(&rt.lock);

    // GL from here on would run without a context
    if (rt.owner == OWNER_RENDER || !rt_take_context(OWNER_MAIN))
        Com_Error(ERR_FATAL, "Couldn't move the GL context to the main thread");
}

static void rt_submit(void)
{
    pthread_mutex_lock(&rt.lock);
    rt_wait_locked();
    pthread_mutex_unlock(&rt.lock);

    rt_drop_context(OWNER_MAIN);

    pthread_mutex_lock(&rt.lock);
    rt.exec = rt.rec;
    rt_post_locked(JOB_FRAME);
    pthread_mutex_unlock(&rt.lock);

    rt.rec ^= 1;
    rt.arenas[rt.rec].cursize = 0;
}

static bool rt_start(void)
{
    int ret;

    // the context is current here already, so this only asks whether
    // the driver can hand it over
    if (!vid || !vid->make_current || !vid->make_current(true))
        return false;

    // the context is current on the main thread until the first submit
    rt.owner = OWNER_MAIN;
    rt.busy = false;
    rt.failed = false;
    rt.arenas[0].cursize = rt.arenas[1].cursize = 0;
    rt.rec = 0;

#ifdef __APPLE__
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_set_qos_class_np(&attr, QOS_CLASS_USER_INTERACTIVE, 0);
    ret = pthread_create(&rt.thread, &attr, rt_main, NULL);
    pthread_attr_destroy(&attr);
#else
    ret = pthread_create(&rt.thread, NULL, rt_main, NULL);
#endif
    if (ret) {
        Com_EPrintf("Couldn't create render thread: %s\n", strerror(ret));
        return false;
    }

    rt.running = true;
    Com_Printf("Render thread started.\n");
    return true;
}

static void rt_stop(void)
{
    if (!rt.running)
        return;

    RT_Acquire();

    pthread_mutex_lock(&rt.lock);
    rt_post_locked(JOB_QUIT);
    rt_wait_locked();
    pthread_mutex_unlock(&rt.lock);

    pthread_join(rt.thread, NULL);
    rt.running = false;

    // State changes recorded outside a frame still have to happen. Called
    // from R_Shutdown mid-frame, this also draws the partial frame here.
    rt_replay(&rt.arenas[rt.rec]);
    rt.arenas[rt.rec].cursize = 0;

    Com_Printf("Render thread stopped.\n");
}

static void rt_update(void)
{
    bool want = r_thread && r_thread->integer > 0;
    bool failed;

    if (rt.running) {
        pthread_mutex_lock(&rt.lock);
        failed = rt.failed;
        pthread_mutex_unlock(&rt.lock);
        if (failed) {
            Com_EPrintf("Render thread couldn't make the GL context current, "
                        "rendering on the main thread.\n");
            rt_stop();
            Cvar_Set("r_thread", "0");
            return;
        }
    }

    if (want && !rt.running) {
        if (!rt_start())
            Cvar_Set("r_thread", "0");
    } else if (!want && rt.running) {
        rt_stop();
    }
}

/*
===============================================================================

CLIENT ENTRY POINTS

===============================================================================
*/

#if USE_TESTS
unsigned R_FramesFinished(void)
{
    return rt.finished;
}
#endif

bool R_FrameReady(void)
{
    bool ready;

    if (!rt.running)
        return true;

    pthread_mutex_lock(&rt.lock);
    ready = !rt.busy;
    pthread_mutex_unlock(&rt.lock);
    return ready;
}

void R_WaitIdle(void)
{
    RT_Wait();
}

bool R_Init(bool total)
{
    if (!r_thread) {
        // on by default where it has had the most play-testing
#ifdef __APPLE__
        r_thread = Cvar_Get("r_thread", "1", 0);
#else
        r_thread = Cvar_Get("r_thread", "0", 0);
#endif
    }
    return RB_Init(total);
}

void R_Shutdown(bool total)
{
    rt_stop();
    rt.depth = 0;
    RB_Shutdown(total);
}

void R_BeginRegistration(const char *map)
{
    RT_Acquire();
    RB_BeginRegistration(map);
}

qhandle_t R_RegisterModel(const char *name)
{
    RT_Acquire();
    return RB_RegisterModel(name);
}

qhandle_t R_RegisterImage(const char *name, imagetype_t type, imageflags_t flags)
{
    qhandle_t handle;

    // HUD code registers pics every frame; known ones need no GL
    if (rt.running && IMG_LookupHandle(name, type, flags, &handle))
        return handle;

    RT_Acquire();
    return RB_RegisterImage(name, type, flags);
}

void R_SetSky(const char *name, float rotate, bool autorotate, const vec3_t axis)
{
    RT_Acquire();
    RB_SetSky(name, rotate, autorotate, axis);
}

void R_EndRegistration(void)
{
    RT_Acquire();
    RB_EndRegistration();
}

void R_ModeChanged(int width, int height, int flags)
{
    RT_Wait();
    RB_ModeChanged(width, height, flags);
}

r_opengl_config_t R_GetGLConfig(void)
{
    RT_Wait();
    return RB_GetGLConfig();
}

void R_LightPoint(const vec3_t origin, vec3_t light)
{
    if (!rt.running) {
        RB_LightPoint(origin, light);
        return;
    }

    // computed by the render thread at the last world view rendered,
    // which is what the client asks for (V_SetLightLevel)
    pthread_mutex_lock(&rt.lock);
    VectorCopy(rt.light, light);
    pthread_mutex_unlock(&rt.lock);
}

void R_UpdateRawPic(int pic_w, int pic_h, const uint32_t *pic)
{
    RT_Acquire();
    RB_UpdateRawPic(pic_w, pic_h, pic);
}

void R_BeginFrame(void)
{
    // A Com_Error between R_BeginFrame and R_EndFrame unwinds to the top
    // of Qcommon_Frame, which then starts a new com_framenum. Forget the
    // abandoned frame, or it would never be submitted. What it recorded
    // replays with the next frame, as an unfinished frame would without
    // the render thread.
    if (rt.depth && rt.framenum != com_framenum)
        rt.depth = 0;

    if (!rt.depth) {
        rt_update();
        rt.framenum = com_framenum;
    }

    rt.depth++;

    if (!rt.running) {
        RB_BeginFrame();
        return;
    }

    rt_cmd(RC_BEGIN_FRAME, 0);
}

void R_EndFrame(void)
{
    if (rt.depth > 0)
        rt.depth--;
    if (!rt.depth)
        rt.finished++;

    if (!rt.running) {
        RB_EndFrame();
        return;
    }

    rt_cmd(RC_END_FRAME, 0);

    if (!rt.depth)
        rt_submit();
}

void R_RenderFrame(const refdef_t *fd)
{
    void *dbg = NULL;
#if DEBUG_DRAWING
    debug_frame_t df;

    GL_DebugDrawSnapshot(&df);
    dbg = &df;
#endif

    if (!rt.running) {
#if DEBUG_DRAWING
        r_debugframe = df;
#endif
        r_eventtime = com_eventTime;
        RB_RenderFrame(fd);
        return;
    }

    rt_record_refdef(fd, dbg);
}

void R_ClearColor(void)
{
    if (!rt.running) {
        RB_ClearColor();
        return;
    }

    rt_cmd(RC_CLEAR_COLOR, 0);
}

void R_SetAlpha(float alpha)
{
    if (!rt.running) {
        RB_SetAlpha(alpha);
        return;
    }

    rt_cmd(RC_SET_ALPHA, 0)->f = alpha;
}

void R_SetColor(uint32_t color)
{
    if (!rt.running) {
        RB_SetColor(color);
        return;
    }

    rt_cmd(RC_SET_COLOR, 0)->u = color;
}

void R_SetClipRect(const clipRect_t *clip)
{
    rcmd_t *c;

    if (!rt.running) {
        RB_SetClipRect(clip);
        return;
    }

    c = rt_cmd(RC_SET_CLIP, 0);
    if (clip) {
        c->u = 1;
        c->clip = *clip;
    }
}

void R_SetScale(float scale)
{
    if (!rt.running) {
        RB_SetScale(scale);
        return;
    }

    rt_cmd(RC_SET_SCALE, 0)->f = scale;
}

void R_DrawChar(int x, int y, int flags, int ch, qhandle_t font)
{
    rcmd_t *c;

    if (!rt.running) {
        RB_DrawChar(x, y, flags, ch, font);
        return;
    }

    c = rt_cmd(RC_DRAW_CHAR, 0);
    c->x = x;
    c->y = y;
    c->w = flags;
    c->i = ch;
    c->pic = font;
}

int R_DrawString(int x, int y, int flags, size_t maxChars, const char *string, qhandle_t font)
{
    size_t len;
    rcmd_t *c;

    if (!rt.running)
        return RB_DrawString(x, y, flags, maxChars, string, font);

    len = strnlen(string, maxChars);
    c = rt_cmd(RC_DRAW_STRING, len + 1);
    c->x = x;
    c->y = y;
    c->w = flags;
    c->i = len;
    c->pic = font;
    memcpy(rt_payload(c), string, len);
    rt_payload(c)[len] = 0;

    // same advance RB_DrawString returns
    return x + len * CONCHAR_WIDTH;
}

static rcmd_t *rt_draw_rect(rcmd_type_t type, int x, int y, int w, int h, qhandle_t pic)
{
    rcmd_t *c = rt_cmd(type, 0);

    c->x = x;
    c->y = y;
    c->w = w;
    c->h = h;
    c->pic = pic;
    return c;
}

void R_DrawPic(int x, int y, qhandle_t pic)
{
    if (!rt.running) {
        RB_DrawPic(x, y, pic);
        return;
    }

    rt_draw_rect(RC_DRAW_PIC, x, y, 0, 0, pic);
}

void R_DrawStretchPic(int x, int y, int w, int h, qhandle_t pic)
{
    if (!rt.running) {
        RB_DrawStretchPic(x, y, w, h, pic);
        return;
    }

    rt_draw_rect(RC_DRAW_STRETCH_PIC, x, y, w, h, pic);
}

void R_DrawKeepAspectPic(int x, int y, int w, int h, qhandle_t pic)
{
    if (!rt.running) {
        RB_DrawKeepAspectPic(x, y, w, h, pic);
        return;
    }

    rt_draw_rect(RC_DRAW_KEEP_ASPECT_PIC, x, y, w, h, pic);
}

void R_DrawStretchRaw(int x, int y, int w, int h)
{
    if (!rt.running) {
        RB_DrawStretchRaw(x, y, w, h);
        return;
    }

    rt_draw_rect(RC_DRAW_STRETCH_RAW, x, y, w, h, 0);
}

void R_TileClear(int x, int y, int w, int h, qhandle_t pic)
{
    if (!rt.running) {
        RB_TileClear(x, y, w, h, pic);
        return;
    }

    rt_draw_rect(RC_TILE_CLEAR, x, y, w, h, pic);
}

void R_DrawFill8(int x, int y, int w, int h, int c)
{
    if (!rt.running) {
        RB_DrawFill8(x, y, w, h, c);
        return;
    }

    rt_draw_rect(RC_DRAW_FILL8, x, y, w, h, 0)->i = c;
}

void R_DrawFill32(int x, int y, int w, int h, uint32_t color)
{
    if (!rt.running) {
        RB_DrawFill32(x, y, w, h, color);
        return;
    }

    rt_draw_rect(RC_DRAW_FILL32, x, y, w, h, 0)->u = color;
}
