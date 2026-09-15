# What-if preview: editing an object field and rendering the result

Plan only — no code written. Goal: in the object browser's `IsolatedObject` panel, change
a field (say `x` from 305 to 35), press a **preview** button next to it, and see the frame
rendered as if that value applied. Ideally for several fields at once.

---

## 1. Why this is not "just edit the field"

The `x: 305` shown in the browser is **render output, not script input**. It is what
`init()` / `time()`, the positioner and parent cascading produced for that frame. There is
no script field holding 305 that could be set to 35 — and even if you edited the script's
`x`, the generator would recompute a different final value.

What *is* tractable is an **override applied after generation and before rendering**:

> generate frame N normally → force shape #12's `x` to 35 → render

That matches the semantics of a what-if preview exactly, and is much cheaper than making
the script produce the value.

Also worth stating plainly: **this is a preview, not an edit.** Making a change stick means
writing back into the project's JavaScript, which is a separate and harder problem.

---

## 2. Two possible routes

### Route A — server-side job override (recommended)
Send overrides with the frame request; patch `data::job::shapes` by `unique_id` before the
renderer sees them. Mirrors the existing `selected_ids` plumbing almost exactly, so it is
low-risk copy-and-adapt work.

### Route B — client-side WASM re-render (not currently viable)
The engine can already ship renderer input to the browser:
`data::frame_request::enable_renderable_shapes()`, handled at `src/starcry.cpp:492`. And
`web/public/client.js` is the *same* `src/rendering_engine.cpp` compiled to WASM. If that
were wired up, the browser would hold the shapes, could patch them locally and re-render
with **no server round-trip and no C++ change at all** — much better interactivity.

It is not wired up. The UI only calls `window.Module.set_texture(buffer)` with a
server-rendered bitmap (`components/endpoints/bitmap.ts:20`), and commit `4a436b6`
("Reverted attempts at binary cerialization to browser with wasm") shows this was tried and
backed out. Reviving it is a substantially bigger project than Route A, but it is the
better long-term answer if interactive editing becomes a real workflow.

---

## 3. Route A, file by file

| where | change |
|---|---|
| `components/IsolatedObject.vue` | track which fields are dirty; add a **preview** button per field (and a reset). Today the `q-input`s are bound to a local copy and discarded — see F5.1 in `ui-review-2025.md` |
| `components/ObjectBrowser2.vue` | carry the overrides alongside the existing isolation state |
| `components/ViewpointComponent.vue` (~line 190) | add `overrides` to the `bitmap_store.outbox` payload, next to `selected` |
| `src/webserver/bitmap_handler.cpp` (~line 44) | parse `json["overrides"]` — the `json["selected"]` block right there is the template |
| `src/data/frame_request.hpp` (~line 37) | add `overrides_` plus setter/getter, mirroring `selected_ids_` |
| `src/starcry.cpp` | after `gen->generate_frame()`, walk `job->shapes` and patch entries matching `unique_id` |
| `src/data/shape.hpp` | map a field *name* to a `data::shape` member — the only genuinely awkward part, see §5 |

**Multiple fields and multiple objects come for free.** The payload is a list
(`[{unique_id, field, value}, …]`) and the patch loop does not care how long it is.

---

## 4. Three gotchas

### 4.1 Motion blur — the one real design decision
`data::job::shapes` is `std::vector<std::vector<data::shape>>`: **one inner vector per
sub-frame**. The browser is only sent the last one (`starcry::serialize_shapes_to_json`,
`src/starcry.cpp:855`), so the 305 you see is the position at the *end* of the frame's
motion.

Consequently, writing `x = 35` into every sub-frame makes a moving object stationary and
collapses its motion blur. Applying the **delta** (`35 - 305 = -270`) to every sub-frame
instead preserves the motion and simply translates the object.

**Recommendation: store overrides as deltas internally for numeric fields**, even though
the UI presents absolute values. For non-numeric fields (text, gradient id) absolute
assignment is the only sensible option.

### 4.2 Children do not follow
By the time the job exists, children have already been positioned against the parent's old
`x`. Moving a parent leaves its children behind. Applying the same delta to descendants
fixes it — and `generator::get_transitive_ids()` (`src/generator.cpp:540`) already computes
that exact set for the isolation feature, so this is reuse, not new logic.

### 4.3 No re-simulation
Collisions, gravity and toroidal wrapping do not re-run. The preview shows "this shape
drawn somewhere else", not "what the scene would have become". Fine for judging a position,
radius or opacity; misleading for anything physics-driven. Worth saying so in the UI.

