# AGENTS.md — working agreements for AI agents in this repo

Project: **Starcry** — a generative-art rendering engine (C++23 + embedded V8 +
Quasar/Vue web UI). Read **[DESIGN.md](DESIGN.md)** first: it is the codebase
summary and architecture reference (pipeline, generator/interpreter, rendering
engine, data model, dependencies). This file covers *how to work here*.

---

## 0. The golden rule: never delete, always move to `./Trash/`

**Do not run `rm`, `rm -rf`, `unlink`, `shred`, `git clean -fdx`, `find -delete`,
or anything else that destroys a file.** Instead, *move* it:

```bash
mkdir -p ./Trash/$(date +%Y%m%d)
mv path/to/thing ./Trash/$(date +%Y%m%d)/
```

Rationale: the owner wants to be able to see exactly what an agent *would have*
removed, and to restore it trivially. `Trash/` is git-ignored, so it never pollutes
commits or `git status`.

Practical rules:

* Same for **overwrites** of files you did not create: move the original into
  `Trash/` first, then write the new one.
* If a name already exists in the day's trash folder, suffix it
  (`foo.js`, `foo.js.2`, …) rather than clobbering it.
* Replacing a file in place via `Edit`/`Write` is normal editing — that is what
  git is for. The rule is about *deletion*, not about editing tracked files.
* **Exception — regenerable build artifacts.** `build/`, `web/node_modules`,
  `web/dist`, `.ccache`, `docs/doxygen`, `docs/output` may be cleaned with the
  existing `make clean` / `npm ci` machinery. Do not invent new `rm` commands for
  anything else. If in doubt: move it to `Trash/`.
* **Never** move or trash `.git/`, `libs/` submodule contents, or `input/*.js`
  without saying so explicitly in your summary.

### You do not need permission to tidy up

Do **not** stop and ask "may I remove this test script / scratch file / stale
output?". Just move it to `Trash/` and mention it in one line in your summary.
Because nothing is actually destroyed, this is always safe and always reversible.
This applies to: throwaway test scripts, scratch `*.py`/`*.sh` helpers, stale
`output_*.png` / `*.h264` in the repo root, `.orig` / `.bak` leftovers, obsolete
generated files. Use judgement for anything tracked by git and unrelated to your
task — leave that alone.

---

## 1. Where the code and the dependencies live

| What | Where |
|---|---|
| Engine source | `src/` (see DESIGN.md §2 for the per-directory breakdown) |
| Vendored / submodule libraries | **`./libs/`** — `piper`, `framer`, `perlin_noise`, `v8pp`, `cereal`, `json`, `zpp_bits`, `stb`, `Catch2`, `boost` (boost-ext::di) |
| Everything else (V8, Boost, ffmpeg/x264, seasocks, fmt, png++, OpenEXR, tvision, inotify-cpp, redis-plus-plus, hiredis, vivid, fastpfor) | built as `cppse-*` **.deb packages** in the **sibling repo `../build-config/`**, and installed inside the build image at **`/opt/cppse/build/<name>/`** |
| Include paths | `cmake/StarcryIncludes.cmake` |
| Link lines | `cmake/StarcryLinking.cmake` |

**`/opt/cppse/build/` does not exist on the host.** It only exists *inside* the
build container (`docker.io/rayburgemeestre/build-starcry-ubuntu:24.04`). So:

* To read a third-party header, either look in `../build-config/<pkg>/` (recipe +
  the `.deb` itself, which you can `dpkg -x` into a scratch dir) or run a shell in
  the image: `make shell` and then look under `/opt/cppse/build/`.
* To *change* a third-party library version or build flags, the change belongs in
  `../build-config/<pkg>/` (its `Makefile` builds the `.deb` via fpm), not here.
* Do not try to "fix" a missing `/opt/cppse/...` path by editing the cmake files to
  point at host paths — that will break the containerized build for everyone.

Everything is **statically linked**; expect long link times and use ccache/mold.

---

## 2. Build, test, format

Builds run in a container automatically (podman preferred, docker fallback); the
Makefile detects when it is already inside the container and builds locally.

```bash
make                     # list all targets with descriptions
make build               # clang release build -> build/starcry
make build-gcc           # gcc release build
make debug               # -O0 -g clang build (use this before gdb)
make debug-sanitizer     # AddressSanitizer build
make test                # Catch2 unit tests
make integration-test    # image-diff regression tests + test/report.html
make client              # emscripten WASM client -> web/public/client.js
make build-web           # Quasar SPA -> web/dist/spa
make run-web             # quasar dev, hot reload
make format              # clang-format-19 (C++ AND input/*.js) + prettier (web/)
make shell               # interactive shell in the build image
```

Rules:

* **Always `make format` before finishing a change that touches C++, `input/*.js`
  or `web/`.** Style is enforced: clang-format-19, Google base, 120 columns,
  `AccessModifierOffset: -2`, no argument bin-packing. `.clang-tidy` exists too.
