# The Qt ceiling: Qt 6.12

The project builds, tests and ships Qt 6.9.3; this plan moves it to Qt 6.12.0, the newest release and the long-term
support version after 6.8. What held it back was issue #367: from Qt 6.10, on GNOME's Wayland session with Mutter
before 50.2, no picture follows the cursor while an element is dragged. Ubuntu 26.04 LTS ships Mutter 50.1 and has no
fix scheduled, so the plan first takes the drag's picture out of the compositor's hands: the scene draws it. Then Qt
6.12.0 everywhere the project names a Qt it uses: CI, the releases, the documents' recipes, the dev container.

- **Depends on:** the Qt 6.8 floor's plan, landed, since the plans run one at a time, in their order (the plans'
  rules, §1). The floor sets the oldest Qt the project accepts; this plan the Qt it uses (QC.D1).
- **Lands before:** the exception boundaries' plan, then plan 3. The order is 1, 2, the floor, this ceiling, the
  exception boundaries, 3 to 9, the model-cost round, then 10.
- **Branch and ledger:** `02b-qt-ceiling`, from `02a-qt-floor` once the floor has landed. `LEDGER_qt_ceiling.md` has a
  row for every step, check and open question below.
- **Rules every plan shares** are the plans' rules (`PLAN_rules.md`) and `CLAUDE.md`'s "Building and testing". This
  file states the ceiling's own: the Qt the project uses is one number, so every place that names it changes in one
  commit; and the drag's picture is the scene's, on every platform, so no setup depends on a compositor for it. Its
  decisions are numbered QC.D1 on.

## 1. Where the floor leaves it

### 1.1 Issue #367

Qt 6.10 made `QWaylandWindow` deliver its expose synchronously, so the drag icon's surface is committed with its
buffer before `wl_data_device.start_drag`. The Wayland protocol allows either order; Mutter before 50.2 and Weston
handled only the other, and draw no icon. The drop still works. QTBUG-142809 (2025-12-18, Qt Designer's drag image)
found it and David Edmundson reported it to GNOME as mutter#4564; QTBUG-145754 (2026-04-12, ours) was closed as a
compositor bug. Two Qt workarounds were abandoned on 2026-05-04: 728105, showing the drag pixmap window before setting
its geometry, and 729290, reordering the requests, which Edmundson rejected on the ground that GNOME would backport its
fix "before the next Qt6"; his own, never written, would defer the icon window's first expose until the drag starts.
Mutter's fix (!5035, `a4ad34d5`, merged 2026-04-22) is in 50.2 and later and was backported to 49 (!5087, 49.6 on),
where its own test is marked to fail; GNOME 48 and older got none.

As read on 2026-10-28: Fedora ships Mutter 50.6 and Arch 51.1, fixed; Ubuntu 26.04 ships 50.1-0ubuntu2.5, its patches
holding none that touches the drag icon, its packaging repository nothing pending and Launchpad no request, Ubuntu
taking 50.x fixes into 26.04 one bug at a time; Debian 13 ships 48.7 and Ubuntu 24.04 46.2. On this machine (Ubuntu
26.04.1, Mutter 50.1-0ubuntu2.4), Qt 6.10.2's drag icon does not show.

### 1.2 The drags

Two drags hand a picture to `QDrag`: the palette's (`ElementLabel::startDrag()`, the element's icon, its hot spot the
icon's centre) and the selection's copy-drag (`ClipboardManager`, an image of the selection rendered at the view's
transform, its hot spot the press point). Both drop into the scene, which accepts their formats in `dragEnterEvent` and
`dragMoveEvent` and places the elements in `dropEvent` through `SceneDropHandler`.

### 1.3 Where 6.9.3 is named

- CI: `build.yml`'s matrix (the 6.9.3 entries on Linux, Windows, macOS 26 and the two ARM runners, and the condition
  of its translatable-strings step), `codeql.yml` (twice), `coverage.yml`, `sanitizers.yml` (version, job name, cache
  key), `translate.yml` (three jobs), `wasm.yml` (twice), `deploy.yml` (every release platform, and its comment on the
  architectures it read with `aqt list-qt`), and `fresh-deps.yml`'s `ldd` paths.
- The documents: `BUILD.md`'s recipes and its two translations, `CONTRIBUTING.md`, the developer guide (setup, Windows,
  the container, the CI table), the dev container (`Dockerfile`, `devcontainer.json`, `README.md`) and `CLAUDE.md`.
- Code comments about Qt 6.9's behaviour: the theme's colour-scheme flush (`ThemeManager.cpp`), the bounded variant
  reader's claim that Qt 6.9 reads a null variant's data (`Serialization.cpp`), the deprecated drag-event constructor
  (`TestUtils.h`, whose 6.12 branch is untested until CI builds 6.12), and the chrono defaults (`TestUtils.h`).
- The 6.8.3 entries and the i386 job's 6.8.2 stay: they test the floor.

## 2. What the plan changes

### 2.1 The scene draws the drag's picture

The two drags stop handing their picture to `QDrag`; each leaves it, with its hot spot, in `DragPreview` for the
length of the drag, and the scene draws it while the drag is over it, removing it when the drag leaves or drops. It is
the same on every platform and every Qt: no setup depends on its compositor for it, and no detection is needed. The
picture shows what the drop will make: the element at its size in the scene and where the drop will put it, snapped
to the grid as the drop is. Outside the scene there is no picture (the palette, briefly, and outside the window).

A prototype (`probe/drag-ghost`, `b459df797`) drew the picture at screen size under the cursor; on this machine with Qt
6.10.2, `QDrag`'s picture showed nothing and the scene's followed the cursor.

