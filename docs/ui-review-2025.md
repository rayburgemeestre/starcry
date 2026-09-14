# Web UI — features added since January 2025

A review checklist for the Quasar/Vue front-end in `web/`. Derived from
`git log --since=2025-01-01 -- web/` (33 commits, `9723f6f`..`1532663`) plus the
current state of `web/src`.

Status column is for you to fill in as features get reviewed.

---

## At a glance

| | |
|---|---|
| Commits touching `web/` since 2025-01-01 | 33 |
| Net change to `web/src` | ~2,700 insertions / ~560 deletions across 36 files |
| New files | 8 (5 components, `core/quadtree.ts`, `core/clientstorage.ts`, `types/types.ts`) |
| Current size of `web/src` | ~6,000 LOC total |

The important consequence: **the 2025 work is most of the current UI**, and it is
concentrated in the largest files. Six of the seven biggest files are either new in
2025 or were substantially rewritten then.

| File | LOC | 2025 status |
|---|---|---|
| `layouts/MainLayout.vue` | 689 | heavily modified (canvas overlay, object picking) |
| `components/GradientsComponent.vue` | 619 | **new** |
| `components/ObjectBrowser2.vue` | 411 | **new** |
| `components/ObjectStateComponent.vue` | 377 | **new** (committed 2026-09-14) |
| `components/ObjectComponent.vue` | 329 | **new** |
| `components/ObjectBrowser.vue` | 322 | pre-2025, superseded but still rendered |
| `pages/EditorPage.vue` | 318 | rewritten (487 lines changed) |

---

## Features by theme

### 1. Object Browser V2  ·  status: ☐
A second-generation object tree, built alongside the original rather than replacing it.

* Commits: `5afafc1` (WIP start), `0cbb041` (implement), `8796ac2` (indent + isolate
  placeholder), `d7041f7` (extract `IsolatedObject`), `f352a98` (share
  `ObjectComponent` between the Objects list and Scene Objects)
* Files: `ObjectBrowser2.vue` (411), `ObjectComponent.vue` (329),
  `IsolatedObject.vue` (126)
* **Known issue:** `pages/ObjectsPage.vue` renders **both** `ObjectBrowser2` *and*
  `ObjectBrowser` — the v1 component (322 LOC) was never removed. Decide which one
  ships and delete the other.
* Check: tree-building and recursion in `ObjectBrowser2`, key/id handling, whether
  `ObjectComponent` genuinely serves both call sites or just accumulates props for
  both.

### 2. Gradient editing  ·  status: ☐
Gradients became clickable, then editable, then moved into their own component.

* Commits: `ab2a801` (clickable), `e01cc3f` (editable), `96eaf5a` (edit name),
  `932c137` (move out of EditorPage "to reduce spaghetti"), `dad6393`
  (`#ffffff@0.7` colour syntax), `e43f143` (TS fixes, `setTimeout` → `nextTick`)
* Files: `GradientsComponent.vue` (619 — the second-largest file in the UI)
* Check: the `#rrggbb@alpha` parser (hand-rolled string parsing is a classic source
  of silent failure), two-way binding back into the script text, and whether 619
  lines in one component is hiding several concerns.

### 3. Specifications / property metadata  ·  status: ☐
Script fields became self-describing, validated against C++-side specs.

* Commits: `a4b39e7` (video specification checking), `6761929` (draft object
  properties + spec placeholder), `3c45acf` (constants as values **and** reflected
  types in JS), `94ad030` (object specifications with comprehensive properties)
* Files: UI side plus C++ `src/specs/video.hpp`, `src/interpreter/project_specifications.{h,cpp}`
* Check: this one crosses the C++/JS boundary — verify the UI's notion of a field's
  type/default actually matches `create_object_spec()` / `create_video_spec()`, and
  what happens when they disagree.

### 4. Editor page  ·  status: ☐
* Commits: `9723f6f` (objects editable as JSON), `53bef08` (video block),
  `f42e451` (key/value add for video properties), `48065c0` (save button in Monaco)
* Files: `EditorPage.vue` (318, rewritten), `MonacoEditor.vue` (220)
* Check: JSON round-tripping — does editing an object as JSON and saving preserve
  everything, including fields the UI does not know about? Unsaved-changes handling.

### 5. Canvas overlay & object picking  ·  status: ☐
The interactive layer drawn over the rendered frame.

