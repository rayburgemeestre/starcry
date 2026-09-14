# Retiring ObjectBrowser V1 in favour of V2

Investigation only — no code changed. Goal: establish exactly what V2 has to gain before
`ObjectBrowser.vue` can be deleted.

---

## 0. One correction up front

The review doc grouped "ObjectBrowser2.vue + ObjectComponent.vue + ObjectBrowser.vue
(1,062 combined)". That was wrong: **`ObjectComponent.vue` is not part of this at all.**

| file | LOC | what it actually is |
|---|---|---|
| `components/ObjectBrowser.vue` | 322 | **V1** browser, on `/objects` |
| `components/ObjectBrowser2.vue` | 411 | **V2** browser, on `/objects` |
| `components/IsolatedObject.vue` | 126 | V2's detail/edit panel, used only by V2 |
| `components/ObjectComponent.vue` | 329 | property table used by **`EditorPage.vue`** (`/editor`), unrelated |

So the duplication is 322 + 411 = **733 lines**, and retiring V1 deletes **322**.
`ObjectComponent.vue` is a separate review item and should not be touched here.

`pages/ObjectsPage.vue` renders both and already labels them:

```html
WIP:          <ObjectBrowser2 />
deprecated:   <ObjectBrowser />
```

---

## 1. Where the data comes from (C++ side)

Both browsers read the same array, `objects_store.objects`, filled by the `/objects`
websocket in `components/endpoints/object.ts` from `json['shapes']`.

Server path: `get_objects` → `starcry::serialize_shapes_to_json` (`src/starcry.cpp:843`)
→ `data::shapes_to_json` (`src/data/shape.hpp`). Per shape it emits ~50 fields, including
everything a browser needs:

* identity/tree: `unique_id`, `id`, `label`, `level`, **`has_children`** (set in
  `shape.hpp:296-300`)
* geometry: `x`, `y`, `z`, `x2`, `y2`, `z2`, `radius`, `radius_size`, `rotate`, `scale`, …
* styling: `r`, `g`, `b`, `hue`, `opacity`, `blending_`, `gradients_`,
  `gradient_id_str`, `textures`, `texture_id_str`, …
* motion: `velocity`, `vel_x`, `vel_y`, `motion_blur`, `steps`, `dist`, `time`

**Selection is a real server feature, not just UI state.** The chain is:

```
script_store.selected                       (browser)
  → bitmap outbox { selected: [...] }       ViewpointComponent.vue:195
  → data::frame_request::set_selected_ids
  → starcry::selected_ids_transitive        starcry.cpp:864
  → generator::get_transitive_ids           generator.cpp:540
  → render_params.selected_ids              → rendering_engine draws only those
```

Two consequences for the plan:

* **No C++ change is needed for this migration.** The engine already supplies `level` and
  `has_children`, and already isolates rendering by id.
* **`generator::get_transitive_ids` already walks descendants server-side**, so V2's
  client-side `getChildrenRecursive` loop in `onIsolateClick` is redundant work. It can be
  dropped — send the clicked id and let the engine expand it.

---

## 2. Feature matrix

| capability | V1 | V2 | notes |
|---|---|---|---|
| Hierarchy by indentation | ✅ `level * 20px` | ✅ `level * 10px` | equivalent |
| Expand / collapse one node | ✅ | ✅ | different mechanisms, see §4 |
| **Expand all / collapse all** | ✅ `toggle_expand()` | ❌ | V1-only |
| **Reset** (clear all selection state) | ✅ `reset()` | ❌ | V1-only |
| **Filter to selected only** | ✅ `filtering()` | ❌ | V1-only |
| **Highlight object on canvas** | ✅ `highlight()` | ❌ | V1-only, **see §3.1** |
| **Multi-object isolation** | ✅ selection is an array | ❌ | V2 clears first, one at a time |
| **Toggle focus off** | ✅ click again | ⚠️ only via panel *Close* | |
| Recursive focus incl. children | ✅ client-side walk | ✅ client-side walk | both redundant, see §1 |
| Per-object property view | ✅ inline table, several at once | ✅ `IsolatedObject`, one at a time | |
| Property **editing** | ❌ | ⚠️ present but inert, **see §5.1** | |
| Type symbol column | ⚠️ written, commented out in template | ✅ `type2` | |
| Columns | fixed 3: `unique_id`, `id`, `gradient` | dynamic from `objects[0]`, reordered | |
| Virtual scroll / large lists | ❌ plain `v-for` | ✅ `q-table virtual-scroll` | V2 better |
| Auto re-render on isolate | ❌ | ✅ `render_requested_by_user_v2` | V2 better |

