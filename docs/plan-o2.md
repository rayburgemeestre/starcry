# Plan: build `integration_tests` with `-O2` (fallback option)

**Status:** not applied. This is the pragmatic escape hatch if the root-cause fix
(see "Why this is a fallback" below) turns out to be too invasive. Written 2026-09-15.

## Context

After rebasing the build image on Ubuntu 26.04, `make integration-test` aborts:

```
include/c++/16/bits/stl_vector.h:1253:
  std::vector<double>::operator[](size_type): Assertion '__n < this->size()' failed.
```

Backtrace: `scenes::get_time()` -> `scenes::prepare_scene()` -> `generator::_generate_frame()`.

The offending line is `src/interpreter/scenes.cpp`:

```cpp
const auto next_scene_duration = scenesettings.scene_durations[scenesettings.current_scene_next];
```

`scene_durations` is **empty** (`size=0`) on every call — measured 1,702,963 times in a
single test run, always `idx=0 size=0`. The read is out of bounds and always has been;
libstdc++ 16 simply started asserting on it.

It does not segfault because `clear()` retains capacity (`cap=1`), so `data()[0]` returns
the value written before the clear. On 24.04 that measured as `1.0`.

## Evidence

| Toolchain | Code | Opt | Result |
|---|---|---|---|
| 24.04 | original | `-O0` | 13/13 pass |
| 26.04 | original | `-O0` | abort |
| 26.04 | original | `-O2` | **13/13 pass, bit-exact against existing references** |
| 24.04 | guard returning 1.0 | `-O0` | 10/13 |
| 26.04 | guard returning 1.0 | `-O0` | 9/13 |

Two conclusions:

* The 24.04 -> 26.04 move produces **bit-identical rendering**. There is no
  floating-point drift and the reference images are still valid.
* Replacing the out-of-bounds read with a constant `1.0` **changes rendering**. The stale
  value is not always 1.0, so no single constant reproduces the legacy behaviour.

The assertion cannot be switched off: `-DNDEBUG`, `-D_GLIBCXX_ASSERTIONS=0` and
`-U_GLIBCXX_ASSERTIONS` all still abort. Only optimization level changes it.

## The change

`CMakeLists.txt` gives `integration_tests` only `${COMMON_FLAGS}`, unlike `starcry`
which also gets `-O2`. Add `-O2` to the `integration_tests` target.

Verify with:

```bash
make integration-test        # expect 13/13
```

A one-off check without editing anything:

```bash
cmake -DCMAKE_CXX_FLAGS=-O2 -B build-o2 && \
  cmake --build build-o2 --target integration_tests -j $(nproc) && \
  ./build-o2/integration_tests -s -d yes --rng-seed 0
```

## What it costs

`AGENTS.md` §3 documents the missing `-O2` as deliberate:

> `integration_tests` is built **without** `-O2` ... It therefore uses far more stack per
> frame than the shipped `starcry` binary, and will hit V8's stack limit on deeply
> recursive scripts long before production does. Treat it as the stricter environment,
> not a broken one.

That early-warning property is exactly what caught the `v8::ToLocalChecked` abort in the
emit/absorb work. Building with `-O2` gives that up: the suite would no longer be the
stricter environment, and stack regressions would surface in production first.

If this option is taken, update AGENTS.md §3 to say so, rather than leaving the note
describing behaviour that no longer holds.

## Why this is a fallback

`-O2` does not fix the bug, it only stops the assertion from being reachable. The
out-of-bounds read stays, and the engine keeps computing scene timing from a stale value
in freed-but-owned storage. Any allocator change, or a reallocation that reuses that
slot, would silently alter rendering. Prefer fixing why `scene_durations` is empty.

---

# Root cause (investigated 2026-09-15)

## What actually happens

`generator.cpp`:

```cpp
initializer_.initialize_all(...);   // parses the script's scenes, fills scene_durations
context->run_array(...);
scenes_.initialize();               // -> refresh_scenesettings(), which wipes them
```

`initialize_all()` computes the durations correctly — traced, 21 runs, always
`exit: size=1 cap=1 total_duration=6`, zero scenes skipped. Seven lines later
`scenes::initialize()` calls `refresh_scenesettings()`, whose job is "get rid of
left-over state", and which resets the whole `scene_settings` — throwing the
script-derived durations away.

`get_time()` then reads `scene_durations[current_scene_next]` on the emptied vector and
**gets the right answer anyway**:

* `scene_settings` declares `virtual ~scene_settings() = default`.
* A user-declared destructor suppresses the implicit move constructor and move
  assignment, so the class is copyable but **not movable**.
* `std::swap(scenesettings, tmp)` therefore falls back to copy-assignment.
* `std::vector::operator=(const vector&)` from an empty source sets `size = 0` but keeps
  the capacity and the bytes.

Verified by address probe: the initializer and `get_time` operate on the same object
(`vec` / `owner` / `global` all identical), size goes 1 -> 0 while `cap` stays 1.

So scene timing has been computed from freed-but-still-resident memory. It is not a
26.04 regression; libstdc++ 16 only started asserting on it. A move constructor on
`scene_settings`, or an allocator that reused the block, would have changed rendering
silently.

## Why the obvious fixes do not work

**Substituting a constant.** Measured `oob_value = 1` for a one-scene script
(`total_duration=6`, normalized `6/6`). But with two scenes (`total_duration=10`)
`durations[0]` is ~0.4. No constant reproduces it. Tried `1.0`: 24.04 went from 13/13 to
10/13.

**Preserving the durations across the refresh.** Carrying `scene_durations` and
`scenes_duration` through `refresh_scenesettings()` removes the out-of-bounds read and
gets 11/13 on 26.04 at `-O0`, but breaks the idempotence check at
`test_integration.cpp:23`:

```cpp
frame_of_interest = 10;  REQUIRE(... frame10 ...);   // line 18 - passes, RMSE exactly 0
frame_of_interest = 11;  REQUIRE(... frame11 ...);   // line 20
REQUIRE(sc.compare(frame10, frame11) > 0.05);        // line 22
frame_of_interest = 10;  REQUIRE(... frame10 ...);   // line 23 - FAILS, RMSE 0.0459
```

The same frame renders differently depending on what preceded it, so **no reference can
satisfy both line 18 and line 23**. Regenerating would defeat the check. Confirmed
deterministic: two unchanged runs gave byte-identical RMSEs (0.0459287, 0.00893947).

The reason is `scene_settings::update()`:

```cpp
if (scene_durations[current_scene_next] < (t - offset_next)) {
  if (scene_durations.size() - 1 > current_scene_next) {
    offset_next += scene_durations[current_scene_next];
```

With `size() == 0`, `size() - 1` underflows to `SIZE_MAX`, so the inner test is always
true and `offset_next` advances. Restoring the durations makes `size() == 1`, the test
becomes `0 > 0`, and it stops advancing. The old rendering depends on **both** bugs
together, so fixing only the first changes scene timing.

## What a real fix has to do

Any correct fix has to decide what `offset_next` advancement should be when a script has
a single scene, and then accept that the reference images change. That is a scene-timing
semantics decision, not a mechanical repair. It should also keep `refresh_scenesettings()`
honest about clearing transient state, so renders stay idempotent — which is what
`test_integration.cpp:23` exists to check.

Suggested order for a follow-up:

1. Decide the intended semantics of `scene_settings::update()` for one-scene scripts.
2. Fix the `size() - 1` underflow and the empty-vector read together.
3. Re-run and expect several references to change; inspect each diff in
   `test/report.html` before regenerating.
4. Keep line 23 passing without regenerating its reference — it is the idempotence
   guard.
