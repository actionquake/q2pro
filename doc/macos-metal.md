# Metal rendering on macOS through ANGLE

On macOS, Apple's OpenGL runs on a compatibility layer that is no longer
developed. Q2PRO's OpenGL ES 3.0 renderer (`gl_profile es3.0`) can run on
[ANGLE](https://github.com/google/angle) instead, whose Metal backend turns
every draw call into Metal. The renderer itself is unchanged.

## Turning it on

The macOS `.app` ships ANGLE in `Contents/Frameworks`. It is off by
default. To use it, set `gl_profile es3.0`: in the console (video restarts
by itself), in `autoexec.cfg`, or with `+set gl_profile es3.0` in the launch
options. `gl_profile gl3.2`, the macOS default, goes back to Apple's OpenGL.

With an ES `gl_profile`, the client loads `libEGL.dylib` and
`libGLESv2.dylib` from `Contents/Frameworks`, or else from next to the
executable, and hands SDL their absolute paths. It also sets
`SDL_OPENGL_ES_DRIVER=1` and `ANGLE_DEFAULT_PLATFORM=metal`, unless these are
already set in the environment. The bundled build needs macOS 11 or later.
On older versions, or without the dylibs, the client warns and falls back to
Apple's OpenGL.

`libopenal.1.dylib` next to the executable is loaded the same way.

## What it changes

Measured on an Apple M3 Pro at 120 Hz, in a 1280x720 window on wfall:

| | ANGLE Metal | Apple OpenGL |
|---|---|---|
| vsync off, `r_maxfps 120` | 120 fps, 8% CPU | 125 fps, 98% CPU |
| same, with `cl_sleep 250` | 120 fps, 10% CPU | 125 fps, 10% CPU |
| vsync off, uncapped | 120 fps | 1000 fps |

- ANGLE presents in step with the display even with vsync off. Apple's
  OpenGL renders as many frames as the loop allows, and without `cl_sleep`
  the loop spins a CPU core between them.
- At the same frame rate, the two cost about the same CPU.
- Uncapped, Apple's OpenGL renders far more frames. ANGLE in a window stays
  at the refresh rate.
- Both render at the display's native (Retina) resolution.

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

CI bundles the build pinned in `.ci/mac/angle.env`. To update it, set
`ANGLE_COMMIT` to the `angle` hash of a Chrome Stable release (listed on
chromiumdash.appspot.com), run the `angle` workflow, and copy the
`ANGLE_SHA256` it prints into the same file. The workflow builds the Metal
backend only, for arm64 and x86_64, and publishes the dylibs with ANGLE's
BSD license as a release asset.

To build it by hand:

```sh
git clone --depth 1 https://chromium.googlesource.com/chromium/tools/depot_tools.git
export PATH="$PWD/depot_tools:$PATH"
mkdir angle && cd angle && fetch angle

gn gen out/Metal --args='is_debug=false is_component_build=false
  angle_enable_metal=true angle_enable_gl=false angle_enable_vulkan=false
  angle_enable_swiftshader=false angle_build_tests=false'
autoninja -C out/Metal libEGL libGLESv2
```

## Latency

- `gl_finish 1` waits for the GPU at the start of each frame, which limits
  how far ahead the CPU runs. On ANGLE's Metal backend this costs almost
  nothing and removes up to two queued frames.
- `MTL_HUD_ENABLED=1` in the environment shows Apple's Metal HUD, including
  whether frames are presented directly or composited. Fullscreen should show
  direct presentation; a window is always composited.
