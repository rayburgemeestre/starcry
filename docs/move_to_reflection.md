# Moving starcry to C++26 reflection

Three phases, each independently shippable. Written 2026-09-15.

**Phase 3 is inferred** — you described phases 1 and 2. What is written below as phase 3
is my reading of where 2 leads; correct it if you meant something else.

---

## Why bother

Not for elegance. `data::shape` is **out of room**:

```cpp
struct shape {
  // TODO: afaik need C++26 to have more than 52
  using serialize2 = zpp::bits::members<52>;
```

`src/data/shape.hpp:51-54`. The struct has **52 members against a cap of 52**. Adding a
53rd field does not compile. zpp_bits reflects aggregates through structured bindings,
which have to be written out one per member, hence the hard ceiling. C++26 reflection
walks members without structured bindings and has no such limit — so this is the feature
that unblocks the data model, not a refactor for its own sake.

Everything else follows from the same mechanism:

| hand-maintained today | size |
|---|---|
| `data::shape_to_json` (`shape.hpp:186`) | 102 lines, almost all `j["name"] = input.name;` |
| `@add_field@` markers | 12 sites across 9 files |
| `object_bridge<T>` accessors | duplicated per shape type |

### Prerequisites, current status

| | |
|---|---|
| Toolchain implementing P2996 | done — image ships `g++-16`, `__cpp_impl_reflection = 202506` |
| V8 headers under `-std=c++26` | verified — `v8.h` + `libplatform.h` + `<meta>` + `template for` compile in one TU |
| Deps rebuilt for gcc | not needed — gcc links the clang-built `.a`s, renders RMSE 0 |
| Primary compiler is still clang-19 | **phase 1** |
| Targets are still `cxx_std_23` | **phase 1** |
| Rest of the codebase as gcc/C++26 | **untested — the real risk** |

---

## Phase 1 — make the toolchain switchable

Goal: `make build` and `make build-gcc` both work, repeatedly, without a clean checkout
in between. No reflection yet.

### 1.1 Separate build directories (the actual blocker)

Both targets currently configure into the same `build/`:

```make
build-impl:      CXX=$$(which c++) cmake ... -B build     # clang, via switch-to-latest-clang
build-gcc:       CXX=$$(which g++) cmake ... -B build     # gcc
```

CMake caches the compiler in `build/CMakeCache.txt`, so alternating between them either
errors or silently reuses the wrong configuration. This is the same failure that appeared
when the base image moved to 26.04 — the fix there was deleting the stale cache.

Introduce a per-compiler directory:

```make
BUILD_DIR ?= build
# build -> build-clang, build-gcc -> build-gcc
```

Then switching is just `make build` / `make build-gcc`, with no cache thrash and both
trees warm.

### 1.2 `build-gcc` must name g++-16 explicitly

`build-gcc` uses `$$(which g++)`, and the image's unversioned `g++` is **15**. GCC 15 does
not implement P2996. Use `g++-16` by name, or add a `switch-to-gcc-16` alongside the
existing `switch-to-latest-clang` (which is just `update-alternatives` on `c++`/`cc`/`ld`,
`/usr/local/bin/switch-to-latest-clang` in the image) so the mechanism matches.

### 1.3 Standard selection per target

`CMakeLists.txt` sets `cxx_std_23` on all four targets (lines 100, 135, 147, 163). Make it
a variable so the gcc path can opt into 26 while clang stays on 23:

```cmake
set(STARCRY_CXX_STD cxx_std_23 CACHE STRING "")
```

and add `-freflection` only for the gcc/C++26 configuration.

### 1.4 Exit criteria

* `make build` and `make build-gcc` both succeed, run back-to-back, twice each.
* `make test` and `make integration-test` pass under **both** compilers.
* Renders match between them — earlier measurement had gcc vs clang at RMSE 0, so any
  drift here is a regression introduced by the standard bump, not by the compiler.
* This is also where "the rest of the codebase under gcc/C++26" gets answered. Expect
  warnings-as-errors fallout; `cmake/warnings.cmake:99` already has a `GNU` branch.

---

## Phase 2 — reflection foundation, with tests as the demonstration

Goal: a working, tested reflection utility that nothing depends on yet.

### 2.1 The utility

Add `src/data/shape_reflect.hpp` exposing a compile-time member walk over `data::shape`:

* `field_count<T>()` — member count
* `field_names<T>()` — names as a static array
* `for_each_field(T&, F&&)` — invoke `f(name, ref)` per member

Built on what is already verified to work in the image:

```cpp
template for (constexpr auto m : std::define_static_array(
      std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current()))) {
  f(std::meta::identifier_of(m), obj.[:m:]);
}
```

### 2.2 Keep clang buildable during this phase

Guard with `#if defined(__cpp_impl_reflection) && __cpp_impl_reflection >= 202506`. In
phase 2 the reflection code is purely additive — nothing else calls it — so the `#else`
branch costs nothing and `make build` (clang) stays green throughout. That removes the
"we have to undo it if we switch back" risk **for this phase**.

That stops being free in phase 3, where the point is to delete the hand-written path. The
guard is a phase-2 convenience, not a permanent dual-implementation strategy.

### 2.3 Unit tests

`test/test_unit.cpp` is plain Catch2 (`add_executable(unit_tests test/test_unit.cpp)`,
CMakeLists:145) and currently covers colour blending and gradients. Add a
`test/reflection_test.cpp` in the same style:

* field count of a small local struct is what you wrote
* field names come back in declaration order
* `for_each_field` visits every member exactly once
* round-trip: populate a `data::shape`, walk it into a map, compare against a handful of
  fields read directly
* **the cap test** — a struct with 60 members walks fine, which is the thing zpp_bits
  cannot do today

The last one is the demonstration worth having: it proves the ceiling is gone.

### 2.4 Exit criteria

`make test` passes under gcc/C++26 with the new cases, and still passes under clang with
them compiled out.

---

## Phase 3 — migrate the consumers (inferred)

Only after 1 and 2 are green. Ordered by risk, lowest first.

1. **`shape_to_json`** — 102 lines of `j["name"] = input.name;` replaced by one walk. No
   UI risk if the emitted JSON is byte-compared against the current output first; that
   comparison is the acceptance test. Watch the special cases: `j["type"]` goes through
   `data::shape_type_to_string(input.type)`, so the walk needs a per-type hook rather
   than a blanket assignment.
2. **Lift the 52-member cap** — replace the `zpp::bits::members<52>` path so a 53rd field
   becomes possible. This is the payoff; do it once 1 proves the walk is sound.
3. **Retire `@add_field@`** — 12 sites across `shape.hpp`, `object_bridge*.cpp`,
   `instantiator.cpp`, `job_to_shape_mapper.cpp`. Each becomes a generated case.
4. **`object_bridge<T>` accessors** — the name -> getter/setter dispatch, which is also
   what `docs/object-field-preview.md` needs for its preview button and what
   `docs/objectbrowser-v2-migration.md` §5.1 needs to make `IsolatedObject` edits real.

Steps 1 and 2 are self-contained. Steps 3 and 4 touch the interpreter and are where the
integration suite earns its keep — it is now green at `-O0` on 26.04, so it will catch
behavioural drift rather than aborting first.

---

## Notes

* Do not couple this to the ObjectBrowser V1 retirement. That migration's blocker is a
  highlight button; see `docs/objectbrowser-v2-migration.md` §9.
* `integration_tests` is built without `-O2` on purpose (AGENTS.md §3). Keep it that way —
  libstdc++ 16's hardened assertions are only reachable there, and they have already
  caught one real bug.
