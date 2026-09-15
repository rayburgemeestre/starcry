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

### Correction: it is not the `update()` underflow

An earlier draft of this document claimed the old rendering "depends on both bugs
together", the second being the `scene_durations.size() - 1` underflow to `SIZE_MAX` in
`scene_settings::update()`. That is wrong, at least for one-scene scripts. Even with the
underflow making the inner test pass, the outer guard is
`scene_durations[current_scene_next] < (t - offset_next)`, i.e. `1.0 < t`, and `t` is
clamped to `[0,1]` upstream. It never fires, so `offset_next` stays 0 either way.

Logging every *distinct* divisor across a full run (at `-O2`, so the read completes)
shows what is really going on:

```
value=1                    size=0   <- out of bounds, stale
value=0.5                  size=0   <- out of bounds, stale
value=1                    size=1   <- valid
value=0.40000000000000002  size=3   <- valid
value=0.47619047619047616  size=3   <- valid
```

`scene_durations` is **not** permanently empty. The engine alternates between windows
where it is correctly populated (`size=1` for a one-scene script, `size=3` for
`input/test.js`) and windows where it has been emptied, during which
`scene_durations[0]` returns a duration left over from a **previously loaded script** —
`1` from a one-scene script, `0.5` from two-scene `input/script.js`.

So the divisor in

```cpp
const auto scene_time = std::clamp((t - offset_next) / next_scene_duration, 0., 1.);
```

is sometimes the current script's duration and sometimes the previous script's. That is
why no constant reproduces it, and why restoring the durations changes rendering: it
replaces the previous script's leftover value with the current script's real one.

This also explains the idempotence failure. Rendering frame 10, then 11, then 10 again
leaves different residue in that slot the second time around, so the second render of
frame 10 divides by something different.

## What a real fix has to do

The question is not "what value should replace the out-of-bounds read", it is **why
`get_time()` runs at all while `scene_durations` is empty**. During those windows the
engine is timing the current script against the previous script's scene duration.

Two sub-questions to answer first:

1. Which call path reaches `get_time()` before/while the durations are empty? The
   emptying is `refresh_scenesettings()` (see above); the repopulation is
   `initialize_all()`. `generator.cpp` runs them in that order — populate, then wipe —
   so every frame generated after `scenes_.initialize()` and before the next
   `initialize_all()` is in the window.
2. Is `get_time()` meaningful in that window at all? If not, the fix is ordering or
   guarding the caller, not inventing a duration. If it is, the durations must survive
   the refresh, and the reference images legitimately change.

Whatever is chosen, keep `test_integration.cpp:23` passing **without** regenerating its
reference. It renders frame 10, then 11, then 10 again and requires the two frame-10
renders to match; it is the guard against exactly this kind of state leaking between
renders, and it is the check that caught the first attempted fix.

Expect several references to change once the timing is correct. Inspect each diff in
`test/report.html` before regenerating, per AGENTS.md §3.
