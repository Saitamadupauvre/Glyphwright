# CLAUDE.md — Glyphwright

## Comments policy (highest priority, overrides all else)

Never write comments in code unless the user explicitly tells you where to put one. When told, the comment must be doxygen/docstring style only — never explanatory, narrative, or WHY-comments elsewhere. Exception: closing-namespace comments (`} // namespace foo`) always allowed, standard good practice.

## Naming

Private/member variables prefix with `_` (e.g. `_count`), never suffix (`count_` wrong).
Types: PascalCase (`World`, `ComponentPool`). Class member functions: camelCase (`addComponent`, not `add_component`). Free functions: snake_case (unchanged, e.g. `align_up`, systems like `move_system`).
Filenames: PascalCase matching the primary class/struct name when a file defines one (`World.hpp`, `ComponentPool.hpp`, `Entity.hpp`). Files that aren't centered on a single type (macro headers, multi-component registration .cpp) stay snake_case.

2D game engine, C++. Strict split: game logic / rendering / editor. Same game code targets terminal (ASCII, or Sixel/Kitty graphics) or native GUI, no gameplay rewrite.

## Layers

```
engine_core    → ECS, game loop, native components (Transform, Hierarchy),
                 gizmo/selection/culling, render/input abstractions,
                 hot-swap loader, InputActionMap/InputContextStack.
engine_stdlib  → optional components (Sprite, Velocity, Collider2D).
renderers/*    → hot-swappable dylibs: ASCII, TUI-graphics (Chafa), native GUI.
                 Each gets only a rect + generic DrawCommand batch.
editor         → FTXUI-based. Panels, inspector reflection UI, gizmo, hot-swap
                 orchestration, build triggering.
game project   → dev components/systems/scenes/assets, compiled to
                 game_logic.so hot-swap dylib during dev.
```

Golden rule: useful in *any* game → engine. Specific to *this* game → dev code. Only exception: `Transform` (native, editor needs direct low-latency access).

## Core rules

- Components: POD structs only. No inheritance, no virtual, no RTTI. Storage = SoA, raw memory alloc by size/alignment — never `new T()`.
- Reflection: `ENGINE_REFLECT(Type, field1, field2, ...)` macro, one line per component. Self-registers at dylib load (static-object pattern). No central registry file. Reflection data used ONLY by editor (inspector/serialization) — never touch it in hot loop, gameplay code always uses real typed struct.
- Systems: free functions + `ENGINE_SYSTEM(fn, Phase::X)`. Phases: `Init`, `FixedUpdate`, `Update`, `Render` — fixed set, no runtime scheduler config. Explicit order via `After()`/`Before()`, resolved by topo sort at load. Persistent system state = reflected singleton component, never a static var.
- Draw calls: always batched (`DrawCommand[]` → one `draw_sprites_batch` call). Never per-entity virtual calls. Expensive asset conversions happen once (load/pipeline), never per frame.
- Renderer ABI: raw C vtable (`RendererVTable`, `extern "C" get_renderer_vtable()`). Nothing but PODs/opaque ptrs/fn ptrs crosses the dylib boundary — no `std::`, no exceptions. Write renderers as a normal C++ `IRenderer` class + `ENGINE_EXPORT_RENDERER(Type)` macro generates the trampolines; don't hand-write the C vtable.
- Renderer hot-swap: allowed mid-Play (state transfer via `serialize_state`/`deserialize_state`). Game-code hot-swap: NOT allowed mid-Play — only Stop → recompile → Play, full reset from serialized scene. Never build field-migration logic for this — by design.
- Input: never assume a key combo reliably arrives (terminal/tmux/multiplexer dependent). Route all input through `InputAction`/`KeyChord`/`InputActionMap`(JSON, rebindable)/`InputContextStack`. Never hard-code physical keys in gameplay code — check `input.is_active("ActionName")`.

## Build

- Engine: `engine_core`, `engine_stdlib`, `renderers/*`, `editor` (FTXUI).
- Game project: `engine_add_game()` in a few lines of CMake, `src/components/*.hpp`, `src/systems/*.hpp/.cpp`, `src/game_module.cpp`. Sources via `GLOB_RECURSE` — editor must force full CMake reconfigure (not incremental) on build trigger to avoid glob staleness.
- Two configs: Edit/Play = `game_logic.so` SHARED lib, Debug/RelWithDebInfo. Export = standalone `player` executable, Release, editor code never linked in.

### Static-registration whole-archive landmine

`ENGINE_REFLECT`/`ENGINE_SYSTEM` self-register via a static object ctor — nothing calls them, no function ties into that TU. When such a component/system lives in a **static lib** (`.a`) — e.g. `engine_stdlib`, any future static game/component lib — the linker only pulls `.o` files that resolve an undefined symbol something else references. Since nothing calls into a pure-registration TU, the linker drops it silently: no compile/link error, reflection/registry just returns null / missing entries at runtime.

Any target linking such a static lib (editor, `player` exe, `game_logic.so` if game components ever land in a static lib, test binaries) MUST force whole-archive inclusion, e.g.:
```cmake
target_link_libraries(<target> PRIVATE $<LINK_LIBRARY:WHOLE_ARCHIVE,engine_stdlib>)
```
Found and fixed for `tests/CMakeLists.txt` (`gw_tests` linking `engine_core`/`engine_stdlib`). Apply same treatment when wiring `editor`, `player`, and any game-project static component libs. Shared libs (`.so`, e.g. `game_logic.so`) load all their object code regardless, so this is a static-lib-only concern.

## When implementing

- New component: reflected struct + `ENGINE_REFLECT`, unless it's `Transform`/`Hierarchy`-class (needs editor gizmo/selection direct access) — ask before adding a new native component, that tier is meant to stay small.
- New system: free function + `ENGINE_SYSTEM`, correct phase. No manual registration wiring.
- New renderer: implement `IRenderer`, `ENGINE_EXPORT_RENDERER`, keep ABI boundary POD-only.
- Don't design around mid-Play game-code hot-reload or field migration — explicitly rejected, see Decisions Log.