* Commits: `6c8723e` (fix resize), plus the uncommitted-until-now hover/label work
  in `1532663`
* Files: `MainLayout.vue` (689 — the largest file), `core/quadtree.ts` (185),
  `core/draw_utils.ts`, `core/position.ts`
* Check: `MainLayout.vue` is doing layout *and* canvas drawing *and* hit-testing in
  one 689-line component — the strongest candidate for splitting. Also verify the
  quadtree port from the old `quadtree.js`.

### 6. Timeline  ·  status: ☐
* Commits: `9378ca5` (hide timeline elements after end of scene)
* Files: `TimelineComponent.vue` (157)
* Check: off-by-one around scene boundaries; the C++ side marks skipped frames via
  `metrics::skip_job` and the timeline colours depend on it.

### 7. Object state panel  ·  status: ☐
* Files: `ObjectStateComponent.vue` (377) — added 2025-11, only committed 2026-09-14
  (it was untracked while `MainLayout.vue` already imported it, so the web build was
  broken on a fresh checkout)
* Check: it is the newest and least-exercised component; verify it is reachable at
  all from the UI and that its reactivity is sound.

### 8. Log console  ·  status: ☐
* Commits: `3062516` (display program log messages in client console), plus C++
  `385f212`, `14ac7cc`, `7a28c83`, `a597d1f`
* Files: `components/endpoints/script.ts` (131), `webserver/script_handler.cpp`
* Check: unbounded growth of the log buffer in the browser; the C++ `log_line`
  signature was fixed twice in a row, so re-verify the wiring.

### 9. TypeScript & structure refactor  ·  status: ☐
* Commits: `993a5b1` (reorganise TS, add typing), `8a89839` (missing headers),
  `4a436b6` (revert binary serialisation to WASM)
* Changes: `components/*.ts` → `core/*.ts`, `quadtree.js` → `quadtree.ts`,
  new `types/types.ts`, `api.ts` reworked
* Check: `types/types.ts` is only 34 lines for a UI this size — how much is still
  implicitly `any` at the websocket boundary?

### 10. Navigation / small UX  ·  status: ☐
* `6a76db6` default page `/editor` instead of `/script`; `54cef93` auto-hide
  connection status on mobile; `9cd6a3d` port 9xxx redirect to 18080; `7f8ae83`
  lower startup delay

---

## Measured quality signals

Re-measured after the cleanup pass; these are facts rather than impressions.

```
eslint     : 16 warnings, 0 errors, exits 0   (was 1738 problems / 21 errors)
typecheck  : 240 errors                        (was: .vue files checked by nothing)
build      : succeeds
tests      : none - package.json still has "test": "echo No test specified"
```

The remaining eslint warnings are all `no-explicit-any`, concentrated in
`EditorPage.vue` (5), `ObjectStateComponent.vue` (2) and `ObjectBrowser2.vue` (2).

`npm run typecheck` (vue-tsc) is the only thing that checks `.vue` files at all -
`quasar build` uses esbuild, which strips types without checking them. **The 240 is a
baseline to compare against, not a to-do list:** a large share is one root cause
repeated, quasar's `QTable` column type being stricter than the `columns` arrays written
here (`align: string` rather than the `'left' | 'right' | 'center'` union, plus untyped
`field: (row) => ...` accessors). Fixing the column definition once would clear a big
fraction of it.

| file | errors |
|---|---|
| `src/layouts/MainLayout.vue` | 77 |
| `src/components/ObjectBrowser.vue` | 27 |
| `src/components/MonacoEditor.vue` | 26 |
| `src/components/ObjectStateComponent.vue` | 24 |
| `src/components/ViewpointComponent.vue` | 19 |
| `src/pages/EditorPage.vue` | 11 |
| `src/pages/DebugPage.vue` | 10 |
| `src/components/endpoints/script.ts` | 8 |
| `src/components/ObjectComponent.vue` | 7 |
| `src/components/GradientsComponent.vue` | 6 |
| `src/components/endpoints/stats.ts` | 5 |
| `src/components/TimelineComponent.vue` | 3 |
| `src/components/endpoints/bitmap.ts` | 2 |
| `src/components/IsolatedObject.vue` | 1 |
| `src/pages/FilesPage.vue` | 1 |

So the hand-written code is not lint-dirty. The real problems are architectural —
oversized components, duplicated concerns, dead code left behind after rewrites — rather
than the kind of thing a linter catches.