---

## 3. What V2 must gain before V1 can go

### 3.1 Object highlighting — the only hard blocker
V1 is the **sole writer** of `script_store.highlighted`:

```js
highlight(unique_id) { scripts_store.highlightObject(unique_id); scripts_store.render_completed_by_server++; }
```

and `MainLayout.vue:324` is the sole reader — it draws highlighted objects differently on
the canvas overlay (`pass === 2`). Delete V1 and the whole feature becomes unreachable:
`script_store.highlightObject()` loses its only caller and `MainLayout:324` can never fire.

**Required:** a highlight toggle in V2 (a column button, or click-on-`unique_id` as in V1).
Small: one button plus the existing store call.

### 3.2 Multi-object isolation
V1 accumulates: `script_store.selected` is an array and `user_select()` toggles entries.
V2's `onIsolateClick` calls `clearSelectedObjects()` first, so isolating B drops A. The
server accepts a list, so this is purely a V2 UI limitation.

**Required:** make isolate additive (ctrl-click, or a checkbox column), with an explicit
"clear" action.

### 3.3 Expand all / collapse all, and Reset
V1 has three buttons V2 has none of. `toggle_expand(false)` sets a global `expand` state
that bypasses the filter entirely; `reset()` clears both stores.

**Required:** equivalents against V2's `expanded[]` array. Straightforward.

### 3.4 Filter to selected
V1's `filtering()` toggles "show only objects that are selected or implied-selected".
V2 always shows root level plus expanded subtrees.

**Required:** decide whether this is still wanted. It overlaps with isolation, and may be
redundant now that isolation drives server-side rendering.

---

## 4. The two expand models are incompatible

This is the subtlety that makes "port the buttons" less trivial than it looks.

* **V1** stores expansion in the **objects store** (`gui_implied_selected`,
  `implied_selected`), entangled with selection: `onUserSelected()` marks all ancestors
  as implied-selected *and* gui-selected, so selecting a deep node auto-expands the path
  to it. Visibility is then `isSelectedArray(parentsLookup(id))`.
* **V2** stores expansion **locally** in the component (`expanded: TableObject[]`) and
  matches on an ancestry `stack` array it builds while iterating. Selection and expansion
  are independent.

V2's model is cleaner and should win. But note what is lost: in V1, selecting an object
anywhere (including **by clicking it on the canvas** — `MainLayout.vue:366` calls
`objects_store.onUserSelected`) automatically revealed it in the tree. V2 will not do
that unless expansion is driven from selection explicitly.

**Decide:** should clicking an object on the canvas scroll to / reveal it in V2's table?
If yes, V2 needs to expand ancestors on selection — which needs `parentsLookup()`, one of
the store methods otherwise slated for deletion.

---

## 5. Problems in V2 to fix regardless

### 5.1 `IsolatedObject` edits go nowhere
`updateObjectProperty()` emits `update:selectedObject`; V2's handler is:

```js
const updateSelectedObject = (updatedObject) => { selectedObject.value = updatedObject; };
```

It writes a local ref. Nothing reaches the objects store, the script, or the engine. The
panel presents editable `q-input`s that silently discard every change. Either wire it to
the script (the way `EditorPage`/`ObjectComponent` write back) or make the fields
read-only until it is wired.