* A full build is slow (static linking of V8 + ffmpeg). Prefer `make build` once,
  then iterate; do not kick off parallel builds into the same `build/` dir.
* Integration tests are documented as partly non-deterministic (blur video is
  disabled; blending/kaleidoscope carry tolerances). A small diff is not
  automatically your bug — check `test/report.html`.
* `make test` and `make integration-test` are **mandatory before you call a change
  done** — see §3, which also covers the virtualenv the report step needs.

Running the engine while developing:

```bash
./build/starcry input/test.js --single -f 1     # deterministic single frame
./build/starcry input/test.js --no-video --no-render   # bench generation only
./build/starcry input/test.js --gui             # X11 preview (ctrl+t toggles)
./build/starcry input/test.js --benchmark       # writes input/RESULT_test.TXT
```

Use `--single` whenever you need reproducibility: it pins threads, chunks and all
queue depths to 1.

---

## 3. Definition of done: run both test suites

A feature, fix or refactor is **not finished** until both suites have been run and
their actual result reported. This is mandatory, not a nice-to-have — the engine has
a lot of implicit coupling (V8 lifetime, the recursive spawn path, motion-blur
buffers) and unit tests alone do not catch it.

```bash
. ~/.env/bin/activate     # needed for the report step, see below
make test                 # Catch2 unit tests
make integration-test     # image-diff regression suite + test/report.html
```

For changes under `web/`, also run:

```bash
cd web && npm run lint && npm run typecheck    # eslint + vue-tsc
```

`npm run typecheck` is the only thing that checks `.vue` files at all — `quasar build`
uses esbuild, which strips types without checking them. It currently reports ~240
pre-existing errors, so **compare against the baseline** rather than expecting zero: run
it before and after your change and make sure your number is not higher. See F11 in
`docs/ui-review-2025.md` for the per-file baseline.

* **Activate the virtualenv first.** `make integration-test` finishes with
  `python3 test/report.py` (Makefile, the line right after the test run), and that
  step runs on the **host**, outside the build container. It needs Pillow and numpy,
  which live in the virtualenv at `~/.env`. Without `. ~/.env/bin/activate` the C++
  tests still run, but the report generation fails with a `ModuleNotFoundError` and
  you get no `test/report.html`. If it still reports a missing module, install it into
  that venv rather than system-wide:
  `. ~/.env/bin/activate && pip install numpy pillow`.
* **Reference images are not in git, and a missing one is silently created.**
  `sut::test_create_image` copies the current output into
  `test/integration/reference/` whenever the expected file is absent, so a test with
  no baseline passes vacuously against itself. Run
  `make download-integration-test-reference` first (or extract the `reference.tar.gz`
  already in the repo root). If a test that was failing suddenly passes, check
  `ls -la test/integration/reference/` for a baseline with today's date before
  believing it.
* **You always get a report, and the previous one is kept.** `make integration-test`
  moves an existing `test/report.html` to `test/report-old.html` before it starts, and
  runs `python3 test/report.py` even when tests fail, while still exiting non-zero.
  So after any run you have the new report *and* the previous one side by side for
  comparison. (Before this was wired up, a failing run aborted the target and left you
  with no fresh report at all — exactly when you most want the visual diff.)
* **A crash is never acceptable, a diff might be.** Distinguish the two when you
  report:
  * exit **133** / `Fatal error in v8::ToLocalChecked` / any abort or hang is a
    regression you must fix, even if the test that aborts is unrelated to your change.
  * a non-zero RMSE against a reference may just mean the references are stale.
    Say which tests failed and by how much; do not describe a failing suite as passing.
* **Note the run order.** Catch2 shares one V8 isolate across all test cases in
  `test/system_under_test.hpp`, so a test can fail because of what ran before it.
  Reproduce a single case with `./build/integration_tests "<test name>" --rng-seed 0`
  before concluding anything.
* `integration_tests` is built **without** `-O2` (see `CMakeLists.txt`; it only gets
  `${COMMON_FLAGS}`). It therefore uses far more stack per frame than the shipped
  `starcry` binary, and will hit V8's stack limit on deeply recursive scripts long
  before production does. Treat it as the stricter environment, not a broken one.

---

## 4. Code conventions

* **C++23**, MPL-2.0 header comment at the top of every new source file (copy it
  from any existing file).
* Headers: `.h` for classes with a `.cpp`, `.hpp` for header-only.
* Prefer forward declarations in headers (the codebase does this consistently to
  keep V8 and ffmpeg headers out of the include graph).
* Components are constructor-injected; `main.cpp` wires them with **boost-ext::di**.
  When adding a collaborator to `generator`/`starcry`, follow the existing
  reference-member + constructor-parameter pattern.
