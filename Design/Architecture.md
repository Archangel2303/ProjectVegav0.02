# Project Vega — C++-first Architecture (Floor 1 roadmap)

Goals
- C++-first core for determinism and performance
- Blueprints only for editor UX, iteration, and designer-facing handlers
- Data-driven abilities/effects (DataAssets / USTRUCTs)
- Clear separation: Data (assets) → Execution (C++ core) → Presentation (Blueprints/UMG)

Folder layout (suggested)
- `Source/ProjectVega/` — C++ module
- `Content/Blueprints/` — BP wrappers, test actors
- `Content/Data/` — DataAssets for abilities/effects
- `Design/` — docs, design notes, demo checklist

Core C++ components (roles)
- `UAbilityDataAsset` (UDataAsset)
  - Holds array of `FEffectPayload` entries and ability metadata (cost, cooldown, target spec)
- `FEffectPayload` (USTRUCT)
  - Common fields: `EEffectType`, duration, stacks, phase, tags, priority
  - Variant sub-structs for parameters: `FDamageEffectData`, `FDOTEffectData`, `FBuffEffectData`
- `FAbilityContext` (plain struct)
  - Runtime-only: caster reference, resolved targets, multipliers, RNG seed, source tags
- `UEffectHandler` (abstract interface / pure virtual class + Blueprint-callable wrapper)
  - Stateless handlers that accept `FEffectPayload` + `FAbilityContext` and return `FEffectResult`
- `UActiveEffectComponent` (ActorComponent)
  - Stores active effects (id, source, remaining duration, stack count)
  - Centralized stack/refresh rules, tick-on-turn hooks
- `UTurnManager` (singleton / GameState subsystem)
  - Discrete turn queue, phases (Start, PreAction, Action, PostAction, End), interrupt handling
- `UAttributeSet` (lightweight) / attribute storage
  - Read/write via single API `ApplyAttributeDelta()` to keep mutations centralized

Execution pipeline (single-entry, deterministic)
1. `ExecuteAbility(UAbilityDataAsset, FAbilityContext)` in C++
2. Validate requirements (tags, costs)
3. Resolve targets (target spec in data asset)
4. For each `FEffectPayload`: dispatch to appropriate `UEffectHandler`
5. Handlers return `FEffectResult` (attribute deltas, events, new ActiveEffect entries)
6. Central `ApplyChanges()` applies deltas and mutates `UActiveEffectComponent`
7. Emit GameplayTag events for reactions/interrupts

Design patterns and rationale
- Composition over inheritance: abilities are lists of payloads; payloads are small parameter structs.
- Stateless handlers: easier to unit-test and reason about; all transient state lives in `FAbilityContext` or `UActiveEffectComponent`.
- Centralized mutation: avoid scattered attribute writes — use `ApplyChanges()` to keep rules consistent and auditable.
- Deterministic turn loop: tie all combat ticks to `UTurnManager` to enable deterministic replays and consistent interrupts.

Blueprint integration
- Expose handler hooks as Blueprint-callable functions or `BlueprintImplementableEvent` when designer-side logic is needed.
- Provide small BP classes for quick authoring: `BP_AbilityWrapper` loads `UAbilityDataAsset` and calls C++ executor.
- Editor utilities (Blueprint Editor Widgets / Python) for bulk-creating ability assets and testing.

Floor 1 demo scope (MVP)
- Level: small multi-room floor with 3 encounter nodes
- Player: one test pawn with 3 abilities (Melee, Ranged, Support)
- Enemy archetypes: Grunt (melee), Sniper (ranged), Totem (support/DOT)
- Systems to implement: TurnManager, Ability executor, ActiveEffectComponent, 3 sample UAbilityDataAsset files, simple UI for HP/turn order
- Determinism: seedable RNG and a debug log that records seeds/actions for replay

Concrete next steps (what I can do for you)
1. Create this doc in `Design/Architecture.md` (done)
2. Scaffold `Source/ProjectVega` module with minimal C++ headers and build files (requires Visual Studio + Unreal build tool)
3. Implement `UAbilityDataAsset` + `FEffectPayload` USTRUCTs and `ExecuteAbility()` skeleton
4. Implement `UActiveEffectComponent` and `UTurnManager` core loops
5. Add sample `UAbilityDataAsset` JSON/text templates and `BP_AbilityWrapper` for designers

Permissions & requirements
- To scaffold C++ code I will add files under `Source/ProjectVega/` — building requires Visual Studio and the Unreal toolchain on your machine. Tell me if you want me to create the C++ scaffolding now.

Floor 1 timeline estimate (rough)
- Scaffolding + core systems (C++ skeleton): 1–2 days
- Implement handlers + sample assets: 1–2 days
- Level assembly + polish UI: 1–2 days

If you want, I can now scaffold the C++ module and add the minimal header/source files for `UAbilityDataAsset`, `FEffectPayload`, `FAbilityContext`, `UActiveEffectComponent`, and `UTurnManager`. Confirm and I will create the scaffolding files next.