### 5.2 The `rows` computed mutates store state
```js
for (let obj of objects_store.objects) {
  const objAny = obj as TableObject;
  objAny.tree = ''; objAny.actions = ''; objAny.type2 = ...; objAny.stack = [...stack];
```
`obj` is a live element of `objects_store.objects`, so this computed writes into the store
on every evaluation — side effects in a computed, and store mutation outside an action.
Should build new row objects instead of decorating the originals.

### 5.3 Columns derive from `objects[0]` only
If the array is empty there are no columns at all; if the first shape lacks a field that
later ones have, that column never appears. Shape types differ (a `line` has `x2`/`y2`, a
`text` has `text`/`text_font`), so this is a live issue, not theoretical. Prefer a
declared column set, or a union of keys across all rows.

### 5.4 Leftovers
`console.log` in `onIsolateClick`, `onExpandClick` and `onRowClick`; `onRowClick` itself is
a no-op whose only body is a log and two commented-out lines.

---

## 6. Dead code once V1 is gone

Verified by grepping for callers outside `ObjectBrowser.vue`.

**`stores/objects.ts` — no remaining callers:** `isSelected`, `isUserSelected`,
`isUserInfoSelected`, `addGuiSelected`, `removeGuiSelected`, `onUserDeSelected`,
`onUserSelectedInfo`, `onUserInfoDeSelected`, `isSelectedArray`, `parentsLookup`,
`parentLookup`, `reset_gui`, `getChildren`, `hasChildren`, plus state `user_selected`,
`user_selected_info`, `implied_selected`, `gui_implied_selected`.

**`stores/script.ts` — no remaining callers:** `highlightObject`, `highlighted` (unless
3.1 is implemented), `removeSelectedObject`.

**Careful — one trap.** `MainLayout.vue:366` still calls `objects_store.onUserSelected()`
on canvas click. After V1 is removed, *nothing reads* the state that writes
(`isUserSelected`/`isSelected`/`isSelectedArray` all disappear). So that call silently
becomes a no-op and should be removed with it — but only after confirming §4, since it is
also what drives V1's reveal-in-tree behaviour. The adjacent
`script_store.addSelectedObject()` on the same line **must stay**: that one feeds
server-side render isolation.

---

## 7. Suggested sequence

Each step is independently shippable and leaves both browsers working.

1. **Fix V2's own bugs first** (§5.1–5.4) — they are independent of the migration and make
   the later comparison honest. Start with 5.2, since a computed mutating the store can
   produce confusing behaviour while testing everything else.
2. **Add highlight to V2** (§3.1) — the only true blocker.
3. **Make isolation additive** (§3.2) and add explicit clear.
4. **Add expand-all / collapse-all / reset** (§3.3).
5. **Decide on filter-to-selected** (§3.4) and canvas reveal (§4) — the two genuine product
   questions. Everything else is mechanical.
6. **Simplify `onIsolateClick`** to send just the clicked id; the engine expands it
   (`get_transitive_ids`). Removes a client/server duplication.
7. **Swap `ObjectsPage.vue`** to render only V2. Leave V1 in the tree for one iteration to
   allow a quick revert.
8. **Delete `ObjectBrowser.vue`** (→ `Trash/`) and prune the dead store surface from §6.

Verification at each step: `npm run lint`, `npm run typecheck` (compare against the 240
baseline in `ui-review-2025.md`), and a manual pass on `/objects` — expand a deep tree,
isolate an object and confirm the render actually isolates, highlight one and confirm the
canvas overlay changes.

---

## 8. Open questions for you

1. **Filter-to-selected** — worth keeping, or has isolation superseded it?
2. **Canvas → tree reveal** — should clicking an object on the canvas reveal/scroll to it
   in the browser? Keeping it is the main reason to retain `parentsLookup()`.
3. **`IsolatedObject` editing** — should it write back to the script (making it a real
   editor), or become read-only? It is currently neither.
4. **Info panel multiplicity** — V1 could show several objects' property tables at once,
   V2 shows one. Does that matter to you in practice?