### 2.2 Qt 6.12.0

Every place §1.3 names moves to 6.12.0 in one commit, with what 6.12 needs: its deprecations under CI's `-Werror`, any
golden it changes, the claims of §1.3's comments read again on 6.12, and the architectures `aqt list-qt` gives for
6.12.0 on each release platform.

## 3. Pitfalls

- **A prototype's numbers are not the plan's.** The prototype drew at screen size and followed the cursor; the drop
  snaps. The ghost and the drop must agree, so the ghost's position comes from the code the drop uses.
- **The ghost is an item in the scene.** Anything that walks the scene's items (bounds, the minimap, saving, selection,
  hit tests) must not see it, or must be shown not to matter for the length of a drag.
- **A static `QPixmap` aborts before the application exists.** The prototype's first build did; the picture's storage
  is created on first use.
- **CI's Qt is not the local Qt.** The suite runs locally on 6.12.0 before the push, and each new check is broken once.

## 4. Steps

| Row | Step | Slice |
|---|---|---|
| QC.1 | The scene draws the drag's picture: `DragPreview`, the ghost at the element's size and drop position, both drags, its tests (§2.1) | 1 |
| QC.2 | Qt 6.12.0: CI, the releases, the documents' recipes, the dev container, `CLAUDE.md`, and what 6.12 needs in the code (§2.2) | 2 |
| QC.3 | Issue #367's resolution: what the issue is told, the user's to post (§5) | after 2 |

## 5. Decisions

- **QC.D1. The floor and the ceiling are separate plans.** The floor sets the oldest Qt the project accepts; this plan
  which Qt it uses, and what a newer Qt breaks. Basis: the user's ruling of 2026-10-28.
- **QC.D2. The scene draws the drag's picture, on every platform.** Not a warning for the affected setups: detecting
  them needs Mutter's version from GNOME Shell over D-Bus, which a distribution's patch makes wrong, and Weston cannot
  be detected. Basis: the user's rulings of 2026-10-28 ("prototype that fix now", then "we can land it now"); tested
  in the prototype.
- **QC.D3. Qt 6.12.0, the newest release.** It is the long-term support version after 6.8 (since 6.8, every fourth
  minor version), released 2026-09-30. Basis: the user's ruling of 2026-10-28 ("bump the Qt version we are using to
  the latest"); read (Qt's "Qt Releases", qtbase's `v6.12.0` tag).
- **QC.D4. CI installs Qt with a pinned commit of aqtinstall's master.** Its latest release, 3.3.0 (2025-06-02),
  fails to read Qt 6.11 and later's Windows x64 repository, which Qt split by compiler; master's "Support Qt 6.11+ for
  Windows x64" (2026-03-01) reads it. Every `install-qt-action` step takes commit `076e1659807d` in place of 3.3.0, one
  aqtinstall for every job; the documents' Linux and macOS recipes keep the release, which installs 6.12.0 there. When
  a release carries the fix, CI goes back to a release. Basis: measured (3.3.0 lists no Windows x64 architecture for
  6.12.0; the commit lists all four and installs `win64_msvc2022_64` with Qt Multimedia).

## 6. Claims to check and open questions

- **Qt 6.12's deprecations and warnings** under the project's GCC warning set at `-O2` and `-O3`, and under CI's
  clang on macOS: read in the first build. Checked at `-O2` with `-Werror`: none; `-O3` and clang are CI's.
- **The release platforms' architectures for 6.12.0** (`aqt list-qt <host> desktop --arch 6.12.0`), and the
  WebAssembly build's Emscripten version for 6.12. Checked: the same architectures as 6.9.3 on every host, Windows x64
  only through aqtinstall's master (QC.D4); Emscripten 5.0.5 (Qt 6.12's "Qt for WebAssembly" page).
- **The bounded variant reader's claim** that Qt reads a null variant's data, on 6.12. Checked: it does, on 6.9.3,
  6.10.2 and 6.12.0.
- **Issue #367:** whether to close it once the ghost has landed, and what to post on it and on QTBUG-142809 and
  Launchpad; each is public, so the user's.

## 7. Risks

| Risk | Mitigation |
|---|---|
| Users lose the picture over the palette and outside the window, where `QDrag` showed it | Both drags end in the scene; the scene's picture shows the drop's result, which `QDrag`'s did not |
| Qt 6.12.0, a `.0` release, carries regressions | Its suite runs on every CI platform before the plan lands; 6.12.1 is taken when it ships |
| A golden changes with Qt's rendering | Each changed golden is read before it is regenerated |

## 8. What other plans take from it

The exception boundaries' plan and plans 3 to 10 build on Qt 6.12; plan 10's `QRhiWidget` gets the LTS release.

## 9. Evidence

| What | Where |
|---|---|
| The prototype, checked with Qt 6.10.2 on Mutter 50.1 | `probe/drag-ghost`: `b459df797` |
| The compositor bug and its fix | mutter#4564; GNOME/mutter!5035 (`a4ad34d5`), !5087 (gnome-49) |
| Qt's bugs and its abandoned workarounds | QTBUG-142809, QTBUG-145754; codereview.qt-project.org 728105, 729290 |
| Each distribution's Mutter, read from its image on 2026-10-28 | `apt-cache policy mutter`, `dnf info mutter`, `pacman -Si mutter` |
| Ubuntu 26.04's mutter patches | salsa.debian.org/gnome-team/mutter, branch `ubuntu/resolute`, tag `ubuntu/50.1-0ubuntu2.4` |
| Qt 6.12's release and LTS | Qt's documentation, "Qt Releases"; qtbase's `v6.12.0` tag (2026-09-30) |
