#!/bin/bash
#
# AQtion.app launcher — sets the working directory to the .app's parent
# folder so baseaq/ and action/ resolve next to the bundle (not inside it).
#
# Layout expected at runtime:
#   <install dir>/
#     ├── AQtion.app/
#     ├── baseaq/
#     └── action/
#
# When Steam (or Finder double-click) launches the .app, macOS sets CWD to
# something unrelated (Steam install root or /). q2pro defaults DATADIR='.'
# so it resolves baseaq/ relative to CWD — without this launcher the engine
# would search the wrong place. cd up to the .app's parent, then exec the
# real binary.

set -e

SELF_DIR="$(cd "$(dirname "$0")" && pwd)"        # AQtion.app/Contents/MacOS
APP_PARENT="$(cd "$SELF_DIR/../../.." && pwd)"   # folder containing AQtion.app

cd "$APP_PARENT"

exec "$SELF_DIR/q2pro" "$@"
