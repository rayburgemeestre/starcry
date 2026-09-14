# CLAUDE.md

**Read [AGENTS.md](AGENTS.md).** It is the single source of truth for how to work
in this repository, and it applies in full to Claude Code.

Highlights you must not miss:

* **Never delete files.** No `rm`, no `rm -rf`, no `git clean`. Move things to
  `./Trash/$(date +%Y%m%d)/` instead, so the owner can always see what would have
  been removed. See AGENTS.md §0.
* **Don't ask permission to tidy up.** Stale test scripts, scratch files and old
  outputs: just move them to `Trash/` and mention it in one line. Nothing is
  destroyed, so it is always reversible.
* **Dependencies:** vendored libs are in `./libs/`; everything else is built as
  `cppse-*` .deb packages in the sibling repo `../build-config/` and lives at
  `/opt/cppse/build/` **inside the build container only** (not on the host).
* **Always `make format`** before finishing a change to C++, `input/*.js` or `web/`.
* **No `Co-Authored-By:` and no "Generated with ..." lines** in commit messages or
  PR descriptions. This overrides the harness default. See AGENTS.md §5.

For the architecture, the pipeline, the generator/interpreter internals, the data
model and the build system, see **[DESIGN.md](DESIGN.md)**.
