# Metal rendering on macOS through ANGLE

On macOS, Apple's OpenGL runs on a compatibility layer that is no longer
developed. Q2PRO's OpenGL ES 3.0 renderer (`gl_profile es3.0`) can run on
[ANGLE](https://github.com/google/angle) instead, whose Metal backend turns
every draw call into Metal. The renderer itself is unchanged.

## Turning it on

Put `libEGL.dylib` and `libGLESv2.dylib` from an ANGLE build next to the
`q2pro` executable. At startup the client then:

- loads both dylibs by absolute path, so no `DYLD_LIBRARY_PATH` is needed
- sets `SDL_OPENGL_ES_DRIVER=1` and `ANGLE_DEFAULT_PLATFORM=metal`, unless
  they are already set in the environment
- defaults `gl_profile` to `es3.0`

Without the dylibs nothing changes. To go back to Apple's OpenGL with the
dylibs in place, use `+set gl_profile gl3.2` (the usual macOS default) or
remove the dylibs.

`libopenal.1.dylib` next to the executable is loaded the same way.

## Checking that it is on Metal

The console prints `GL_RENDERER` at startup. On the Metal path it contains
`ANGLE Metal Renderer`. If an ES context comes up on anything else, the
client prints a warning: ANGLE's OpenGL backend on macOS translates
ES -> Apple GL -> Metal and is about 10 times slower, which is why
`ANGLE_DEFAULT_PLATFORM=metal` matters.

## Retina and resizing

ANGLE renders at the display's native pixel size. The client reads the real
surface size from EGL whenever the window changes size, moves to a display
with a different scale, or switches to or from fullscreen, and it checks
again after each swap because ANGLE resizes the surface at swap time. With
`r_thread 1` the size change is applied on the main thread.

## Building ANGLE

Build the Metal backend only, so there is no OpenGL backend to fall back
to. The first fetch downloads depot_tools and a Chromium toolchain of
about 15–20 GB.

```sh
git clone --depth 1 https://chromium.googlesource.com/chromium/tools/depot_tools.git
export PATH="$PWD/depot_tools:$PATH"
mkdir angle && cd angle && fetch angle

gn gen out/Metal --args='is_debug=false is_component_build=false
  angle_enable_metal=true angle_enable_gl=false angle_enable_vulkan=false
  angle_enable_swiftshader=false angle_build_tests=false'
autoninja -C out/Metal libEGL libGLESv2
```

Copy `out/Metal/libEGL.dylib` and `out/Metal/libGLESv2.dylib` next to
`q2pro`, with ANGLE's `LICENSE` (BSD). Record the ANGLE commit
(`git rev-parse HEAD`) so the build can be reproduced.

## Latency

- `gl_finish 1` waits for the GPU at the start of each frame, which limits
  how far ahead the CPU runs. On ANGLE's Metal backend this costs almost
  nothing and removes up to two queued frames.
- `MTL_HUD_ENABLED=1` in the environment shows Apple's Metal HUD, including
  whether frames are presented directly or composited. Fullscreen should show
  direct presentation; a window is always composited.