---

## Open findings

Open items only. Resolved findings have been removed — they live in the git history
(`git log --grep=' F[0-9]'` and the commits around them), which is why the numbering has
gaps. Numbers are kept stable so existing commit messages still refer to the right thing.

### F4 — `GradientsComponent.vue:273`: `resizeCanvas` cannot be scheduled properly
```js
// nextTick causes infinite loops
setTimeout(resizeCanvas, 100);
setTimeout(resizeCanvas, 500);
setTimeout(resizeCanvas, 1000);
```
"Infinite loops" means `resizeCanvas()` mutates reactive state, so running it inside a
tick queues another tick. The bug is in `resizeCanvas`, not in the scheduling — the three
unconditional timeouts are a guess-and-hope workaround. The sibling case in
`ObjectComponent` was resolved separately (a bounded poll); this half is untouched.

### F7 — `stores/stats.ts`: `rows` and `rows_piper` are typed as `ObjectType[]`
Both hold **table rows** — `[thread.name, seconds_idle, state]` and
`[component.name, component.counter]` — not scene objects. `ObjectType` looks
copy-pasted from `stores/objects.ts`. Surfaced by typing the /stats payload; `tsc`
now reports the mismatch at `endpoints/stats.ts` lines 23, 61 and 64. Related:
`render_value` is declared `string` but assigned `last_job.render_time || last_job.number`,
both numbers. Not fixed - changing these types will cascade into whatever renders them.

### F8 — `endpoints/stats.ts:23`: `Math.round(thread.seconds_idle)` can be `NaN`
`metrics.cpp` only sets `seconds_idle` when the thread is **idle**; for a busy thread it
emits a `job` object instead. The UI rounds it unconditionally, so a busy thread yields
`Math.round(undefined)` = `NaN` in the stats table. Typing `MetricsThread.seconds_idle`
as optional makes `tsc` point straight at it. Needs a fallback, or to show the busy-state
info instead.

### F9 — `ObjectComponent` receives two different shapes, neither matching its prop type
The prop is declared `objects: PropType<Record<string, ObjectData>>`, i.e. a map of
object-name to a single object's properties. `EditorPage` passes two different things:

* line 13: `objects`, declared `ref<Record<string, ObjectDef[]>>` - a map to **arrays**
* line 97: `scene.objects`, declared `ObjectDef[]` - a plain **array**

`for (let k in props.objects)` happens to iterate array indices, so it "works", but the
declared contract is wrong at both call sites and the component silently means something
different in each. Worth settling before relying on the types. Not fixed.

### F10 — `ObjectComponent.onCellChange`: the "copy" does not isolate
```ts
const updatedObjects = { ...props.objects };
updatedObjects[selected.value][prop] = value;
emit('update:objects', updatedObjects);
```
The spread is **shallow**, so `updatedObjects[selected.value]` is the very same nested
object the prop points at: the assignment mutates the parent's data directly and the
emit hands back an object the parent already has. `updateCellValue` likewise assigns
straight into `row[1]`. It works today because the parent assigns the emitted value back,
but nothing here is actually immutable. Not fixed.


---

## Suggested review order

Highest risk first, so stopping early still covers the worst of it. Partially covered
already: `core/api.ts` (callbacks typed, payload shapes derived from the C++ emitters),
`ViewpointComponent.vue` (state moved into `setup()`), `ObjectComponent.vue` (cell values
typed, focus retry bounded) — none of them reviewed end to end, only the specific
problems listed above.

1. `MainLayout.vue` (689) — biggest file, mixes three concerns
2. `GradientsComponent.vue` (619) — biggest single-feature component, hand-rolled parsing
3. `ObjectBrowser2.vue` + `ObjectBrowser.vue` (733 combined) — resolve the v1/v2
   duplication, which deletes 322 lines outright. Investigated in
   [objectbrowser-v2-migration.md](objectbrowser-v2-migration.md).
   (`ObjectComponent.vue` was listed here by mistake: it belongs to `EditorPage`, not to
   the object browsers, and is a separate item.)
4. `ObjectStateComponent.vue` (377) — newest, least exercised
5. `EditorPage.vue` (318) + `MonacoEditor.vue` (220) — JSON round-tripping
6. The `core/` and `stores/` layer — smaller, but everything above depends on it
