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

#pragma once

// Render thread (r_thread). thread.c implements the client-facing R_*
// entry points below: per-frame drawing is recorded and replayed on the
// render thread, everything else runs on the main thread with the GL
// context handed over. Inside the refresh module the real
// implementations are named RB_* (mapped below; thread.c opts out).

#ifndef RT_IMPLEMENTATION
#define R_Init              RB_Init
#define R_Shutdown          RB_Shutdown
#define R_BeginRegistration RB_BeginRegistration
#define R_RegisterModel     RB_RegisterModel
#define R_RegisterImage     RB_RegisterImage
#define R_SetSky            RB_SetSky
#define R_EndRegistration   RB_EndRegistration
#define R_RenderFrame       RB_RenderFrame
#define R_ClearColor        RB_ClearColor
#define R_SetAlpha          RB_SetAlpha
#define R_SetColor          RB_SetColor
#define R_SetClipRect       RB_SetClipRect
#define R_SetScale          RB_SetScale
#define R_DrawChar          RB_DrawChar
#define R_DrawString        RB_DrawString
#define R_DrawPic           RB_DrawPic
#define R_DrawStretchPic    RB_DrawStretchPic
#define R_DrawKeepAspectPic RB_DrawKeepAspectPic
#define R_DrawStretchRaw    RB_DrawStretchRaw
#define R_UpdateRawPic      RB_UpdateRawPic
#define R_TileClear         RB_TileClear
#define R_DrawFill8         RB_DrawFill8
#define R_DrawFill32        RB_DrawFill32
#define R_BeginFrame        RB_BeginFrame
#define R_EndFrame          RB_EndFrame
#define R_ModeChanged       RB_ModeChanged
#define R_GetGLConfig       RB_GetGLConfig
#define R_LightPoint        RB_LightPoint
#endif

#include "refresh/refresh.h"
#include "common/bsp.h"

bool    RB_Init(bool total);
void    RB_Shutdown(bool total);
void    RB_BeginRegistration(const char *map);
qhandle_t RB_RegisterModel(const char *name);
qhandle_t RB_RegisterImage(const char *name, imagetype_t type, imageflags_t flags);
void    RB_SetSky(const char *name, float rotate, bool autorotate, const vec3_t axis);
void    RB_EndRegistration(void);
void    RB_RenderFrame(const refdef_t *fd);
void    RB_ClearColor(void);
void    RB_SetAlpha(float alpha);
void    RB_SetColor(uint32_t color);
void    RB_SetClipRect(const clipRect_t *clip);
void    RB_SetScale(float scale);
void    RB_DrawChar(int x, int y, int flags, int ch, qhandle_t font);
int     RB_DrawString(int x, int y, int flags, size_t maxChars, const char *string, qhandle_t font);
void    RB_DrawPic(int x, int y, qhandle_t pic);
void    RB_DrawStretchPic(int x, int y, int w, int h, qhandle_t pic);
void    RB_DrawKeepAspectPic(int x, int y, int w, int h, qhandle_t pic);
void    RB_DrawStretchRaw(int x, int y, int w, int h);
void    RB_UpdateRawPic(int pic_w, int pic_h, const uint32_t *pic);
void    RB_TileClear(int x, int y, int w, int h, qhandle_t pic);
void    RB_DrawFill8(int x, int y, int w, int h, int c);
void    RB_DrawFill32(int x, int y, int w, int h, uint32_t color);
void    RB_BeginFrame(void);
void    RB_EndFrame(void);
void    RB_ModeChanged(int width, int height, int flags);
r_opengl_config_t RB_GetGLConfig(void);
void    RB_LightPoint(const vec3_t origin, vec3_t light);

bool    IMG_LookupHandle(const char *name, imagetype_t type, imageflags_t flags, qhandle_t *handle);

// com_eventTime of the frame being rendered (sampled when it was recorded)
extern unsigned r_eventtime;

// Main thread: wait for the render thread to go idle and take the GL
// context. Every main-thread entry into GL (cvar callbacks, commands,
// registration) must call this first; it may be called in the middle of
// recording a frame. The main thread then has the renderer to itself
// until it submits the next frame. No-op without the render thread.
void    RT_Acquire(void);

// Main thread: wait for the render thread to go idle, leaving the
// context where it is. For CPU-side renderer state the render thread
// reads or writes, with the same exclusivity as RT_Acquire.
void    RT_Wait(void);

#if DEBUG_DRAWING
// bot debug overlays for one frame, see GL_DebugDrawSnapshot
typedef struct {
    int                 num_boxes;
    int                 num_arrows;
    bool                selection;
    draw_selection_t    sel;
    draw_boxes_t        *boxes;
    draw_arrows_t       *arrows;
} debug_frame_t;

extern debug_frame_t r_debugframe;  // what RB_RenderFrame draws

// main thread, once per R_RenderFrame
void    GL_DebugDrawSnapshot(debug_frame_t *f);
#endif