---

## 5. The field-name mapping, and C++26 reflection

Patching requires turning the string `"x"` into `shape.x`. C++23 has no reflection, so
today that is a hand-written chain:

```cpp
if (name == "x") s.x = v;
else if (name == "y") s.y = v;
...
```

This is the same maintenance burden as the existing `@add_field@` markers, which already
span 10 files. **Start with a small whitelist of numeric fields** —
`x, y, z, radius, radius_size, opacity, hue, scale, rotate` — rather than all 52 members.
That covers the interesting cases and keeps the chain short.

C++26 reflection (P2996) would remove this entirely: a compile-time walk over
`data::shape`'s members generating the name→setter mapping, which would also subsume the
`@add_field@` problem and possibly the `shapes_to_json` serializer.

### Toolchain feasibility (re-checked 2026-09-15)

**Now available.** The build image was rebased on Ubuntu 26.04, whose `g++-16` implements
P2996R13. Verified by compiling and running a member walk over a 3-field struct inside
`rayburgemeestre/build-ubuntu:26.04`:

```
$ g++-16 -std=c++26 -freflection refl.cpp -o refl && ./refl
__cpp_impl_reflection=202506  members=3
  x
  y
  radius
```

That used `nonstatic_data_members_of(^^shape, access_context::current())`, `template for`
(P1306R5 expansion statements), `std::define_static_array` (P3491R3) and `identifier_of` —
i.e. exactly the mechanism that would replace the `@add_field@` markers.

| | |
|---|---|
| build image today | Ubuntu 26.04, `clang++-19`, `g++-15` (default `g++`) and `g++-16` |
| **`g++-16 -std=c++26 -freflection`** | **works**, `__cpp_impl_reflection = 202506` |
| **V8 headers under `-std=c++26`** | **verified 2026-09-15.** `v8.h` + `libplatform.h` + `<meta>` + a `template for` expansion compile together in one TU |
| `clang-19 -std=c++26` | accepted, but `__cpp_reflection` undefined and `<meta>` not found |
| clang mainline | `clang.llvm.org/cxx_status.html` lists **Reflection P2996R13: No**, as well as P3394R4 / P3293R3 / P3491R3 / P3096R12 / P3598R0. True for every version apt.llvm.org publishes (up to 23) |

**What it still costs:**

* **Switch the primary compiler from clang to gcc.** `make build` runs
  `switch-to-latest-clang`; reflection is gcc-only, so gcc would have to become the default
  (`make build-gcc` already exists, so the path is there). Note the image's unversioned
  `g++` is **15**, not 16 — reflection needs `g++-16` named explicitly.
* **Move the project from C++23 to C++26**, which will surface its own warnings and
  incompatibilities. The V8 headers are no longer the open question — they compile. What is
  untested is the other ~200 translation units under gcc *and* a newer standard at once.
* **The dependencies do *not* need rebuilding for the compiler switch.** Verified
  empirically: a clean `g++-13` configure/build in a separate build dir links all twelve
  clang-built static libraries — `libv8_monolith.a` included — and the resulting binary
  runs and renders **pixel-identical** output to the clang build (RMSE 0). That works
  because both compilers follow the Itanium C++ ABI and the project already forces
  `-stdlib=libstdc++` for clang (`CMakeLists.txt:58,60`), so the two share one C++ runtime.
  The dep `.a` files also carry no LTO, so they are not compiler-locked.
* **The packaging problem is resolved.** `docs/install_deps.sh:13` points apt at
  `https://cppse.nl/repo/ $(lsb_release -cs) main`, which on 26.04 resolves to `resolute`;
  `build-config` now builds and publishes all 17 `cppse-*` packages for that codename.

**Recommendation: do not block this feature on it.** The field mapping here needs a
~9-entry whitelist; reflection would be a sledgehammer for that specific problem. The
genuine payoff is elsewhere — `shapes_to_json` hand-writes 52 fields, `@add_field@` spans
10 files, and `object_bridge` duplicates getters/setters per shape type. Now that the
toolchain supports it, prototype reflection against the serializer first, since that is
where it would actually earn its keep.

---

## 6. Effort and suggested first slice

Roughly **half a day** for a working version. Most of it is C++ plumbing that closely
mirrors `selected_ids`, so the risk is low.

Smallest end-to-end slice worth building first, to prove the chain:

1. one object, one field (`x`), absolute value in the UI
2. applied as a delta across all sub-frames
3. no descendant propagation yet
4. no reset button — closing the panel discards it

If that renders correctly, everything else is additive.