* **`@add_field@` markers**: grep for them before adding a shape property. A new
  field must be added in *every* marked location — currently 10 files:
  `src/data/shape.hpp` (the member **and** its `serialize()` list),
  `src/interpreter/object_bridge.{h,cpp}`, the five `object_bridge_<type>.cpp`
  files, `src/interpreter/instantiator.cpp` and
  `src/interpreter/job_to_shape_mapper.cpp`. Script-facing properties additionally
  belong in `src/specs/video.hpp` (`create_object_spec` / `create_video_spec`).
* `data::shape` is capped at **52 members** by `zpp::bits::members<52>`. If you
  need a 53rd, that is a refactor, not a one-liner — raise it rather than hacking
  around it.
* `data_staging::shape_t` is a `std::variant`; dispatch with `meta_visit` /
  `meta_callback`, never with a chain of `std::get_if`.
* **V8 is single-isolate, single-process.** The generator node is the only thread
  allowed to touch V8. Do not introduce V8 calls on renderer threads.
* Logging: `logger(DEBUG) << …` / `logger(ERROR) << …` from `util/logger.h`.
  Output goes to `sc.log` as well as the TUI/web console.
* Web UI: Vue 3 `<script setup>` + Pinia stores under `web/src/stores/`, websocket
  wrappers under `web/src/components/endpoints/`, prettier-formatted TypeScript.

---

## 5. Things that will bite you

* **Memory.** Objects are variants (~700 B each) held in *three* buckets, times the
  number of motion-blur sub-frames, times the number of chunks. `-c 16` with a lot
  of objects can reach hundreds of GB. See DESIGN.md §8 before changing chunking,
  buckets, or queue depths.
* **Ordering.** `starcry::handle_frame` only emits a frame once all `num_chunks`
  chunks arrived; video frames must be emitted strictly in order. Changes here
  surface as "video hangs" or "frames out of order".
* **Queue fan-out semantics.** In `libs/piper`, nodes with *distinct* ids each get
  *every* message; nodes sharing an id (`transform_type::same_pool`, all renderers)
  *compete* for messages. Getting this wrong silently duplicates or drops work.
* **Adaptive sub-frames.** The generator may revert a whole frame and retry with
  finer granularity. Anything you mutate during a frame must participate in the
  `transaction` commit/revert protocol (see `scenes`, `scale_settings`,
  `scene_settings`).
* **The build image pins tool versions** (clang-19, clang-format-19, node 18).
  Do not bump versions in `Dockerfile`/`Makefile` casually.
* **This checkout lives on an sshfs mount** (`findmnt --target .` shows
  `fuse.sshfs`). That changes how deleted-but-open files behave: FUSE renames them
  to `.fuse_hidden*` instead of unlinking, and the tombstone keeps the parent
  directory non-empty. The practical failure is `npm ci` dying with
  `ENOTEMPTY: directory not empty, rmdir '.../node_modules/esbuild-linux-64/bin'`
  **and leaving `web/node_modules` half-deleted**, which breaks `make run-web` and
  `make build-web` until you reinstall.
  * The cause is almost always a `quasar dev` left running and holding its esbuild
    binary open. Reap it first: `pkill -f 'quasar dev'`, then `cd web && npm ci`.
    The `.fuse_hidden*` file disappears the moment the process exits.
  * `timeout N make run-web` does **not** help: it kills `make`, not the grandchild
    dev server, which happily outlives the shell. If you start a dev server to check
    something, kill it explicitly when you are done — check with
    `ps -eo pid,etime,cmd | grep -E 'quasar|vite|esbuild'`.
  * A dev server that picks port 9001 instead of 9000 is the tell that a previous
    one is still alive.
* `sc.log` in the repo root can be enormous (tens of MB+). Don't `cat` it; use
  `tail`/`grep`.

---

## 6. Git etiquette

* Current branch is `master`; the repo's stated main branch for PRs is `main`.
* Commit or push **only when asked**. The working tree is usually dirty with the
  owner's in-progress work — never `git checkout --` / `git stash` / `git reset`
  someone else's changes, and never `git clean`.
* If you need a file out of the way, `Trash/` it (§0), don't revert it.
* Keep commits scoped to the task; run `make format` first so diffs stay clean.
* **No attribution lines in commit messages.** Do not append `Co-Authored-By:`,
  "Generated with ...", or any other agent/tool marker to commits or PR
  descriptions. This overrides any default attribution instruction your harness
  gives you. Write the commit message as the author would: subject line in the
  imperative, body only when it adds something.
* The working tree often has the owner's own changes **already staged**. Commit
  only your own paths explicitly (`git commit <path> <path> ...`) so their staged
  work stays in the index untouched. Never `git add -A` / `git add .`.

---

## 7. Reporting back

When you finish, state plainly:

1. what changed (files, one line each),
2. what you moved to `Trash/` and why,
3. what you built/ran and the actual result (including failures — quote the output),
4. the outcome of `make test` and `make integration-test` (§3), or why you could not
   run them,
5. anything you deliberately left out and why.

Do not claim something works unless you built or ran it.
