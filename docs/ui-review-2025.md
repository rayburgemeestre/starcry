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

Run on the current tree, so these are facts rather than impressions.

* **ESLint: 1,738 problems — but 1,674 of them are in `public/client.js`**, the
  generated emscripten glue, which `.eslintignore` does not exclude. The
  hand-written source accounts for only ~64 problems across 15 files. Worth adding
  `/public/client.js` to `.eslintignore` so the signal is visible at all.
* Of the remaining problems, the notable rules are `no-unused-vars` (111 total,
  mostly in the generated file) and `no-explicit-any` (29).
* Type-safety escape hatches in `src/`: 17 × `: any`, 1 × `as any`, 18 ×
  `eslint-disable`, 0 × `@ts-ignore`. Low, but `types/types.ts` at 34 lines suggests
  much of the websocket payload shape is simply untyped rather than escaped.
* No test infrastructure at all: `package.json` has
  `"test": "echo \"No test specified\" && exit 0"`.

So the hand-written code is not lint-dirty. Expect the real problems to be
architectural — oversized components, duplicated concerns, dead code left behind
after rewrites — rather than the kind of thing a linter catches.

---

## Concrete findings already on the table

Found while triaging ESLint output; recorded here because the cleanup that silenced
the warnings would otherwise erase the trail. Each one still needs a decision.

### F1 — `ViewpointComponent.vue`: module-scope state and an import-time side effect
Lines 93-110 sit **outside** `setup()`:

```ts
let viewpoint_store_raw = useViewpointStore();          // line 93, module scope
let viewpoint_store = storeToRefs(useViewpointStore());
let rows = ref([]);
let timer: NodeJS.Timeout | null = null;
viewpoint_store.previous_hash.value = serialize_viewpoint();   // side effect at import
export default defineComponent({ setup() { ...            // line 111
```

Two problems: that state is shared across every instance of the component instead of
being per-instance, and `useViewpointStore()` runs at module load, which depends on the
active Pinia instance already existing. Should almost certainly move into `setup()`.

### F2 — `ViewpointComponent.vue:108`: `timer` declared, never wired
`let timer: NodeJS.Timeout | null = null;` is never assigned again. Someone intended a
debounce or poll and did not finish it. Decide: wire it up, or drop it.

### F3 — `ViewpointComponent.vue:96`: disabled texture rows
`script_store_refs` is unused only because the two lines that used it are commented out:

```ts
// rows.value.push(['texture_w', script_store_refs.texture_w]);
// rows.value.push(['texture_h', script_store_refs.texture_h]);
```

Why were `texture_w` / `texture_h` removed from the viewpoint panel? Restore or delete.

### F4 — `ObjectComponent.vue`: abandoned `setTimeout` -> `nextTick` migration
Commit `e43f143` ("fix dumb setTimeout's with nextTick()") did this for
`GradientsComponent` (2 uses). In `ObjectComponent` it was started and reverted:
`nextTick` imported at line 78, `// await nextTick();` commented out at line 222, and
`setTimeout(try_focus, 10)` still in place at line 226. Either finish it or remove the
remnants.

### F5 — `MainLayout.vue:175` + `:179`: selection state that "doesn't even work"
`let object = ref(objects_store.object);` is assigned and never read, directly above the
author's own `// TODO: this shit is ugly and doesn't even work` on `selectedObjectId`.
Object selection in the canvas overlay needs a proper look.

### F6 — `MainLayout.vue:137`: a Node builtin imported into browser code
`import { mainModule } from 'process';` - unused, and `process` is a Node module that has
no business in a browser bundle. Almost certainly an accidental IDE auto-import. Removed
in the lint cleanup, noted here in case it hints at a misconfigured editor.

---

## Suggested review order

Highest risk first, so stopping early still covers the worst of it:

1. `MainLayout.vue` (689) — biggest file, mixes three concerns
2. `GradientsComponent.vue` (619) — biggest single-feature component, hand-rolled parsing
3. `ObjectBrowser2.vue` + `ObjectComponent.vue` + `ObjectBrowser.vue` (1,062 combined)
   — resolve the v1/v2 duplication first, since it may delete 322 lines outright
4. `ObjectStateComponent.vue` (377) — newest, least exercised
5. `EditorPage.vue` (318) + `MonacoEditor.vue` (220) — JSON round-tripping
6. The `core/` and `stores/` layer — smaller, but everything above depends on it
