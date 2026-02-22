# CLAUDE.md

## Repository

The canonical repository for this project is **`actionquake/q2pro`** on GitHub.

Always pass `-R actionquake/q2pro` to all `gh` commands (issues, PRs, releases, etc.):

```bash
gh issue view 294 -R actionquake/q2pro
gh pr create -R actionquake/q2pro ...
gh issue comment 123 -R actionquake/q2pro --body-file /tmp/body.md
```

The `upstream` remote pointing at `skullernet/q2pro` is the original Q2Pro engine and is not relevant to AQ2 development — do not use it for issue/PR operations.

## Game Code

All AQ2-specific game logic lives in **`src/action/`**. This is the only directory to read or modify for gameplay, bot, and mod work.

**Do not modify `src/game/`** — that is the vanilla Quake 2 single-player game library (monsters, SP maps, etc.) and is unrelated to AQ2.

Key subdirectories:
- `src/action/botlib/` — BOTLIB bot system (new, preferred)
- `src/action/acesrc/` — Legacy acebot system
- `src/action/addons/` — Optional addon modules

## Documentation

| File | Contents |
|---|---|
| `doc/action.md` | AQ2-TNG feature reference — cvars, commands, game modes, bots |
| `doc/espionage.md` | Espionage mode (ATL/ETV) — setup, scene file format |
| `doc/server.md` | Q2Pro server cvars and commands |
| `doc/client.md` | Q2Pro client cvars, commands, and macros |

When researching how a feature works or what cvars exist, check `doc/action.md` first before searching source code.
